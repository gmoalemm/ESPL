#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        return -1;
    }

    int lim = atoi(argv[1]);

    while (lim > 0)
    {
        printf("lim is now %d\n", lim--);
    }

    return 0;
}
