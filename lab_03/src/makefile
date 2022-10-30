main.exe: main.o errors.o
	gcc -o main.exe main.o errors.o

main.o: main.c
	gcc -c main.c

errors.o: errors.c
	gcc -c errors.c

PHONY: clean

clean:
	rm -rf *.o *.exe
