#include "rtp.h"

RTP::~RTP()
{
    cout << "delete rtp " <<endl;
    stop();
    rxcallback = NULL;
}


int RTP::rtp_init(string sess, int port)
{
    session = sess;
    udp_class_init(port);
    udp_read_init();
    e_poll_add(socket_fd);

    return 0;
}

void RTP::set_protocol(void * pro)
{
    next_p = pro;
}

void RTP::run()
{

    char buf[2048];
    int ret;
//    this->set_timeover(5);
    while (1)
    {

        if(wait_fd_change(1000) != -1)
        {

           while((ret = read(socket_fd, buf,sizeof(buf))) >0)
           {
               if(this->rxcallback != NULL)
                    this->rxcallback(this, buf, ret);
           }

        }
        else
        {
            printf("wati over!\n");
        }
    }
}
