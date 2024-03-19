#include "tcp_client.h"
#include <stdio.h>
#include <errno.h>
TCP_CLIENT::TCP_CLIENT()
{
    ;
}

TCP_CLIENT::~TCP_CLIENT()
{
    cout << "TCP_CLIENT delete!" << endl;;
}


int TCP_CLIENT::tcp_client_connect(void)
{
    int ret;
    ret = connect(socket_fd,(struct sockaddr *)&addr, sizeof(addr));
    if(ret < 0)
    {
        printf("tcp connect failed, errno:%d, %s\n", errno, strerror(errno));
    }
    return ret;
}
