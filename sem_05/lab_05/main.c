#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <windows.h>

HANDLE can_write;
HANDLE can_read;

int writers_id[3];
int readers_id[5];

HANDLE writers_threads[3];
HANDLE readers_threads[5];

int shared_variable = 0;

long count_active_readers = 0;

////-------------------------------------------------------------------////
int init_events();
int init_threads();

DWORD WINAPI init_writer(CONST LPVOID param);
DWORD WINAPI init_reader(CONST LPVOID param);

void start_write();
void stop_write();

void start_read();
void stop_read();

void wait_threads();
void close_handles();
////-------------------------------------------------------------------////


int init_events()
{
    // Атрибут защиты по умолчанию, ручной сброс, состояние сигнальное, имя нет
    can_write = CreateEvent(NULL, TRUE, TRUE, NULL);
    if(can_write == NULL)
    {
        perror("Error with CreateEvent (can_write)");
        return -1;
    }

    // Атрибут защиты по умолчанию, автоматический сброс, состояние сигнальное, имя нет
    can_read = CreateEvent(NULL, FALSE, TRUE, NULL);
    if(can_read == NULL)
    {
        perror("Error with CreateEvent (can_read)");
        return -1;
    }

    return 0;
}

int init_threads()
{
    for(int i = 0; i < 3; i++)
    {
        writers_id[i] = i;
        writers_threads[i] = CreateThread(
                NULL,       // дескриптор не может быть унаследован
                0,              // начальный размер стека в байтах
                &init_writer,                  //  начальный адрес потока == какую функцию нужно выполнить
                writers_id + i, // передаем потоку указатель
                0,           // поток запуститься сразу после создания
                NULL             // не возвращать идентификатор
        );

        if (writers_threads[i] == NULL)
        {
            perror("Error with CreateThread (writer)");
            return -1;
        }
    }

    for (int i = 0; i < 5; i++)
    {
        readers_id[i] = i;
        readers_threads[i] = CreateThread(
                NULL,
                0,
                &init_reader,
                readers_id + i,
                0,
                NULL
        );

        if(readers_threads[i] == NULL)
        {
            perror("Error with CreateThread (reader)");
            return -1;
        }
    }
    return 0;
}


DWORD WINAPI init_writer(CONST LPVOID param)
{
    int id = *(int *)param;
    int sleep_time;

    for(int i = 0; i < 8; i++)
    {
        sleep_time = 100 + rand() * time(NULL) % 400;
        Sleep(sleep_time);

        start_write();

        ++shared_variable;
        printf("Writer %d  ----> set value: %d          !!!\n", id, shared_variable);

        stop_write();
    }
}

void start_write()
{
    WaitForSingleObject(can_write, INFINITE);
}

void stop_write()
{
    SetEvent(can_read);
}


DWORD WINAPI init_reader(CONST LPVOID param)
{
    int id = *(int *)param;
    int sleep_time;

    for(int i = 0; i < 8; i++)
    {
        sleep_time = 100 + rand() * time(NULL) % 200;
        Sleep(sleep_time);

        start_read();
        printf("Reader %d  ----> get value: %d\n", id, shared_variable);
        stop_read();
    }
}

void start_read()
{
    WaitForSingleObject(can_read, INFINITE);
    WaitForSingleObject(can_write, INFINITE);
    ResetEvent(can_write);

    InterlockedIncrement(&count_active_readers);
}

void stop_read()
{
    InterlockedDecrement(&count_active_readers);

    SetEvent(can_read);

    if(count_active_readers == 0)
        SetEvent(can_write);
}

void wait_threads()
{
    WaitForMultipleObjects(3, writers_threads, TRUE, INFINITE);
    WaitForMultipleObjects(5, writers_threads, TRUE, INFINITE);
}

void close_handles()
{
    for(int i = 0; i < 3; i++)
        CloseHandle(writers_threads[i]);

    for(int i = 0; i < 5; i++)
        CloseHandle(readers_threads[i]);

    CloseHandle(can_write);
    CloseHandle(can_read);
}

int main() {
    setbuf(stdout, NULL);
    srand(time(NULL));

    int rc = init_events();
    if (rc)
        return -1;

    rc = init_threads();
    if (rc)
        return -1;

    wait_threads();
    close_handles();

    return 0;
}
