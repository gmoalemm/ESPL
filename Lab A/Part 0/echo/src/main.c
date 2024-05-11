#include <stdio.h>

int main(int argc, char **argv)
{
    int i;

    // starting at 1 to skip the program's name
    // ends before the last element to prevent a redundant space
    for (i = 1; i < argc - 1; i++)
    {
        printf("%s ", argv[i]);
    }

    // if argc > 1 there were words to print
    if (argc > 1)
    {
        printf("%s\n", argv[i]);
    }

    return 0;
}
