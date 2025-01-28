#include <sys/poll.h>
#include <stdlib.h>

typedef struct pollfd_handler { 
    struct pollfd *pollfd_ptr;
    size_t arr_size;
    size_t pollfd_num;
} pfdhandler_t;

int append_pollfd(pfdhandler_t *pollhandler, struct pollfd *append_fd) {
    if(pollhandler->pollfd_num >= pollhandler->arr_size) {
        realloc(pollhandler->pollfd_ptr, (pollhandler->arr_size)*2);
        pollhandler->arr_size = pollhandler->arr_size*2;
    }
    for(int i = 0; i < pollhandler->arr_size; i++) {
        if(pollhandler->pollfd_ptr[i].fd == NULL) {
            pollhandler->pollfd_ptr[i] = append_fd;
        }
    }
}