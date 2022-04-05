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
#include <pthread.h>
#include "server.h"

#define COUNT_OF_WAITING_CONNECTIONS  10
#define FILE_NAME_SIZE 20
#define FILE_PART_SIZE 512
#define THREADS_COUNT 5

pthread_mutex_t file_mutex;
pthread_mutex_t accept_mutex;
int listen_fd;
char* directory_name;

void *get_in_addr(struct sockaddr *sa)
{

    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int create_file(char* file_name, char* final_path)
{
    final_path = calloc(strlen(directory_name) + strlen(file_name) + 2, sizeof(char));
    strcat(final_path, directory_name);
    strcat(final_path, "/");
    strcat(final_path, file_name);
    pthread_mutex_lock(&file_mutex);
    int fopen = open(final_path, O_CREAT | O_EXCL | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
    pthread_mutex_unlock(&file_mutex);
    return fopen;
}

int file_loading(int connection_fd)
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
    char* path;
    int fopen = create_file(buffer_for_name, path);
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

int listen_on_port(const char* port)
{
    struct addrinfo pattern, *server_info, *p;
    int socket_fd;
    int yes = 1;
    memset(&pattern, 0, sizeof pattern);
    pattern.ai_family = AF_UNSPEC;
    pattern.ai_socktype = SOCK_STREAM;
    pattern.ai_flags = AI_PASSIVE;
    int status = getaddrinfo(NULL, port, &pattern, &server_info);
    if (status != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        exit(1);
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
    return socket_fd;
}

void* work_with_client()
{
    while(1)
    {
        struct sockaddr_storage connection_addr;
        socklen_t sin_size = sizeof connection_addr;
        char s[INET6_ADDRSTRLEN];
        pthread_mutex_lock(&accept_mutex);
        int connection_fd = accept(listen_fd, (struct sockaddr *)&connection_addr, &sin_size);
        pthread_mutex_unlock(&accept_mutex);
        if (connection_fd == -1)
        {
            perror("accept");
            continue;
        }
        inet_ntop(connection_addr.ss_family,
                  get_in_addr((struct sockaddr *)&connection_addr),
                  s, sizeof s);

        file_loading(connection_fd);
    }
}

int start_server(const char* port, char* directory)
{
    pthread_t threads[THREADS_COUNT];
    directory_name = directory;
    listen_fd = listen_on_port(port);
    if (mkdir(directory_name, S_IRWXU | S_IRWXG | S_IRWXO) != 0 && errno != EEXIST)
    {
        perror("Failed to create directory");
        exit(1);
    }
    pthread_mutex_init(&accept_mutex, NULL);
    pthread_mutex_init(&file_mutex, NULL);
    for (int i = 0; i < THREADS_COUNT; i++)
    {
        pthread_create(&threads[i], NULL, work_with_client, NULL);
    }
    for (int i = 0; i < THREADS_COUNT; i++)
    {
        pthread_join(threads[i], NULL);
    }
    close(listen_fd);
    return 0;
}
