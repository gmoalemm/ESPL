
#include <stdio.h>

int count_digits(char *s);

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        puts("Usage: ./ntsc <string>");
    }
    else{
        int counter = count_digits(argv[1]);
        printf("String contains %d digits, right???\n", counter);
    }

    return 0;
}

int count_digits(char *s)
{
    int counter = 0;

    if(!s)
    {
        return 0;
    }

    while(*s != '\0'){
        if(*s >= '0' && *s <= '9')
        {
            counter++;
        }

        s++;
    }

    return counter;
}
