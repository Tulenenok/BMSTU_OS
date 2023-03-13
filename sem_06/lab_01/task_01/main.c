#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#define CHILDS_QTY 1

void child_work(int index, int* sockets)
{
    char buf[1024];
    char msg[100];

    sprintf(msg, "%d", getpid());
    close(sockets[1]);

    printf("CHILD[%d] sent ---------------> %s\n", index, msg);

    write(sockets[0], msg, sizeof(msg));
    sleep(1);
    read(sockets[0], buf, sizeof(buf));

    printf("CHILD[%d] recieved <----------- %s\n", index, buf);

    close(sockets[0]);
}


int main()
{
    int sockets[2];

    char buf[10];
    pid_t childpid;
    char p_msg[10];
    
    sprintf(p_msg, "%d", getpid());

    if(socketpair(AF_UNIX, SOCK_DGRAM, 0, sockets) == -1)
    {
        perror("socketpair() failed");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < CHILDS_QTY; ++i)
    {
        childpid = fork();
        if (childpid == -1)
        {
            perror("fork error");
            return EXIT_FAILURE;
        }
        else if (childpid == 0)
        {
            child_work(i, sockets);
            return EXIT_SUCCESS;
        }
        else
        {
            close(sockets[0]);

            read(sockets[1], buf, sizeof(buf));
            printf("MSG received by parent <------ %s \n", buf);

            sleep(3);

            write(sockets[1], p_msg, sizeof(p_msg));
            printf("Parent sent to child[%d] -----> %s \n", i, p_msg);

            close(sockets[1]);
        }
    }

    return EXIT_SUCCESS;
}
