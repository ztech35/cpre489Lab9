CC = gcc
CFLAGS = -g -Wall -O3

all: sender receiver

sender: sender.o AddCongestion.o ccitt16.o
	$(CC) sender.o AddCongestion.o ccitt16.o -o sender

receiver: receiver.o ccitt16.o
	$(CC) receiver.o ccitt16.o -o sender

AddCongestion.o: AddCongestion.c AddCongestion.h
	$(CC) AddCongestion.c -c

sender.o: sender.c
	$(CC) sender.c -c

receiver.o: receiver.c
	$(CC) receiver.c -c

clean:
	rm -rf receiver.o sender.o sender receiver AddCongestion.o
