#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define IN_MAX 32

typedef struct state { 
  char debug_mode; 
  char file_name[128]; 
  int unit_size; 
  unsigned char mem_buf[10000]; 
  size_t mem_count; 
  char display_mode;    // true for hex, false for dec
} state;

typedef struct fun_desc {
  char *name;
  void (*fun)(state*);
} fun_desc; 

void toggleDebugMode(state *);
void setFileName(state *);
void setUnitSize(state *);
void loadIntoMemory(state *);
void toggleDisplayMode(state *);
void memoryDisplay(state *);
void saveIntoFile(state *);
void memoryModify(state *);
void quit(state *);

void printMenu(fun_desc *menu, state *s)
{
    unsigned i = 0;

    if (s->debug_mode)
    {
        fprintf(stderr, "debug: unit_size = %d, file_name = %s, mem_count = %d\n",
               s->unit_size, s->file_name, s->mem_count);
    }

    puts("Choose action:");

    while (menu->fun)
    {
        printf("%d - %s\n", i++, menu->name);
        menu++;
    }
}

int main(void)
{
    fun_desc menu[] = { 
        {"Toggle Debug Mode", toggleDebugMode},
        {"Set File Name", setFileName},
        {"Set Unit Size", setUnitSize},
        {"Load Into Memory", loadIntoMemory},
        {"Toggle Display Mode", toggleDisplayMode},
        {"Memory Display", memoryDisplay},
        {"Save Into File", saveIntoFile},
        {"Memory Modify", memoryModify},
        {"Quit", quit},
        {NULL, NULL}
     };

    unsigned option = -1, menuLen = sizeof(menu) / sizeof(menu[0]);

    char input[IN_MAX] = { 0 };

    state s = {0, "", 1, "", 0, 0};

    while (1)
    {
        printMenu(menu, &s);

        printf(">> ");

        fgets(input, IN_MAX, stdin);

        option = atoi(input);

        if (option < 0 || option >= menuLen)
        {
            fprintf(stderr, "invalid menu option!\n");
            continue;
        }

        menu[option].fun(&s);
    }
    
    return 0;
}

void toggleDebugMode(state *s)
{
    s->debug_mode = !(s->debug_mode);

    if (s->debug_mode)
    {
        puts("debug flag now on");
    }
    else
    {
        puts("debug flag now off");
    }
}

void setFileName(state *s)
{
    char input[100] = { 0 };

    printf(">> file_name = ");

    fgets(input, 100, stdin);

    strcpy(s->file_name, input);

    s->file_name[strlen(s->file_name) - 1] = '\0';

    if (s->debug_mode)
    {
        fprintf(stderr, "debug: file name set to %s\n", s->file_name);
    }
}

void setUnitSize(state *s)
{
    char input[8] = { 0 };
    unsigned n;

    printf(">> unit_size = ");

    fgets(input, 8, stdin);

    n = atoi(input);

    if (n == 1 || n == 2 || n == 4)
    {
        s->unit_size = n;

        if (s->debug_mode)
        {
            fprintf(stderr, "debug: set size to %d\n", s->unit_size);
        }
    }
    else
    {
        puts("invalid unit size!");
    }
}

void loadIntoMemory(state *s)
{
    FILE *file = NULL;
    unsigned location = -1, length = -1, items = -1;
    char input[16] = { 0 }, buff[10000];

    if (!strcmp(s->file_name, ""))
    {
        if (s->debug_mode)
        {
            puts("empty file name");
        }

        return;
    }

    if ((file = fopen(s->file_name, "rb")) == NULL)
    {
        perror("cannot open this file");
        return;
    }

    printf("please enter <location> <length>: ");
    fgets(input, 16, stdin);
    sscanf(input, "%x %d\n", &location, &length);

    if (s->debug_mode)
    {
        fprintf(stderr, "file_name = %s, location = %x, length = %d\n",
                s->file_name, location, length);
    }

    if ((fseek(file, location, SEEK_SET)) == -1)
    {
        perror("could not seek");
        fclose(file);
        return;
    }

    clearerr(file);

    items = fread(buff, s->unit_size, length, file);

    if (ferror(file))
    {
        perror("error reading file");
    }

    memcpy(s->mem_buf, buff, (s->mem_count = items * s->unit_size));

    if (s->debug_mode)
    {
        fprintf(stderr, "loaded %d units into memory\n", items);
    }

    fclose(file);
}

void toggleDisplayMode(state *s)
{
    s->display_mode = !(s->display_mode);

    if (s->debug_mode)
    {
        puts("display flag now on, hexadecimal representation");
    }
    else
    {
        puts("display flag now off, decimal representation");
    }
}

void memoryDisplay(state *s)
{
    static char* hex_formats[] = {"%#hhx\n", "%#hx\n", "No such unit", "%#x\n"}; 
    static char* dec_formats[] = {"%#hhd\n", "%#hd\n", "No such unit", "%#d\n"};

    FILE *file = NULL;
    int u = 0, addr = 0, bytesread = 0, bytesToPrint = 0, val = 0, i = 0;
    char input[16] = { 0 }, buffer[10000];

    printf("enter address and length: ");
    fgets(input, 16, stdin);
    sscanf(input, "%x %d\n", &addr, &u);

    if (!addr)
    {
        memcpy(buffer, s->mem_buf, (bytesread = s->mem_count));
    }
    else
    {
        if (!strcmp(s->file_name, ""))
        {
            puts("empty file name");
            return;
        }

        if ((file = fopen(s->file_name, "rb")) == NULL)
        {
            perror("cannot open this file");
            return;
        }

        if ((fseek(file, addr, SEEK_SET)) == -1)
        {
            perror("could not seek");
            fclose(file);
            return;
        }

        clearerr(file);

        bytesread = fread(buffer, s->unit_size, u, file);

        if (ferror(file))
        {
            perror("error reading file");
        }

        fclose(file);
    }

    bytesToPrint = bytesread > (u * s->unit_size) ? (u * s->unit_size) : bytesread;

    for (i = 0; i < bytesToPrint; i += s->unit_size)
    {
        val = (s->unit_size == 1) ? *(buffer + i) : 
            (s->unit_size == 2) ? *(short*)(buffer + i) : *(int*)(buffer + i);

        if (s->display_mode)
        {
            printf(hex_formats[s->unit_size - 1], val);
        }
        else
        {
            printf(dec_formats[s->unit_size - 1], val); 
        }
    }
}

void saveIntoFile(state *s) {
    char input[32] = { 0 };
    unsigned int srcAddr, tarLoc, length;
    unsigned char *realSrc;
    int fd;
    off_t fileSize;

    if (!strcmp(s->file_name, ""))
    {
        puts("empty file name");
        return;
    }

    fd = open(s->file_name, O_RDWR);

    if (fd == -1) {
        if (s->debug_mode)
        {
            perror("could not open file");
        }
        return;
    }

    // check file size
    if ((fileSize = lseek(fd, 0, SEEK_END)) == -1) {
        perror("could not determine file size");
        close(fd);
        return;
    }

    if (tarLoc > fileSize) {
        fprintf(stderr, "traget is beyond file's limit\n");
        close(fd);
        return;
    }

    printf("please enter <source-address> <traget-location> <length>: ");
    fgets(input, 32, stdin);
    sscanf(input, "%x %x %d\n", &srcAddr, &tarLoc, &length);

    realSrc = (!srcAddr) ? s->mem_buf : (unsigned char *)srcAddr;

    if (lseek(fd, tarLoc, SEEK_SET) == -1) {
        perror("faild seeking");
        close(fd);
        return;
    }

    if (write(fd, realSrc, length * s->unit_size) <= 0) {
        perror("error writing to file");
        close(fd);
        return;
    }

    close(fd);
}

void memoryModify(state *s)
{
    unsigned location, val;
    char input[32] = { 0 };

    printf("please enter <location> <val>: ");
    fgets(input, 32, stdin);
    sscanf(input, "%x %x\n", &location, &val);

    if (s->debug_mode)
    {
        printf("location = %x, val = %x\n", location, val);
    }

    if (location < 0 || location >= 1000)
    {
        if (s->debug_mode)
        {
            fprintf(stderr, "invalid location\n");
        }
    }

    memcpy((s->mem_buf + location), &val, s->unit_size);

    if (location + s->unit_size > s->mem_count)
    {
        s->mem_count = location + s->unit_size;
    }
}

void quit(state *s)
{
    if (s->debug_mode)
    {
        fprintf(stderr, "quitting\n");
    }

    exit(0);
}
