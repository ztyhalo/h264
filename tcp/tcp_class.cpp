#include "tcp_class.h"

TCP_CLASS::TCP_CLASS()
{

}
TCP_CLASS::~TCP_CLASS()
{
    zprintf3("tcp class delete!\n");
}

int TCP_CLASS::tcp_write(void * buf, size_t n)
{
    if(socket_fd < 0)
        return -1;
    if(buf == NULL)
        return -2;
    return write(socket_fd, buf, n);
}

int TCP_CLASS::tcp_read(void * buf, size_t n)
{
    return read(socket_fd, buf, n);
}

int TCP_CLASS::tcp_recv(void * buf, size_t n)
{
    return recv(socket_fd,  buf, n, 0);
}
