#include <stdio.h>
#include "server.h"

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Not enough arguments. Enter the port and directory name");
        return 1;
    }
    start_server(argv[0], argv[1]);
}
