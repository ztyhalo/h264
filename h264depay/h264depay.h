#ifndef H264DEPAY_H
#define H264DEPAY_H

#include <string.h>
#include "mutex/mutex_class.h"

#include "rtp/rtpinfo.h"
#include "vpudec/vpudec.h"
#include "epoll/e_poll.h"
#include <semaphore.h>
#include "rtp/rtp.h"

#define H264_MAX_FRAME 150
#define FRAME_MAX_NUM  200  //260
#define H264_DATA_SIZE 1248
#define FRAME_HEAR_OFF 4


#define H264_HEAD_MIN_LEN 12
#define H264_SPSPPS_LEN   64

typedef struct
{
    unsigned char data[H264_SPSPPS_LEN];		/*buffer virtual addr*/
    unsigned int nSize;		/*valid data length */
}SPSPPSInfo;

class H264Buf:public MUTEX_CLASS
{
public:

    uint8_t data[H264_MAX_FRAME][FRAME_MAX_NUM * H264_DATA_SIZE];
    uint32_t bufsize[H264_MAX_FRAME];
    uint32_t seqid[H264_MAX_FRAME];
//    uint8_t savedata[FRAME_MAX_NUM * H264_DATA_SIZE];
//    uint32_t savesize;
    int  wr;
    int  rd;
    int  sz;
    int wr_mark;
    int stream_type;
    int receive_frame;
    SPSPPSInfo info;
    sem_t                mgsem;
    int   spsmark;
    int ppsmark;
    int runok;
public:
    H264Buf();
    ~H264Buf()
    {
        sem_destroy(&mgsem);
    }
    void print_h264data(void);
    int write_h264buf(uint8_t * buf, int size, int q, int drop);
//    int read_h264buf(uint8_t * buf);
    int get_h264buf(uint8_t * * addr);
    int get_write_h264buf(uint8_t * * addr);
    int write_h264buf(int size);
    int add_buf_rd(void);
    int set_sps_info(uint8_t * buf, int size);
    int set_pps_info(uint8_t * buf, int size);

};


class H264Depay:public Pth_Class
{
public:
    H264Depay();
    virtual ~H264Depay();
public:
    uint16_t seq;
    uint16_t next_seq;
    H264Buf * h264buf;
    VpuDec *  vpudec;

    int      proframe;

    int data_porcess(uint8_t * buf, int size);

    void rtp_h264_init(void * pro);
    void rtp_h264_init(void);
    void run();



};

int h264_pro_rxdata_callback(RTP * pro, void * buf, int size);

#endif // H264DEPAY_H
