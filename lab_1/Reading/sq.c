#include <stdio.h>

int getSquare(int x)
{
    return x * x;
}

int inc(int x)
{
    return x + 1;
}

int main()
{
    for (int i = 0; i < 10; i++)
    {
        if (i % 2)
        {
            printf("%d ", getSquare(inc(i)));
        }
        else
        {
            printf("%d ", getSquare(i));
        }
    }

    return 0;
}
