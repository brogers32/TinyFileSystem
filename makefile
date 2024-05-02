a.out: main.o drive.o
	gcc main.c drive.c

main.o: main.c drive.h
	gcc -c main.c

drive.o: drive.c drive.h
	gcc -c drive.c

clean: main.o drive.o a.out