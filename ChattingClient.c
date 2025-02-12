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
#include "pollsockhandling.h"
#include <ncurses.h>


char* username_conf(char *username_arr) {
    char username[32];
    memset(username, 0, sizeof(username));
    int MaxX, MaxY;
    initscr();

    getmaxyx(stdscr, MaxY, MaxX);

    WINDOW *username_box = newwin(3, MaxX/2, (MaxY/2)-3, MaxX/4);
    WINDOW *username_box_invis = newwin(1, (MaxX/2)-2, (MaxY/2)-2, (MaxX/4)+1);
    refresh();
    box(username_box,0,0);
    wrefresh(username_box);

    wgetstr(username_box_invis,username);

    strncpy(username_arr, username, 32);

    endwin();

    delwin(username_box);
    delwin(username_box_invis);

    return username_arr;
}

int main() {
    
    // Hint init 
    struct addrinfo hints;  
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    // Addrinfo init
    struct addrinfo* serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    getaddrinfo("localhost", "8080", &hints, &serveraddr);

    // pollsock init
    struct pollfd *server_fd = (struct pollfd *) calloc(1, sizeof(struct pollfd)); 
    server_fd->events = POLLIN | POLLOUT; 

    SOCKET server_soc = socket(serveraddr->ai_family, serveraddr->ai_socktype, 
            serveraddr->ai_protocol);
    if(!ISVALIDSOCKET(server_soc)) {
        printf("socket err.\n");
        return 1;
    }

    // username conf
    char *username;
    char temp_user[32];
    username = (username_conf(temp_user));

    printf("%s\n", username);
    // username conf 


    int status = connect(server_soc, serveraddr->ai_addr, serveraddr->ai_addrlen);
    if (status < 0) {
        printf("Connect error.\n");
        return 1;
    }
    freeaddrinfo(serveraddr);
    server_fd->fd = server_soc;

    // Connection conf
    char connection_conf[32];
    memset(connection_conf, 0, sizeof(connection_conf));
    int conn_recv_err = recv(server_soc, connection_conf, sizeof(connection_conf), 0);
    if(conn_recv_err == -1) {
        printf("Error from recv() %d",GETSOCKETERRNO());
    }
    printf("%s\n", connection_conf);
    // Connection conf 

    // // Username sending 
    send(server_soc, username, sizeof(username), 0);
    // // Username sending

    while(1) {
        int poll_err = poll(server_fd, 1, -1);
        if (poll_err < 0) {
            printf("Error from poll() %d",GETSOCKETERRNO());
        } 

        if(server_fd[0].revents == POLLIN) {
            char recv_msg[2048];
            memset(recv_msg, 0, sizeof(recv_msg));
            int recv_err = recv(server_fd[0].fd, recv_msg, sizeof(recv_msg), 0);
            if(recv_err < 0) {
                printf("Error from recv() %d\n", GETSOCKETERRNO());
            }
            printf("%s", recv_msg);

    // how to send from client ?
    // use fgets or smht? but how 
    // to know when to send and not random
    // probably better to write a seperate function 
    
        // } else if(server_fd[0].revents == POLLOUT) {
        //     char send_msg[2048];
        //     memset(send_msg, 0, sizeof(send_msg));
        //     int send_err = send(server_fd[0].fd, send_msg, sizeof(send_msg), 0);
        //     if(send_err < 0) {
        //         printf("Erorr from send() %d", GETSOCKETERRNO());
        //     }
        // }

    }
    }


    CLOSESOCKET(server_soc);

    return 0;
}