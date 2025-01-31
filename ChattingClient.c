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
    getaddrinfo("localhost", "8080", &hints, &serveraddr);

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
    char username_query[100];
    recv(server_soc, username_query, (size_t) sizeof(username_query), 0 );
    printf("%s", username_query);

    char username[32];
    fgets(username, sizeof(username), stdin);

    send(server_soc, username, sizeof(username), 0);

    // Username conf

    while(1)
    {
    char readfromstdin[4096];
    memset(readfromstdin, 0, sizeof(readfromstdin));
    fgets(readfromstdin, sizeof(readfromstdin), stdin);
    
    // char msglen[8];
    // memset(msglen, 0, sizeof(msglen));
    // itoa(strlen(readfromstdin), msglen, 10);

    // send(server_soc, msglen, strlen(msglen), 0);

    send(server_soc, readfromstdin, strlen(readfromstdin), 0);

    // char recv_msg[4096];
    // memset(recv_msg, 0, sizeof(recv_msg)); 
    // recv(server_soc, recv_msg, sizeof(recv_msg), 0);
    // printf("%s\n",recv_msg);
    }

    CLOSESOCKET(server_soc);

    return 0;
}