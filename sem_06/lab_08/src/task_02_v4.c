#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

void *thread_func(void *args)
{
  	int fd2 = open("q.txt",O_RDWR);

    for (char c = 'b'; c <= 'z'; c += 2)
    {
        write(fd2, &c, 1);
    }
    
    close(fd2);
}

int main(void)
{
 	int fd1 = open("q.txt",O_RDWR);
    
    pthread_t thread;
    if (pthread_create(&thread, NULL, thread_func, 0) != 0)
	{
		perror("error in pthread_create\n");
		return -1;
	}

    for (char c = 'a'; c <= 'z'; c += 2)
    {
        write(fd1, &c, 1);
    }

    close(fd1);

    return 0;
}
