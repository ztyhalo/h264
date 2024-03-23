#ifndef NETLINKSTATUS_H
#define NETLINKSTATUS_H
#include "epoll/e_poll.h"

class NetlinkStatus:public Pth_Class
{
public:
    NetlinkStatus(string ethname = "eth1");
//    ~NetlinkStatus();
    virtual ~NetlinkStatus(){
        zprintf1("delete virtual netlinkstatus!\n");
    }
    int getLinkstate(void);
    void run();

private:
    int linkstate;
    string eth;


};

#endif // NETLINKSTATUS_H
