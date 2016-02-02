#include <netinet/in.h>
#include "sock_passing.h"
#include <wait.h>
#include <strings.h>

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
