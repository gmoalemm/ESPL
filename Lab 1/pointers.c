#include <stdio.h>

int main()
{
    int a, b;
    int *pa = &a, *pb = &b;

    printf("pa: %d, pb: %d, diff: %d\n", pa, pb, pa - pb);                             // 1
    printf("pa: %d, pb: %d, diff as ints: %d\n", pa, pb, (intptr_t)pa - (intptr_t)pb); // 4

    return 0;
}
