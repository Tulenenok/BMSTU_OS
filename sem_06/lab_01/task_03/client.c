#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>


int main(int argc, char *argv[])
{
	int sock_fd;
	char msg[256];

	sock_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (sock_fd == -1)
	{
		perror("create socket failed");
		exit(1);
	}

    sprintf(msg, "%d", getpid());

    struct sockaddr server_name;
	server_name.sa_family = AF_UNIX;
	strcpy(server_name.sa_data, "socket.soc");

    struct sockaddr client_name;
    client_name.sa_family = AF_UNIX;
	strcpy(client_name.sa_data, msg);

    if (bind(sock_fd, &client_name, strlen(client_name.sa_data) + sizeof(client_name.sa_family) + 1) == -1)
	{
		perror("bind failed");
		exit(1);
	}

    if (sendto(sock_fd, msg, strlen(msg) + 1, 0, &server_name, strlen(server_name.sa_data) + sizeof(server_name.sa_family) + 1) < 0)
	{
		perror("sendto failed");
		exit(1);
	}
    printf("client sent: %s\n", msg);

    int bytes = recv(sock_fd, msg, 256, 0);

    if (bytes > 0)
    {
        msg[bytes] = 0;
        printf("client recieved: %s\n", msg);
    }

	close(sock_fd);
	return 0;
}


























