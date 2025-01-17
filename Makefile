CC=gcc
FLAGS= -O2 -Wall -Werror 

server: ChattingServer.c
	${CC} ChattingServer.c -o server $(FLAGS)

clean: server
	rm server