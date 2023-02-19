#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>  
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/shm.h>

#include "../inc/producer.h"

struct sembuf producer_begin[] =
{
    {2, -1, 0},    // Уменьшить количество свободных ячеек 
	{1, -1, 0}     // Ожидает, пока другой производитель или потребитель выйдет из критической зоны.
};
struct sembuf producer_end[] =
{
	{1, 1, 0},    // Освобождает критическую зону.
	{0, 1, 0}     // Увеличивает кол-во заполненных ячеек.
};

void producer_work(buffer_s *const buffer, const int sem_id, const int pro_id)
{
	int sleep_time = rand() % 1 + 1;
	sleep(sleep_time);

	int rv = semop(sem_id, producer_begin, 2);
	if (rv == -1)
	{
		perror("Производитель не может изменить значение семафора.\n");
		exit(-1);
	}

	// Началась критическая зона
    if (write_buffer(buffer) == -1)
	{
        perror("Производитель не может записать символ в буфер.\n");
        exit(-1);
    }
	printf("\033[94mProducer #%d \twrite: \t%c \tsleep: %d\e[0m \n", pro_id, *(buffer->pptr - 1), sleep_time);
	// Закончилась критическая зона

	rv = semop(sem_id, producer_end, 2);
	if (rv == -1)
	{
		perror("Производитель не может изменить значение семафора.\n");
		exit(-1);
	}
}

void producer_create(buffer_s *const buffer, const int pro_id, const int sem_id)
{
	pid_t childpid = fork();
	if (childpid == -1)
	{
		perror("Ошибка при порождении процесса производителя.");
		exit(-1);
	}
	else if (childpid == 0)
	{
		// В процессе потомке каждый производитель производит 8 товаров.
		for (int i = 0; i < 8; i++)
			producer_work(buffer, sem_id, pro_id);

		exit(0);
	}
}