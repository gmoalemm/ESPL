#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

int main(void)
{
    int pid;
    int stat;
    char *countArg[3] = {0};

    countArg[0] = "./count";

    if (pid = fork())
    {
        waitpid(pid, &stat, 0);
        printf("parent:\n");
        countArg[1] = "10";
        FILE *f = fopen("hello.jcd", "r");

        if (f == NULL)
        {
            perror("file error");
        }

        execvp(countArg[0], countArg);
    }
    else
    {
        printf("child:\n");
        countArg[1] = "5";
        execvp(countArg[0], countArg);
    }

    return 0;
}
