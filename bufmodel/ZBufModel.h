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
    uint                 m_end;
    SUPPLEM              m_supm[N];

public:
    ZBufModel()
    {
        m_wr = 0;
        m_rd = 0;
        m_num = 0;
        m_end = 0;
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
    if((m_wr == 0) && (m_end == 1))
    {
        int i;
        int j;
        int err = 0;
        FILE * g_fpd;
        zprintf4("write start!\n");
        g_fpd = fopen("/opt/lsh264", "w");
        if(NULL == g_fpd)
        {
            zprintf4("file open error end!\n");
            return 0;
        }
        for(i = 0; i < N; i++)
        {
            zprintf4("write size %d!\n", m_size[i]);
            err = fwrite(m_size+i, 1, sizeof(uint), g_fpd);
            if(err == -1)
            {
                zprintf4("write file error!\n");
            }
            err = fwrite(m_buf[i], 1, m_size[i], g_fpd);
            if(err == -1)
            {
                zprintf4("write buf file error!\n");
            }
        }
        zprintf4("write end!\n");
        m_end = 1;
        fclose(g_fpd);
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
    m_size[m_wr] = err;
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
