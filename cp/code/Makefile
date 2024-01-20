
obj-m += core_module.o

all: cli.o core_module.o

cli.o: cli.c core_module.h
	gcc -o cli.o cli.c	

core_module.o: core_module.c
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	rm -rf cli *.o
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean