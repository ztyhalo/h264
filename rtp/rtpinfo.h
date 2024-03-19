#ifndef RTP_INFO_H
#define RTP_INFO_H

#include <stdint.h>

typedef struct _GstRTPHeader
{
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
  unsigned int csrc_count:4;    /* CSRC count */
  unsigned int extension:1;     /* header extension flag */
  unsigned int padding:1;       /* padding flag */
  unsigned int version:2;       /* protocol version */
  unsigned int payload_type:7;  /* payload type */
  unsigned int marker:1;        /* marker bit */
#elif G_BYTE_ORDER == G_BIG_ENDIAN
  unsigned int version:2;       /* protocol version */
  unsigned int padding:1;       /* padding flag */
  unsigned int extension:1;     /* header extension flag */
  unsigned int csrc_count:4;    /* CSRC count */
  unsigned int marker:1;        /* marker bit */
  unsigned int payload_type:7;  /* payload type */
#else
#error "G_BYTE_ORDER should be big or little endian."
#endif
  unsigned int seq:16;          /* sequence number */
  unsigned int timestamp:32;    /* timestamp */
  unsigned int ssrc:32;         /* synchronization source */
  uint8_t csrclist[4];           /* optional CSRC list, 32 bits each */
} GstRTPHeader;

#define GST_RTP_HEADER_VERSION(data)      (((GstRTPHeader *)(data))->version)
#define GST_RTP_HEADER_PADDING(data)      (((GstRTPHeader *)(data))->padding)
#define GST_RTP_HEADER_EXTENSION(data)    (((GstRTPHeader *)(data))->extension)
#define GST_RTP_HEADER_CSRC_COUNT(data)   (((GstRTPHeader *)(data))->csrc_count)
#define GST_RTP_HEADER_MARKER(data)       (((GstRTPHeader *)(data))->marker)
#define GST_RTP_HEADER_PAYLOAD_TYPE(data) (((GstRTPHeader *)(data))->payload_type)
#define GST_RTP_HEADER_SEQ(data)          (((GstRTPHeader *)(data))->seq)
#define GST_RTP_HEADER_TIMESTAMP(data)    (((GstRTPHeader *)(data))->timestamp)
#define GST_RTP_HEADER_SSRC(data)         (((GstRTPHeader *)(data))->ssrc)
#define GST_RTP_HEADER_CSRC_LIST_OFFSET(data,i)        \
    data + G_STRUCT_OFFSET(GstRTPHeader, csrclist) +   \
    ((i) * sizeof(uint32_t))
#define GST_RTP_HEADER_CSRC_SIZE(data)   (GST_RTP_HEADER_CSRC_COUNT(data) * sizeof (uint32_t))




#endif // RTP_INFO_H
