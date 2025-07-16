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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <poll.h>
#include <pthread.h>
#include "pollsockhandling.h"

typedef struct thread_args{
  struct pollfd *server_fd;
  WINDOW *message_box;
  WINDOW *main_window;
  bool flag;
  char *username;
} thread_args_t;

void *send_thread(void *thread_args) {
    // Char array to store message  
    char msg[2048];

    thread_args_t *con_thread_args = (thread_args_t *) thread_args;
    struct pollfd *server_fd = con_thread_args->server_fd;
    WINDOW *input_box =  con_thread_args->message_box;
    WINDOW *main_window = con_thread_args->main_window;
    while(1){
      // Clear message box buffer  
      wclear(input_box);
      wrefresh(input_box);

      // Memset msg array to have no artefacts from previous use memset(msg, 0, sizeof(msg));

      // Waiting for input from user 
      wgetnstr(input_box, msg, sizeof(msg));
      
      // If user types "/exit", use the flag to tell main thread to exit from client
      // exiting also includes freeing all dyn memory as well as proper closing of sockets
      if(strcmp(msg, "/exit") == 0)    {  
          // shutdown signals closing of socket 
          con_thread_args->flag = true;    
          pthread_exit(NULL); 
      } else {
          int send_conf = send(server_fd[0].fd, msg, sizeof(msg), 0);
          wprintw(main_window, "You: %s\n", msg);    
          wrefresh(main_window);
          if(send_conf < 0) {
              wprintw(stdscr,"Error from send() %d\n", GETSOCKETERRNO());
              refresh();
        }
      }
    }
  }

char* username_conf(char *username_arr) {
    char username[32] = {0};
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
    struct pollfd *server_fd = calloc(1, sizeof(struct pollfd)); 
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
    username = username_conf(temp_user);
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
        free(server_fd);
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

    // Username sending 
    send(server_soc, username, sizeof(username), 0);
    wrefresh(main_text_invis);
    // Username sending

    // Drawing boxes
    box(main_text,0,0);
    box(input_box,0,0);
    wrefresh(main_text);
    wrefresh(input_box);

    /*
    typedef struct thread_args{
      struct pollfd *server_fd;
      WINDOW *message_box;
      bool flag;
    } thread_args_t;
    */

    // user input thread 
    pthread_t *user_in_thread = calloc(1, sizeof(pthread_t *));
   
    // user_in args error handling
     if(user_in_thread == NULL) {
      perror("Calloc call failed.");
      
      // Clear the window, print given error 
      clear();
      wprintw(stdscr, "Error from calloc() ");
      getch();
      delwin(main_text);
      delwin(main_text_invis);
      delwin(input_box);
      delwin(input_box_invis);
      free(server_fd);
	  free(user_in_thread); 
	  endwin();
      return 1;
    } 

    // thread args struct init 
    thread_args_t *thread_args = calloc(1, sizeof(thread_args_t)); 

    // thread args error handling  
    if(thread_args == NULL) {
      perror("Calloc call failed.");
      
      // Clear the window, print given error 
      clear();
      wprintw(stdscr, "Error from calloc() ");
      getch();
      delwin(main_text);
      delwin(main_text_invis);
      delwin(input_box);
      delwin(input_box_invis);
      free(server_fd);
	  free(user_in_thread);
	  free(thread_args);
	  endwin();
      return 1;
    } 

    thread_args->server_fd = NULL;
    thread_args->message_box = input_box_invis; 
    thread_args->flag = false;  
    thread_args->username = username;
    thread_args->main_window = main_text_invis;    
    pthread_create(user_in_thread, NULL, &send_thread, thread_args);

    while(1) {
        int poll_err = poll(server_fd, 1, -1);
        
        if(thread_args->server_fd == NULL) {
          thread_args->server_fd = server_fd;
        }

        if(thread_args->flag == true){
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
        }

        if (poll_err < 0) {
            clear();
            wprintw(stdscr,"Error from poll() %d",GETSOCKETERRNO());
            refresh();
        } 

        if(server_fd[0].revents & POLLIN) {
            // clear();
            // endwin();
			char recv_msg[2048] = {0};
            int recv_err = recv(server_fd[0].fd, recv_msg, sizeof(recv_msg), 0);
            if(recv_err < 0) {
                clear();
                wprintw(stdscr,"Error from recv() %d\n", GETSOCKETERRNO());
                refresh();
            }
            else {
                wprintw(main_text_invis,"Other: %s\n", recv_msg);
                wrefresh(main_text_invis);
            }

        }
    
   }

    getch();
    delwin(main_text);
    delwin(main_text_invis);
    delwin(input_box);
    delwin(input_box_invis);
    free(user_in_thread);
    free(thread_args);
    free(server_fd);
	endwin();



    return 0;
}
