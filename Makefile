CC=gcc
CFLAGS=-Wall -Iincludes -Wextra -std=c99 -ggdb
VPATH=src

all: client server

client: client.c
	${CC} ${CFLAGS} client.c -o client

server: server.c
	${CC} ${CFLAGS} server.c -o server

clean:
	rm -f client server

.PHONY : clean all
