#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>


int main(void)
{	
	int sock_fd = socket(AF_UNIX, SOCK_DGRAM, 0);

    if (sock_fd == -1)
    {
        printf("socket() failed");
        return EXIT_FAILURE;
    }
	
    struct sockaddr srvr_name;
    srvr_name.sa_family = AF_UNIX;
    strcpy(srvr_name.sa_data, "mysocket.soc");
	
	char buf[256];
    sprintf(buf, "pid %d", getpid());
	
	while (1)
	{
		if (sendto(sock_fd, buf, strlen(buf), 0, &srvr_name, strlen(srvr_name.sa_data) + sizeof(srvr_name.sa_family)) == -1)
		{
			printf("sendto() failed");
			close(sock_fd);
			return EXIT_FAILURE;
		}
		
		printf("Client sent: %s\n", buf);
		sleep(3);
	}
	
    close(sock_fd);
    return EXIT_SUCCESS;
}
