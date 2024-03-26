#ifndef NETLINKSTATUS_H
#define NETLINKSTATUS_H
#include "epoll/e_poll.h"

class NetlinkStatus:public Pth_Class
{
public:
    NetlinkStatus(string ethname = "eth1");
//    ~NetlinkStatus();
    virtual ~NetlinkStatus();
    int getLinkstate(void);
    void run();

    int (*netlinkcb)(NetlinkStatus * pro, int s);
public:
    void * netlinkfater;

private:
    int linkstate;
    string eth;
    int nl_sock;


};

#endif // NETLINKSTATUS_H
