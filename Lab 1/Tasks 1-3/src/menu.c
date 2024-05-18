#include <stdlib.h>
#include <stdio.h>
#include <string.h>
 
char* map(char *array, int array_length, char (*f) (char)){
  char* mapped_array = (char*)(malloc(array_length*sizeof(char)));
  int i;

  for (i = 0; i < array_length; i++)
  {
    mapped_array[i] = f(array[i]);
  }

  return mapped_array;
}

char toUpper(char lower)
{
  return lower - ('a' - 'A');
}

/* Ignores c, reads and returns a character from stdin using fgetc. */
char my_get(char c)
{
  return (char)fgetc(stdin);
}

/* If c is a number between 0x20 and 0x7E, 
 cprt prints the character of ASCII value c followed by a new line. 
 Otherwise, cprt prints the dot ('.') character. 
 After printing, cprt returns the value of c unchanged. */
char cprt(char c)
{
  printf("%c\n", (0x20 <= c && c <= 0x7E) ? c : '.');

  return c;
}

/* Gets a char c. 
 If c is between 0x20 and 0x4E add 0x20 to its value and return it. 
 Otherwise return c unchanged */
char encrypt(char c)
{
  return (0x20 <= c && c <= 0x4E) ? (c + 0x20) : c;
}

/* Gets a char c and returns its decrypted form subtractng 0x20 from its value. 
 But if c was not between 0x40 and 0x7E it is returned unchanged */
char decrypt(char c)
{
  return (0x40 <= c && c <= 0x7E) ? (c - 0x20) : c; // why 7E and not 6E?
}

/* xoprt prints the value of c in a hexadecimal representation, 
 then in octal representation, 
 followed by a new line, and returns c unchanged. */ 
char xoprt(char c)
{
  printf("%x %o\n", c, c);
  
  return c;
}


typedef struct fun_desc {
  char *name;
  char (*fun)(char);
} fun_desc; 

fun_desc functions[] = { 
  {"my_get", my_get}, 
  {"cprt", cprt}, 
  {"encrypt (to lowercase)", encrypt}, 
  {"decrypt (to UPPERCASE)", decrypt}, 
  {"xoprt", xoprt}, 
  {NULL, NULL}};

void menu()
{
  int i = 0;
  int arrayLen = 0;

  char line[16] = { 0 };
  char *carray = (char*)calloc(sizeof(char), 5);
  char *oldCarray = NULL;
  int op;

  // calculate array's len
  while (functions[arrayLen].name)
  {
    arrayLen++;
  }
  
  printf("Select operation from the following menu:\n");

  for (i = 0; i < arrayLen; i++)
  {
    printf("%d. %s\n", i, functions[i].name);
  }
  
  printf("Choose a function: ");

  while (fgets(line, 16, stdin) != NULL)
  {
    op = atoi(line);

    if (op < 0 || op >= arrayLen)
    {
      printf("Not within bounds.\n");
      break;
    }

    printf("Within bounds.\n");

    oldCarray = carray;

    carray = map(carray, arrayLen, functions[op].fun);

    free(oldCarray);

    printf("\n\nSelect operation from the following menu:\n");

    for (i = 0; i < arrayLen; i++)
    {
      printf("%d. %s\n", i, functions[i].name);
    }

    printf("Choose a function: ");
    fflush(stdin);
  } 

  free(carray);
}

int main(int argc, char **argv){
  menu();
  printf("\n");
}

