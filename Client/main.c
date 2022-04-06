#include <stdio.h>
#include "client.h"
int main(int argc, char **argv)
{
    if (argc < 4)
    {
        fprintf(stderr, "Not enough arguments. Enter the ip, port, path to the file, the name with which the file will be created on the server\n");
    }
    send_file_to_server(argv[0], argv[1], argv[2], argv[3]);
    return 0;
}
