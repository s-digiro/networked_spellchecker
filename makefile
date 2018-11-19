# Filename:	makefile
# Author:	Sean DiGirolamo
# Version:	1.0.0
# Date:		03-25-18
# Purpose:	Makefile for networked spell checker server program
# 		Simply run "make" to compile the program
# 		or run "make clean" to remove any files created during the 
# 		compile process

CC = gcc
CFLAGS = -std=c99 -g -Wall

default: main

main: main.c fixed_q.o command.o log.o
	$(CC) $(CFLAGS) -pthread -o spell_check-server main.c fixed_q.o command.o log.o

fixed_q.o: fixed_q.c
	$(CC) $(CFLAGS) -o fixed_q.o -c fixed_q.c

command.o: command.c
	$(CC) $(CFLAGS) -o command.o -c command.c

log.o: log.c
	$(CC) $(CFLAGS) -o log.o -c log.c

clean:
	rm spell_check-server fixed_q.o command.o log.o
