#include <stdio.h>

int add(int a, int b)
{
    return a + b;
}

int sub(int a, int b)
{
    return a - b;
}

int mul(int a, int b)
{
    return a * b;
}

int div(int a, int b)
{
    return a / b;
}

int main()
{
    int a, b;
    int *pa = &a, *pb = &b;

    printf("pa: %d, pb: %d, diff: %d\n", pa, pb, pa - pb);                             // 1
    printf("pa: %d, pb: %d, diff as ints: %d\n", pa, pb, (intptr_t)pa - (intptr_t)pb); // 4

    const char *longString = "This is a splitted "
                             "line because the message is "
                             "long!";

    printf("long string: %s\n", longString);

    typedef int (*arithmeticFunc)(int, int);

    arithmeticFunc funcs[] = {&add, &sub, &mul, &div};

    for (int i = 0; i < 4; i++)
        printf("%d ", funcs[i](3, 5));

    return 0;
}
