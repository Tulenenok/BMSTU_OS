/*
 * Не надо делать sockstream, sockdgram, на обоих сторонах что-то
 * unlink (не только close)
 * fd sock (подчеркиваем, что это файловый дескриптор)
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <signal.h>

#define BUF_SIZE 256
#define PORT 3425
#define SOCK_ADDR "localhost"


int sock_fd;


void sigtstp_handler(int signum)
{
	printf("\nCatch SIGTSTP\n");
    if (close(sock_fd) == -1) 
    {
        printf("close() failed\n");
        return;
    }
    exit(0);
}


int main(void)
{
	srand(time(NULL));

    struct sockaddr_in serv_addr;
    struct hostent *host;
    char message[BUF_SIZE];
	
	
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0)
    {
      printf("socket() failed\n");
      return EXIT_FAILURE;
    }
	
	signal(SIGTSTP, sigtstp_handler);
	
    host = gethostbyname(SOCK_ADDR);
    if (!host)
    {
        printf("gethostbyname() failed\n");
        return EXIT_FAILURE;
    }

    serv_addr.sin_family = AF_INET; 
	serv_addr.sin_port = htons(PORT); 
    serv_addr.sin_addr = *((struct in_addr*) host->h_addr_list[0]); 

	if (connect(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    {
		printf("connect() failed\n");
        return EXIT_FAILURE;
    }


    while(1)
    {
        memset(message, 0, BUF_SIZE);
        sprintf(message, "pid = %d\n", getpid());

        if (send(sock_fd, message, sizeof(message), 0) == -1)
        {
            printf("send() failed\n");
            return EXIT_FAILURE;
        }


		memset(message, 0, BUF_SIZE);
        int bytes = recv(sock_fd, message, BUF_SIZE, 0);

        if (bytes > 0)
        {
            message[bytes] = 0;
            printf("Client recieved from server: %s\n", message);
        }

        sleep(1 + rand() % 3);
    }

    close(sock_fd);

    return EXIT_SUCCESS;
}