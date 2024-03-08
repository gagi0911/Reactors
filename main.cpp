#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "Tcp/TcpServer.h"
int main(int argc, char* argv[])
{
#if 0
    if (argc < 3)
    {
        printf("./a.out port path\n");
        return -1;
    }
    unsigned short port = atoi(argv[1]);
    // change to target file path
    chdir(argv[2]);
    //initialize listen socket
#else
    unsigned short port = 8000;
    chdir("/home/seed/web");
#endif
    TcpServer* server = new TcpServer(port, 4);
    server->Run();
    return 0;
}