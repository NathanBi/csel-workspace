#include <sys/types.h>
#include <sys/socket.h>


int socketpair (int domain, int type, int protocol, int fd[2]);


int fd[2];
int err = socketpair(AF_UNIX, SOCK_STREAM, 0, fd);
if (err == -1)
/* error*/
    print("Error %d\n", err);