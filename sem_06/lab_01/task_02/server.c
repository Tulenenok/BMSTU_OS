#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>

int sock_fd;

void del_socket(void)
{
    if (close(sock_fd) == -1)              
    {
        printf("close() failed");
        return;
    }
    if (unlink("mysocket.soc") == -1)     
    {
        printf("unlink() returned -1");
    }
}

void sigint_handler(int signum)
{
	printf("\nCatch SIGTSTP\n");
    del_socket();
    exit(0);
}

int main(void)
{
    struct sockaddr srvr_name;
    char buf[256];
    int  bytes;

    if ((sock_fd = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1)
    {
        printf("socket() failed");
        return EXIT_FAILURE;
    }

    srvr_name.sa_family = AF_UNIX;
    strcpy(srvr_name.sa_data, "mysocket.soc");
	
    if (bind(sock_fd, &srvr_name, strlen(srvr_name.sa_data) + sizeof(srvr_name.sa_family)) == -1)
    {
        printf("bind() failed\n");
        del_socket();
        return EXIT_FAILURE;
    }

    signal(SIGTSTP, sigint_handler);
	
    printf("Waiting for messages.\nPress Ctrl + Z to stop...\n");

    while (1)
    {
        bytes = recvfrom(sock_fd, buf, sizeof(buf),  0, NULL, NULL);
        if (bytes < 0)
        {
            del_socket();
            printf("recvfrom() failed");
            return EXIT_FAILURE;
        }
        buf[bytes] = 0;
        printf("Server recieved: %s\n", buf);
    }

    del_socket();
    return EXIT_SUCCESS;
}