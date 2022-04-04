#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include "server.h"

#define COUNT_OF_WAITING_CONNECTIONS  10
#define FILE_NAME_SIZE 20
#define FILE_PART_SIZE 512

void *get_in_addr(struct sockaddr *sa)
{

    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int file_loading(int connection_fd, char* directory_name)
{
    char buffer_for_name[FILE_NAME_SIZE + 1];
    char buffer_for_file[FILE_PART_SIZE];
    ssize_t read_bytes;
    ssize_t write_bytes;
    read_bytes = recv(connection_fd, buffer_for_name, FILE_NAME_SIZE, 0);
    if (read_bytes < 0)
    {
        perror("Failed get file name");
        close(connection_fd);
        return 1;
    }
    buffer_for_name[read_bytes] = '\0';
    char* path = calloc(strlen(directory_name) + strlen(buffer_for_name) + 1, sizeof(char));
    strcat(path, directory_name);
    strcat(path, "/");
    strcat(path, buffer_for_name);
    int fopen = open(path, O_CREAT|O_EXCL|O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
    if (fopen < 0)
    {
        perror("File create error");
        send(connection_fd, "n", 1, 0);
        close(connection_fd);
        free(path);
        return 1;
    }
    if (send(connection_fd, "y", 1, 0) < 0)
    {
        perror("Failed response");
        close(connection_fd);
        free(path);
        return 1;
    }
    while ((read_bytes = recv(connection_fd, buffer_for_file, FILE_PART_SIZE, 0)) > 0)
    {
        write_bytes = write(fopen, buffer_for_file, read_bytes);
        if (write_bytes != read_bytes)
        {
            perror("Write in file error");
            write_bytes = -1;
            break;
        }
    }
    close(fopen);
    close(connection_fd);
    if (read_bytes < 0 || write_bytes < 0)
    {
        if (remove(path) == -1)
        {
            perror("Remove file error");
        }
        free(path);
        return 1;
    }
    free(path);
    return 0;
}

int start_server(const char* port, char* directory_name)
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
        exit(1);
    }
    freeaddrinfo(server_info);
    if (listen(socket_fd, COUNT_OF_WAITING_CONNECTIONS) == -1)
    {
        perror("listen");
        exit(1);
    }
    if (mkdir(directory_name, S_IRWXU | S_IRWXG | S_IRWXO) != 0 && errno != EEXIST)
    {
        perror("Failed to create directory");
        exit(1);
    }
    printf("Server listening on port:%s", port);
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

        file_loading(connection_fd, directory_name);
    }
}
