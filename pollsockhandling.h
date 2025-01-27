#include <sys/poll.h>
#include <stdlib.h>

typedef struct pollfd_handler { 
    struct pollfd *pollfd_arr;
    size_t arr_size;
    size_t pollfd_num;
} pfdhandler_t;

int append_pollfd(pfdhandler_t *pollhandler, struct pollfd append_fd) {
    if(pollhandler->pollfd_num >= pollhandler->arr_size) {
        realloc(pollhandler->pollfd_arr, (pollhandler->arr_size)*2);
        pollhandler->arr_size = pollhandler->arr_size*2;
        // TODO: either make it a for loop where it just finds empty OR
        // add one to the pollfd_num which makes it a lot more complicated to 
        // delete but faster (likely)
    }
}