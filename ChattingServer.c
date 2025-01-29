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
#include "pollsockhandling.h"

int main() {    

    // Hash table initialization & declaration 
    table_t *user_table = (table_t*) calloc(1, sizeof(table_t));
    table_elem *table = (table_elem*) calloc(1, sizeof(table_elem));
    user_table -> table = table; 
    user_table -> table_size = 1;
    user_table -> num_of_elems = 0;

    // Pollfd array initialization & declaration 
    pfdhandler_t *pfthander = calloc(1, sizeof(pfthander));
    struct pollfd *pollsocket_ptr = calloc(1, sizeof(struct pollfd));
    pfthander->pollfd_ptr = pollsocket_ptr;
    pfthander->arr_size = 1;
    pfthander->pollfd_num = 0;
    
    // getaddrinfo hints
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

    // Dont need to use listen for poll()? 
    // int listenerr = listen(socket_listen, 5);
    // if (listenerr < 0) {
    //     printf("listen() failed. %d\n", GETSOCKETERRNO());
    //     return 1;
    // }

    // Poll() for windows might be WSAPoll() ? 
    // last arg for poll is timeout (-1 means forever)
    // if timeout is made active poll will return 0 when it fully times out


    // Create listener poll fd for appending 
    struct pollfd *listenerfd = (struct pollfd *) calloc(1, sizeof(struct pollfd *)); 
    listenerfd->fd = socket_listen; 
    listenerfd->events = POLLIN;
    append_pollfd(pfthander, listenerfd);

    printf("Waiting for connections.\n");

    while(1) {
        int poll_event = poll(pfthander->pollfd_ptr, pfthander->pollfd_num, -1);
        if (poll_event >= 0) {
            for(int i = 0; i < pfthander->pollfd_num; i++) {

                if(pfthander->pollfd_ptr[i].revents == (POLLIN)) {
                    // if active poll is socket_listen
                    struct sockaddr_storage client_sock_addr;
                    socklen_t addrlen = sizeof(client_sock_addr);
                    if(pfthander->pollfd_ptr[i].fd == socket_listen) {
                        // accept socket, add to fds list in pfthandler
                        SOCKET client_socket = accept(socket_listen, &client_sock_addr, &addrlen);
                        struct pollfd *new_fd = (struct pollfd*) calloc(1, sizeof(struct pollfd));
                        new_fd->events = (POLLIN || POLLOUT || POLLHUP);
                        new_fd->fd = client_socket;
                        append_pollfd(pfthander, new_fd);
                    } else {
                        char msgbuff[2048];
                        memset(msgbuff, 0, sizeof(msgbuff));
                        recv(pfthander->pollfd_ptr[i].fd, msgbuff, sizeof(msgbuff), 0);
                        // TODO: error handling for recv call
                        for(int l = 0; l < pfthander->pollfd_num; l++) {
                            // if socket can recieve messages
                            if(pfthander->pollfd_ptr[l].revents == (POLLOUT)) {
                                // TODO: dont send to listner socket
                                send(pfthander->pollfd_ptr[l].fd, msgbuff, 2048, 0);
                            }
                        }
                    }

                } // pfthander->pollfd_ptr[i].revents == (POLLIN)

                // when client disconnects
                else if(pfthander->pollfd_ptr[i].revents == (POLLHUP)) {
                    // remember to CLOSESOCKET() when disconnecting 
                }



            } // for (int i = 0; i < pfthander->pollfd_num; i++)

            // for(...) { if poll.revents == (some event); add socket, remove socket, read from socket, send to socket}
        } else {
            fprintf(stderr, "Poll() call failed.\n");
            return -1;
        }

    }


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
    free(user_table);
    free(table);
    free(pfthander);
    free(pollsocket_ptr);
    free(listenerfd);
    return 0;
}