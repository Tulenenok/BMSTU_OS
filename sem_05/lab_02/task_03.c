/*
 * Потомки переходят на выполнение других программ, которые передаются системному вызову exec() в качестве параметра. 
 * Потомки должны выполнять разные программы. Предок ждет завершения своих потомков с анализом кодов завершения. 
 * На экран выводятся соответствующие сообщения.
*/

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>

#define N 2
#define TIME_SLEEP 2

int main()
{
    setbuf(stdout, NULL);
    int child[N];
    char* commands[N] = { "./p_1.out", "./p_2.out"};
    
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
            printf("Child %d \nPID: %d, PPID: %d, GROUP: %d\n", i + 1, getpid(), getppid(), getpgrp());

            int rc = execlp(commands[i], commands[i], 0);
            if (rc == -1)
            {
                perror("Can't exec");
                return EXIT_FAILURE;
            }

            exit(EXIT_SUCCESS);
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

    printf("Parent process --> FINISHED \nChildren: %d, %d \nParent: PID: %d, GROUP: %d\n\n", child[0], child[1], getpid(), getpgrp());

    return EXIT_SUCCESS;
}
