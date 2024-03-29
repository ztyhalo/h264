#ifndef __UDP_H__
#define __UDP_H__

#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
//#include <QObject>
#include "socket/socket.h"
#include "zprint/zprint.h"


class UDP_CLASS:public SOCKET_ADDR_CLASS,public SOCKET_FD_CLASS
{
public:
     SOCKET_ADDR_CLASS localaddr;
public:
    UDP_CLASS(){
        ;
    }
    UDP_CLASS(uint16_t port):SOCKET_ADDR_CLASS(port)
    {
        creat_socket_fd(SOCK_DGRAM);
    }
    UDP_CLASS(uint16_t port, char * ip):SOCKET_ADDR_CLASS(port,ip)
    {
        creat_socket_fd(SOCK_DGRAM);
    }
    ~UDP_CLASS()
    {
         zprintf4("UDP_CLASS delete!\n");
    }

    void udp_class_init(uint16_t port)
    {
        socket_add_init(port);
        creat_socket_fd(SOCK_DGRAM);

    }
    void udp_class_init(uint16_t port, char * ip)
    {
        socket_add_init(port, ip);
        creat_socket_fd(SOCK_DGRAM);
    }
    int udp_send(void * buf, size_t n);
    int udp_read_init(void);
    int udp_read(void * buf, size_t n);
    int udp_read(void * buf, size_t n, struct sockaddr_in * soure);
    void bind_port(uint16_t port);
    void set_timeover(int time);
    void set_mstimeover(int ms);

};

class UDP_MULTI_CLASS:public UDP_CLASS
{
private:
    struct ip_mreq join_addr;
    int live_time;
public:
    UDP_MULTI_CLASS(){
        ;
    }
    UDP_MULTI_CLASS(uint16_t port):UDP_CLASS(port)
    {
        ;
    }
    UDP_MULTI_CLASS(uint16_t port,char * ip):UDP_CLASS(port,ip)
    {
        ;
    }
    int udp_mul_read_init(char * group, char * localip);
    int udp_mul_read_init(char * group);
    void udp_mul_send_init(int time);


};

class UDP_BROAD_CLASS:public UDP_CLASS
{
private:
    int live_time;
    int opt_so_broadcast;
public:
    UDP_BROAD_CLASS(){
        ;
    }
    UDP_BROAD_CLASS(uint16_t port):UDP_CLASS(port)
    {
        ;
    }
    UDP_BROAD_CLASS(uint16_t port, char * ip):UDP_CLASS(port, ip)
    {
        ;
    }

    void udp_broad_send_init(int time);
};


#endif //__UDP_H__
