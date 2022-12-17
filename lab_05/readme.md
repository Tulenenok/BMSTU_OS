# Лабораторная работа №5

## Введение

Семафорные механизмы имеют ряд недостатков:

* семафор не указывает непостредственно на синхронизирующее условие или на критический ресурс
* при построении сложных схем синхронизации алгоритмы решения задач порой получаются весьма непростыми, ненаглядными и затруднительными для доказательства их правильности

Поэтому нужен механизм, который позволил бы программистам без лишних усилий создавать параллельные взаимодействующие программы.

## Монитор

Монитор (в параллельном программировании) -- это механизм организации параллелизма, который содержит как данные, так и процедуры, необходимые для реализации динамического распределения конкретного общего ресурса или группы общих ресурсов.

> Есть планировщик, ресур и процессы.
> Когда процесс желает получить ресурс, он обращается к планировщику.
> Планировщик имеет переменные, по которым он определяет занят ресурс или свободен.
> При этом процедуру-планировщик разделяют все процессы.
> А обслужить больше одного процесса одновременно нельзя.
>
> Вот эта процедура планировщик и есть монитор.

Про алгоритм работы более подробно:

* процесс, желая получить доступ к разделяемым переменным, должен обратиться к монитору 
* вход в монитор находится под жестким контролем -- здесь осуществляется взаимоисключение процессов (в каждый момент времени только один процесс может войти в монитор)
* если монитор занят, то процессу приходится ждать. Причем режимом ожидания автоматически управляет сам монитор -- он блокирует обратившийся к нему процесс и определяет условие, по которому процесс ждет (проверку условия осуществляет монитор)
* если процесс обращается к некоторой процедуре и обнаруживается, что ресурс уже занят, процедура выдает команду ожидания WAIT с указанием условия ожидания. 
* при этом процесс ждет вне монитора, пока процесс освободиться (чтобы не противоречить принципу взаимоисключения)
* когда ресурс возвращается системе, соответствующая процедура монитора или ждет пока не придет заявка на этот ресурс или выполнит команду извещения SIGNAL, чтобы один из ожидающих процессов мог получить данный реурс и покинуть монитор

Еще одна причина, почему важно, чтобы ожидающий ресурс процесс ждал вне монитора:

> Другому процессу, когда он захочет вернуть ресурс, нужно будет зайти в монитор -- монитор должен быть свободен.

Как сделать так, чтобы ожидающий процесс не ждал ресурс вечно?

> Ожидающий процесс имеет более высокий приоритет, чем новый процесс, пытающийся войти в монитор. В противном случае новый процесс мог бы перехватить ожидаемый ресурс до того, как ожидающий процесс вновь войдетв монитор.
>
> (для систем реального времени можно допустить использование дисциплины обслуживания на основе абсолютных или динамически изменяемых приоритетов)

## Некоторые преимущества подхода

* использование монитора освобождает процессы от необходимости явно разделять между собой информацию
* доступ к разделяемым переменным ограничен телом монитора -> разделяемые переменные становятся системными переменными (так как монитор входит в состав ядра ос) -> это автоматически исключает критические интервалы (два процесса никогда не будут пользоваться ресурсом одновременно)
* мониторы очень гибки -- в форме мониторов можно реализоваать не только семафоры, но и многие другие синхронизирующие операции
* локализация всех разделяемых переменных внутри тела монитора позволяет избавиться от малопонятных конструкций в синхронизируемых процессах
* мониторы дают возможность совместно использовать программные модули, представляющие собой критические секции (не нужно для каждого процесса иметь собственный экземпляр критической секции)

ИТОГ: мониторы обеспечивают по сравнению с семафорами значительное упрощение организации взаимодействующих процессов и большую наглядность при незначительной потере в эффективности.

------

## Читатели-писатели

### Постановка задачи

Реализовать классический монитор Хоара на примере задачи "читатели-писатели" с использованием событий и с включением в программу одного мьютекса.

Использовать четыре функции:

* `BeginWrite()` - функция блокировки разделяемого ресурса для его инкемента
* `EndWrite()` - функция снятия блокировки с разделяемого ресурса после записи
* `BeginRead()` - функция блокировки разделяемого ресурса для выполнения операции чтения
* `EndRead()` - функция снятия блокировки с разделяемого ресурса после завершения операции чтения

В качестве разделяемого ресурса использовать счетчик, который в процессе работы писателя инкрементируется на 1. Минимальное количество писателей и читателей. -- 5 и 3 соответственно.

Предполагаются следующие правила работы потоков:

* когда один поток пишет в область общих данных -- другие потоки не могут ни считывать, ни записывать в эту область
* когда один поток читает из области общих данных -- другие потоки не могут туда ничего записывать, но могут читать данные

### Решение

Для работы с монитором будем использовать параллельные потоки.

У нас будет переменная `writeLock` - когда писатель работает она будет иметь истинное значение.

Переменная `readers` - указывает количество активных читателей. Когда количество читателей оказывается равным нулю, ожидающий поток-писатель получает возможность начать работу. 

Есть два события `CanRead` (со сбосом вручную) и `CanWrite` (с автоматическим сбросом). Новый поток читатель не может продолжить свое выполнение, пока не настанет событие `CanRead`. А новый поток писатель -- пока не настанет событие `CanWrite`.

Когда читателю необходимо выполнить чтение данных, он вызывает процедуру входа в монитор `BeginRead`, затем инкрементирует переменную `queueReaders`, показывая свое присутствие в очереди на чтение данных новому потоку-писателю, поток-читатель может находиться внутри функции `BeginRead` пока никакой другой писатель не пишет данные и не претендует на выполнение записи данных.

Функция `BeginRead` завершается оповещением переменной `CanRead`, чтобы другие ждущие потоки смогли продолжить чтение данных.

По окончанию процесса чтение читатель вызывает функцию `EndRead`, в ней читател уменьшает переменную `readers` на 1, сокращая тем самым количество активных читателей. Когда в системе не остается ни одного читателя, поток выдается команду оповещения `CanWrite`, чтобы писатели смогли продолжить работу (тем самым исключая бесконечное откладывание ждущих писателей)

### Схема работы читателя

<img width="638" alt="image-20221113171348769" src="https://pro-prof.com/wp-content/uploads/2020/02/flowchart-hoar_2.png">

### Схема работы писателя 

<img width="638" alt="image-20221113171348769" src="https://pro-prof.com/wp-content/uploads/2020/02/flowchart-hoar_1.png">

-----

## Источники

1. [Общие термины](https://studfile.net/preview/5946428/page:72/)
2. [Решение задачи читатели-писатели](https://pro-prof.com/forums/topic/hoar_modelling)