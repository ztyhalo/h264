#ifndef RTP_H
#define RTP_H

#include "epoll/e_poll.h"
#include "udp/udp.h"
using namespace std;



class RTP:public NCbk_Poll,public UDP_CLASS
{
public:
    RTP(int fnum =1):NCbk_Poll(fnum),UDP_CLASS()
    {
        seqnum = 0;
        next_seq = 0;
        next_p = NULL;
        state = 0;
        rxcallback = NULL;
        net_father = NULL;

        netstatecb = NULL;

    }
    virtual ~RTP();
public:
    string session;
    int port;
    int seqnum;
    int next_seq;
    void * next_p;  //协议格式
    void * net_father;  //网络状态通知
//    int first;
    int state;     //通讯连接状态


    int (*rxcallback)(RTP * pro, void* data, int n);
    int (*netstatecb)(RTP * pro, int s);
    int rtp_init(string sess, int port);
    void set_protocol(void * pro);
    void run();

};

#endif // RTP_H
