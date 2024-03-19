#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include "tcp_class.h"

class TCP_CLIENT:public TCP_CLASS
{
public:
    TCP_CLIENT();
     ~TCP_CLIENT();
    TCP_CLIENT(uint16_t port):TCP_CLASS(port)
    {
        ;
    }
    TCP_CLIENT(uint16_t port, char * ip):TCP_CLASS(port,ip)
    {
       ;
    }

    void tcp_client_init(uint16_t port)
    {
        tcp_class_init(port);

    }
    void tcp_client_init(uint16_t port,const  char * ip)
    {
        tcp_class_init(port, ip);
    }
    int tcp_client_connect(void);


};

#endif // TCP_CLIENT_H
