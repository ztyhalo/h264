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
    uint                 m_maxnum;
    SUPPLEM              m_supm[N];

public:
    ZBufModel()
    {
        m_wr = 0;
        m_rd = 0;
        m_num = 0;
        memset(m_size, 0x00, sizeof(m_size));
        memset(m_buf, 0x00, sizeof(m_buf));
        memset(m_supm,0x00, sizeof(m_supm));
    }
    ~ZBufModel()
    {
        ;
    }

    int buf_basewrite_data(DTYPE * val, int num, SUPPLEM para);
    int buf_write_data(DTYPE * val, int num, SUPPLEM para);
    int buf_write_data_from_file(FILE * fp, int num, SUPPLEM para);
    int get_buf_data(DTYPE ** addr, SUPPLEM * para);
    int add_buf_rd(void);

};

template<class DTYPE, class SUPPLEM,int N, int SIZE>
int ZBufModel<DTYPE,SUPPLEM,N,SIZE>::buf_basewrite_data(DTYPE * val, int num, SUPPLEM para)
{
    if(m_num >= N)
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
    m_supm[m_wr] = para;
    m_wr++;
    m_wr %= N;
    m_num++;
    if(m_num > m_maxnum)
    {
        m_maxnum = m_num;
        zprintf1("buf max write is %d!\n", m_maxnum);
    }

    return 0;
}

template<class DTYPE, class SUPPLEM, int N, int SIZE>
int ZBufModel<DTYPE,SUPPLEM,N,SIZE>::buf_write_data(DTYPE * val, int num, SUPPLEM para)
{
    int err;
    lock();
    err = buf_basewrite_data(val, num, para);
    unlock();
    return err;
}
template<class DTYPE, class SUPPLEM, int N, int SIZE>
int ZBufModel<DTYPE,SUPPLEM,N,SIZE>::buf_write_data_from_file(FILE * fp, int num, SUPPLEM para)
{
    int err = 0;

    if(fp == NULL)
        return -1;
    lock();

    if(m_num >= N)
    {
        zprintf1("Write buf over!\n");
        err =  -2;
        goto WRITEEND;
    }
    if(num > SIZE)
    {
        zprintf1("Write data over!\n");
        err = -3;
        goto WRITEEND;
    }

    err = fread(m_buf[m_wr], 1, num, fp);

    if(err > 0 && err <= num)
    {
        zprintf4("zty test read %d ok!\n", err);
    }
    else
    {
        zprintf1("zty file read %d error!\n", err);
        err = -5;
        goto WRITEEND;
    }
    m_size[m_wr] = num;
    m_supm[m_wr] = para;
    m_wr++;
    m_wr %= N;
    m_num++;

WRITEEND:
    unlock();

    return err;
}

template<class DTYPE, class SUPPLEM, int N, int SIZE>
int ZBufModel<DTYPE,SUPPLEM,N,SIZE>::get_buf_data(DTYPE ** addr, SUPPLEM * para)
{
    int size = 0;

    lock();
    if(m_num > 0)
    {
        *addr = m_buf[m_rd];
        size = m_size[m_rd];
        *para = m_supm[m_rd];
    }
    unlock();

    return size;

}

template<class DTYPE, class SUPPLEM, int N, int SIZE>
int ZBufModel<DTYPE,SUPPLEM,N,SIZE>::add_buf_rd(void)
{
    int err = 0;
    lock();
    if(m_num >0)
    {
        m_rd++;
        m_rd %= N;
        m_num--;
    }
    else
    {
        zprintf1("zbufmodel add buf rd error!\n");
        err = -1;
    }
    unlock();
    return err;
}


#endif
