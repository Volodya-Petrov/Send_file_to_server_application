#include <stdio.h>
#include "server.h"

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        fprintf(stderr, "Not enough arguments. Enter the port and directory name");
        return 1;
    }
    start_server(argv[1], argv[2]);
}
