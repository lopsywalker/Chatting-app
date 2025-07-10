CC=gcc
FLAGS= -O2 -Wall -Werror -lncurses

server: ./src/ChattingServer.c
	${CC} ./src/ChattingServer.c ./include/pollsockhandling.c ./include/usernamekeytable.c -I./include -o server $(FLAGS)

client: ./src/ChattingClient.c
	${CC} ./src/ChattingClient.c ./include/pollsockhandling.c -I./include -o client $(FLAGS)

debug_server: ./src/ChattingServer.c
	${CC} -g ./src/ChattingServer.c ./include/pollsockhandling.c ./include/usernamekeytable.c -I./include -o debug_server $(FLAGS)

debug_client: ./src/ChattingClient.c
	${CC} -g ./src/ChattingClient.c ./include/pollsockhandling.c -I./include -o debug_client $(FLAGS)

clean: server client
	rm server client
