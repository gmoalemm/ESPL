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
EXAMPLES
    virusDetector
    virusDetector -FILE infected
*/

#include <stdio.h>
#include <linux/limits.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

/* MACROS */

#define INPUT_MAX 8
#define BUFFER_MAX 10 << 10

#define DEFAULT_SIGFILE "signatures-L"

#define ERR_PRE "!>"
#define REG_PRE ">>"
#define MSG_PRE "*>"

#define MISSING_FILE_ERR "missing file name"
#define UNKNOWN_ARG_ERR "unknown argument"
#define FAILED_OPEN_ERR "couldn't open the file"
#define SEEK_ERR "seeking failed"
#define WRITE_ERR "failed overwriting the virus's signature"
#define NOTHING_TO_SCAN_ERR "no file to scan"

#define PRINT_ERROR(MSG) fprintf(stderr, "%s %s\n", ERR_PRE, MSG)

/* STRUCTURES */

typedef struct virus
{
    unsigned short SigSize;
    char virusName[16];
    unsigned char *sig;
} virus;

// linked list of viruses
typedef struct link
{
    struct link *nextVirus;
    virus *vir;
} link;

// linked list of bytes locations
typedef struct posLink
{
    struct posLink *nextVirus;
    size_t position;
} posLink;

// function descriptor
typedef struct fun_desc
{
    char *name;
    void (*fun)(void);
} fun_desc;

/* REQUIRED AUXILIARY METHODS */

void SetSigFileName();
virus *readVirus(FILE *);
void printVirus(virus *);
void list_print(link *, FILE *);
link *list_append(link *, virus *);
void list_free(link *);
void quit();
void detectViruses();
void fixFile();
void detect_virus(char *, unsigned int, link *);
void neutralize_virus(char *, int);

/* ADDITIONAL AUXILIARY METHODS */

void printHexToFile(FILE *, unsigned char *, size_t);
void openSigFile();
bool reachedEnd(FILE *);
void printVirusToFile(FILE *, virus *);
void loadViruses();
void printViruses();
void reset();
posLink *scanFile(char *, unsigned int, link *, bool);

/* GLOBALS */

char signaturesFilename[PATH_MAX] = {0};
char *fileToScan = NULL;
bool usingBigEndian = false;
FILE *signaturesFile = NULL;
link *knownVirusesList = NULL;

int main(int argc, char **argv)
{
    fun_desc menuItems[] = {
        {"set signatures file name", SetSigFileName},
        {"load signatures", loadViruses},
        {"print signatures", printViruses},
        {"detect viruses", detectViruses},
        {"fix file", fixFile},
        {"quit", quit}};
    int numOfOptions = sizeof(menuItems) / sizeof(menuItems[0]);
    int option = -1, i = 0;
    char input[INPUT_MAX] = {0};
    bool errorOccurred = false;

    for (i = 1; i < argc && !errorOccurred; i++)
    {
        if (!strcmp(argv[i], "-FILE"))
        {
            if (++i < argc)
            {
                fileToScan = argv[i];
            }
            else
            {
                PRINT_ERROR(MISSING_FILE_ERR);
                errorOccurred = true;
            }
        }
        else
        {
            PRINT_ERROR(UNKNOWN_ARG_ERR);
            errorOccurred = true;
        }
    }

    strcpy(signaturesFilename, DEFAULT_SIGFILE);

    // no signature file name means the user wants to quit
    while (!errorOccurred && strcmp(signaturesFilename, ""))
    {
        // print the menu

        for (i = 0; i < numOfOptions; i++)
        {
            printf("#%d %s\n", i, menuItems[i].name);
        }

        printf("%s choose an option: ", REG_PRE);

        if (fgets(input, INPUT_MAX, stdin))
        {
            option = atoi(input);

            if (option < 0 || option >= numOfOptions)
            {
                PRINT_ERROR("not an option");
                errorOccurred = true;
            }
            else
            {
                menuItems[option].fun();
            }
        }
        else
        {
            PRINT_ERROR(FAILED_OPEN_ERR);
            errorOccurred = true;
        }
    }

    if (errorOccurred)
    {
        quit();
    }

    return errorOccurred;
}

/**
 * @brief this function queries the user for a new signature file name, and
sets the signature file name accordingly.
 */
void SetSigFileName()
{
    printf("%s signatures file name: ", REG_PRE);

    if (fgets(signaturesFilename, PATH_MAX, stdin))
    {
        // remove new line
        signaturesFilename[strlen(signaturesFilename) - 1] = '\0';
    }
}

/**
 * @brief this function reads the next virus from a file.
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
        newVirus->SigSize = (newVirus->SigSize << 8) | (newVirus->SigSize >> 8);
    }

    // get the name and the signature of the virus

    newVirus->sig = (unsigned char *)calloc(newVirus->SigSize,
                                            sizeof(unsigned char));

    fread(newVirus->sig, sizeof(unsigned char), newVirus->SigSize, file);

    return newVirus;
}

/**
 * @brief prints a virus' data to stdout.
 *
 * @param virus a virus.
 */
void printVirus(virus *virus)
{
    printVirusToFile(stdout, virus);
}

/**
 * @brief print the data of every link in list to the given stream.
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
 * @brief add a new link with the given data at the beginning of the list.
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

/**
 * @brief neutralize all viruses in the scanned file.
 */
void fixFile()
{
    FILE *file = NULL;
    char buffer[BUFFER_MAX] = {0};
    size_t bytesRead;
    posLink *infections = NULL, *next;

    if (!fileToScan)
    {
        PRINT_ERROR(NOTHING_TO_SCAN_ERR);
        return;
    }

    if ((file = fopen(fileToScan, "r")) == NULL)
    {
        PRINT_ERROR(FAILED_OPEN_ERR);
        return;
    }

    bytesRead = fread(buffer, sizeof(char), BUFFER_MAX, file);
    fclose(file);

    infections = scanFile(buffer, bytesRead, knownVirusesList, false);

    while (infections)
    {
        neutralize_virus(fileToScan, infections->position);

        next = infections->nextVirus;
        free(infections);
        infections = next;
    }
}

/**
 * @brief detect all the viruses in the scanned file.
 */
void detectViruses()
{
    FILE *file = NULL;
    char buffer[BUFFER_MAX] = {0};
    size_t bytesRead;

    if (!fileToScan)
    {
        PRINT_ERROR(NOTHING_TO_SCAN_ERR);
        return;
    }

    if ((file = fopen(fileToScan, "r")) == NULL)
    {
        PRINT_ERROR(FAILED_OPEN_ERR);
        return;
    }

    bytesRead = fread(buffer, sizeof(char), BUFFER_MAX, file);
    fclose(file);

    // assuming bytesRead <= BUFFER_MAX
    detect_virus(buffer, bytesRead, knownVirusesList);
}

/**
 * @brief compares the content of the buffer byte-by-byte with the virus
 * signatures stored in the virus_list linked list. If a virus is detected,
 * for each detected virus the function prints its name, sig. len and
 * starting byte in the file.
 *
 * @param buffer content of a file.
 * @param size the minimum between the size of the buffer and the size of the
 * suspected file in bytes
 * @param virus_list list of known viruses' signatures.
 */
void detect_virus(char *buffer, unsigned int size, link *virus_list)
{
    posLink *infections = scanFile(buffer, size, virus_list, true);
    posLink *next;

    while (infections)
    {
        next = infections->nextVirus;
        free(infections);
        infections = next;
    }
}

/**
 * @brief deactivates a virus in a given file.
 *
 * @param fileName the infected file's name.
 * @param signatureOffset the first byte of the virus' signature in the file.
 */
void neutralize_virus(char *fileName, int signatureOffset)
{
    FILE *infected = fopen(fileName, "r+");
    const char RET[] = {(char)0xC3};

    if (infected)
    {
        if (fseek(infected, signatureOffset, SEEK_SET) == -1)
        {
            PRINT_ERROR(SEEK_ERR);
        }
        else
        {
            if (fwrite(RET, 1, 1, infected) != 1)
            {
                PRINT_ERROR(WRITE_ERR);
            }

            fclose(infected);
        }
    }
    else
    {
        PRINT_ERROR(FAILED_OPEN_ERR);
    }
}

/**
 * @brief prints memory in hexadecimal format.
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

    signaturesFile = fopen(signaturesFilename, "r");
    char magicNumber[4] = {0};

    if (signaturesFile)
    {
        fread(magicNumber, sizeof(char), 4, signaturesFile);

        // using strncmp because the number is not terminated by a null
        if (strncmp(magicNumber, "VIRB", 4) == 0)
        {
            usingBigEndian = true;
        }
        else if (strncmp(magicNumber, "VIRL", 4))
        {
            // the number is not VIRL nor VIRB (then strncmp would have
            // returned 0 and the condition wouldn't have been met)

            fclose(signaturesFile);

            signaturesFile = NULL;
        }
    }
    else
    {
        PRINT_ERROR(FAILED_OPEN_ERR);
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
 * @brief prints the virus data to a file.
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

    if (signaturesFile)
    {
        fseek(signaturesFile, 4, SEEK_SET);

        knownVirusesList = NULL;

        while (!reachedEnd(signaturesFile))
        {
            knownVirusesList = list_append(knownVirusesList,
                                           readVirus(signaturesFile));
        }
    }
}

/**
 * @brief print all the currently loaded viruses to stdout.
 */
void printViruses()
{
    if (!knownVirusesList)
    {
        printf("%s no viruses to show\n", MSG_PRE);
    }
    else
    {
        list_print(knownVirusesList, stdout);
    }
}

/**
 * @brief free all allocated memory and set the file and list of viruses
 * to null.
 */
void reset()
{
    if (signaturesFile)
    {
        fclose(signaturesFile);

        signaturesFile = NULL;
    }

    list_free(knownVirusesList);

    knownVirusesList = NULL;
}

/**
 * @brief quit the program.
 */
void quit()
{
    reset();

    memset(signaturesFilename, 0, PATH_MAX);

    printf("%s bye!\n", REG_PRE);
}

/**
 * @brief scan a buffer for viruses.
 *
 * @param buffer a buffer to scan.
 * @param size the size of the buffer.
 * @param virus_list viruses to look for.
 * @param print should the method inform the use about each virus it finds?
 * @return posLink* a list of position of viruses in the buffer.
 */
posLink *scanFile(char *buffer, unsigned int size, link *virus_list, bool print)
{
    size_t i;
    link *current;
    posLink *head = NULL, *tmp = NULL;

    if (!virus_list)
    {
        return head;
    }

    for (i = 0; i < size; i++)
    {
        current = virus_list;

        while (current)
        {
            if (!memcmp(buffer + i, current->vir->sig, current->vir->SigSize))
            {
                if (print)
                {
                    printf("# %s (%d) @ 0x%04x\n",
                           current->vir->virusName, current->vir->SigSize, i);
                }

                tmp = (posLink *)calloc(1, sizeof(posLink));
                tmp->position = i;
                tmp->nextVirus = head;
                head = tmp;
            }

            current = current->nextVirus;
        }
    }

    return head;
}
