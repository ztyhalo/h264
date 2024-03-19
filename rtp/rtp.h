#ifndef RTP_H
#define RTP_H

#include "epoll/e_poll.h"
#include "udp/udp.h"
using namespace std;



class RTP:public UDP_CLASS, public NCbk_Poll
{
public:
    RTP(int fnum =1):UDP_CLASS(),NCbk_Poll(fnum)
    {
        seqnum = 0;
        next_seq = 0;
        next_p = NULL;
    }
    ~RTP();
public:
    string session;
    int port;
    int seqnum;
    int next_seq;
    void * next_p;  //协议格式
//    int first;

    int (*rxcallback)(RTP * pro, void* data, int n);
    int rtp_init(string sess, int port);
    void set_protocol(void * pro);
    void run();

};

#endif // RTP_H
