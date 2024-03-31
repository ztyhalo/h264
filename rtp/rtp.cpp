#include "rtp.h"

RTP::~RTP()
{
    zprintf4("delete rtp");
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

                if(this->netstatecb != NULL)
                {
                    this->netstatecb(this, state);
                }
           }


           while((ret = read(socket_fd, buf,sizeof(buf))) >0)
           {
               if(this->rxcallback != NULL)
               {
                    this->rxcallback(this, buf, ret);
               }
           }

        }
        else
        {
            overnum++;
            if(overnum > 3)
            {
                zprintf1("wati over!\n");
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
