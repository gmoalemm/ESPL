#include <stdio.h>

typedef struct BinaryNode
{
    int data;
    struct BinaryNode *left, *right;
} BinaryNode;

void inOrder(BinaryNode *root)
{
    printf("%d ", root->data);

    if (root->left)
    {
        inOrder(root->left);
    }

    if (root->right)
    {
        inOrder(root->right);
    }
}

typedef union Number
{
    short small;
    int medium;
    long large;
} Number;

typedef enum AnimalClass
{
    MAMMALS,
    BIRDS,
    REPTILES,
    AMPHIBIANS,
    INSECTS
} AnimalClass;

const char *getAnimal(AnimalClass animal)
{
    switch (animal)
    {
    case MAMMALS:
        return "dog";
    case BIRDS:
        return "owl";
    case REPTILES:
        return "snake";
    case AMPHIBIANS:
        return "frog";
    case INSECTS:
        return "fly";
    default:
        return "alien";
    }
}

int main(void)
{
    BinaryNode ll = {2, NULL, NULL};
    BinaryNode lr = {3, NULL, NULL};
    BinaryNode l = {1, &ll, &lr};
    BinaryNode rl = {5, NULL, NULL};
    BinaryNode rr = {6, NULL, NULL};
    BinaryNode r = {4, &rl, &rr};
    BinaryNode root = {0, &l, &r};

    inOrder(&root);

    printf("\n");

    Number num;

    num.large = 3000000000L;

    printf("%d %d %ld\n", num.small, num.medium, num.large);

    num.small = 4321;

    printf("%d %d %ld\n", num.small, num.medium, num.large);

    num.medium = 1234567890;

    printf("%d %d %ld\n", num.small, num.medium, num.large);

    printf("A %s is a mammal", getAnimal(MAMMALS));
    printf(" and a %s is a reptile!\n", getAnimal(REPTILES));

    return 0;
}
