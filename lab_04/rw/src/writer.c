#include <sys/sem.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "../inc/writer.h"

extern int *counter;

struct sembuf start_write[] =
{
    {1, 0, 0},    // проверяет, есть ли активные читатели 
    {0, 0, 0},    // проверка, есть ли активный писатели
    {0, 1, 0}     // увеличили кол-во активных писателей
};

struct sembuf stop_write[] = {
    {0, -1, 0}    // уменьшили кол-во активных писателей
};


void writer_work(const int sem_id, const int writer_id)
{
	int sleep_time = rand() % 2 + 1;
	sleep(sleep_time);

	int rv = semop(sem_id, start_write, 3);     // Начать писать
	if (rv == -1)
	{
		perror("Писатель не может изменить значение семафора\n");
		exit(-1);
	}

	(*counter)++;
	printf("\033[93mWriter #%d \twrite: \t%d \tsleep: %d\e[0m\n",
				writer_id, *counter, sleep_time);


	rv = semop(sem_id, stop_write, 1);          // Закончить писать
	if (rv == -1)
	{
		perror("Писатель не может изменить значение семафора.\n");
		exit(-1);
	}
}

void writer_create(const int sem_id, const int writer_id)
{
	pid_t childpid = fork();
	if (childpid == -1)
	{
		perror("Ошибка при порождении писателя\n");
		exit(-1);
	}
	else if (childpid == 0)
	{
		while (*counter < 20)                
			writer_work(sem_id, writer_id);

		exit(0);
	}
}