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
#include <ncurses.h>
#include <poll.h>
#include <pthread.h>
#include "pollsockhandling.h"


char* username_conf(char *username_arr) {
    char username[32];
    memset(username, 0, sizeof(username));
    int MaxX, MaxY;
    initscr();

    getmaxyx(stdscr, MaxY, MaxX);
    mvwprintw(stdscr,(MaxY/2)-5,MaxX/4, "What is your username?");
    WINDOW *username_box = newwin(3, MaxX/2, (MaxY/2)-3, MaxX/4);
    WINDOW *username_box_invis = newwin(1, (MaxX/2)-2, (MaxY/2)-2, (MaxX/4)+1);
    refresh();
    box(username_box,0,0);
    wrefresh(username_box);

    wgetstr(username_box_invis,username);

    strncpy(username_arr, username, 32);

    delwin(username_box);
    delwin(username_box_invis);

    endwin();
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
    // username conf 


    initscr();
    int MaxX, MaxY;
    curs_set(0);
    getmaxyx(stdscr, MaxY, MaxX);
    WINDOW *main_text = newwin(MaxY-6, MaxX-10, 0, 5);
    WINDOW *main_text_invis = newwin(MaxY-8, MaxX-12, 1, 6);
    WINDOW *input_box = newwin(3, MaxX-10, MaxY-5, 5);
    WINDOW *input_box_invis = newwin(1, MaxX-12, MaxY-4, 6);
    keypad(stdscr, TRUE);
    refresh();

    int status = connect(server_soc, serveraddr->ai_addr, serveraddr->ai_addrlen);
    if (status < 0) {
        clear();
        wprintw(stdscr, "Connect error.\n");
        getch();
        delwin(main_text);
        delwin(main_text_invis);
        delwin(input_box);
        delwin(input_box_invis);
        endwin();
        return 1;
    }
    freeaddrinfo(serveraddr);
    server_fd->fd = server_soc;

    // Connection conf
    char connection_conf[32];
    memset(connection_conf, 0, sizeof(connection_conf));
    int conn_recv_err = recv(server_soc, connection_conf, sizeof(connection_conf), 0);
    if(conn_recv_err == -1) {
        clear();
        wprintw(stdscr, "Error from recv() %d",GETSOCKETERRNO());
        getch();
        delwin(main_text);
        delwin(main_text_invis);
        delwin(input_box);
        delwin(input_box_invis);
        endwin();
    }

    clear();
    // Connection conf 

    // // Username sending 
    send(server_soc, username, sizeof(username), 0);
    // // Username sending

    // Drawing boxes
    box(main_text,0,0);
    box(input_box,0,0);
    wrefresh(main_text);
    wrefresh(input_box);

    while(1) {
        char msg[2048];
        memset(msg, 0, sizeof(msg));
        int poll_err = poll(server_fd, 1, -1);

        wclear(input_box_invis);
        wrefresh(input_box_invis);

        // int in = getch();

        // if(in == KEY_MOUSE) {
        //     if(getmouse(&mouse_event) == OK) {
        //         if(mouse_event.bstate & BUTTON4_PRESSED) {
        //             // print
        //         }
        //     }
        // }


        // TODO: dont bother with history when scrolling up, pad doesnt work the way u wnat
        //  It is just better to learn principles of networking over actually making cahtting app
        
        if (poll_err < 0) {
            clear();
            wprintw(stdscr,"Error from poll() %d",GETSOCKETERRNO());
            refresh();
        } 

        if(server_fd[0].revents & POLLIN) {
            clear();
            endwin();

            char recv_msg[2048];
            memset(recv_msg, 0, sizeof(recv_msg));
            int recv_err = recv(server_fd[0].fd, recv_msg, sizeof(recv_msg), 0);
            refresh();
            if(recv_err < 0) {
                clear();
                wprintw(stdscr,"Error from recv() %d\n", GETSOCKETERRNO());
                refresh();
            }
            else {
                wprintw(main_text_invis,"%s\n",recv_msg);
                wrefresh(main_text_invis);
            }

        }
    

        wgetnstr(input_box_invis,msg,sizeof(msg));
        if(strcmp(msg,"/exit") == 0)    {  
            // shutdown signals closing of socket 
            shutdown(server_soc, SHUT_RDWR);
            shutdown(server_fd[0].fd, SHUT_RDWR);
            CLOSESOCKET(server_soc);
            CLOSESOCKET(server_fd[0].fd);
            // Clears all visuals from screen
            clear();
            
            // clears dynamic mem
            delwin(main_text);
            delwin(main_text_invis);
            delwin(input_box);
            delwin(input_box_invis);

            // Prints exiting 
            mvwprintw(stdscr,(MaxY/2),(MaxX/2)-strlen("Press a key to exit."), "Press a key to exit.");
            getch();
            endwin();

            return 0;
        } else {
            int send_conf = send(server_fd[0].fd, msg, sizeof(msg), 0);
            if(send_conf < 0) {
                wprintw(stdscr,"Error from send() %d\n", GETSOCKETERRNO());
                refresh();
            }
        }

        
        
    // SOL: calling the send call directly from ncurses


    }

    getch();
    delwin(main_text);
    delwin(main_text_invis);
    delwin(input_box);
    delwin(input_box_invis);
    endwin();



    return 0;
}