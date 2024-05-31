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
#include <linux/limits.h> // for PATH_MAX
#include <string.h>       // for strcpy, strlen
#include <stdlib.h>       // for calloc
#include <stdbool.h>

// a structure that represents a vidrus' name and signature.
typedef struct virus
{
    unsigned short SigSize;
    char virusName[16];
    unsigned char *sig;
} virus;

// auxiliary functions (required)

void SetSigFileName();
virus *readVirus(FILE *);
void printVirus(virus *);

// auxiliary functions (not required)

void printHex(unsigned char *, size_t);
FILE *openSigFile();
void freeVirus(virus *);
bool reachedEnd(FILE *);

// globals
// ? should they be globals?

char sigFileName[PATH_MAX] = {0};

bool usingBigEndian = false;

int main(int argc, char **argv)
{
    FILE *sigFile = NULL;
    virus *currentVirus;

    strcpy(sigFileName, "signatures-L");

    // SetSigFileName();

    // open the signature file
    sigFile = openSigFile();

    if (!sigFile)
    {
        fprintf(stderr, "Error opening signature file.");
        return 1;
    }

    while (!reachedEnd(sigFile))
    {
        currentVirus = readVirus(sigFile);
        printVirus(currentVirus);
        printf("\n");
        freeVirus(currentVirus);
    }

    fclose(sigFile);

    return 0;
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
 * @brief this function receives a pointer to a virus structure.
 *
 * @param virus a virus.
 */
void printVirus(virus *virus)
{
    printf("# name:         %s\n", virus->virusName);
    printf("# sig_size:     %d\n", virus->SigSize);
    printf("# signature:    ");
    printHex(virus->sig, virus->SigSize);
}

/**
 * @brief prints length bytes from memory location buffer,
 * in hexadecimal format.
 *
 * @param buffer a memory location to read bytes from.
 * @param length number of bytes to read.
 */
void printHex(unsigned char *buffer, size_t length)
{
    for (size_t i = 0; i < length; i++)
    {
        // 02 for a two-digit representation, hh is for a char
        printf("%02hhx ", buffer[i]);
    }
}

/**
 * @brief open the signature file for reading and set the endian accordingly.
 *
 * @return FILE* the signature file in sigFileName global or NULL if either
 * the file doesn't exist or begins in an invalid magic number.
 */
FILE *openSigFile()
{
    FILE *file = fopen(sigFileName, "r");
    char magicNumber[4] = {0};

    if (file)
    {
        fread(magicNumber, sizeof(char), 4, file);

        // using strncmp because the number is not terminated by a null
        if (strncmp(magicNumber, "VIRB", 4) == 0)
        {
            usingBigEndian = true;
        }
        else if (strncmp(magicNumber, "VIRL", 4))
        {
            // the number is not VIRL nor VIRB (then strncmp would have
            // returned 0 and the condition wouldn't have been met)

            fclose(file);

            file = NULL;
        }
    }

    return file;
}

void freeVirus(virus *v)
{
    free(v->sig);
    free(v);
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
