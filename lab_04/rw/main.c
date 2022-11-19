#include <stdio.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/shm.h>

#include "inc/writer.h"
#include "inc/reader.h"


int *counter = NULL;

int main(void)
{
    int status;

	int perms = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    int shmid = shmget(IPC_PRIVATE, sizeof(int), perms);
	if (shmid == -1)
	{
		perror("Ошибка при создании разделяемого сегмента\n");
		return -1;
	}

    counter = shmat(shmid, NULL, 0);
	if (*(char *)counter == -1)
	{
		perror("Ошибка при возврата указателя на сегмент\n");
		return -1;
	}
    
    *counter = 0;  
    
    int sem_descr = semget(IPC_PRIVATE, 2, IPC_CREAT | perms);
	if (sem_descr == -1)
	{
		perror("Ошибка при создании набора семафоров\n");
		return -1;
	}

    if (semctl(sem_descr, 0, SETVAL, 0) == -1)           // Семафор для писателей
    {
        perror("Ошибка доступа к семафору" );
        return -1;
    }

    if (semctl(sem_descr, 1, SETVAL, 0) == -1)           // Семафор для читателей
    {
        perror( "Ошибка доступа к семафору" );
        return -1;
    }


    for (int i = 0; i < 5; i++)
		reader_create(sem_descr, i + 1);

	for (int i = 0; i < 3; i++)
		writer_create(sem_descr, i + 1);


    for (size_t i = 0; i < 8; i++)
    {
        int status;
        if (wait(&status) == -1)
        {
            perror("Ошибка ожидания процессов-потомков");
            return -1;
        }
        if (!WIFEXITED(status)) {
            printf("Один из процессов-потомков завершился с ошибкой\n");
            return 1;
        }
    }

	if (shmctl(shmid, IPC_RMID, NULL))
	{
		perror("Ошибка при попытке пометить сегмент как удаленный\n");
		return -1;
	}

	if (shmdt(counter) == -1)
    {
		perror("Ошибка при попытке отключить разделяемый сегмент от адресного пространства процесса.");
        return -1;
    }
	
	if (semctl(sem_descr, 0, IPC_RMID, 0) == -1)
	{
		perror("Ошибка при попытке удаления набора семафоров");
		return -1;
	}

    return 0;    
}
