#include <unistd.h>     // pipe, fork
#include <stdio.h>
#include <sys/wait.h>   // waitpid

int main(void)
{
    int p[2];   // [r, w]
    pid_t pid1, pid2;

    char *ls[] = {"ls", "-l", NULL};
    char *tail[] = {"tail", "-n 2", NULL};

    pipe(p);

    fprintf(stderr, "!> parent: forking...\n");
    pid1 = fork();

    if (!pid1)
    {
        // child1

        fprintf(stderr, "!> child1: redirecting stdout to the write end...\n");
        close(STDOUT_FILENO);
        dup(p[1]);
        close(p[1]);

        fprintf(stderr, "!> child1: executing command: %s...\n", "ls -l");
        execvp(ls[0], ls);
    }
    else if (pid1 > 0)
    {
        // parent

        fprintf(stderr, "!> parent: created process #%d\n", pid1);

        // * keeping the write end open means we never get to the EOF, thus,
        // * the tail command will never end. (Also, though Ctrl+D means EOF,
        // * it is the terminal's interpretation, it works if we read stdin).
        fprintf(stderr, "!> parent: closing the write end...\n");
        close(p[1]);

        fprintf(stderr, "!> parent: forking...\n");
        pid2 = fork();

        if (!pid2)
        {
            // child2

            fprintf(stderr, "!> child2: redirecting stdin to the read end...\n");
            close(STDIN_FILENO);
            dup(p[0]);
            close(p[0]);

            fprintf(stderr, "!> child2: executing command: %s...\n", "tail -n 2");
            execvp(tail[0], tail);
        }
        else if (pid2 > 0)
        {
            // still parent

            fprintf(stderr, "!> parent: created process #%d\n", pid2);

            // * this doesn't actually affect this program - as long as the
            // * write end is closed, the read end sees the EOF and outputs
            fprintf(stderr, "!> parent: closing the read end...\n");
            close(p[0]);

            // * without this step, even when the closing of the write end,
            // * the parent will exit, and with it - the pipe's file descriptors.
            fprintf(stderr, "!> parent: waiting for child to terminate...\n");
            waitpid(pid1, NULL, 0);
            waitpid(pid2, NULL, 0);

            fprintf(stderr, "!> parent: exiting...\n");
        }
    }

    return 0;
}
