#include <elf.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#define INPUT_MAX 256

typedef struct menuItem 
{
    char *description;
    void (*function) (void);
} menuItem;

typedef struct elfFile
{
    char *fileName;
    int fd;
    off_t fileSize;
    void *mapStart;
    struct elfFile *next;
} elfFile;

typedef struct sectionType
{
    char *name;
    int type;
} sectionType;

bool debug = false;

elfFile *openFiles = NULL;
int openFilesCount = 0;

// Section types, more may be added later
sectionType sectionTypes[] = {
    {"NULL", SHT_NULL},
    {"PROGBITS", SHT_PROGBITS},
    {"SYMTAB", SHT_SYMTAB},
    {"STRTAB", SHT_STRTAB},
    {"RELA", SHT_RELA},
    {"HASH", SHT_HASH},
    {"DYNAMIC", SHT_DYNAMIC},
    {"NOTE", SHT_NOTE},
    {"NOBITS", SHT_NOBITS},
    {"REL", SHT_REL},
    {"SHLIB", SHT_SHLIB},
    {"DYNSYM", SHT_DYNSYM},
    {"INIT_ARRAY", SHT_INIT_ARRAY},
    {"FINI_ARRAY", SHT_FINI_ARRAY},
    {"PREINIT_ARRAY", SHT_PREINIT_ARRAY},
    {"GROUP", SHT_GROUP},
    {"SYMTAB_SHNDX", SHT_SYMTAB_SHNDX},
    {"NUM", SHT_NUM}
};

// Section indices, more may be added later
sectionType sectionIndices[] = {
    {"UND", SHN_UNDEF},
    {"ABS", SHN_ABS}
};

void toggleDebug();
void examineELF();
void printSections();
void printSymbols();
void checkForMerge();
void merge();
void quit();

void printMenu(const menuItem*, const int);
void printELFHeader(Elf32_Ehdr*);
void addFile(char*, int, off_t, void*);
void freeFile(elfFile*);
void printSectionTypeName(int);
void printSectionIndexName(int);
bool isSymbolDefined(elfFile*, char*);
int symbolDefinitionCount(char*);

int main(int argc, char *argv[])
{
    menuItem menu[] = {
        {"Toggle Debug Mode", toggleDebug},
        {"Examine ELF File", examineELF},
        {"Print Section Names", printSections},
        {"Print Symbols", printSymbols},
        {"Check Files for Merge", checkForMerge},
        {"Merge ELF Files", merge},
        {"Quit", quit},
        {NULL, NULL}
    };

    const int menuSize = sizeof(menu) / sizeof(menuItem) - 1;
    int menuChoice = -1;
    char userInput[INPUT_MAX] = { 0 };

    while (true)
    {
        printMenu(menu, menuSize);

        printf("Choice: ");
        fgets(userInput, INPUT_MAX, stdin);
        sscanf(userInput, "%d", &menuChoice);

        if (menuChoice < 0 || menuChoice >= menuSize)
        {
            puts("Invalid choice!\n");
            continue;
        }

        menu[menuChoice].function();
    }
    
    return 0;
}

void toggleDebug()
{
    debug = !debug;

    if (debug)
    {
        puts("Debug flag now on\n");
    }
    else
    {
        puts("Debug flag now off\n");
    }
}

void examineELF()
{
    char fileNames[INPUT_MAX] = { 0 }, *currentFileName = NULL;
    int fd = -1;
    off_t fileSize = 0;
    void *mapStart = NULL;
    Elf32_Ehdr *header = NULL;
    elfFile *file = NULL;
    
    printf("Enter file names to examine: ");

    fgets(fileNames, INPUT_MAX, stdin);

    // Remove newline character

    fileNames[strlen(fileNames) - 1] = 0;
    
    // Iterate the input

    currentFileName = strtok(fileNames, " ");

    while (currentFileName != NULL)
    {
        printf("\nFile %s\n", currentFileName);

        fd = open(currentFileName, O_RDONLY);

        if (fd < 0)
        {
            perror("open");
            printf("\n");
            currentFileName = strtok(NULL, " ");
            continue;
        }

        // Get file size
        fileSize = lseek(fd, 0, SEEK_END);
        lseek(fd, 0, SEEK_SET);

        if (fileSize < 0)
        {
            perror("lseek");
            printf("\n");
            close(fd);
            currentFileName = strtok(NULL, " ");
            continue;
        }

        mapStart = mmap(NULL, fileSize, PROT_READ, MAP_PRIVATE, fd, 0);

        if (mapStart == MAP_FAILED)
        {
            perror("mmap");
            printf("\n");
            close(fd);
            currentFileName = strtok(NULL, " ");
            continue;
        }

        header = (Elf32_Ehdr *)mapStart;

        // Check if the file is an ELF file
        if (header->e_ident[EI_MAG0] != ELFMAG0 ||
            header->e_ident[EI_MAG1] != ELFMAG1 ||
            header->e_ident[EI_MAG2] != ELFMAG2 ||
            header->e_ident[EI_MAG3] != ELFMAG3)
        {
            puts("Not an ELF file");
            printf("\n");
            munmap(mapStart, fileSize);
            close(fd);
            continue;
        }

        printELFHeader(header);

        printf("\n");

        addFile(currentFileName, fd, fileSize, mapStart);
        
        currentFileName = strtok(NULL, " ");
    }
}

void printSections()
{
    elfFile *current = openFiles;
    Elf32_Ehdr *header = NULL;
    Elf32_Shdr *sectionHeaders = NULL, *sectionHeaderStrTable = NULL;
    char *sectionsNames = NULL;
    int i = 0;

    if (current == NULL)
    {
        puts("No files are open");
        return;
    }

    while (current != NULL)
    {
        printf("\nFile %s\n", current->fileName);

        header = (Elf32_Ehdr *)current->mapStart;

        sectionHeaders = (Elf32_Shdr *)(current->mapStart + header->e_shoff);

        // Select the string table of the section headers
        sectionHeaderStrTable = &sectionHeaders[header->e_shstrndx];   

        // This is the actual string table
        sectionsNames = (char *)(current->mapStart + sectionHeaderStrTable->sh_offset);

        if (debug)
        {
            printf("  (shstrndx = %d)\n", header->e_shstrndx);
        }

        for (i = 0; i < header->e_shnum; i++)
        {
            if (debug)
            {
                printf("  (name offset = %d)\t", sectionHeaders[i].sh_name);
            }

            // %-18s is used to left-align the string with a width of 18 (same as readelf -S does on my computer)
            printf("  [%2d]\t%-18s\t%08x\t%06x\t%06x\t", i, 
                sectionsNames + sectionHeaders[i].sh_name, 
                sectionHeaders[i].sh_addr, 
                sectionHeaders[i].sh_offset, 
                sectionHeaders[i].sh_size);

            printSectionTypeName(sectionHeaders[i].sh_type);

            printf("\n");
        }

        printf("\n");

        current = current->next;
    }
}

void printSymbols()
{
    elfFile *current = openFiles;

    Elf32_Ehdr *header = NULL;
    Elf32_Shdr *sectionHeaders = NULL, *sectionHeaderStrTable = NULL;
    Elf32_Shdr *symbolTable = NULL, *stringTable = NULL;   
    Elf32_Sym *symbols = NULL;  // actual symbols entries
    char *sectionsNames = NULL, *stringTableData = NULL;
    int i = 0, j = 0, entries = -1;

    while (current != NULL)
    {
        header = (Elf32_Ehdr*) current->mapStart;

        sectionHeaders = (Elf32_Shdr*) (current->mapStart + header->e_shoff);

        sectionHeaderStrTable = &sectionHeaders[header->e_shstrndx];
        sectionsNames = (char*) (current->mapStart + sectionHeaderStrTable->sh_offset);

        for (i = 0; i < header->e_shnum; i++)
        {
            // ? should we check for SHT_DYNSYM as well?
            if (sectionHeaders[i].sh_type == SHT_SYMTAB)
            {
                symbolTable = &sectionHeaders[i];
            }
            // We do not want the section header string table, we want the string table for the symbols!
            else if (sectionHeaders[i].sh_type == SHT_STRTAB && i != header->e_shstrndx)
            {
                stringTable = &sectionHeaders[i];
            }
        }

        if (symbolTable == NULL || stringTable == NULL)
        {
            puts("   No symbol table or string table found!");
            current = current->next;
            continue;
        }

        symbols = (Elf32_Sym*) (current->mapStart + symbolTable->sh_offset);

        stringTableData = (char*) (current->mapStart + stringTable->sh_offset);

        printf("\nFile %s\n", current->fileName);

        entries = symbolTable->sh_size / sizeof(Elf32_Sym);

        if (debug)
        {
            printf("   (Symbol table '%s' contains %d entries and its size is %d bytes)\n", 
                sectionsNames + symbolTable->sh_name, entries, symbolTable->sh_size);
        }

        // Iterate the symbols

        for (j = 0; j < entries; j++)
        {
            printf("   %3d %08x  ", j, symbols[j].st_value);  // index and value

            printSectionIndexName(symbols[j].st_shndx);

            // If the section index is less than the number of section headers, 
            // we can print the section name
            if (symbols[j].st_shndx < header->e_shnum)
            {
                printf(" %-18s ", sectionsNames + sectionHeaders[symbols[j].st_shndx].sh_name);
            }
            else
            {
                printf(" %-18s ", "");
            }

            // Print the symbol name
            printf("%s\n", &stringTableData[symbols[j].st_name]);
        }

        printf("\n");

        current = current->next;
    }
}

void checkForMerge()
{
    elfFile *current = openFiles;
    Elf32_Ehdr *header = NULL;
    Elf32_Shdr *sectionHeaders = NULL, *symbolTable = NULL, *stringTable = NULL;
    Elf32_Sym *symbols = NULL;
    int i = 0, cnt = 0, entries = 0;
    char *stringTableData = NULL, *sym = NULL;

    if (openFilesCount < 2)
    {
        puts("Not enough files are open\n");
        return;
    }

    // Check if each file has exactly one symbol table

    while (current != NULL)
    {
        cnt = 0;
        header = (Elf32_Ehdr *)current->mapStart;
        sectionHeaders = (Elf32_Shdr *)(current->mapStart + header->e_shoff);

        for (i = 0; i < header->e_shnum; i++)
        {
            if (sectionHeaders[i].sh_type == SHT_SYMTAB)
            {
                cnt++;
            }
        }

        if (cnt != 1)
        {
            puts("Feature not supported\n");
            return;
        }

        current = current->next;
    }

    current = openFiles;

    // Check if each symbol is defined in all files exactly once

    while (current != NULL)
    {
        header = (Elf32_Ehdr *)current->mapStart;
        sectionHeaders = (Elf32_Shdr *)(current->mapStart + header->e_shoff);

        for (i = 0; i < header->e_shnum; i++)
        {
            if (sectionHeaders[i].sh_type == SHT_SYMTAB)
            {
                symbolTable = &sectionHeaders[i];
            }
            else if (sectionHeaders[i].sh_type == SHT_STRTAB && i != header->e_shstrndx)
            {
                stringTable = &sectionHeaders[i];
            }
        }

        symbols = (Elf32_Sym*) (current->mapStart + symbolTable->sh_offset);

        stringTableData = (char*) (current->mapStart + stringTable->sh_offset);

        entries = symbolTable->sh_size / sizeof(Elf32_Sym);

        for (i = 1; i < entries; i++)
        {
            sym = &stringTableData[symbols[i].st_name];

            if (strcmp(sym, "") == 0)
            {
                continue;
            }

            if (symbolDefinitionCount(sym) == 0)
            {
                printf("Symbol '%s' is undefined\n", sym);
            }
            else if (symbolDefinitionCount(sym) > 1)
            {
                printf("Symbol '%s' is defined multiple times\n", sym);
            }
        }

        current = current->next;
    }

    printf("\n");
}

void merge()
{
    puts("not implemented yet"); 
}

void quit()
{
    elfFile *current = openFiles, *next = NULL;

    puts("Goodbye!");

    while (current != NULL)
    {
        next = current->next;

        freeFile(current);

        current = next;
    }

    exit(0);
}

void printMenu(const menuItem *menu, const int size)
{
    int i = 0;

    for (i = 0; i < size; i++)
    {
        printf("%d-%s\n", i, menu[i].description);
    }
}

void printELFHeader(Elf32_Ehdr *header)
{
    puts("ELF Header:");

    printf("  Magic:\t\t\t%c%c%c\n", header->e_ident[EI_MAG1],
           header->e_ident[EI_MAG2],
           header->e_ident[EI_MAG3]);

    printf("  Data:\t\t\t\t%s\n", header->e_ident[EI_DATA] == ELFDATANONE ? 
        "Invalid data encoding" : header->e_ident[EI_DATA] == ELFDATA2LSB ? 
        "2's complement, little endian" : "2's complement, big endian");

    printf("  Entry point address:\t\t0x%x\n", header->e_entry);

    printf("  Start of section headers:\t%d (bytes into file)\n", header->e_shoff);
    printf("  Number of section headers:\t%d\n", header->e_shnum);
    printf("  Size of section headers:\t%d (bytes)\n", header->e_shentsize);

    printf("  Start of program headers:\t%d (bytes into file)\n", header->e_phoff);
    printf("  Number of program headers:\t%d\n", header->e_phnum);
    printf("  Size of program headers:\t%d (bytes)\n", header->e_phentsize);
}

void addFile(char *fileName, int fd, off_t fileSize, void *mapStart)
{
    elfFile *file = NULL;

    file = (elfFile *)malloc(sizeof(elfFile));

    file->fd = fd;
    file->fileSize = fileSize;
    file->mapStart = mapStart;
    file->next = openFiles;
    file->fileName = (char *)malloc(strlen(fileName) + 1);
    strcpy(file->fileName, fileName);

    openFiles = file;
    openFilesCount++;
}

void freeFile(elfFile* file)
{
    munmap(file->mapStart, file->fileSize);
    close(file->fd);
    free(file->fileName);
    free(file);
}

void printSectionTypeName(int type)
{
    int i = 0, arraySize = sizeof(sectionTypes) / sizeof(sectionType);

    for (i = 0; i < arraySize; i++)
    {
        if (sectionTypes[i].type == type)
        {
            printf("%s", sectionTypes[i].name);
            return;
        }
    }

    printf("%08x", type);
}

void printSectionIndexName(int index)
{
    int i = 0, arraySize = sizeof(sectionIndices) / sizeof(sectionType);

    for (i = 0; i < arraySize; i++)
    {
        if (sectionIndices[i].type == index)
        {
            printf("%-3s", sectionIndices[i].name);
            return;
        }
    }

    printf("%3d", index);
}

bool isSymbolDefined(elfFile *file, char *sym)
{
    Elf32_Ehdr *header = (Elf32_Ehdr *)file->mapStart;
    Elf32_Shdr *sectionHeaders = (Elf32_Shdr *)(file->mapStart + header->e_shoff);
    Elf32_Shdr *symbolTable = NULL, *stringTable = NULL;
    Elf32_Sym *symbols = NULL;
    char *stringTableData = NULL;
    int i = 0, entries = -1;

    for (i = 0; i < header->e_shnum; i++)
    {
        if (sectionHeaders[i].sh_type == SHT_SYMTAB)
        {
            symbolTable = &sectionHeaders[i];
        }
        else if (sectionHeaders[i].sh_type == SHT_STRTAB && i != header->e_shstrndx)
        {
            stringTable = &sectionHeaders[i];
        }
    }

    symbols = (Elf32_Sym *)(file->mapStart + symbolTable->sh_offset);
    stringTableData = (char *)(file->mapStart + stringTable->sh_offset);

    entries = symbolTable->sh_size / sizeof(Elf32_Sym);

    for (i = 1; i < entries; i++)
    {
        if (strcmp(&stringTableData[symbols[i].st_name], sym) == 0 && symbols[i].st_shndx != SHN_UNDEF)
        {
            return true;
        }
    }

    return false;
}

int symbolDefinitionCount(char *sym)
{
    int count = 0;

    elfFile *current = openFiles;

    while (current != NULL)
    {
        if (isSymbolDefined(current, sym))
        {
            count++;
        }

        current = current->next;
    }

    return count;
}
