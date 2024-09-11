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



class H264Depay
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

    VpuDec *  vpudec;

    int      proframe;
    FILE  *   m_fp ;

    int data_porcess(uint8_t * buf, int size);
    int data_parse(uint8_t * buf, int size, int q, int drop);

    void rtp_h264_init(void);
    void rtp_restart_process(void);

};

int h264_pro_rxdata_callback(RTP * pro, void * buf, int size);

#endif // H264DEPAY_H
