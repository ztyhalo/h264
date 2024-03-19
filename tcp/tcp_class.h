#ifndef TCP_CLASS_H
#define TCP_CLASS_H

#include "socket/socket.h"

class TCP_CLASS:public SOCKET_ADDR_CLASS,public SOCKET_FD_CLASS
{
public:
    TCP_CLASS();
    ~TCP_CLASS();

    TCP_CLASS(uint16_t port):SOCKET_ADDR_CLASS(port)
    {
        creat_socket_fd(SOCK_STREAM);
    }
    TCP_CLASS(uint16_t port, char * ip):SOCKET_ADDR_CLASS(port,ip)
    {
        creat_socket_fd(SOCK_STREAM);
    }

    void tcp_class_init(uint16_t port)
    {
        socket_add_init(port);
        creat_socket_fd(SOCK_STREAM);

    }
    void tcp_class_init(uint16_t port, const char * ip)
    {
        socket_add_init(port, ip);
        creat_socket_fd(SOCK_STREAM);
    }

    int tcp_write(void * buf, size_t n);
    int tcp_read(void * buf, size_t n);
    int tcp_recv(void * buf, size_t n);
//    int udp_send(void * buf, size_t n);
//    int udp_read_init(void);
//    int udp_read(void * buf, size_t n);
//    int udp_read(void * buf, size_t n, struct sockaddr_in * soure);
//    void bind_port(uint16_t port);
};

#endif // TCP_CLASS_H
