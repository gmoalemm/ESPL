#include <stdio.h>
#include <unistd.h> // for pipe, fork
#include <string.h>
#include <sys/wait.h>

#define MSG_MAX 1024

int main(void)
{
    // pipefd[0] refers to the read end of the pipe.
    // pipefd[1] refers to the write end of the pipe. (~man)
    int pipefd[2];

    int pid, stat;

    char in[MSG_MAX] = {0}, out[MSG_MAX] = {0};

    pipe(pipefd);

    pid = fork();

    if (pid > 0)
    {
        waitpid(pid, &stat, 0);
        read(pipefd[0], out, MSG_MAX);
        printf("parent got %s", out);

        close(pipefd[0]);
        close(pipefd[1]);
    }
    else if (pid < 0)
    {
        perror("fork failed");
        return 1;
    }
    else
    {
        printf("child says ");
        fgets(in, MSG_MAX, stdin);
        write(pipefd[1], in, strlen(in));
    }

    return 0;
}
