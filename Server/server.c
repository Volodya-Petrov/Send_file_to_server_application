#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#define COUNTOFWAITINGCONNECTIONS  10

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int start_server(const char* port, const char* directory_name)
{
    struct addrinfo pattern, *server_info, *p;
    char s[INET6_ADDRSTRLEN];
    int socket_fd;
    int yes = 1;
    memset(&pattern, 0, sizeof pattern);
    pattern.ai_family = AF_UNSPEC;
    pattern.ai_socktype = SOCK_STREAM;
    pattern.ai_flags = AI_PASSIVE;
    int status = getaddrinfo(NULL, port, &pattern, &server_info);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 1;
    }
    for (p = server_info; p != NULL; p = p->ai_next)
    {
        socket_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (socket_fd == - 1)
        {
            perror("server: socket");
            continue;
        }
        if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes,sizeof(int)) == -1)
        {
            perror("setsockopt");
            exit(1);
        }
        if (bind(socket_fd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(socket_fd);
            perror("server: bind");
            continue;
        }
        break;
    }
    if (p == NULL)
    {
        fprintf(stderr, "server: failed to bind\n");
        return 2;
    }
    freeaddrinfo(server_info);
    if (listen(socket_fd, COUNTOFWAITINGCONNECTIONS) == -1)
    {
        perror("listen");
        exit(1);
    }
    while (1)
    {
        struct sockaddr_storage connection_addr;
        socklen_t sin_size = sizeof connection_addr;
        int connection_fd = accept(socket_fd, (struct sockaddr *)&connection_addr, &sin_size);
        if (connection_fd == -1)
        {
            perror("accept");
            continue;
        }
        inet_ntop(connection_addr.ss_family,
                  get_in_addr((struct sockaddr *)&connection_addr),
                  s, sizeof s);
        if (!fork())
        {
            close(socket_fd);

            close(connection_fd);
            exit(0);
        }
        close(connection_fd);
    }
}