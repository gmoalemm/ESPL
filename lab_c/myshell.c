#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/limits.h> // for PATH_MAX
#include <unistd.h>       // for execvp, close, dup, chdir, fork, pipe and constants
#include <sys/wait.h>     // for waitpid
#include <signal.h>       // for kill and constants
#include <fcntl.h>        // for open
#include <sys/stat.h>     // for permission constants
#include "LineParser.h"

#define LINE_MAX 2048
#define FALSE 0
#define TRUE 1

#define TERMINATED -1
#define RUNNING 1
#define SUSPENDED 0

#define HISTLEN 20
#define MAX_BUF 200

typedef struct process
{
    cmdLine *cmd;         /* the parsed command line*/
    pid_t pid;            /* the process id that is running the command*/
    int status;           /* status of the process: RUNNING/SUSPENDED/TERMINATED */
    struct process *next; /* next process in chain */
} process;

/*** lab c - history */

void printHistory(char hist[HISTLEN][MAX_BUF], int oldest, int newest)
{
    int i = oldest, c = 0;

    while (i != newest)
    {
        printf("%d\t%s", c++, hist[i]);
        i = (i + 1) % HISTLEN;
    }

    if (hist[i])
    {
        printf("%d\t%s", c, hist[i]);
    }
}

/*** lab c - processes manager */

int getProcStatus(pid_t pid)
{
    int stat;
    pid_t result = waitpid(pid, &stat, WNOHANG);

    return (result == 0) ? RUNNING : 
        (result != pid) ?                           TERMINATED :
        (WIFEXITED(stat) || WIFSIGNALED(stat)) ?    TERMINATED :
        WIFSTOPPED(stat) ?                          SUSPENDED : TERMINATED;
}

/**
 * @brief free the cmdLine struct if and only if it is the first in the chain.
 * That is because we know that if a node is not first in the chain, it will
 * eventually get freed when the firt command in chain will get freed.
 * If we'd try to free it before the first, we'll get an error.
 * @param node
 */
void freeIfFirstInChain(cmdLine *node)
{
    if (node && node->idx == 0)
    {
        freeCmdLines(node);
    }
}

void removeTerminatedProcesses(process **process_list)
{
    process *curr = *process_list, *prev = NULL;

    // remove from the head
    while (curr && (curr->status == TERMINATED))
    {
        *process_list = curr->next;
        freeIfFirstInChain(curr->cmd);
        free(curr);
        curr = *process_list;
    }

    // left with empty list
    if (!curr)
    {
        return;
    }

    // first is still there, scan the rest

    prev = curr;
    curr = curr->next;

    while (curr)
    {
        if (curr->status == TERMINATED)
        {
            prev->next = curr->next;
            freeIfFirstInChain(curr->cmd);
            free(curr);
            curr = prev->next;
        }
        else
        {
            curr = curr->next;
        }
    }
}

void updateProcessStatus(process *process_list, int pid, int status)
{
    process *curr = process_list;

    while (curr && (curr->pid != pid))
    {
        curr = curr->next;
    }

    if (curr)
    {
        curr->status = status;
    }
}

// ? should this function remove terminated processes?
void updateProcessList(process **process_list)
{
    process *curr = *process_list;

    while (curr)
    {
        curr->status = getProcStatus(curr->pid);
        curr = curr->next;
    }
}

void freeProcessList(process *process_list)
{
    process *curr = process_list, *next;

    while (curr)
    {
        next = curr->next;
        freeIfFirstInChain(curr->cmd);
        free(curr);
        curr = next;
    }
}

/// @param pid the process id (pid) of the process running the command.
void addProcess(process **process_list, cmdLine *cmd, pid_t pid)
{
    process *proc = (process *)calloc(1, sizeof(process));

    proc->cmd = cmd;
    proc->pid = pid;
    proc->status = getProcStatus(pid);
    proc->next = *process_list;

    *process_list = proc;
}

void printProcessList(process **process_list)
{
    process *curr;
    int i = 0, j = 0;

    updateProcessList(process_list);

    puts("#\tPID\tSTAT\tCMD");

    curr = *process_list;

    while (curr)
    {
        printf("%d\t%d\t%s\t%s", i++, curr->pid,
                    curr->status == RUNNING     ? "RUNN" :
                    curr->status == SUSPENDED   ? "SUSP"
                                                : "TERM",
               curr->cmd->arguments[0]);

        for (j = 1; j < curr->cmd->argCount; j++)
        {
            printf(" %s", curr->cmd->arguments[j]);
        }

        puts("");

        curr = curr->next;
    }

    removeTerminatedProcesses(process_list);
}

/*** lab c - pipes */

/// @brief check if a command that uses piping is valid (redirections make sense).
int validPiping(cmdLine *cmd)
{
    return !(cmd->next) || (!(cmd->outputRedirect || cmd->next->inputRedirect));
}

/*** lab 2 - shell */

/**
 * @brief execute a command.
 *
 * @param pCmdLine a command to execute.
 * @return int 1 iff the execution failed, otherwise the function never returns.
 */
int execute(cmdLine *pCmdLine)
{
    execvp(pCmdLine->arguments[0], pCmdLine->arguments);

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
    int flags = (oldfd == STDOUT_FILENO) ? O_WRONLY | O_CREAT : O_RDONLY;

    // close the old fd (should be low) so the new file could take its place
    if (close(oldfd) == -1)
    {
        if (debug)
        {
            perror("!> couldn't close file");
        }

        return 1;
    }

    // try to open and get the fd of the file
    if ((filedp = open(file, flags, S_IRWXU | S_IRGRP | S_IROTH)) == -1)
    {
        if (debug)
        {
            perror("!> couldn't open file");
        }

        return 1;
    }

    if (filedp != oldfd)
    {
        if (debug)
        {
            fprintf(stderr, "!> open returned something unexpected.");
        }

        if (close(filedp) == -1) // no need to keep it open twice
        {
            if (debug)
            {
                perror("!> couldn't close file");
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
 * @param debug indicates if errors should be printed to stderr.
 */
void runChildProcess(cmdLine *cmd, int debug)
{
    int i;
    pid_t pid = getpid();

    if (debug)
    {
        fprintf(stderr, "!> pid = %d | cmd = %s", pid, cmd->arguments[0]);

        for (i = 1; i < cmd->argCount; i++)
        {
            fprintf(stderr, " %s", cmd->arguments[i]);
        }

        fprintf(stderr, "\n");
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
        perror("!> execution failed");
    }

    _exit(1);
}

/**
 * @brief check if the current command is a special command for signaling children.
 * 
 * @param processes processes list.
 * @return int TRUE if signaled, FALSE otehrwise.
 */
int signalProc(cmdLine *command, int debug, process *processes)
{
    pid_t pid;

    // ! sleep works, but the looper still runs, why? is it like that in others' assignments?

    int sig = (!strcmp(command->arguments[0], "alarm")) ? SIGCONT   :
            (!strcmp(command->arguments[0], "blast"))   ? SIGINT    :
            (!strcmp(command->arguments[0], "sleep"))   ? SIGTSTP   : -1;

    int newStat = (sig == SIGCONT)  ? RUNNING : 
                (sig == SIGINT)     ? TERMINATED : SUSPENDED;

    if (sig != -1)
    {
        pid = atoi(command->arguments[1]);

        if (kill(pid, sig) == -1)
        {
            if (debug)
            {
                perror("!> signaling failed");
            }
        }
        else
        {
            updateProcessStatus(processes, pid, newStat);
        }

        freeCmdLines(command);

        return TRUE;
    }

    return FALSE;
}

int main(int argc, char **argv)
{
    char cwd[PATH_MAX] = {0}, line[LINE_MAX] = {0}, *tmp = NULL, history[HISTLEN][MAX_BUF];
    cmdLine *command = NULL;
    int execError = FALSE, debug = FALSE, i, p[2], newest = -1, oldest = -1;;
    pid_t pid1, pid2;
    process *processes = NULL;

    // scan for line arguments
    for (i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], "-d") == 0)
        {
            debug = TRUE;
        }
    }

    for (i = 0; i < HISTLEN; i++)
    {
        history[i][0] = '\0';
    }

    getcwd(cwd, PATH_MAX);

    do
    {
        printf("%s: ", cwd);

        // get the next command from the user
        fgets(line, LINE_MAX, stdin);

        if (feof(stdin))
        {
            break;
        }

        // parse and execute it

        if (!(command = parseCmdLines(line)))
        {
            continue;
        }

        if (strcmp(command->arguments[0], "quit") == 0)
        {
            break;
        }

        if (command->arguments[0][0] == '!')
        {
            i = -1;

            // repeat last command
            if (strcmp(command->arguments[0], "!!") == 0)
            {
                // no history
                if (newest == -1)
                {
                    continue;
                }

                i = newest;
            }
            else
            {
                // extract the index
                i = (int) strtol(command->arguments[0] + 1, &tmp, 10);

                // check if the entire string after the '!' was a number
                // (this test is mentioned in the manual for strtol)
                if (!(command->arguments[0][1] != '\0' && *tmp == '\0'))
                {
                    puts("*> not an index.");
                    i = -1;
                }
                else if (i < 0 || i >= HISTLEN || newest == -1)
                {
                    puts("*> invalid index.");
                    i = -1;
                }
                else
                {
                    i = (oldest + i) % HISTLEN;

                    if (history[i][0] == '\0')
                    {
                        puts("*> invalid index.");
                        i = -1;
                    }
                }
            }

            freeCmdLines(command);

            if (i == -1)
            {
                continue;
            }

            // copy the desired command
            strcpy(line, history[i]);

            if (!(command = parseCmdLines(line)))
            {
                continue;
            }
        }

        // add the command to the history of commands list

        if (oldest == -1)
        {
            oldest = 0;
        }

        newest = (newest + 1) % HISTLEN;

        if (history[newest][0] != '\0')
        {
            oldest = (oldest + 1) % HISTLEN;
        }

        strcpy(history[newest], line);

        if (strcmp(command->arguments[0], "history") == 0)
        {
            printHistory(history, oldest, newest);

            freeCmdLines(command);
        }
        else if (strcmp(command->arguments[0], "cd") == 0)
        {
            if (chdir(command->arguments[1]) == -1)
            {
                if (debug)
                {
                    perror("!> couldn't change directory");
                }
            }
            else
            {
                getcwd(cwd, PATH_MAX);
            }

            freeCmdLines(command);
        }
        else if (signalProc(command, debug, processes))
        {
            continue;
        }
        else if (strcmp(command->arguments[0], "procs") == 0)
        {
            printProcessList(&processes);

            freeCmdLines(command);
        }
        else if (!validPiping(command))
        {
            if (debug)
            {
                fprintf(stderr, "!> invalid piping!\n");
            }

            freeCmdLines(command);
        }
        else if (command->next)
        {
            pipe(p);

            if (!(pid1 = fork()))
            {
                // child 1

                close(STDOUT_FILENO);
                dup(p[1]);
                close(p[1]);

                runChildProcess(command, debug);
            }
            else if (pid1 > 0)
            {
                // parent

                addProcess(&processes, command, pid1);

                close(p[1]);

                if (!(pid2 = fork()))
                {
                    // child 2

                    close(STDIN_FILENO);
                    dup(p[0]);
                    close(p[0]);

                    runChildProcess(command->next, debug);
                }
                else if (pid2 > 0)
                {
                    // parent again

                    addProcess(&processes, command->next, pid2);

                    close(p[0]);

                    // ? should i always wait?
                    waitpid(pid1, NULL, 0);
                    waitpid(pid2, NULL, 0);
                }
            }
        }
        else
        {
            if ((pid1 = fork()) > 0)
            {
                addProcess(&processes, command, pid1);

                if (command->blocking)
                {
                    waitpid(pid1, NULL, 0);
                }
            }
            else if (pid1 < 0)
            {
                perror("!> fork failed");
                execError = TRUE;
            }
            else
            {
                runChildProcess(command, debug);
            }
        }
    } while (!execError);

    freeCmdLines(command);
    freeProcessList(processes);

    return execError;
}
