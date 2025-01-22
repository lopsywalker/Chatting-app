// macros provided by Lewis Van Winkle
#if defined(_WIN32)
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

#endif


#if defined(_WIN32)
#define ISVALIDSOCKET(s) ((s) != INVALID_SOCKET)
#define CLOSESOCKET(s) closesocket(s)
#define GETSOCKETERRNO() (WSAGetLastError())

#else
#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)
#define SOCKET int
#define GETSOCKETERRNO() (errno)
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TODO, client and server aren't communicating for some reason 

int main() {
    struct addrinfo hints; 
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo* serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    getaddrinfo(0, "8080", &hints, &serveraddr);

    SOCKET server_soc = socket(serveraddr->ai_family, serveraddr->ai_socktype, 
            serveraddr->ai_protocol);
    if(!ISVALIDSOCKET(server_soc)) {
        printf("socket err.\n");
        return 1;
    }

    int status = connect(server_soc, serveraddr->ai_addr, serveraddr->ai_addrlen);
    if (status < 0) {
        printf("Connect error.\n");
        return 1;
    }
    printf("Connected\n");
    freeaddrinfo(serveraddr);


    // Username conf
    char username_query[32];
    recv(server_soc, username_query, (size_t) strlen("What is your username?\n"), 0 );
    printf("%s", username_query);

    char username[32];
    fgets(username, sizeof(username), stdin);

    send(server_soc, username, sizeof(username), 0);
    // Username conf

    // char msg[100];
    // // memset(msg, 0, sizeof(msg));

    // printf("sending message\n");
    // int sentbytes = send(server_soc, "smth", strlen("smth"), 0);
    // printf("%d", sentbytes);
    CLOSESOCKET(server_soc);


    return 0;
}