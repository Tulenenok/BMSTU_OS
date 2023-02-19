/*
 * Предок и потомки аналогично п.4 обмениваются сообщениями через неименованный программный канал. 
 * В программу включается собственный обработчик сигнала. С помощью сигнала меняется ход выполнения программы. 
 * При получении сигнала потомки записывают сообщения в канал, если сигнал не поступает, то не записывают. 
 * Предок ждет завершения своих потомков и анализирует коды их завершений. Вывод соответствующих сообщений на экран.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdbool.h> 
#include <signal.h>

#define N 2
#define TIME_SLEEP 2
#define LEN 64

_Bool flag = false;


void catch_sig(int sig_num)
{
	flag = true;
	printf("\ncatch_sig: %d\n", sig_num);
}

int main()
{
    int child[N];
    int fd[N];
    char text[LEN] = { 0 };
    char *mes[N] = {"aaaaaaaaa\n", "bbb\n"};

    if (pipe(fd) == -1)
    {
        perror("Can't pipe!");
        return EXIT_FAILURE;
    }

    printf("Parent process --> START \nPID: %d, GROUP: %d\n", getpid(), getpgrp());
    signal(SIGINT, catch_sig);
    sleep(2);

    for (int i = 0; i < N; i++)
    {
        int child_pid = fork();

        if(child_pid == -1)
        {
            perror("Can\'t fork()\n");
            return EXIT_FAILURE;
        }
        else if (!child_pid)
        {
            if (flag)
            {
                close(fd[0]);
                write(fd[1], mes[i], strlen(mes[i]));
                printf("Message %d sent to parent: %s", i + 1, mes[i]);
            }

            return EXIT_SUCCESS;
        }        
        else
        {
            child[i] = child_pid;
        }
    }

	for (int i = 0; i < N; i++)
	{
		int status;
		int statval = 0;

		pid_t child_pid = wait(&status);

		printf("Child process %d --> FINISHED\nStatus: %d\n", child_pid, status);

		if (WIFEXITED(statval))
		{
			printf("Child process %d --> FINISHED\nCode: %d\n", i + 1, WEXITSTATUS(statval));
		}
		else if (WIFSIGNALED(statval))
		{
			printf("Child process %d --> FINISHED FROM SIGNAL\nCode: %d\n", i + 1, WTERMSIG(statval));
		}
		else if (WIFSTOPPED(statval))
		{
			printf("Child process %d --> FINISHED STOPPED. Number signal: %d\n", i + 1, WSTOPSIG(statval));
		}
	}

    
    printf("\nMessage receive:\n");
    close(fd[1]);
    read(fd[0], text, LEN);
    printf("%s\n", text);
    

    printf("Parent process --> FINISHED\nChildren: %d, %d! \nParent: PID: %d, GROUP: %d\n", child[0], child[1], getpid(), getpgrp());

    return EXIT_SUCCESS;
}
