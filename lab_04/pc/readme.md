## Задача производитель - потребитель

### **Постановка задачи**

Имеется **буфер** фиксированного размера. **Производитель** (producer) может произвести **единичный объект** и поместить его в буфер. **Потребитель** (consumer) может выбирать объекты из буфера по одному и «потреблять» их. 

Необходимо обеспечить **монопольный доступ** производителей и потребителей к буферу: когда производитель помещает элемент в буфер, ни другой производитель или потребитель не должен иметь доступ к буферу; аналогично, когда потребитель берет элемент из буфера, ни другой потребитель или какой-нибудь производитель не могут получить доступ к буферу. 

В этой задаче буфер является **критическим ресурсом**.

---

### **Коротко план программы**
1. Создаем новый сегмент разделяемой памяти (с помощью функции `shmget`)
2. Подлючаем этот сегмент к адресному пространству вызывающего процесса (с помощью функции `shmat`)
3. Инициализируем буфер разделяемой памяти (своя функция `init_buffer`)
4. Получаем идентификатор набора семафоров (с помощью функции `semget`)
5. Устанавливаем значения 3-х семафоров (с помощью функции `semctl` c параметром `SETVAL`)
    * 0 уст. в 0 - это семафор количества заполненных ячеек
    * 1 уст. в 1 - это бинарный семафор, регулирующий доступ процессов к буферу
    * 2 уст. в 5 - это семафор количества НЕ заполненных ячеек
6. Создать трех производителей (каждый - отдельный процесс-потомок) и заставить их произвести 8 товаров
    * Аналогично потребителю
7. Создать трех потребителей (каждый - отдельный процесс-потомок) и заставить их получить 8 товаров
    * Для каждого потребителя создается отдельный поток (с помощью `fork`)
    * Потребитель считывает 8 товаров (букв)
    * Сначала генериться случайная задержка 
    * Потом потребитель получает доступ к критической зоне (с помощью функции `semop` и двух семафоров - один проверяет можно ли что-то считать, через второй получаем доступ к критической зоне)
    * Потребитель считывает из буфера (начала функция `read_buffer`)
    * Потребитель освобождает критическую зону (с помощью функции `semop` и двух семафоров - один освобождает доступ к критической зоне, другой проверяет увеличивает кол-во свободных ячеек на одну)
8. Дождаться завершения всех процессов потомков (с помощью функции `wait`)
9. Пометить сегмент разделяемой памяти как удаленный (с помощью функции `shmctl` с параметром `IPC_RMID`)
10. Отстыковать сегмент разделяемой памяти от адресного пространства процесса (с помощью функции `shmdt`)
11. Удалить набор семафоров из системы (с помощью функции `semctl` с параметром `IPC_RMID`)

---

### **Вопросы**

А это вообще законно использовать массив для типа создания? 