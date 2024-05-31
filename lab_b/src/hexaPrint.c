/**
NAME
    hexaPrint - prints the hexdecimal value of the input bytes from a given file
SYNOPSIS
    hexaPrint FILE
DESCRIPTION
    hexaPrint receives, as a command-line argument, the name of a "binary" file,
    and prints the hexadecimal value of each byte to the standard output,
    separated by spaces.
*/

#include <stdio.h>

#define BUFFER_SIZE 64

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

int main(int argc, char **argv)
{
    // use chars buffer so each byte is read seperately

    FILE *file;
    unsigned char buffer[BUFFER_SIZE] = {0};
    size_t itemsRead;

    if (argc > 2)
    {
        fprintf(stderr, "Too many arguments!\n");
        return 1;
    }

    if (argc == 1)
    {
        fprintf(stderr, "Missing file path!\n");
        return 1;
    }

    if (!(file = fopen(argv[1], "r")))
    {
        perror("Couldn't open the file");
        return 1;
    }

    while (!feof(file))
    {
        if ((itemsRead = fread(buffer, sizeof(char), BUFFER_SIZE, file)))
        {
            printHex(buffer, itemsRead);
            printf(" ");
        }
    }

    printf("\n");

    if (fclose(file) == EOF)
    {
        perror("Couldn't close the file");
        return 1;
    }

    return 0;
}
