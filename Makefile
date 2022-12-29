N=`getconf _NPROCESSORS_ONLN`

all: example1 example

example1: example.c scheduler1.o context.o
	gcc -O0 example.c scheduler1.o context.o -lpthread -o example1

example: example.c scheduler.o context.o
	gcc -O0 example.c scheduler.o context.o -lpthread -o example

scheduler.o: scheduler.c scheduler.h
	gcc -O3 -DNUM_THREADS=$N -c scheduler.c -o scheduler.o

scheduler1.o: scheduler.c scheduler.h
	gcc -O3 -DNUM_THREADS=1 -c scheduler.c -o scheduler1.o

context.o: run_x86_64.asm
	nasm -f elf64 run_x86_64.asm -o context.o

clean:
	rm -rf example1 example scheduler.o scheduler1.o context.o
