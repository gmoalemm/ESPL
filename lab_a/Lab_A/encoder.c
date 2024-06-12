#include <stdbool.h> // stackoverflow
#include <stdio.h>

void encode(FILE *in, FILE *out, char *key);
char encodeChar(int change, char orgc);

bool debugMode = true;

int main(int argc, char **argv)
{
    int i = 0; // used as an index. its value is -1 if an error has occured

    FILE *in = stdin, *out = stdout;

    // assuming we'll get at most one of each flag, this is the first and last init of these variables

    char *key = NULL, *inFileName = NULL, *outFileName = NULL;

    for (i = 1; i < argc; i++)
    {
        if (debugMode)
        {
            // print the flag/arg
            fprintf(stderr, "%s\n", argv[i]);
        }

        // an argument must contain at least 2 characters (+/- and a letter)
        if (argv[i][0] == '\0' || argv[i][1] == '\0')
        {
            if (debugMode)
            {
                fprintf(stderr, "Invalid argument!");
            }

            i = -1;
            break;
        }

        // update the debugging mode here to avoid printing "+D" and to print "-D".
        // the first argument is always printed.
        if (argv[i][1] == 'D' && (argv[i][0] == '-' || argv[i][0] == '+'))
        {
            debugMode = argv[i][0] == '+';
        }
        // all the other argument must get a non-empty parameter
        else if (argv[i][2] != '\0')
        {
            if (argv[i][1] == 'e' && (argv[i][0] == '-' || argv[i][0] == '+'))
            {
                key = argv[i];
            }
            else if (argv[i][1] == 'I' && argv[i][0] == '-')
            {
                inFileName = argv[i] + 2; // idea of +2 for trimming is also from stackoverflow
            }
            else if (argv[i][1] == 'O' && argv[i][0] == '-')
            {
                outFileName = argv[i] + 2;
            }
        }
        else
        {
            if (debugMode)
            {
                fprintf(stderr, "Invalid argument!");
            }

            i = -1;
            break;
        }
    }

    // try to open the files and encode only if a key was provided
    if (i >= 0 && key)
    {
        // open the files if needed, else they stay the standard I/O streams
        in = inFileName ? fopen(inFileName, "r") : in;
        out = outFileName ? fopen(outFileName, "a") : out;

        if (!in)
        {
            fprintf(stderr, "Failed to open %s!\n", inFileName);
        }

        if (!out)
        {
            fprintf(stderr, "Failed to open %s!\n", outFileName);
        }

        // encode only if both streams exist
        if (in && out)
        {
            encode(in, out, key);
        }
        else
        {
            i = -1;
        }
    }

    if (in)
    {
        fclose(in);
    }

    if (out)
    {
        fclose(out);
    }

    return i < 0; // 1 when i is -1
}

/**
 * Encode text using a given encoding key.
 *
 * Input:
 *  in - the input stream.
 *  out - the output stream.
 *  key - a key string, beginning in "+e" or "-e" and then a series of digits.
 *
 * Output:
 *  None (the encoded message gets written into the output stream).
 */
void encode(FILE *in, FILE *out, char *key)
{
    size_t i = 2;
    int curr = fgetc(in);

    while (!feof(in))
    {
        fputc(encodeChar((key[0] == '+') ? (key[i] - '0') : ('0' - key[i]), curr), out);

        i = key[i + 1] ? (i + 1) : 2; // if the next cha is the end of the key, go back to the beginning

        curr = fgetc(in);
    }
}

/**
 * Encode a single character.
 *
 * Input:
 *  change - the direction and the size of the change.
 *  orgc - the original character from the input.
 *
 * Output:
 *  An encodec char.
 */
char encodeChar(int shift, char orgc)
{
    char shiftedChar = orgc;

    if ('0' <= orgc && orgc <= '9')
    {
        // +10 ensures no negative values (here, -1 % 10 = -1 and not 9)
        shiftedChar = '0' + ((orgc - '0' + shift + 10) % 10);
    }
    else if ('a' <= orgc && orgc <= 'z')
    {
        // +26 ensures no negative values
        shiftedChar = 'a' + ((orgc - 'a' + shift + 26) % 26);
    }

    if (debugMode)
    {
        if (orgc == shiftedChar)
        {
            fprintf(stderr, "# Did not shift char %d\n", orgc);
        }
        else
        {
            fprintf(stderr, "# Shifted %c by %d to get %c.\n", orgc, shift, shiftedChar);
        }
    }

    return shiftedChar;
}
