#include "rtp.h"

RTP::~RTP()
{
    cout << "delete rtp " <<endl;
    stop();
    rxcallback = NULL;
    netstatecb = NULL;
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

int  RTP::rtp_run_stop(void)
{
    running = 0;
    return 0;
}

void RTP::run()
{

    char buf[2048];
    int ret;
    int overnum = 0;
//    this->set_timeover(5);
    while (running)
    {

        if(wait_fd_change(1000) != -1)
        {
           overnum = 0;

           if(state != 1)
           {
                state = 1; //已经有数据
//                printf("zty send rtp have data!\n");
                if(this->netstatecb != NULL)
                {
                    this->netstatecb(this, state);
                }
           }


           while((ret = read(socket_fd, buf,sizeof(buf))) >0)
           {
//               printf("rtp have data!\n");
               if(this->rxcallback != NULL)
               {
//                   printf("rtp have data process!\n");
                    this->rxcallback(this, buf, ret);
               }
           }

        }
        else
        {
            overnum++;
            if(overnum >3)
            {
                printf("wati over!\n");
                overnum = 0;
                if(state != 0)
                {
                    state = 0;
                    if(this->netstatecb != NULL)
                    {
                        this->netstatecb(this, state);
                    }
                }
            }
        }
    }
}
