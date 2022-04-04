#include <stdio.h>
#include "client.h"
int main()
{
    send_file_to_server("127.0.0.1", "1488", "/home/devyatka/programming/send-file-to-server-application/Client/sss.txt", "test.txt");
    return 0;
}
