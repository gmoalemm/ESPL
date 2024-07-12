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
            puts("Invalid choice!");
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
        puts("Debug flag now on");
    }
    else
    {
        puts("Debug flag now off");
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
    Elf32_Shdr *sectionHeaderTable = NULL, *stringTableSection = NULL, *section = NULL;
    char *stringTable = NULL;
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

        sectionHeaderTable = (Elf32_Shdr *)(current->mapStart + header->e_shoff);

        // Select the string table section
        stringTableSection = &sectionHeaderTable[header->e_shstrndx];   

        // This is the actual string table
        stringTable = (char *)(current->mapStart + stringTableSection->sh_offset);

        if (debug)
        {
            printf("  (shstrndx = %d)\n", header->e_shstrndx);
        }

        for (i = 0; i < header->e_shnum; i++)
        {
            section = &sectionHeaderTable[i];

            if (debug)
            {
                printf("  (name offset = %d)\t", section->sh_name);
            }

            // %-18s is used to left-align the string with a width of 18 (same as readelf -S does on my computer)
            printf("  [%2d]\t%-18s\t%08x\t%06x\t%06x\t", i, 
                stringTable + section->sh_name, 
                section->sh_addr, 
                section->sh_offset, 
                section->sh_size);

            printSectionTypeName(section->sh_type);

            printf("\n");
        }

        printf("\n");

        current = current->next;
    }
}

void printSymbols()
{
    puts("not implemented yet"); 
}

void checkForMerge()
{
    puts("not implemented yet"); 
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
