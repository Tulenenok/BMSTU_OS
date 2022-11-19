/*
 *                  ПОТРЕБИТЕЛЬ
 *
 * КРАТКАЯ ИСТОРИЧЕСКАЯ СПРАВКА
   struct sembuf 
   {
    unsigned short sem_num;    // Кол-во семафоров в наборе 
    short sem_op;              // Операция (<0, 0 или >0)
    short sem_flg;             // IPC_NOWAIT, SEM_UNDO
   }
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include "../inc/concumer.h"

struct sembuf consume_begin[] =
{
	{0, -1, 0},             // Ожидает, что будет заполнена хотя бы одна ячейка буфера (то есть будет, что потребить)
	{1, -1, 0}              // Ожидает, пока другой производитель или потребитель выйдет из критической зоны
};

struct sembuf consume_end[] =
{
	{1, 1, 0},              // Освобождает критическую зону
    {2, 1, 0}               // Увеличить кол-во свободных ячеек 
};

void consumer_work(buffer_s* const buffer, const int sem_id, const int con_id)
{
	int sleep_time = rand() % 2 + 1;      // Создаем случайные задержки
	sleep(sleep_time);

	int rv = semop(sem_id, consume_begin, 2);               // Получаем доступ к критической зоне
	if (rv == -1)
	{
		perror("Потребитель не может изменить значение семафора\n");
		exit(-1);
	}

	char ch;	
	// Началась критическая зона
    if (read_buffer(buffer, &ch) == -1) 
	{
        perror("Потребитель не смог считать буфер\n");
        exit(-1);
    }
    printf(" \e[1;32mConsumer #%d \tread:  \t%c \tsleep: %d\e[0m\n", con_id, ch, sleep_time);
    // Закончилась критическая зона

	rv = semop(sem_id, consume_end, 2);
	if (rv == -1)
	{
		perror("Потребитель не может изменить значение семафора\n");
		exit(-1);
	}
}

void consumer_create(buffer_s *const buffer, const int con_id, const int sem_id)
{
	pid_t childpid = fork();
	if (childpid == -1)
	{
		perror("Ошибка при порождении процесса потребителя.");
		exit(-1);
	}
	else if (childpid == 0)
	{
		// В процессе потомке каждый потребитель потребляет 8 товаров.
		for (int i = 0; i < 8; i++)
			consumer_work(buffer, sem_id, con_id);

		exit(0);
	}
}