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
#include <sys/poll.h>

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
// macros provided by Lewis Van Winkle

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "usernamekeytable.h"

int main() {    

    // Hash table declaration 
    table_t *user_table = (table_t*) calloc(1, sizeof(table_t));
    table_elem *table = (table_elem*) calloc(1, sizeof(table_elem));
    user_table -> table = table; 
    user_table -> table_size = 1;
    user_table -> num_of_elems = 0;

    // Pollfd array declaration 
    struct pollfd *pollsocket_arr = calloc(1, sizeof(struct pollfd));
    

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; 

    struct addrinfo *hostaddr;
    int addrinfocheck = getaddrinfo("localhost", "8080", &hints, &hostaddr);
    // getaddrinfo returns not int on failure
    if (addrinfocheck < 0) {
        printf("getaddrinfo() failed.\n");
        return 1;
    }

    SOCKET socket_listen;
    socket_listen = socket(hostaddr->ai_family, hostaddr->ai_socktype,
                        hostaddr->ai_protocol);
    if(!ISVALIDSOCKET(socket_listen)) {
        printf("socket() failed. %d\n", GETSOCKETERRNO());
        return 1;
    }
    
   
    if(bind(socket_listen, hostaddr->ai_addr, hostaddr->ai_addrlen)) {
        printf("bind() failed. %d\n", GETSOCKETERRNO());
        return 1;
    }
    // double freeaddr was in while loop
    freeaddrinfo(hostaddr);

    int listenerr = listen(socket_listen, 5);
    if (listenerr < 0) {
        printf("listen() failed. %d\n", GETSOCKETERRNO());
        return 1;
    }



    printf("Waiting for connections.\n");

    struct sockaddr_storage client_address;
    socklen_t client_len = sizeof(client_address);
    SOCKET socket_client = accept(socket_listen,
            (struct sockaddr*) &client_address, &client_len);
    if (!ISVALIDSOCKET(socket_client)) {
        printf("accept() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }

        // getting socket info
        // TODO getting client socket info to log
        
        // struct sockaddr client_addr;
        // memset(&client_addr, 0, sizeof(client_addr));
        // socklen_t client_addr_len = sizeof(client_addr);
        // int socknameerr = getpeername(socket_client, &client_addr, &client_addr_len);
        // if (socknameerr < 0 ) {
        //     printf("getsockname() failed. (%d)\n", GETSOCKETERRNO());
        // }
        // printf("%s %d", client_addr.sa_data, client_addr.sa_family);
        

        printf("forked process\n");
        char username_conf[] = {"What is your username?\n"};
        char fin_username[32];
        memset(fin_username, 0, sizeof(fin_username));

        send(socket_client, username_conf, strlen(username_conf), 0);

        recv(socket_client, fin_username, 32, 0);
        printf("%s\n", fin_username);

        table_elem new_user = {&socket_client, fin_username};
        append_element(user_table, new_user);


    // closes accepted socket

    // recv ?       

    // Try Ncurses
    CLOSESOCKET(socket_listen);
    free(user_table);
    free(table);
    return 0;
}