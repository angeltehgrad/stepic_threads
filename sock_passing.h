#ifndef SOCK_PASSING_INCLUDED
#define SOCK_PASSING_INCLUDED

#include <sys/types.h>
#include <sys/socket.h>

#ifndef NULL
#define NULL (void*)0
#endif // NULL

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

ssize_t
sock_fd_write(int sock, void *buf, ssize_t buflen, int fd);
ssize_t
sock_fd_read(int sock, void *buf, ssize_t bufsize, int *fd);


#endif // SOCK_PASSING_INCLUDED
