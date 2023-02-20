/*
 * Задание:
 *   - Организовать взаимодействие параллельных процессов на отдельном компьютере
 *   - Организовать взаимодействие параллельных процессов в сети
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#define SOCK_NAME "mysocket.soc"
#define BUF_SIZE 256


int main(void)
{	
    int sock_fd = socket(AF_UNIX, SOCK_DGRAM, 0);

    if (sock_fd == -1)
    {
        printf("error with socket");
        return EXIT_FAILURE;
    }

    struct sockaddr srvr_name;

    srvr_name.sa_family = AF_UNIX;
    strcpy(srvr_name.sa_data, SOCK_NAME);

    char buf[BUF_SIZE];
    sprintf(buf, "pid %d", getpid());

    while (1)
	{
		if (sendto(sock_fd, buf, strlen(buf), 0, &srvr_name, strlen(srvr_name.sa_data) + sizeof(srvr_name.sa_family)) == -1)
		{
			printf("error with sendto");
			close(sock_fd); 

			return EXIT_FAILURE;
		}
		
		printf("Client sent: %s\n", buf);
		sleep(3);
	}
	
    close(sock_fd);
    return EXIT_SUCCESS;
}