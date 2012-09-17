#------------------------------------------------------------
# Makefile
#------------------------------------------------------------
# Won't be needing any included header files or libraries
# for this project.
#------------------------------------------------------------
CC = gcc
CFLAGS = -Wall

all: controller worker

controller:
	${CC} ${CFLAGS} controller.c -o controller

worker:
	${CC} ${CFLAGS} worker.c -o worker

clean:
	rm -f controller worker *.o log