#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>

int main(int argc, char **argv)
{
    int child[3];
    int sockets[2];

    char buf[100];
    char *mess[3] = {"aaaa", "bbb", "ccccc"};

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) == -1)
    {
        perror("error with socketpair");
        exit(1);
    }

    for (int i = 0; i < 3; i++)
    {
        int pid = fork();
        if (pid == -1)
        {
            perror("error with fork");
            exit(1);
        }
        else if (pid == 0)
        {
            close(sockets[1]);

            write(sockets[0], mess[i], strlen(mess[i]));
            printf("child sent %s\n", mess[i]);

            close(sockets[0]);
            return 0;
        }
        else
        {
            child[i] = pid;
        }
    }

    for (int i = 0; i < 3; i++)
    {
        int status;
        int stat_val = 0;

        if (waitpid(child[i], &status, WCONTINUED) == -1)
        {
            perror("error with waitpid");
            exit(1);
        }
    }

    sleep(3);
    close(sockets[0]);

    read(sockets[1], buf, sizeof(buf));
    printf("parent read %s\n", buf);

    close(sockets[1]);
    return 0;
}