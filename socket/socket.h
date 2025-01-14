#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
//#include <QObject>
#include <iostream>
#include "zprint/zprint.h"

using namespace std;

class SOCKET_ADDR_CLASS
{
public:
    struct sockaddr_in addr;

public:
    SOCKET_ADDR_CLASS()
    {
        socket_add_init();
    }
    SOCKET_ADDR_CLASS(uint16_t port, char * ip)
    {
        socket_add_init(port, ip);
    }
    SOCKET_ADDR_CLASS(uint16_t port)
    {
       socket_add_init(port);
    }
    ~SOCKET_ADDR_CLASS()
    {
        zprintf3("socket add class delete!\n");
    }

    void socket_add_init(void)
    {
        memset(&addr, 0x00, sizeof(addr));
        addr.sin_family = AF_INET;
    }

    void socket_add_init(uint16_t port)
    {
        socket_add_init();
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);

    }
    void socket_add_init(uint16_t port, const char * ip)
    {
        socket_add_init();
        addr.sin_addr.s_addr = inet_addr(ip);
        addr.sin_port = htons(port);
    }
};

class SOCKET_FD_CLASS
{
public:
     int                socket_fd;
public:
     SOCKET_FD_CLASS()
     {
         socket_fd = 0;
     }
     ~SOCKET_FD_CLASS()
     {
         zprintf3("SOCKET_FD_CLASS %d delete!\n", socket_fd);
         if(socket_fd > 0)
         {
             close(socket_fd);
             socket_fd = 0;
         }
     }
     int creat_socket_fd(int type)
     {
         socket_fd = socket(PF_INET, type, 0);
         if(socket_fd < 0)
         {
             perror("socket");
             return -1;
         }
         return 0;
     }
     void close_fd(void)
     {
         if(socket_fd > 0){
             shutdown(socket_fd, SHUT_WR);
             close(socket_fd);
             socket_fd = 0;
         }
     }

};




#endif //__SOCKET_H__
