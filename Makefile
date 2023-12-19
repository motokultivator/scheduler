N=`getconf _NPROCESSORS_ONLN`

all: example1 example

example1: example.c scheduler1.o context.o
	gcc -O0 example.c scheduler1.o context.o -lpthread -fno-stack-protector -o example1

example: example.c scheduler.o context.o
	gcc -O0 example.c scheduler.o context.o -lpthread -fno-stack-protector -o example

scheduler.o: scheduler.c scheduler.h
	gcc -O3 -DNUM_THREADS=$N -c scheduler.c -fno-stack-protector -o scheduler.o

scheduler1.o: scheduler.c scheduler.h
	gcc -O3 -DNUM_THREADS=1 -c scheduler.c -fno-stack-protector -o scheduler1.o

context.o: run_x86_64.asm
	nasm -f elf64 run_x86_64.asm -o context.o

example_aarch64: context_aarch64.o scheduler_aarch64.o example.c
	aarch64-linux-gnu-gcc -O0 example.c scheduler_aarch64.o context_aarch64.o -lpthread -o example_aarch64

scheduler_aarch64.o: scheduler.c scheduler.h
	aarch64-linux-gnu-gcc -O3 -DNUM_THREADS=4 -c scheduler.c -o scheduler_aarch64.o

context_aarch64.o: run_aarch64.S
	aarch64-linux-gnu-as run_aarch64.S -o context_aarch64.o

clean:
	rm -rf example1 example scheduler.o scheduler1.o context.o example_aarch64 context_aarch64.o scheduler_aarch64.o
