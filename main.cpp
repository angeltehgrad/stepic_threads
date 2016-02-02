#include <netinet/in.h>
#include <wait.h>
#include <strings.h>

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



int main()
{
    int mastersocket, slavesocket, sockfd[2];
    int childpid;
    struct sockaddr_in serveraddr;
    char c = 'l';
    mastersocket = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&serveraddr, sizeof(serveraddr));

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(12345);
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(mastersocket, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
    listen(mastersocket, SOMAXCONN);

    slavesocket = accept(mastersocket, NULL, NULL);
    socketpair(AF_LOCAL, SOCK_STREAM, 0, sockfd);
    if((childpid = fork()) == 0)
    {
        close(sockfd[0]);
        //use sockfd[1]
        char buf;
        int recvsock;
        sock_fd_read(sockfd[1], &buf, 1, &recvsock);
        char buffer[1024];
        bzero(buffer, 1024);
        read(recvsock, buffer, 1024);
        write(recvsock, buffer, 1024);

    }
    close(sockfd[1]);
    //use sockfd[0]
    sock_fd_write(sockfd[0], &c, 1, slavesocket);
    int status;
    waitpid(childpid, &status, 0);
    close(sockfd[0]);
    return 0;
}

ssize_t
sock_fd_write(int sock, void *buf, ssize_t buflen, int fd)
{
    ssize_t     size;
    struct msghdr   msg;
    struct iovec    iov;
    union {
        struct cmsghdr  cmsghdr;
        char        control[CMSG_SPACE(sizeof (int))];
    } cmsgu;
    struct cmsghdr  *cmsg;

    iov.iov_base = buf;
    iov.iov_len = buflen;

    msg.msg_name = 0;
    msg.msg_namelen = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    if (fd != -1) {
        msg.msg_control = cmsgu.control;
        msg.msg_controllen = sizeof(cmsgu.control);

        cmsg = CMSG_FIRSTHDR(&msg);
        cmsg->cmsg_len = CMSG_LEN(sizeof (int));
        cmsg->cmsg_level = SOL_SOCKET;
        cmsg->cmsg_type = SCM_RIGHTS;

        //printf ("passing fd %d\n", fd);
        *((int *) CMSG_DATA(cmsg)) = fd;
    } else {
        msg.msg_control = NULL;
        msg.msg_controllen = 0;
        //printf ("not passing fd\n");
    }

    size = sendmsg(sock, &msg, 0);

    if (size < 0)
        perror ("sendmsg");
    return size;
}

ssize_t
sock_fd_read(int sock, void *buf, ssize_t bufsize, int *fd)
{
    ssize_t     size;

    if (fd) {
        struct msghdr   msg;
        struct iovec    iov;
        union {
            struct cmsghdr  cmsghdr;
            char        control[CMSG_SPACE(sizeof (int))];
        } cmsgu;
        struct cmsghdr  *cmsg;

        iov.iov_base = buf;
        iov.iov_len = bufsize;

        msg.msg_name = NULL;
        msg.msg_namelen = 0;
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;
        msg.msg_control = cmsgu.control;
        msg.msg_controllen = sizeof(cmsgu.control);
        size = recvmsg (sock, &msg, 0);
        if (size < 0) {
            perror ("recvmsg");
            exit(1);
        }
        cmsg = CMSG_FIRSTHDR(&msg);
        if (cmsg && cmsg->cmsg_len == CMSG_LEN(sizeof(int))) {
            if (cmsg->cmsg_level != SOL_SOCKET) {
                fprintf (stderr, "invalid cmsg_level %d\n",
                     cmsg->cmsg_level);
                exit(1);
            }
            if (cmsg->cmsg_type != SCM_RIGHTS) {
                fprintf (stderr, "invalid cmsg_type %d\n",
                     cmsg->cmsg_type);
                exit(1);
            }

            *fd = *((int *) CMSG_DATA(cmsg));
            //printf ("received fd %d\n", *fd);
        } else
            *fd = -1;
    } else {
        size = read (sock, buf, bufsize);
        if (size < 0) {
            perror("read");
            exit(1);
        }
    }
    return size;
}

