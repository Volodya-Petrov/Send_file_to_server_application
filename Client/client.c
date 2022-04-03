#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdlib.h>
#include "client.h"

#define FILE_NAME_SIZE 15
#define FILE_PART_SIZE 512

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int file_load(int connection_fd, int fopen, char* file_name)
{
    ssize_t read_bytes;
    ssize_t write_bytes;
    char response_buffer[2];
    char buffer[FILE_PART_SIZE];
    write_bytes = send(connection_fd, file_name, strlen(file_name), 0);
    if (write_bytes < 0)
    {
        perror("Send file name error");
        return 1;
    }
    read_bytes = recv(connection_fd, response_buffer, 1, 0);
    if (read_bytes < 1)
    {
        sprintf(stderr, "Response failed");
        return 1;
    }
    if (response_buffer[0] != 'y')
    {
        sprintf(stderr, "Failed to create a file by the specified name, try another name");
        return 1;
    }
    while ((read_bytes = read(fopen, buffer, FILE_PART_SIZE)) > 0)
    {
        write_bytes = send(connection_fd, buffer, read_bytes, 0);
        if (write_bytes != read_bytes)
        {
            sprintf(stderr, "Sending file failed");
            return 1;
        }
    }
    if (read_bytes < 0)
    {
        perror("Error read data from file");
        return 1;
    }
    return 0;
}

int send_file_to_server(char* ip, char* port, char* file_path, char* file_name)
{
    struct addrinfo pattern, *server_info, *p;
    char s[INET6_ADDRSTRLEN];
    int socket_fd;
    memset(&pattern, 0, sizeof pattern);
    pattern.ai_family = AF_UNSPEC;
    pattern.ai_socktype = SOCK_STREAM;
    if (strlen(file_name) > FILE_NAME_SIZE)
    {
        fprintf(stderr, "Max file name size is 15");
        return 1;
    }
    int fopen = open(file_path, O_RDONLY);
    if (fopen < 0)
    {
        perror("Open file");
        exit(1);
    }
    int status = getaddrinfo(ip, port, &pattern, &server_info);
    if (status != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        exit(1);
    }
    for (p = server_info; p != NULL; p = p->ai_next)
    {
        socket_fd = socket(p->ai_family, p->ai_socktype,p->ai_protocol);
        if (socket_fd == -1)
        {
            perror("client: socket");
            continue;
        }

        if (connect(socket_fd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(socket_fd);
            perror("client: connect");
            continue;
        }

        break;
    }
    if (p == NULL)
    {
        fprintf(stderr, "client: failed to connect\n");
        exit(1);
    }
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s);
    freeaddrinfo(server_info);
    int result = file_load(socket_fd, fopen, file_name);
    exit(result);
}