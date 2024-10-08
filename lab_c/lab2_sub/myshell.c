#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/limits.h> // for PATH_MAX
#include <unistd.h>       // for execvp, close, dup, chdir, fork and constants
#include <sys/wait.h>     // for waitpid
#include <signal.h>       // for kill and constants
#include <fcntl.h>        // for open
#include <sys/stat.h>     // for permission constants
#include "LineParser.h"

#define LINE_MAX 2048
#define FALSE 0
#define TRUE 1

/**
 * @brief execute a command.
 *
 * @param pCmdLine a command to execute.
 * @return int 1 iff the execution failed, otherwise the function never returns.
 */
int execute(cmdLine *pCmdLine)
{
    execvp(pCmdLine->arguments[0], pCmdLine->arguments);

    // this will happen only if the execution will fail.
    return 1;
}

/**
 * @brief redirect input/output stream to another file.
 *
 * @param file the new file, the destination.
 * @param oldfd the stream to close (STDIN_FILENO or STDOUT_FILENO only).
 * @param debug indicates if errors should be printed to stderr.
 * @return int 0 in success, 1 in failure.
 */
int redirect(const char *file, int oldfd, int debug)
{
    int filedp;
    int flags;

    if (oldfd == STDIN_FILENO)
    {
        flags = O_RDONLY;
    }
    else if (oldfd == STDOUT_FILENO)
    {
        flags = O_WRONLY | O_CREAT;
    }

    // close the old fd (which should be low) so the new file could take its
    // place
    if (close(oldfd) == -1)
    {
        if (debug)
        {
            perror("(d) Couldn't close file");
        }

        return 1;
    }

    // try to open and get the fd of the file
    // ! the mode is ignored when O_CREAT is not specified
    if ((filedp = open(file, flags, S_IRWXU | S_IRGRP | S_IROTH)) == -1)
    {
        if (debug)
        {
            perror("(d) Couldn't open file");
        }

        return 1;
    }

    if (filedp != oldfd)
    {
        if (debug)
        {
            // I use fprintf here because the call actually worked,
            // just not as expected
            fprintf(stderr, "(d) open returned something unexpected");
        }

        if (close(filedp) == -1) // no need to keep it open twice
        {
            if (debug)
            {
                perror("(d) Couldn't close file");
            }
        }

        return 1;
    }

    return 0;
}

/**
 * @brief start a child process to run a command.
 *
 * @param cmd a command to run in a child process.
 * @param rawCommand the actual line written by the user.
 * @param debug indicates if errors should be printed to stderr.
 */
void runChildProcess(cmdLine *cmd, char *rawCommand, int debug)
{
    if (debug)
    {
        fprintf(stderr, "\n(d) pid = %d | command = %s", getpid(), rawCommand);
    }

    if ((cmd->inputRedirect &&
         redirect(cmd->inputRedirect, STDIN_FILENO, debug)) ||
        (cmd->outputRedirect &&
         redirect(cmd->outputRedirect, STDOUT_FILENO, debug)))
    {
        // redirecting failed, exit
        _exit(1);
    }

    execute(cmd);

    // * this part will happen only if the execution will fail.

    if (debug)
    {
        perror("(d) Execution failed");
    }

    _exit(1);
}

int main(int argc, char **argv)
{
    char cwd[PATH_MAX] = {0}, line[LINE_MAX] = {0};
    cmdLine *command = NULL;
    int execError = FALSE, pid, stat;
    int debug = FALSE;
    int i;

    // scan for line arguments
    for (i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], "-d") == 0)
        {
            debug = TRUE;
        }
    }

    getcwd(cwd, PATH_MAX);

    do
    {
        printf("%s: ", cwd);

        // get the next command from the user
        fgets(line, LINE_MAX, stdin);

        if (feof(stdin))
        {
            freeCmdLines(command);
            break;
        }

        // parse and execute it

        command = parseCmdLines(line);

        if (strcmp(command->arguments[0], "quit") == 0)
        {
            freeCmdLines(command);
            break;
        }

        if (strcmp(command->arguments[0], "cd") == 0)
        {
            if (chdir(command->arguments[1]) == -1)
            {
                if (debug)
                {
                    perror("(d) Couldn't change directory");
                }
            }
            else
            {
                getcwd(cwd, PATH_MAX);
            }
        }
        else if (strcmp(command->arguments[0], "alarm") == 0)
        {
            if (kill(atoi(command->arguments[1]), SIGCONT) == -1 && debug)
            {
                perror("(d) Signaling failed");
            }
        }
        else if (strcmp(command->arguments[0], "blast") == 0)
        {
            if (kill(atoi(command->arguments[1]), SIGINT) == -1 && debug)
            {
                perror("(d) Signaling failed");
            }
        }
        else
        {
            if ((pid = fork()) > 0)
            {
                if (command->blocking)
                {
                    waitpid(pid, &stat, 0);
                }
            }
            else if (pid < 0)
            {
                perror("fork failed");
                execError = TRUE;
            }
            else
            {
                runChildProcess(command, line, debug);
            }
        }

        freeCmdLines(command);

        command = NULL;
    } while (!execError);

    return execError;
}
