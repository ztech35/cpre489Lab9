CC = gcc
CFLAGS = -g -Wall -O3 -lm

all: sender receiver

sender: sender.o AddCongestion.o ccitt16.o
	$(CC) $(CFLAGS) sender.o AddCongestion.o ccitt16.o -o sender

receiver: receiver.o ccitt16.o
	$(CC) $(CFLAGS) receiver.o ccitt16.o -o receiver

AddCongestion.o: AddCongestion.c AddCongestion.h
	$(CC) $(CFLAGS) AddCongestion.c -c

sender.o: sender.c
	$(CC) $(CFLAGS) sender.c -c

receiver.o: receiver.c
	$(CC) $(CFLAGS) receiver.c -c

clean:
	rm -rf receiver.o sender.o sender receiver AddCongestion.o
