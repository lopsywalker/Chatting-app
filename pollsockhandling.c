#if defined(_WIN32)
#define ISVALIDSOCKET(s) ((s) != INVALID_SOCKET)
#define CLOSESOCKET(s) closesocket(s)
#define GETSOCKETERRNO() (WSAGetLastError())
#endif

#include <sys/poll.h>
#include <stdlib.h>
#include <string.h>
#include "pollsockhandling.h"


int append_pollfd(pfdhandler_t *pollhandler, struct pollfd *append_fd) {
    if(pollhandler->pollfd_num == pollhandler->arr_size) {
        struct pollfd *new_ptr = calloc((pollhandler->arr_size)*2, sizeof(struct pollfd *));
        memcpy(new_ptr, pollhandler->pollfd_ptr, (pollhandler->arr_size));
        free(pollhandler->pollfd_ptr);
        pollhandler->pollfd_ptr = new_ptr;
        pollhandler->arr_size = ((pollhandler->arr_size)*2);
    }
    for(int i = 0; i < pollhandler->arr_size; i++) {
        if(pollhandler->pollfd_ptr[i].fd == 0) {
            pollhandler->pollfd_ptr[i].fd = append_fd->fd;
            pollhandler->pollfd_ptr[i].events = append_fd->events;
            pollhandler->pollfd_num = pollhandler->pollfd_num + 1; 
            return 0;
        }
    }
    return 1;
}

int remove_pollfd(pfdhandler_t *pollhandler, struct pollfd *remove_fd) {
    for(int i = 0; i < pollhandler->arr_size; i++) {
        if(pollhandler->pollfd_ptr[i].fd == remove_fd->fd) {
            pollhandler->pollfd_ptr[i].fd = 0;
            pollhandler->pollfd_ptr[i].events = 0;
            pollhandler->pollfd_ptr[i].revents = 0;
            pollhandler->pollfd_num = pollhandler->pollfd_num - 1;
            return 0;
        }
    }
    return 1;
}