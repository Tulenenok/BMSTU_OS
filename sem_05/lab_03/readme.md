# Лабораторная работа №2

## Инструкция по запуску

~~Сначала читаем краткую историческую справку ниже~~

1. Запускаем первый терминал

   ```bash
   make
   sudo ./main.exe
   ps -ajx | grep "./main"
   ```
2. Запускаем второй терминал

   ```bash
   cd /var/log
   sudo cat syslog
   ```
3. Наслаждаемся
   ![image](https://user-images.githubusercontent.com/78147880/198903358-48121956-cde4-4c1d-9d1b-b60e57b3171d.png)

4. Как убить процесс (или передать ему сигнал)
   ```bash
   ps -ajx | grep "./main"   # смотри PID (вторая колонка) процесса
   sudo kill -SIGHUB 31591   # sudo kill -<сигнал> <PID>
   sudo kill 31591           # если нужно остановить процесс (SIGTERM будет по умолчанию)
   ```
   ![image](https://user-images.githubusercontent.com/78147880/198907005-15d2d2c7-d17c-4552-be84-000bda617281.png)


-----

# Вопросы

* `S` - прерываемый процесс (режим прерываемого сна?)
* `s` - лидер сессии 
* `l` - многопоточный процесс
* `Зачем отнимать управляющий терминал?` - чтобы пользователь не мог взаимодействовать с демоном
* `Зачем закрывать файловые дескрипторы?` - они нам не нужны, поэтому закрываем
* `Зачем мы подключили файловые дескрипторы 0, 1, 2?` - чтобы не было ошибок, если демон будет использовать функции работающие с stdin, stdout, stderr
* `Какой сон можно прервать?`- (находитс в состоянии) ожидания завершения события.
* `Где процесс становится демоном?` - показать на вызов функции daemonize

-----

# Процессы-демоны

## Введение

Демоны - это долгоживущие процессы. Зачастую они запускаются во время загрузки системы и завершают работу вместе с ней. Так как они не имеют управляющего терминала, говорят, что они работают в фоновом режиме

## Характеристики демонов

Выведем информацию о процессах в системе:

```bash
ps -axj

# -a -- вывод процессов, которыми владеют другие пользователи
# -x -- вывод процессов, не имеющих управляющего терминала
# -j -- вывод доп. сведений, имеющих отношение к заданиям (
# идентификатор сессии, идентификатор группы процессов, управляющего терминала
# идентификатора группы процессов терминала )
```

Получим примерно такую картину:

|           PPID            |     PID     |        PGID         |    SID    |      TTY      |             TPGID             |       UID       |     COMAND     |
| :-----------------------: | :---------: | :-----------------: | :-------: | :-----------: | :---------------------------: | :-------------: | :------------: |
|             0             |      1      |          0          |     0     |       ?       |              -1               |        0        |      Init      |
|             1             |      2      |          1          |     1     |       ?       |              -1               |        0        |   [keventd]    |
|             1             |      3      |          1          |     1     |       ?       |              -1               |        0        |    [kapmd]     |
|            ...            |     ...     |         ...         |    ...    |      ...      |              ...              |       ...       |      ...       |
| ID родительского процесса | ID процесса | ID группы процессов | ID сессии | Имя терминала | ID группы процессов терминала | ID пользователя | Строка команды |

*ID - Идентификатор*

```
Здесь идентификатор сессии - идентификатор процесса лидера сессии. 
Для систем, основанных на BSD, будет выведен адрес структуры session, соотв. группе
процессов, которой принадлежит данных процесс.
```

Список процессов может разниться от одной ОС к другой. Важны здесь следующие моменты:

1. `PPID = 0` -> процесс был запущен ядром в процессе загрузки системы.(Исключение - процесс init, это команда уровня пользователя, которая запускается ядром во время загрузки.)

   ```
   Процессы ядра - особые процессы, существующие все время, пока работает система. 
   Обладают привилегиями суперпользователя и не имеют управляющего терминала и строки команды.
   ```

2. `PID = 1` -> обычно это процесс init. Это системный демон, который отвечает за запуск различных системных служб на различных уровнях загрузки. 

3. `UID = 0` -> большинство демонов управляют привилегиями суперпользователя.

4. `TTY = ? (и TPGID = -1)` -> демоны не имеют управляющего терминала

   ```
   Демоны ядра запускаются без управляющего терминала.
   Отсутствие упр. терминала у демонов пользовательского уровня - вероятно, результат вызова функции setsid.
   
   Все демоны пользовательского уровня являются лидерами групп и лидерами сессий. 
   (они являются единственными процессами в своих группах процессов и сессиях)
   ```

5. Родителем большинства является `init`. 

*Краткую справку по некоторым демонам можно найти в приложении*

## Правила программирования демонов

1. **Нужно вызвать функцию `umask`, чтобы сбросит маску режима создания файлов в значение 0.**

   Маска режима создания файлов, наследуемая от запускающего процесса, может маскировать некоторые биты прав доступа. (если вдруг демн будет создавать файлы, то может потребоваться установка определенных битов прав доступа).

2. **Вызвать функцию `fork` и завершить родительский процесс.**

   Зачем?

   1. Если демон был запущен, как обычная команда оболочки, то завершив родительский процесс, мы заставим командную оболочку думать, что команда была выполнена.
   2. Дочерний процесс унаследует идентификатор группы процессов у родителя, но получит своей PID => дочерний процесс не будет являтся лидером группы, а это необходимое условие для вызова `setsid`

3. **Создать новую сессию, обратившись к функции `setsid`**

   При этом процесс становится лидером новой сессии, лидером новой группы и лишается управляющего терминала

4. **Сделать корневой каталог текущим рабочим каталогом**

   Текущий рабочий каталог может находится на смонтированной файловой системе. Если там будет демон, то ее невозможно будет отмонтировать.

5. **Закрыть все ненужные файловые дескрипторы.**

   Это предотвращает удержание в открытом состоянии некоторых дескрипторов. 

   C помощью функции `getlimit` можно определить максимально возможный номер дескриптора и закрыть все дескрипторы вплоть до этого номера.

6. **Некоторые демоны открывают файловые дескрипторы с номерами 0, 1, 2 на устройстве /dev/null**

   Чтобы попытки стандартного ввода или вывода не оказывали никакого влияния.  (Демон не связан ни с одним терминальным устройством => он не может взаимодействовать с пользователем в интерактивном режиме)

## Журналирование ошибок

Проблема -> обслуживание сообщений об ошибках.

`syslog` - центраизованный механизм регистрации сообщений об ошибках.

Существует три способа регистарации сообщений:

1. Процедуры ядра могут обращаться к функции  **`log`**  (нас не интересует)
2. Большиство пользовательских процессов (демонов) используют **`syslog`**. Эта функция отправляет сообщения через сокет **домена UNIX - `/dev/log`**
3. Пользовательский процесс, который выполняется на данном или каком-либо другом компьютере, соединенном с данным компьютером сетью TCP/IP, может отправлять сообщения **по протоколу UDP на порт 514**. (функция syslog никогда не генерирует дейтаграммы UDP - данная функциональность требует, чтобы сама программа поддерживала сетевые взаимодействия)

Обычно демон `syslogd` понимает все три способа регистрации сообщений. На запуске он считывает конфигурационный файл (`/etc/syslog.conf`), в котором определяется, куда должны передаваться различные классы сообщений. Например, срочные -> в консоль системного администратора, предупреждения -> в файл.

В нашем случае взаимодействие с этим механизмом осуществляется посредством функции `syslog`

```c
#include <syslog.h>
```

```c
void openlog(const char *ident, int option, int facility);
// ident - строчка идентификации, которая содержит имя программы (например, cron)
// option - битовая маска, которая определяет различные способы вывода сообщений
// facility - как должны обрабатыватся сообщения из раазных источников
// списки можно посмотреть в приложении
```

```c
void syslog(int priority, const chat *format, ...);
// priority - комбинация значения для аргумента facility и уровня важности сообщения.
// уровни важности можно посмотреть в таблице в приложении
// format передается функции vsprintf для создания строки сообщения
// %m заменяется сообщением об ошибке (strerror), которое соотв. значению переменной errno
```

```c
void closelog(void);
// В целом, openlog и closelog необязательны к написанию
```

```c
int setlogmask(int maskpri); 
// Возващает предыдущее значение маски приоритета журналируемых сообщений
// Может использоваться для установки маски приоритетов сообщений процесса
// Если маска приоритетов установлена, то сообщения, уровень приоритета которых не 
// содержиться в маске, не будут журналироватся.
// 0 - журналируются все сообщения
```

P.S. во многих системах имеется программа `logger`, которая может передавать сообщения механизму `syslogz`. Предназначена для использования в сценариях на языке командной оболочки, которые исполняются в неинтерактивном режиме и нуждаются в механизме журналирования сообщений.

Большиство реализаций `syslogd` помещают сообщения в очередь. Если в это время демону поступит два одинаковых сообщения, то в журнал будет записано только одно, но в конце будет добавлена строка ~ такого содержания "last message repeated N times".

## Демоны в единственном экзмепляре

Некоторые демоны реализованы таким образом, что допускают одновременную работу только одной своей копии. Причина - например, требование монопольного владения каким-либо ресурсом.

Один из основных механизмов - **блокировка файлов и записей**.

> Если каждый из демонов создаст файл и попытается установить для этого файла блокировку записи, то система разрешит установить блокировку только одну такую блокировку. Все последующие попытки установить блокировку потерпят неудачу -> сообщая остальным копиям демона о том, что демон уже запущен.

Это удобный механизм взаимного исключения. Завершится демон - снимется блокировка файла или записи -> в случае ошибки не нужно вручную удалять блокировку.

Как работает?

1. Каждая копия пытается создать файл и записать в него идентификатор процесса. 
2. Если файл уже заблокирован вернется 1, c `EACCESS` или `EAGAIN` в `errno`, что будет говорит о том, что демон уже запущен.
3. В противном случае усекаем размер файла до 0 (чтобы полностью удалить информацию о предыдущем идентификаторе) и вернем 0.

## Соглашения для демонов

В системе UNIX демоны придерживаются следующих соглашений:

1. Если демон использует файл блокировки, то этот файл помещается в каталог `/var/run`. (но для этого демон должен обладать привилегиями суперпользователя). Имя файла обычно `name.pid`, где name - имя демона или службы.
2. Если демон поддерживает определение доп. настроек, то они обычно соханяются в каталоге `/etc`. Имя конфигурационного файла обычно `name.pid`, где name - имя демона или службы.
3. Демоны могут запускаться из командной строки, но обычно запуск демонов производится из сценариев инициализации системы (`/etc/rc*` или `/etc/init.d/*`). Если после завершения демон должен автоматически перезапускаться, можно указать на это процессу `init`, добавив запись `respawn` в файл `/etc/inittab`.
4. Если демон имеет конфигурационный файл, то настройки из него считываются демонов во время запуска, и затем он обычно не обращается к этому файлу (если в конф. файл были внесены изменения, то демон пришлось бы останавливать и перезапускат снова). Во избежание этого некоторые демоны устанавливают обработчики сигнала `SIGHUP` , в который производится считывание конф файла и перенастройка демона (так как демоны не имеют упр терминала, у них нет причин ожидать этот сигнал -> он может использоваться для других целей)ы

## Приложение

### Некоторые демоны из Linux

1. `keventd` - предоставляет контекст процесса для запуска задач из очереди планировщика.
2. `kapmd` - обеспечивает поддержку расширенного управления питанием
3. `kswapd` - демон выгрузки страниц (поддерживает подсистему виртуальной памяти, в фоновом режиме записывая на диск страницы, измененные со времени их чтения с диска, благодаря чему они могут быть использованы снова)
4. `bdflush` и `kupdated` - сбрасывание кэшированных данных на диск (первый сбрасывает, когда объем свободной памяти уменьшается до определенного уровня, второй через регулярные интервалы времени)
5. `portmap` - осуществляет преобразование номеров программ RPC в номера сетевых портов.
6. `syslogd` - может использоваться программами для вывода системных сообщений в журнал просмотра оператором.
7. `inetd` - ожидает поступления из сети запросов к различным сететвым серверам.
8. `nfsd`, `lockd`, `rpciod` - обеспечивают поддержку сетевой файловой системы
9. `cron` - производит запуск команд в определенное время.
10. `cupsd` - сервер печати, обслуживает запросы к принтеру.

### Схема работы syslogd

<img width="684" alt="image-20221030174255452" src="https://user-images.githubusercontent.com/78147880/198892087-870b7095-d92f-42ed-b088-a098f6868faf.png">

### Возможные значения, которые могут быть включены в аргумент option функции openlog

<img width="638" alt="image-20221030181155241" src="https://user-images.githubusercontent.com/78147880/198892100-bb1b14b2-9e5c-4620-9188-17e75d897833.png">

### Возможные значения аргемента facility функции openlog

<img width="638" alt="image-20221030181642664" src="https://user-images.githubusercontent.com/78147880/198892115-5f0c7b79-87b1-4ee1-9d53-67ce4a5fe3cc.png">

### Уровни важности сообщений

<img width="638" alt="image-20221030181714369" src="https://user-images.githubusercontent.com/78147880/198892135-8fc14753-7a84-4b91-a2e3-322de244ec79.png">

### О процессе SIGHUB

<img width="687" alt="image" src="https://user-images.githubusercontent.com/78147880/198905993-7a9f2729-261b-4ffe-bcfb-6eefd5eb37b8.png">

### О процессе SIGTERM

<img width="687" alt="image" src="https://user-images.githubusercontent.com/78147880/198906036-2dbb4505-6021-433b-be32-fbdbcb035dbe.png">

### О буковках 
![image](https://user-images.githubusercontent.com/78147880/199207218-0bb231e1-4480-4a88-882e-17e4fd2617c8.png)
![image](https://user-images.githubusercontent.com/78147880/199207431-07a4aa9c-dfea-4c21-b8ae-77520d63df0f.png)