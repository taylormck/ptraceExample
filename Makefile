#------------------------------------------------------------
# Makefile
#------------------------------------------------------------
# Won't be needing any included header files or libraries
# for this project.
#------------------------------------------------------------
CC = gcc
CFLAGS = -Wall

all: controller worker

controller: controller.o
	${CC} ${CFLAGS} -o controller controller.o

worker: worker.o
	${CC} ${CFLAGS} -o worker worker.o

controller.o: controller.c

worker.o: worker.c

clean:
	rm -f controller worker *.o log

test: clean controller worker
	./controller ./worker > log && cat log

test2: clean controller
	./controller ls > log && cat log