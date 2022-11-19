/*
 * КРАТКАЯ ИСТОРИЧЕСКАЯ СПРАВКА
 * S_IRUSR - пользователь имеет права на чтение файла
 * S_IWUSR - пользователь имеет права на запись в файл
 * S_IRGRP - группа имеет права на чтение файла
 * S_IROTH - все остальные имеют право на чтение файла
 * 
 * IPC_CREAT - создать новый сегмент 
 */


#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <time.h>
#include <sys/stat.h>

#include "inc/buffer.h"
#include "inc/producer.h"
#include "inc/concumer.h"

int main(void)
{
	setbuf(stdout, NULL);     // Ради нормального вывода
    srand(time(NULL));        // Инициализация генератора случайных чисел

	int perms = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;                        // Права доступа (расшифровка выше)

	// Создаем новый сегмент разделяемой памяти размером buffer_s
    int shmid = shmget(IPC_PRIVATE, sizeof(buffer_s), IPC_CREAT | perms);
    if (shmid == -1) 
	{
        perror("Ошибка при создании нового разделяемого сегмента\n");
        return -1;
    }

	// Подключаем сегмент разделяемой памяти shmid к адресному пространству вызывающего процесса
    buffer_s *buffer = (buffer_s*)shmat(shmid, 0, 0);
    if (buffer == (buffer_s*)-1) 
	{
        perror("Ошибка при попытке подключить разделяемый сегмент к адресному пространству процесса\n");
        return -1;
    }

    if (init_buffer(buffer) == -1) 
	{
        perror("Ошибка при инициализации буфера\n");
        return -1;
    }

	// Получить идентификатор набора семафоров из 3 штук
    int sem_descr = semget(IPC_PRIVATE, 3, IPC_CREAT | perms);
	if (sem_descr == -1)
	{
		perror("Ошибка при создании набора семафоров\n");
		return -1;
	}
	
	// semctl - выполнить операцию над набором семафоров
	// SETVAL - Установить значение поля `semval` для семафора с номером `semnum`. Значение определяется в  `arg.

	// Для семафора с номером 0 устанавливаем значение 0
	// это семафор "БуферПолонНа" - он отслеживает количество заполненных ячеек
	if (semctl(sem_descr, 0, SETVAL, 0) == -1)
	{
		perror( "Не удалось выполнить операцию над набором семафоров\n" );
		return -1;
	}

	// Для семафора с номером 1 устанавливаем значение 1
	// этот семафор будет следить за доступом процессов к буферу (он бинарный)
	if (semctl(sem_descr, 1, SETVAL, 1) == -1)
	{
		perror( "Не удалось выполнить операцию над набором семафоров" );
		return -1;
	}

	// Для семафора с номером 2 устанавливаем значение 5
	// это семафор "НасколькоБуферПуст" - он отслеживает количество пустых элементов
    if (semctl(sem_descr, 2, SETVAL, 5) == -1)
    {
        perror( "Не удалось выполнить операцию над набором семафоров\n" );
        return -1;
    }

	// 	Создаем трех производителей, каждый из которых производит 8 товаров 
    for (int i = 0; i < 3; i++)
	{
		producer_create(buffer, i + 1, sem_descr);
	}

	// 	Создаем трех потребителей, каждый из которых потребит 8 товаров 
	for (int i = 0; i < 3; i++)
	{
		consumer_create(buffer, i + 1, sem_descr);
	}

	// Ожидаем завершения всех созданных нами процессов
    for (size_t i = 0; i < 6; i++)
    {
        int status;
        if (wait(&status) == -1) 
		{
            perror("Ошибка ожидания процессов-потомков\n");
            return -1;
        }
        if (!WIFEXITED(status)){
            printf("Один из процессов-потомков завершился с ошибкой\n");
            return 1;
        }
    }

	// 	Помечаем сегмент как удаленный 
	if (shmctl(shmid, IPC_RMID, NULL))
	{
		perror("Ошибка при попытке пометить сегмент как удаленный\n");
		return -1;
	}

	// Отстыковываем сегмент разделяемой памяти от адресного пространства вызвающего процесса. 
	if (shmdt((void*)buffer) == -1)
	{
		perror("Ошибка при попытке отключить разделяемый сегмент от адресного пространства процесса\n");
		return -1;
	}

	// Немедленно удалить из системы набор семафоров и структуры его данных
	if (semctl(sem_descr, 0, IPC_RMID, 0) == -1)
	{
		perror("Ошибка при попытке удаления набора семафоров\n");
		return -1;
	}

	return 0;
}
