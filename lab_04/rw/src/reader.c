#include <sys/sem.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "../inc/reader.h"

extern int *counter;

struct sembuf start_read[] =
{
    {0, 0, 0},     // проверка, есть ли активный писатель (нужно, чтобы их не было)
    {1, 1, 0}      // есть активный читатель  
	// Здесь нет еще одной структуры как у писателя, так как читать одновременно можно
};

struct sembuf stop_read[] =
{
    {1, -1, 0}    // количество активных читателей уменьшилось на 1
};

void reader_run(const int sem_id, const int reader_id)
{
	int sleep_time = rand() % 1 + 1;
	sleep(sleep_time);

	int rv = semop(sem_id, start_read, 2); 
	if (rv == -1)
	{
		perror("Читатель не может изменить значение семафора\n");
		exit(-1);
	}

	printf("\033[92mReader #%d \tread: \t%d \tsleep: %d\e[0m \n",
				reader_id, *counter, sleep_time);
	
	rv = semop(sem_id, stop_read, 1);
	if (rv == -1)
	{
		perror("Читатель не может изменить значение семафора\n");
		exit(-1);
	}
}

void reader_create(const int sem_id, const int reader_id)
{
	pid_t childpid = fork();
	if (childpid == -1)
	{
		perror("Ошибка при порождении читателя\n");
		exit(-1);
	}
	else if (childpid == 0)
	{
		while (*counter < 20)
			reader_run(sem_id, reader_id);

		exit(0);
	}
}