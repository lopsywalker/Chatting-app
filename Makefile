CC=gcc
FLAGS= -O2 -Wall -Werror -lncurses

server: ChattingServer.c
	${CC} ChattingServer.c -o server $(FLAGS)

client: ChattingClient.c
	${CC} ChattingClient.c -o client $(FLAGS)

clean: server client
	rm server client