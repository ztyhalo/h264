#ifndef NETLINKSTATUS_H
#define NETLINKSTATUS_H
#include "epoll/e_poll.h"
#include <vector>

struct  NetInfo
{
    char m_name[32]; //网卡名称
    char m_ip[16];   //ip地址
    char m_netmask[16];
    char m_gw[16];
    char m_dns[16];
    int  m_speed;
    int  m_status;
};

class NetlinkStatus:public Pth_Class
{
public:
    NetlinkStatus(string ethname = "eth1");
//    ~NetlinkStatus();
    virtual ~NetlinkStatus();
    int getLinkstate(void);
    void run();
    int getNetInfo(void);
    int getNetState(string name, struct  NetInfo & val);
    int (*m_netlinkcb)(NetlinkStatus * pro, int s);
public:
    void * m_linkFather;

private:
    int     m_linkState;
    string  m_eth;
    int     m_nlSock;
    vector<struct  NetInfo> m_netInfo;

};

#endif // NETLINKSTATUS_H
