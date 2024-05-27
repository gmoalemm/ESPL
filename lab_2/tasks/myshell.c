#include <stdio.h>
#include <linux/limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "LineParser.h"

#define LINE_MAX 2048

int execute(cmdLine *pCmdLine)
{

    // * running "ls" with execv won't work because it requires the full path, as opposed to execvp which searches
    // * "in the colon-separated list of directory pathnames specified in the PATH environment variable" (~man).

    // * in addition, wildcards (for example) won't work, because it is the shell that is doing the expansion, and not the command itself.
    execvp(pCmdLine->arguments[0], pCmdLine->arguments);

    // this part (printing an error and returning 1) will happen only if the execution will fail.
    // "The exec() functions return only if an error has occurred." (~man)

    perror("Execution failed");

    return 1;
}

int main(void)
{
    char *cwd = NULL, line[LINE_MAX] = {0};
    cmdLine *command = NULL;
    int execError = 0, quit = 0;

    do
    {
        // print the current working directory
        cwd = getcwd(cwd, PATH_MAX);
        printf("%s: ", cwd);

        // get the next command from the user
        fgets(line, LINE_MAX, stdin);

        // parse and execute it

        command = parseCmdLines(line);
        quit = !strcmp(command->arguments[0], "quit");

        if (!quit)
        {
            // ! note that the execution will end after the command is executed (if an error didn't occur).
            // ! this happend becuase the execute method uses the execv/execvp system call which
            // ! "replaces the current process image with a new process image" (~man).
            // * (meaning,) this system call replaces this program with the commands program.
            execError = execute(command);
        }

        free(command);
        command = NULL;
    } while (!execError && !quit);

    return execError;
}
