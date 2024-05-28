#include <stdio.h>
#include <linux/limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include "LineParser.h"

#define LINE_MAX 2048
#define FALSE 0
#define TRUE 1

int execute(cmdLine *pCmdLine)
{

    // * running "ls" with execv won't work because it requires the full path, as opposed to execvp which searches
    // * "in the colon-separated list of directory pathnames specified in the PATH environment variable" (~man).

    // * in addition, wildcards (for example) won't work, because it is the shell that is doing the expansion, and not the command itself.
    execvp(pCmdLine->arguments[0], pCmdLine->arguments);

    // this will happen only if the execution will fail.
    // "The exec() functions return only if an error has occurred." (~man)
    return 1;
}

int main(int argc, char **argv)
{
    char *cwd = NULL, line[LINE_MAX] = {0};
    cmdLine *command = NULL;
    int execError = 0, quit = 0;
    int debug = FALSE;
    int i;
    int pid;

    // scan for line arguments
    for (i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], "-d") == 0)
        {
            debug = TRUE;
        }
    }

    do
    {
        execError = 0;

        // print the current working directory
        cwd = getcwd(cwd, PATH_MAX);
        printf("%s: ", cwd);

        // get the next command from the user
        fgets(line, LINE_MAX, stdin);

        // parse and execute it

        command = parseCmdLines(line);

        if (strcmp(command->arguments[0], "quit") == 0)
        {
            quit = TRUE;
        }
        else if (strcmp(command->arguments[0], "cd") == 0)
        {
            if (chdir(command->arguments[1]) == -1 && debug)
            {
                perror("Couldn't change directory");
            }
        }
        else
        {   
            if ((pid = fork()) > 0)
            {
                // parent

                // ? this is weird, for example, when calling "ls" without '&', 
                // ? the command will print its output and the parent porocess
                // ? will print the cwd simultaneously and they'll mix.
                // ? is this supposed to happen?
                if (command->blocking)
                {
                    waitpid(pid, &execError, 0);
                }
            }
            else
            {
                // child

                if (debug)
                {
                    fprintf(stderr, "(d) pid = %d\tcommand = %s", getpid(), line);
                }

                // ! note that the execution will end after the command is executed (if an error didn't occur).
                // ! this happend becuase the execute method uses the execv/execvp system call which
                // ! "replaces the current process image with a new process image" (~man).
                // * (meaning,) this system call replaces this program with the commands program.
                execError = execute(command);

                if (execError && debug)
                {
                    // this part will happen only if the execution will fail.
                    // "The exec() functions return only if an error has occurred." (~man)

                    perror("Execution failed");

                    // * using _exit instead of exit becuase the latter doesn't just exits, but also, for example,
                    // * flushes buffered I/O, and closes open file descriptors.
                    _exit(1);
                }
            } // ignoring the case of -1
        }

        free(command);
        command = NULL;
    } while (!execError && !quit);

    return execError;
}
