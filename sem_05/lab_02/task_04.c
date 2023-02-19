/*
 * Предок и потомки обмениваются сообщениями через неименованный программный канал. 
 * Причем оба потомка пишут свои сообщения в один программный канал, а предок их считывает из канала. 
 * Потомки должны посылать предку разные сообщения по содержанию и размеру. 
 * Предок считывает сообщения от потомков и выводит их на экран. Предок ждет завершения своих потомков и анализирует код их завершения. 
 * Вывод соответствующих сообщений на экран.
 * 
 * Системный вызов pipe() создает неименованный программный канал. 
 * Неименованные программные каналы могут использоваться для обмена сообщениями между процессами родственниками
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#define N 2
#define TIME_SLEEP 2
#define LEN 64


int main()
{
    int child[N];
    int fd[N];
    char text[LEN] = { 0 };
    char *messages[N] = {"first message\n", "aaa\n"};

    if (pipe(fd) == -1)
    {
        perror("Can't pipe!");
        return EXIT_FAILURE;
    }

    printf("Parent process --> START \nPID: %d, GROUP: %d\n\n", getpid(), getpgrp());

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
            printf("Child %d --> START\nPID: %d, PPID: %d, GROUP: %d\n\n", i + 1, getpid(), getppid(), getpgrp());
            close(fd[0]);
            write(fd[1], messages[i], strlen(messages[i]));
            printf("Message %d sent to parent: %s\n", i + 1, messages[i]);

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

		printf("Child process %d --> FINISHED \nStatus: %d\n\n", child_pid, status);

		if (WIFEXITED(statval))
		{
            /* Ненулевой, если дочерний процесс завершен нормально */
			printf("Child process %d --> FINISHED \nCode: %d\n\n", i + 1, WEXITSTATUS(statval));
		}
		else if (WIFSIGNALED(statval))
		{
            /* Ненулевой, если дочерний процесс завершается не перехватываемым сигналом */
			printf("Child process %d --> FINISHED from signal with code: %d\n\n", i + 1, WTERMSIG(statval));
		}
		else if (WIFSTOPPED(statval))
		{
            /* Ненулевой, если дочерний процесс остановился */
			printf("Child process %d --> FINISHED stopped \nNumber signal: %d\n\n", i + 1, WSTOPSIG(statval));
		}
	}

    printf("\nMessage receive:\n");
    close(fd[1]);
    read(fd[0], text, LEN);
    printf("%s\n", text);

    printf("Parent process --> FINISHED \nChildren: %d, %d \nParent: PID: %d, GROUP: %d\n\n", child[0], child[1], getpid(), getpgrp());

    return EXIT_SUCCESS;
}
