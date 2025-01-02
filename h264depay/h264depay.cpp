
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

H264Depay::H264Depay()
{
    seq = 0;
    next_seq = 0;
    proframe = 0;
    m_size = 0;
    m_runok = true;
    m_streamtype = 0;
    vpudec = NULL;

    zprintf4("zty h264init %d!\n", next_seq);
}

H264Depay::~H264Depay()
{
    zprintf3("delete h264 depay!\n");
//    stop();

    if(vpudec != NULL)
    {
        delete vpudec;
        vpudec = NULL;
    }

}


int H264Depay::data_parse(uint8_t * buf, int size, int q, int drop)
{
    uint8_t nal_unit_type;
    uint32_t nalu_size;
    uint32_t outsize;
    uint32_t payload_len = size;

    VPUDataType datatype;

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
    datatype.m_datatype = H264_DATA_TYPE;
    datatype.m_seq = q;

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
                vpudec->vpu_write_buffer_data(m_buf, m_size, datatype);
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
                break;

            }

            vpudec->vpu_write_buffer_data(m_buf, m_size, datatype);
            break;
          }
    }

    return 0;
}

void H264Depay::rtp_restart_process(void)
{
    next_seq = 0;
}

int H264Depay::data_porcess(uint8_t * buf, int size)
{

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

void H264Depay::rtp_h264_init(void)
{
    if(vpudec == NULL)
        vpudec = new VpuDec;
    vpudec->vpu_init();
    vpudec->vpu_open();
    vpudec->start("vpudec_thread");

}


