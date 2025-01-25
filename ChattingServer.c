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
// macros provided by Lewis Van Winkle

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Hashtable.h"

int main() {    

    // Hash table declaration 
    table_t *user_table = (table_t*) calloc(1, sizeof(table_t));
    table_elem *table = (table_elem*) calloc(1, sizeof(table_elem));
    user_table -> table = table; 
    user_table -> table_size = 1;
    user_table -> num_of_elems = 0;

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

    int listenerr = listen(socket_listen, 5);
    if (listenerr < 0) {
        printf("listen() failed. %d\n", GETSOCKETERRNO());
        return 1;
    }


    printf("Waiting for connections.\n");

    while(1) {
        struct sockaddr_storage client_address;
        socklen_t client_len = sizeof(client_address);
        SOCKET socket_client = accept(socket_listen,
                (struct sockaddr*) &client_address, &client_len);
        if (!ISVALIDSOCKET(socket_client)) {
            printf("accept() failed. (%d)\n", GETSOCKETERRNO());
            return 1;
        }
        freeaddrinfo(hostaddr);

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
        
        printf("Receiving Messages. \n");

        pid_t child = fork();

        if (child == 0) {
            CLOSESOCKET(socket_listen);
            printf("forked process\n");
            char username_conf[] = {"What is your username?\n"};
            char fin_username[32];
            memset(fin_username, 0, sizeof(fin_username));

            send(socket_client, username_conf, strlen(username_conf), 0);

            recv(socket_client, fin_username, 32, 0);
            printf("%s\n", fin_username);

            table_elem new_user = {&socket_client, fin_username};
            append_element(user_table, new_user);

            // TODO : Buffer overflows when code has loop below (for checking username)

            while(1) {
                // Finds the number of bytes of message
                char message_len[4];
                memset(message_len, 0, sizeof(message_len));
                recv(socket_client, message_len, sizeof(message_len), 0);
                printf("%s\n", message_len);
                
                // TODO: catch errors
                // Recv according to bytes
                char fin_message[(int) strtol(message_len, NULL, 10)];
                memset(fin_message, 0, sizeof(fin_message));
                recv(socket_client, fin_message, strlen(fin_message), 0);
                printf("%s", fin_message);

                // TODO: encryption 
                for(int i = 0; i < user_table->num_of_elems; i++)
                {
                    if(*(user_table->table->key) != socket_client)
                    {
                        int senderr = send(*(user_table->table->key) , fin_message, strlen(fin_message), 0);
                        if(senderr < 0)
                        {
                            printf("send() failed.\n");
                            return 1;
                        }
                    }
                    
                }

            } // while(1)
            

        } // if child == 0

        CLOSESOCKET(socket_client);
        
    } // while(1)


    // closes accepted socket

    // recv ?       

    // Try Ncurses
    CLOSESOCKET(socket_listen);
    free(user_table);
    free(table);
    return 0;
}