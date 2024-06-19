#include "util.h"

#define SYS_WRITE 4
#define STDOUT 1
#define SYS_OPEN 5
#define O_RDWR 2
#define SYS_SEEK 19
#define SEEK_SET 0
#define SHIRA_OFFSET 0x291

extern int system_call();
extern void infection();
extern void infector();

int main(int argc, char *argv[], char *envp[])
{
    unsigned int i;
    char *filename = 0;

    for (i = 1; i < argc; i++)
    {
        if (!strncmp(argv[i], "-a", 2))
        {
            filename = argv[i] + 2;
            system_call(SYS_WRITE, STDOUT, filename, strlen(filename));
            system_call(SYS_WRITE, STDOUT, "\n", 1);
            break;
        }
    }

    infection();

    if (filename)
        infector(filename);

    return 0;
}
