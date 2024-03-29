#ifndef Z_BUF_MODEL_H
#define Z_BUF_MODEL_H

#include "zprint/zprint.h"
#include "mutex/mutex_class.h"

template<class DTYPE,class SUPPLEM, int N = 2, int SIZE = 2048>
class ZBufModel:public MUTEX_CLASS
{
public:
    DTYPE                m_buf[N][SIZE];
    uint                 m_size[N];
    uint                 m_wr;
    uint                 m_rd;
    uint                 m_num;
    SUPPLEM              m_supm[N];

public:
    ZBufModel()
    {
        m_wr = 0;
        m_rd = 0;
        m_size = 0;
        memset(m_size, 0x00, sizeof(m_size));
        memset(m_buf, 0x00, sizeof(m_buf));
        memset(m_supm,0x00, sizeof(m_supm));
    }
    ~ZBufModel()
    {
        ;
    }

    int buf_basewrite_data(DTYPE * val, int num);
    int buf_write_data(DTYPE * val, int num);

};

template<class DTYPE, class SUPPLEM,int N, int SIZE>
int ZBufModel<DTYPE,SUPPLEM,N,SIZE>::buf_basewrite_data(DTYPE * val, int num)
{
    if(m_size >= N)
    {
        zprintf1("Write buf over!\n");
        return -1;
    }
    if(sizeof(DTYPE)*num > SIZE)
    {
        zprintf1("Write data over!\n");
        return -2;
    }
    memcpy(m_buf[m_wr], val, sizeof(DTYPE)*num);
    m_size[m_wr] = sizeof(DTYPE)*num;
    m_wr++;
    m_wr %= N;

    return 0;
}

template<class DTYPE, class SUPPLEM, int N, int SIZE>
int ZBufModel<DTYPE,SUPPLEM,N,SIZE>::buf_write_data(DTYPE * val, int num)
{
    int err;
    lock();
    err = buf_basewrite_data(val, num);
    unlock();
    return err;
}


#endif
