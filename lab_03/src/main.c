#include "apue.h"
#include <syslog.h>
#include <fcntl.h>
#include <sys/resource.h>

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>

#define LOCKFILE "/var/run/daemon.pid"
#define LOCKMODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)

/* Эту функцию может вызывать приложение, желающее стать демоном */

void daemonize(const char * cmd)
{
    int i, fd0, fd1, fd2;
    pid_t pid;
    struct rlimit rl;
    struct sigaction sa;

    /* Сбросить маску режима создания файла */
    umask(0);

    /* Получаем максимально возможный номер дескриптора файла */
    if (getrlimit(RLIMIT_NOFILE, &rl) < 0)
        err_quit("%s: невозможно получить максимальный номер дескриптора ", cmd);
    
    /* Стать лидером новой сессии, чтобы утратить управляющий терминал */
    if ((pid = fork()) < 0 )
        err_quit("%s: ошибка вызова функции fork", cmd);
    else if (pid != 0)
        exit(0);   /* Родительский процесс */

    setsid(); /* Создали новую сессию */

    /* Обеспечить невозможность обретения управляющего терминала в будущем. */
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
	
    if (sigaction(SIGHUP, &sa, NULL) < 0)
        quit("%s: невозможно игнорировать сигнал SIGHUP ", cmd);


    /* 
     * Назначить корневой каталог текущим рабочим каталогом,
     * чтобы впоследствии можно было отмонтировать файловую систему
     */
    if (chdir("/") < 0)
        err_quit("%s: невозмжно сделать текущим рабочим каталогом /");

    /* Закрыть все открытые файловые дескриторы */
    if (rl.rlim_max == RLIM_INFINITY)
        rl.rlim_max = 1024;

    for (i = 0; i < rl.rlim_max; i++)
        close(i);

    /* Присоединить файловые дескрипторы 0, 1, 2 к dev/null */
    fd0 = open("/dev/null", O_RDWR);
    fd1 = dup(0);
    fd2 = dup(0);

    /* Инициализировать файл журнала */
    openlog(cmd, LOG_CONS, LOG_DAEMON);
    if (fd0 != 0 || fd1 != 1 || fd2 != 2)
    {
        syslog(LOG_ERR, "ошибочные файлвые дескрипторы %d %d %d", fd0, fd1, fd2);
        exit(1);
    }
}


/*
 * Если функция будет вызвана из программы, которая затем приостанавливает работу
 * мы сможем проверить состояние демона с помощью команды ps -axj
 * При этом можно убедиться, что демон относится к осиротевшей группе процессов
 * и не является лидером сессии, поэтому не имеет возможности обрести 
 * управляющий терминал. (но это результат второго форка, возможно у нас не так)
 */

void main(int argc, char*argv[]) {
    char *cmd;

    if ((strrchr(argv[0], '/')) == NULL)
        cmd = argv[0];
    else
        cmd++;

    daemonize(cmd);

    for (;;) {
        syslog(LOG_INFO, "++++++++++++++++++++");
        sleep(1);
    }
    
    exit(0);
}   

