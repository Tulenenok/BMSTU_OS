/*
 * Процессы-сироты. В программе создаются не менее двух потомков. В потомках вызывается sleep(). 
 * Чтобы предок гарантированно завершился раньше своих потомков. Продемонстрировать с помощью соответствующего вывода информацию об 
 * идентификаторах процессов и их группе. Продемонстрировать «усыновление». 
 * Для этого надо в потомках вывести идентификаторы: собственный, предка, группы до блокировки и после блокировки.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define N 2
#define TIME_SLEEP 2

int main()
{
    int child[N];

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
            printf("Child %d --> BEFORE SLEEP \nPID: %d, PPID: %d, GROUP: %d \n\n", i + 1, getpid(), getppid(), getpgrp());
            sleep(TIME_SLEEP);
            printf("Child %d --> AFTER SLEEP \nPID: %d, PPID: %d, GROUP: %d\n\n", i + 1, getpid(), getppid(), getpgrp());
            exit(EXIT_SUCCESS);
        }
        else
        {
            child[i] = child_pid;
        }
        
    }

    printf("Parent process --> FINISHED \nChildren: %d, %d \nParent: PID: %d, GROUP: %d\n\n", child[0], child[1], getpid(), getpgrp());

    return EXIT_SUCCESS;
}
