#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>


void close_socket(int sock)
{
	close(sock);
	unlink("socket.soc");
}

int main(int argc, char *argv[])
{
	setbuf(stdout, 0);

	struct sockaddr server_name;
	char msg[100];
	int sock;

	sock = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (sock == -1)
	{
		perror("socket failed");
		exit(1);
	}

	server_name.sa_family = AF_UNIX;
	strcpy(server_name.sa_data, "socket.soc");

	if (bind(sock, &server_name, strlen(server_name.sa_data) + sizeof(server_name.sa_family) + 1) == -1)
	{
		perror("bind failed");
		exit(1);
	}

	printf("Waiting for messages.\nPress Ctrl + Z to stop...\n");

	while (1)
	{
		int bytes = recvfrom(sock, msg, sizeof(msg), 0, NULL, NULL);

		if (bytes < 0)
		{
			perror("recvfrom failed");
			close_socket(sock);
			exit(1);
		}
		msg[bytes] = 0;
		printf("received message: %s\n", msg);

		struct sockaddr client_name;
		client_name.sa_family = AF_UNIX;
		strcpy(client_name.sa_data, msg);

		char new_mes[256] = {0};
		sprintf(new_mes, "%s %d", msg, getpid());

		if (sendto(sock, new_mes, strlen(new_mes) + 1, 0, &client_name, strlen(client_name.sa_data) + sizeof(client_name.sa_family) + 1) == -1)
		{
			perror("sendto failed");
			close_socket(sock);
			exit(1);
		}
	}

	close_socket(sock);

	return 0;
}

