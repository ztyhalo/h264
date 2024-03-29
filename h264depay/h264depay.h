#ifndef H264DEPAY_H
#define H264DEPAY_H

#include <string.h>
#include "mutex/mutex_class.h"

#include "rtp/rtpinfo.h"
#include "vpudec/vpudec.h"
#include "epoll/e_poll.h"
#include <semaphore.h>
#include "rtp/rtp.h"

#define H264_MAX_FRAME 120
#define FRAME_MAX_NUM  260  //260
#define H264_DATA_SIZE 1248
#define FRAME_HEAR_OFF 4


#define H264_HEAD_MIN_LEN 12
#define H264_SPSPPS_LEN   64

typedef struct
{
    unsigned char data[H264_SPSPPS_LEN];		/*buffer virtual addr*/
    unsigned int nSize;		/*valid data length */
}SPSPPSInfo;

enum {
    H264_DATA_TYPE = VPU_V_AVC,
    JPG_DATA_TYPE = VPU_V_MJPG
};

class H264Buf:public MUTEX_CLASS
{
public:

    uint8_t data[H264_MAX_FRAME][FRAME_MAX_NUM * H264_DATA_SIZE];
    uint32_t bufsize[H264_MAX_FRAME];
    uint32_t seqid[H264_MAX_FRAME];
    uint8_t m_type[H264_MAX_FRAME];  //数据类型
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
    int write_h264data_buf(uint8_t * buf, int size, int q);
//    int read_h264buf(uint8_t * buf);
    int get_h264buf(uint8_t * * addr, uint8_t * ty);
    int get_write_h264buf(uint8_t * * addr);
    int write_h264buf(int size, uint8_t dataty = 0);
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
    uint8_t  m_buf[FRAME_MAX_NUM * H264_DATA_SIZE];
    int      m_size;
    int      m_runok;
    int      m_streamtype;
    SPSPPSInfo m_info;

    int     m_spsmark;
    int     m_ppsmark;


    H264Buf * h264buf;
    VpuDec *  vpudec;

    int      proframe;

    int data_porcess(uint8_t * buf, int size);
    int set_sps_info(uint8_t * buf, int size);
    int set_pps_info(uint8_t * buf, int size);
    int data_parse(uint8_t * buf, int size, int q, int drop);

    void rtp_h264_init(void * pro);
    void rtp_h264_init(void);
    void run();



};

int h264_pro_rxdata_callback(RTP * pro, void * buf, int size);

#endif // H264DEPAY_H
