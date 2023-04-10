CC := gcc

daemon.exe : daemon.o my_error.o
	$(CC) -o daemon.exe daemon.o my_error.o

daemon.o : daemon.c
	$(CC) -c daemon.c

my_error.o : my_error.c
	$(CC) -c my_error.c

PHONY : clean

clean :
	rm -rf *.o *.exe
