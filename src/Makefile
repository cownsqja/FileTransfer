CC = gcc

all : client server

client : client.c
		gcc -o client client.c -pthread -w

server : server.c
		gcc -o server server.c -pthread -w

clean :
		rm client server