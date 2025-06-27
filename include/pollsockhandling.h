#ifndef POLLSOCKHANDLING_H
#define POLLSOCKHANDLING_H

// struct needed before the defined functions
typedef struct pollfd_handler { 
    struct pollfd *pollfd_ptr;
    nfds_t arr_size;
    nfds_t pollfd_num;
} pfdhandler_t;


int append_pollfd(pfdhandler_t *pollhandler, struct pollfd *append_fd);

int remove_pollfd(pfdhandler_t *pollhandler, struct pollfd *remove_fd);

#endif 