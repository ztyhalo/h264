
#include "h264depay.h"
#include "rtp/rtp.h"
#include <byteswap.h>

static const uint8_t sync_bytes[] = { 0, 0, 0, 1 };


//static const uint8_t ppsspsinfo[] = {0x1,0x64,0x0,0x2a,0xff,0xe1,0x0,0x1a,0x67,0x64,0x0,0x2a,0xac,0x2c,0x6a,0x81,0xe0,0x8,0x9f,
//                                     0x96,0x6a,0x2,0x2,0x2,0x80,0x0,0x0,0x3,
//                                     0x0,0x80,0x0,0x0,0x19,0x42,0x1,0x0,0x5,0x68,0xee,0x31,0xb2,0x1b};
template <typename Tbyteswap>

void tbyte_swap(Tbyteswap * turndata, uint8_t size)
{
    int i;
    int bytesize = 0;
    Tbyteswap * midvp = NULL;

    bytesize = sizeof(Tbyteswap);
    if(size%bytesize)
        return;
    if(turndata == NULL)
        return;
    midvp = (Tbyteswap *)turndata;
    for(i = 0; i < size/bytesize; i++, midvp++)
    {
        if(bytesize == 4)
            *midvp = bswap_32((*midvp));
        else if(bytesize == 2)
            *midvp = bswap_16((*midvp));
    }
}

void data_printf(char * mes, uint8_t * buf, int size)
{
    int i;
    printf("%s %d!\n", mes, size);
    for(i = 0; i < size; i++)
    {
        printf("0x%x ", buf[i]);
    }
    printf("\n");
}

H264Buf::H264Buf()
{
    wr = 0;
    rd = 0;
    sz = 0;
    wr_mark = 0;
    stream_type = 0;
    receive_frame = 0;
    ppsmark = 0;
    spsmark = 0;
    runok = true;
    memset(bufsize, 0, sizeof(bufsize));
    memset(data, 0, sizeof(data));
    memset(m_type, 0, sizeof(m_type));
    sem_init(&mgsem, 0, 0);
}

void H264Buf::print_h264data(void)
{
//    int i;
//    int j;
//    int len = bufsize[wr] > 30 ? 30 : bufsize[wr];
//    if(linshibug == 0)
//    {
//        len = bufsize[wr];
//        linshibug = 1;
//    }
//    else
//        return ;
//    printf("receive h264 frame %d!\n", bufsize[wr]);

//    j= 0;
//    for(i = 0; i < len; i++)
//    {
//        printf("0x%x ", data[wr][i]);
//                         if(j >15)
//                         {
//                             printf("\n");
//                             j = 0;
//                         }
//                         j++;
//    }
//    printf("\n");
}

int H264Buf::set_sps_info(uint8_t * buf, int size)
{

    info.data[0] = 0x1;
    info.data[1] = buf[5]; //profile
    info.data[2] = buf[6]; //profile compat
    info.data[3] = buf[7]; //level
    info.data[4] = 0xff;   /* 6 bits reserved | 2 bits lengthSizeMinusOn */
//    info.data[5] = 0xe0 | buf[3];
    info.data[5] = 0xe1;  /* [5]: 3 bits reserved (111) + 5 bits number of sps (00001) */

    memcpy(info.data +6, buf +2, size -2);
    info.nSize = 6 + size -2;
//    printf("set sps info len %d!\n", info.nSize);
//    data_printf("set sps info len ", info.data, info.nSize);
    return 0;
}
int H264Buf::set_pps_info(uint8_t * buf, int size)
{

    /*number of pps*/
    /*16bits: pps_size*/
    /*pps data */
    info.data[info.nSize] = 0x01;
    memcpy(info.data+info.nSize+1, buf+2, size -2);
    info.nSize += (size -1);
//    data_printf("set sps pps info len ", info.data, info.nSize);
    return 0;
}

static int firstdrop = 0;


int H264Buf::write_h264buf(uint8_t * buf, int size, int q, int drop)
{
//    uint8_t nal_ref_idc;
    uint8_t nal_unit_type;
    uint32_t nalu_size;
    uint32_t outsize;
    uint32_t payload_len = size;
    /* +---------------+
     * |0|1|2|3|4|5|6|7|
     * +-+-+-+-+-+-+-+-+
     * |F|NRI|  Type   |
     * +---------------+
     *
     * F must be 0.
     */
//    nal_ref_idc = (buf[0] & 0x60) >> 5;

    if(sz >= H264_MAX_FRAME)
    {
        if(firstdrop == 0)
        {
            printf("receive data soquick wr %d rd %d seq %d!\n", wr, rd, seqid[rd]);
            printf("rd size %d receive %d!\n", bufsize[rd], receive_frame);
            firstdrop = 1;
        }
        return 0;
    }
    if(drop == 1)
    {
        runok = false;
    }

    nal_unit_type = buf[0] & 0x1f;

    switch(nal_unit_type)
    {
        case 0:
        case 30:
        case 31:
            zprintf1("nala error type %d!\n", nal_unit_type);
        break;
        case 25:
        case 24:
        case 26:
        case 27:
            zprintf1("nala test add type %d!\n", nal_unit_type);
           break;

        case 28:
        case 29:
        {
            /* FU-A      Fragmentation unit                 5.8 */
            /* FU-B      Fragmentation unit                 5.8 */
            bool S, E;

            /* +---------------+
             * |0|1|2|3|4|5|6|7|
             * +-+-+-+-+-+-+-+-+
             * |S|E|R|  Type   |
             * +---------------+
             *
             * R is reserved and always 0
             */
            S = (buf[1] & 0x80) == 0x80;
            E = (buf[1] & 0x40) == 0x40;

            if(runok == false && S) //重新开始取
                runok = true;
            else if(runok ==false)
                break;

            if (S) {
              /* NAL unit starts here */
              uint8_t nal_header;

              /* reconstruct NAL header */
              nal_header = (buf[0] & 0xe0) | (buf[1] & 0x1f);

              /* strip type header, keep FU header, we'll reuse it to reconstruct
               * the NAL header. */
              buf += 1;
              payload_len -= 1;
              nalu_size = payload_len;
              outsize = nalu_size + sizeof (sync_bytes);

              memcpy (data[wr] + sizeof (sync_bytes), buf, nalu_size);
              data[wr][sizeof (sync_bytes)] = nal_header;
              bufsize[wr] = outsize;
              seqid[wr] = q;

            } else {
              /* strip off FU indicator and FU header bytes */
              buf += 2;
              payload_len -= 2;

              memcpy(data[wr] + bufsize[wr], buf, payload_len);
              bufsize[wr] += payload_len;

            }

            if (E)
            {

                if (stream_type) {
                  memcpy (data[wr], sync_bytes, sizeof (sync_bytes));
                } else {
                    outsize = bufsize[wr] - 4;
                    data[wr][0] = (outsize >> 24);
                    data[wr][1] = (outsize >> 16);
                    data[wr][2] = (outsize >> 8);
                    data[wr][3] = (outsize);
                }
                if(bufsize[wr] > FRAME_MAX_NUM * H264_DATA_SIZE)
                {
                    printf("frame data over %d!\n", bufsize[wr]);
                    zprintf1("frame data over %d!\n", bufsize[wr]);
                    runok = false;
                    break;
                }

                lock();
                wr++;
                wr %= H264_MAX_FRAME;
                sz++;

                receive_frame++;
                unlock();
                sem_post(&mgsem);
            }
            break;
        }

        default:
        {


            /* 1-23   NAL unit  Single NAL unit packet per H.264   5.6 */
            /* the entire payload is the output buffer */

           if(runok ==false)
               break;
            nalu_size = size;
            outsize = nalu_size + sizeof (sync_bytes);

            {

                if (stream_type) {
                  memcpy (data[wr], sync_bytes, sizeof (sync_bytes));
                } else {
                    data[wr][0] = data[wr][1] = 0;
                    data[wr][2] = nalu_size >> 8;
                    data[wr][3] = nalu_size & 0xff;
                }


                memcpy (data[wr] + sizeof (sync_bytes), buf, nalu_size);
                bufsize[wr] = outsize;
                if(outsize > FRAME_MAX_NUM * H264_DATA_SIZE)
                {
                    printf("frame over!\n");
                    zprintf1("frame over!\n");

                }

            }
            if(nal_unit_type == 7)
            {
                if(spsmark == 0)
                {
                    set_sps_info(data[wr], bufsize[wr]);
                    spsmark =1;
                }
                lock();
                wr++;
                wr %= H264_MAX_FRAME;
                sz++;
                receive_frame++;
                seqid[wr] = q;
                unlock();
                sem_post(&mgsem);
            }
            else if(nal_unit_type == 8)
            {
                if(ppsmark == 0)
                {
                    set_pps_info(data[wr], bufsize[wr]);
                    ppsmark = 1;
                }
                lock();
                wr++;
                wr %= H264_MAX_FRAME;
                sz++;
                receive_frame++;
                seqid[wr] = q;
                unlock();
                sem_post(&mgsem);
            }
            else
            {

                lock();
                wr++;
                wr %= H264_MAX_FRAME;
                sz++;
                receive_frame++;
                seqid[wr] = q;
                unlock();
                sem_post(&mgsem);
            }

            break;
          }
    }

    return 0;
}

int H264Buf::write_h264buf(int size, uint8_t dataty)
{
//    lock();
    bufsize[wr] = size;
    m_type[wr] = dataty;
    wr++;
    wr %= H264_MAX_FRAME;
    sz++;
    receive_frame++;

//    unlock();
    sem_post(&mgsem);
    return 0;
}

int H264Buf::write_h264data_buf(uint8_t * buf, int size, int q)
{
    if(sz >= H264_MAX_FRAME)
    {
        if(firstdrop == 0)
        {
            printf("receive data soquick wr %d rd %d seq %d!\n", wr, rd, seqid[rd]);
            printf("rd size %d receive %d!\n", bufsize[rd], receive_frame);
            firstdrop = 1;
        }
        return 0;
    }

    lock();
    memcpy(data[wr], buf, size);

    bufsize[wr] = size;
    m_type[wr] = H264_DATA_TYPE;
    seqid[wr] = q;
    wr++;
    wr %= H264_MAX_FRAME;
    sz++;
    receive_frame++;
    unlock();
    sem_post(&mgsem);
    return 0;

}
int H264Buf::get_write_h264buf(uint8_t * * addr)
{
     int err = -1;
//     lock();
     if(sz < H264_MAX_FRAME)
     {
         *addr = data[wr];
         err = 0;
     }
//     unlock();

     return err;
}

int H264Buf::get_h264buf(uint8_t ** addr, uint8_t * ty)
{
    uint8_t * p = NULL;
    int size = 0;
    lock();
    if(sz > 0)
    {
        p = data[rd];
        size = bufsize[rd];
        *ty = m_type[rd];
    }
    unlock();
    *addr = p;
    return size;

}
int H264Buf::add_buf_rd(void)
{
    lock();
    rd++;
    rd %= H264_MAX_FRAME;
    sz--;
    unlock();
    return 0;
}

H264Depay::H264Depay()
{
    h264buf = new H264Buf;
    seq = 0;
    next_seq = 0;
    proframe = 0;
    m_size = 0;
    m_runok = true;
    m_streamtype = 0;
    m_spsmark = 0;
    m_ppsmark = 0;

    zprintf4("zty h264init %d!\n", next_seq);
}

H264Depay::~H264Depay()
{
    cout << "delete h264 depay!" <<endl;
    zprintf3("delete h264 depay!");
    stop();

    if(vpudec != NULL)
        delete vpudec;

    if(h264buf != NULL)
        delete h264buf;

}

int H264Depay::set_sps_info(uint8_t * buf, int size)
{

    m_info.data[0] = 0x1;
    m_info.data[1] = buf[5]; //profile
    m_info.data[2] = buf[6]; //profile compat
    m_info.data[3] = buf[7]; //level
    m_info.data[4] = 0xff;   /* 6 bits reserved | 2 bits lengthSizeMinusOn */
//    info.data[5] = 0xe0 | buf[3];
    m_info.data[5] = 0xe1;  /* [5]: 3 bits reserved (111) + 5 bits number of sps (00001) */

    memcpy(m_info.data +6, buf +2, size -2);
    m_info.nSize = 6 + size -2;
//    printf("set sps info len %d!\n", info.nSize);
//    data_printf("set sps info len ", info.data, info.nSize);
    return 0;
}
int H264Depay::set_pps_info(uint8_t * buf, int size)
{

    /*number of pps*/
    /*16bits: pps_size*/
    /*pps data */
    m_info.data[m_info.nSize] = 0x01;
    memcpy(m_info.data+m_info.nSize+1, buf+2, size -2);
    m_info.nSize += (size -1);
//    data_printf("set sps pps info len ", info.data, info.nSize);
    return 0;
}

int H264Depay::data_parse(uint8_t * buf, int size, int q, int drop)
{
//    uint8_t nal_ref_idc;
    uint8_t nal_unit_type;
    uint32_t nalu_size;
    uint32_t outsize;
    uint32_t payload_len = size;
    /* +---------------+
     * |0|1|2|3|4|5|6|7|
     * +-+-+-+-+-+-+-+-+
     * |F|NRI|  Type   |
     * +---------------+
     *
     * F must be 0.
     */


    if(drop == 1)
    {
        m_runok = false;
    }

    nal_unit_type = buf[0] & 0x1f;

    switch(nal_unit_type)
    {
        case 0:
        case 30:
        case 31:
            zprintf1("nala error type %d!\n", nal_unit_type);
        break;
        case 25:
        case 24:
        case 26:
        case 27:
            zprintf1("nala test add type %d!\n", nal_unit_type);
           break;

        case 28:
        case 29:
        {
            /* FU-A      Fragmentation unit                 5.8 */
            /* FU-B      Fragmentation unit                 5.8 */
            bool S, E;

            /* +---------------+
             * |0|1|2|3|4|5|6|7|
             * +-+-+-+-+-+-+-+-+
             * |S|E|R|  Type   |
             * +---------------+
             *
             * R is reserved and always 0
             */
            S = (buf[1] & 0x80) == 0x80;
            E = (buf[1] & 0x40) == 0x40;

            if(m_runok == false && S) //重新开始取
                m_runok = true;
            else if(m_runok ==false)
                break;

            if (S) {
              /* NAL unit starts here */
              uint8_t nal_header;

              /* reconstruct NAL header */
              nal_header = (buf[0] & 0xe0) | (buf[1] & 0x1f);

              /* strip type header, keep FU header, we'll reuse it to reconstruct
               * the NAL header. */
              buf += 1;
              payload_len -= 1;
              nalu_size = payload_len;
              outsize = nalu_size + sizeof (sync_bytes);

              memcpy (m_buf + sizeof (sync_bytes), buf, nalu_size);
              m_buf[sizeof (sync_bytes)] = nal_header;
              m_size = outsize;
//              seqid[wr] = q;

            } else {
              /* strip off FU indicator and FU header bytes */
              buf += 2;
              payload_len -= 2;

              memcpy(m_buf + m_size, buf, payload_len);
              m_size += payload_len;

            }

            if (E)
            {

                if (m_streamtype)
                {
                    memcpy (m_buf, sync_bytes, sizeof (sync_bytes));
                } else {
                    outsize = m_size - 4;
                    m_buf[0] = (outsize >> 24);
                    m_buf[1] = (outsize >> 16);
                    m_buf[2] = (outsize >> 8);
                    m_buf[3] = (outsize);
                }
                if(m_size > FRAME_MAX_NUM * H264_DATA_SIZE)
                {
                    printf("frame data over %d!\n", m_size);
                    zprintf1("frame data over %d!\n", m_size);
                    m_runok = false;
                    break;
                }
//                printf("write h264 iframe!\n");
                h264buf->write_h264data_buf(m_buf, m_size, q);
            }
            break;
        }

        default:
        {


            /* 1-23   NAL unit  Single NAL unit packet per H.264   5.6 */
            /* the entire payload is the output buffer */

           if(m_runok ==false)
               break;
            nalu_size = size;
            outsize = nalu_size + sizeof (sync_bytes);

            {

                if (m_streamtype) {
                  memcpy (m_buf, sync_bytes, sizeof (sync_bytes));
                } else {
                    m_buf[0] = m_buf[1] = 0;
                    m_buf[2] = nalu_size >> 8;
                    m_buf[3] = nalu_size & 0xff;
                }


                memcpy (m_buf + sizeof (sync_bytes), buf, nalu_size);
                m_size = outsize;
                if(outsize > FRAME_MAX_NUM * H264_DATA_SIZE)
                {
                    printf("frame over!\n");
                    zprintf1("frame over!\n");

                }

            }
            if(nal_unit_type == 7)
            {
                if(m_spsmark == 0)
                {
                    set_sps_info(m_buf, m_size);
                    m_spsmark =1;
                }

            }
            else if(nal_unit_type == 8)
            {
                if(m_ppsmark == 0)
                {
                    set_pps_info(m_buf, m_size);
                    m_ppsmark = 1;
                }

            }
//            printf("write h264 frame %d!\n", nal_unit_type);
            h264buf->write_h264data_buf(m_buf, m_size, q);
            break;
          }
    }

    return 0;
}

int H264Depay::data_porcess(uint8_t * buf, int size)
{
//    GstRTPHeader
//    uint32_t time = GST_RTP_HEADER_TIMESTAMP(buf);
//    printf("receive seq %d next %d!\n", seq, next_seq);

    int drop = 0;

    seq = GST_RTP_HEADER_SEQ(buf);
    tbyte_swap((uint16_t *)(&seq), 2);

    if(next_seq == 0)
    {
        next_seq = seq +1;
    }
    else
    {
        if(seq != next_seq)
        {
            zprintf1("zty data receive drop seq %d next %d, size %d!\n",seq, next_seq, size);
            drop = 1;
            next_seq = seq;
        }        
        next_seq++;
    }
//   zprintf1("receive end seq %d next %d!\n", seq, next_seq);

//    h264buf->write_h264buf(buf+H264_HEAD_MIN_LEN, size-H264_HEAD_MIN_LEN, seq, drop);
    data_parse(buf+H264_HEAD_MIN_LEN, size-H264_HEAD_MIN_LEN, seq, drop);
    return 0;
}



/***********************************************************************************
 * 函数名：h264_pro_rxdata_callback
 * 功能：rtp数据接收处理回调函数
 *
 ***********************************************************************************/
int h264_pro_rxdata_callback(RTP * pro, void * buf, int size)
{
    H264Depay * midpro = (H264Depay*) pro->next_p;

   return midpro->data_porcess((uint8_t *)buf, size);

}

void H264Depay::rtp_h264_init(void * pro)
{
    ((RTP *) pro)->rxcallback = h264_pro_rxdata_callback;
    vpudec = new VpuDec;
    vpudec->vpu_init();

}

void H264Depay::rtp_h264_init(void)
{
    vpudec = new VpuDec;
    vpudec->vpu_init();
}

void H264Depay::run()
{
    int size;
    uint8_t * buf;
    uint8_t nal_unit_type;
    uint8_t datatype;
    int first = 0;

    vpudec->vpu_open();

    while(1)
    {
        sem_wait(&h264buf->mgsem);
        size = h264buf->get_h264buf(&buf, &datatype);

//        printf("buf data type %d vpu type %d!\n", datatype, vpudec->m_frametype);


        if(size > 0 && buf != NULL)
        {
            if(datatype != vpudec->m_frametype)
            {
                printf("buf data type %d vpu type %d!\n", datatype, vpudec->m_frametype);
                zprintf1("buf data type %d vpu type %d!\n", datatype, vpudec->m_frametype);
            }
            else
            {

                nal_unit_type = buf[4] & 0x1f;
                if(nal_unit_type == 7 || nal_unit_type == 8)
                {
                    if(first == 0 && nal_unit_type == 8)
                    {
                        first = 1;
    //                    vpudec->set_vpu_codec_data(h264buf->info.data, h264buf->info.nSize);
                        vpudec->set_vpu_codec_data(m_info.data, m_info.nSize);
                    }
                }
                else
                {
    //                vpudec->vpu_decode_process(buf, size, NULL, 0, &ret);
                    vpudec->vpu_decode_process(buf, size);
                }
            }

            h264buf->add_buf_rd();

            proframe++;
        }

    }
}

