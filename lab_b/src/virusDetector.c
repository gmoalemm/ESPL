/**
NAME
    virusDetector - detects a virus in a file from a given set of viruses.
SYNOPSIS
    virusDetector FILE.
DESCRIPTION
    virusDetector compares the content of the given FILE byte-by-byte with a
    pre-defined set of viruses described in the file. The comparison is done
    according to a naive algorithm described in task 2.
    FILE - the suspected file.
*/

#include <stdio.h>
#include <linux/limits.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct virus
{
    unsigned short SigSize;
    char virusName[16];
    unsigned char *sig;
} virus;

typedef struct link
{
    struct link *nextVirus;
    virus *vir;
} link;

typedef struct fun_desc
{
    char *name;
    void (*fun)(void);
} fun_desc;

// auxiliary functions (required)

void SetSigFileName();
virus *readVirus(FILE *);
void printVirus(virus *);
void list_print(link *, FILE *);
link *list_append(link *, virus *);
void list_free(link *);
void detectViruses();
void fixFile();

// auxiliary functions (not required)

void printHexToFile(FILE *, unsigned char *, size_t);
void openSigFile();
bool reachedEnd(FILE *);
void printVirusToFile(FILE *, virus *);
void loadViruses();
void printViruses();
void reset();
void quit();

// globals
// ? should they be globals?

char sigFileName[PATH_MAX] = {0};
bool usingBigEndian = false;
FILE *sigFile = NULL;
link *viruses = NULL;

int main(int argc, char **argv)
{
    fun_desc functions[] = {
        {"Set signatures file name", SetSigFileName},
        {"Load signatures", loadViruses},
        {"Print signatures", printViruses},
        {"Detect viruses", detectViruses},
        {"Fix file", fixFile},
        {"Quit", quit}};
    int functionsNo = sizeof(functions) / sizeof(functions[0]);
    int op = -1, i = 0;
    char line[8] = {0};
    bool error = false;

    // default signatures file
    // TODO: remove "files/" when submitting
    strcpy(sigFileName, "files/signatures-L");

    do
    {
        for (i = 0; i < functionsNo; i++)
        {
            printf("%d) %s\n", i, functions[i].name);
        }

        printf(">> choose an option: ");

        if (fgets(line, 8, stdin))
        {
            op = atoi(line);

            if (op < 0 || op >= functionsNo)
            {
                fprintf(stderr, "!> not an option.\n");
                error = true;
            }
            else
            {
                functions[op].fun();
            }
        }
        else
        {
            fprintf(stderr, "!> error reading option.\n");
            error = true;
        }
    } while (!error && strcmp(sigFileName, ""));

    if (error)
    {
        quit();
    }

    return error;
}

/**
 * @brief this function queries the user for a new signature file name, and
sets the signature file name accordingly.
 *
 * @note the default file name (before this function is
called) should be "signatures-L" (without the quotes).
 */
void SetSigFileName()
{
    printf("Signatures file name: ");

    if (fgets(sigFileName, PATH_MAX, stdin))
    {
        sigFileName[strlen(sigFileName) - 1] = '\0'; // remove new line
    }
}

/**
 * @brief this function receives a file pointer and returns a virus* that
represents the next virus in the file.
 *
 * @param file a file to read/scan.
 *
 * @pre current position in file is a beginning of a virus.
 * @return virus* the next virus in the file.
 */
virus *readVirus(FILE *file)
{
    virus *newVirus = (virus *)calloc(1, sizeof(virus));

    // get the size of the signature and the virus name, together

    fread(newVirus, sizeof(unsigned char), 2 + 16, file);

    // swap the bytes
    if (usingBigEndian)
    {
        newVirus->SigSize = (newVirus->SigSize << 8) + (newVirus->SigSize >> 8);
    }

    // get the name and the signature of the virus

    newVirus->sig = (unsigned char *)calloc(newVirus->SigSize,
                                            sizeof(unsigned char));

    fread(newVirus->sig, sizeof(unsigned char), newVirus->SigSize, file);

    return newVirus;
}

/**
 * @brief The function prints the virus data to stdout.
 *
 * @param virus a virus.
 */
void printVirus(virus *virus)
{
    printVirusToFile(stdout, virus);
}

/**
 * @brief Print the data of every link in list to the given stream.
 * Each item followed by a newline character.
 *
 * @param virus_list a linked list of viruses.
 * @param stream an output stream.
 * @pre stream is open.
 */
void list_print(link *virus_list, FILE *stream)
{
    while (virus_list)
    {
        printVirusToFile(stream, virus_list->vir);
        fputc('\n', stream);
        virus_list = virus_list->nextVirus;
    }
}

/**
 * @brief Add a new link with the given data at the beginning of the list.
 *
 * @param virus_list a list of viruses.
 * @param data a virus to append.
 * @return link* a pointer to the list (i.e., the first link in the list).
 */
link *list_append(link *virus_list, virus *data)
{
    link *newLink = (link *)calloc(1, sizeof(link));

    newLink->nextVirus = virus_list;
    newLink->vir = data;

    return newLink;
}

/**
 * @brief free the memory allocated by the list.
 *
 * @param virus_list
 */
void list_free(link *virus_list)
{
    link *curr = virus_list, *next = NULL;

    while (curr)
    {
        next = curr->nextVirus;

        free(curr->vir->sig);
        free(curr->vir);
        free(curr);

        curr = next;
    }
}

void detectViruses()
{
    // TODO
    printf("?> Not implemented\n");
}

void fixFile()
{
    // TODO
    printf("?> Not implemented\n");
}

/**
 * @brief prints length bytes from memory location buffer,
 * in hexadecimal format.
 *
 * @param buffer a memory location to read bytes from.
 * @param length number of bytes to read.
 * @param file an output stream.
 *
 * @pre file is open.
 */
void printHexToFile(FILE *file, unsigned char *buffer, size_t length)
{
    for (size_t i = 0; i < length; i++)
    {
        // 02 for a two-digit representation, hh is for a char
        fprintf(file, "%02hhx ", buffer[i]);
    }
}

/**
 * @brief open the signature file for reading and set the endian accordingly.
 */
void openSigFile()
{
    reset();

    sigFile = fopen(sigFileName, "r");
    char magicNumber[4] = {0};

    if (sigFile)
    {
        fread(magicNumber, sizeof(char), 4, sigFile);

        // using strncmp because the number is not terminated by a null
        if (strncmp(magicNumber, "VIRB", 4) == 0)
        {
            usingBigEndian = true;
        }
        else if (strncmp(magicNumber, "VIRL", 4))
        {
            // the number is not VIRL nor VIRB (then strncmp would have
            // returned 0 and the condition wouldn't have been met)

            fclose(sigFile);

            sigFile = NULL;
        }
    }
}

/**
 * @brief checks if the entire file was read.
 *
 * @param file a file to check.
 *
 * @pre file is an open file.
 * @return true if we read the entire file.
 * @return false if we did not read the entire file.
 */
bool reachedEnd(FILE *file)
{
    // read one byte, then check EOF
    if (fgetc(file) == EOF)
    {
        return true;
    }

    // "undo"
    fseek(file, -1, SEEK_CUR);

    return false;
}

/**
 * @brief The function prints the virus data to a file.
 *
 * @param file an output stream.
 * @param virus a virus.
 *
 * @pre file is open.
 */
void printVirusToFile(FILE *file, virus *virus)
{
    fprintf(file, "# name:         %s\n", virus->virusName);
    fprintf(file, "# sig_size:     %d\n", virus->SigSize);
    fprintf(file, "# signature:    ");
    printHexToFile(file, virus->sig, virus->SigSize);
}

/**
 * @brief load the viruses from the current signature file.
 */
void loadViruses()
{
    openSigFile();

    fseek(sigFile, 4, SEEK_SET);

    viruses = NULL;

    while (!reachedEnd(sigFile))
    {
        viruses = list_append(viruses, readVirus(sigFile));
    }
}

/**
 * @brief print all the currently loaded viruses to stdout.
 */
void printViruses()
{
    list_print(viruses, stdout);
}

/**
 * @brief free all allocated memory and set the file and list of viruses
 * to null.
 */
void reset()
{
    if (sigFile)
    {
        fclose(sigFile);

        sigFile = NULL;
    }

    if (viruses) // ! redundant check (we know list_free works on null)
    {
        list_free(viruses);

        viruses = NULL;
    }
}

/**
 * @brief quit the program.
 */
void quit()
{
    reset();

    memset(sigFileName, 0, PATH_MAX);

    printf(">> bye!\n");
}
