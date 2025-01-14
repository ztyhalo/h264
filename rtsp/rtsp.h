#ifndef RTSP_H
#define RTSP_H

#include <iostream>
#include <stdio.h>
#include <string>
#include "tcp/tcp_client.h"

#include "rtp/rtp.h"
#include "epoll/e_poll.h"
#include "h264depay/h264depay.h"
#include "netlinkstatus/netlinkstatus.h"

using namespace std;

/**
 * GstRTSPVersion:
 * @GST_RTSP_VERSION_INVALID: unknown/invalid version
 * @GST_RTSP_VERSION_1_0: version 1.0
 * @GST_RTSP_VERSION_1_1: version 1.1.
 *
 * The supported RTSP versions.
 */
typedef enum {
  GST_RTSP_VERSION_INVALID = 0x00,
  GST_RTSP_VERSION_1_0     = 0x10,
  GST_RTSP_VERSION_1_1     = 0x11
} GstRTSPVersion;

/**
 * GstRTSPMethod:
 * @GST_RTSP_INVALID: invalid method
 * @GST_RTSP_DESCRIBE: the DESCRIBE method
 * @GST_RTSP_ANNOUNCE: the ANNOUNCE method
 * @GST_RTSP_GET_PARAMETER: the GET_PARAMETER method
 * @GST_RTSP_OPTIONS: the OPTIONS method
 * @GST_RTSP_PAUSE: the PAUSE method
 * @GST_RTSP_PLAY: the PLAY method
 * @GST_RTSP_RECORD: the RECORD method
 * @GST_RTSP_REDIRECT: the REDIRECT method
 * @GST_RTSP_SETUP: the SETUP method
 * @GST_RTSP_SET_PARAMETER: the SET_PARAMETER method
 * @GST_RTSP_TEARDOWN: the TEARDOWN method
 * @GST_RTSP_GET: the GET method (HTTP).
 * @GST_RTSP_POST: the POST method (HTTP).
 *
 * The different supported RTSP methods.
 */
typedef enum {
  GST_RTSP_INVALID          = 0,
  GST_RTSP_DESCRIBE         = (1 <<  0),
  GST_RTSP_ANNOUNCE         = (1 <<  1),
  GST_RTSP_GET_PARAMETER    = (1 <<  2),
  GST_RTSP_OPTIONS          = (1 <<  3),
  GST_RTSP_PAUSE            = (1 <<  4),
  GST_RTSP_PLAY             = (1 <<  5),
  GST_RTSP_RECORD           = (1 <<  6),
  GST_RTSP_REDIRECT         = (1 <<  7),
  GST_RTSP_SETUP            = (1 <<  8),
  GST_RTSP_SET_PARAMETER    = (1 <<  9),
  GST_RTSP_TEARDOWN         = (1 << 10),
  GST_RTSP_GET              = (1 << 11),
  GST_RTSP_POST             = (1 << 12)
} GstRTSPMethod;
/**
 * GstRTSPMsgType:
 * @GST_RTSP_MESSAGE_INVALID: invalid message type
 * @GST_RTSP_MESSAGE_REQUEST: RTSP request message
 * @GST_RTSP_MESSAGE_RESPONSE: RTSP response message
 * @GST_RTSP_MESSAGE_HTTP_REQUEST: HTTP request message.
 * @GST_RTSP_MESSAGE_HTTP_RESPONSE: HTTP response message.
 * @GST_RTSP_MESSAGE_DATA: data message
 *
 * The type of a message.
 */
typedef enum
{
  GST_RTSP_MESSAGE_INVALID,
  GST_RTSP_MESSAGE_REQUEST,
  GST_RTSP_MESSAGE_RESPONSE,
  GST_RTSP_MESSAGE_HTTP_REQUEST,
  GST_RTSP_MESSAGE_HTTP_RESPONSE,
  GST_RTSP_MESSAGE_DATA
} GstRTSPMsgType;



/**
 * GstRTSPStatusCode:
 *
 * Enumeration of rtsp status codes
 */
typedef enum {
  GST_RTSP_STS_INVALID                              = 0,
  GST_RTSP_STS_CONTINUE                             = 100,
  GST_RTSP_STS_OK                                   = 200,
  GST_RTSP_STS_CREATED                              = 201,
  GST_RTSP_STS_LOW_ON_STORAGE                       = 250,
  GST_RTSP_STS_MULTIPLE_CHOICES                     = 300,
  GST_RTSP_STS_MOVED_PERMANENTLY                    = 301,
  GST_RTSP_STS_MOVE_TEMPORARILY                     = 302,
  GST_RTSP_STS_SEE_OTHER                            = 303,
  GST_RTSP_STS_NOT_MODIFIED                         = 304,
  GST_RTSP_STS_USE_PROXY                            = 305,
  GST_RTSP_STS_BAD_REQUEST                          = 400,
  GST_RTSP_STS_UNAUTHORIZED                         = 401,
  GST_RTSP_STS_PAYMENT_REQUIRED                     = 402,
  GST_RTSP_STS_FORBIDDEN                            = 403,
  GST_RTSP_STS_NOT_FOUND                            = 404,
  GST_RTSP_STS_METHOD_NOT_ALLOWED                   = 405,
  GST_RTSP_STS_NOT_ACCEPTABLE                       = 406,
  GST_RTSP_STS_PROXY_AUTH_REQUIRED                  = 407,
  GST_RTSP_STS_REQUEST_TIMEOUT                      = 408,
  GST_RTSP_STS_GONE                                 = 410,
  GST_RTSP_STS_LENGTH_REQUIRED                      = 411,
  GST_RTSP_STS_PRECONDITION_FAILED                  = 412,
  GST_RTSP_STS_REQUEST_ENTITY_TOO_LARGE             = 413,
  GST_RTSP_STS_REQUEST_URI_TOO_LARGE                = 414,
  GST_RTSP_STS_UNSUPPORTED_MEDIA_TYPE               = 415,
  GST_RTSP_STS_PARAMETER_NOT_UNDERSTOOD             = 451,
  GST_RTSP_STS_CONFERENCE_NOT_FOUND                 = 452,
  GST_RTSP_STS_NOT_ENOUGH_BANDWIDTH                 = 453,
  GST_RTSP_STS_SESSION_NOT_FOUND                    = 454,
  GST_RTSP_STS_METHOD_NOT_VALID_IN_THIS_STATE       = 455,
  GST_RTSP_STS_HEADER_FIELD_NOT_VALID_FOR_RESOURCE  = 456,
  GST_RTSP_STS_INVALID_RANGE                        = 457,
  GST_RTSP_STS_PARAMETER_IS_READONLY                = 458,
  GST_RTSP_STS_AGGREGATE_OPERATION_NOT_ALLOWED      = 459,
  GST_RTSP_STS_ONLY_AGGREGATE_OPERATION_ALLOWED     = 460,
  GST_RTSP_STS_UNSUPPORTED_TRANSPORT                = 461,
  GST_RTSP_STS_DESTINATION_UNREACHABLE              = 462,
  GST_RTSP_STS_KEY_MANAGEMENT_FAILURE               = 463, /* since 1.4 */
  GST_RTSP_STS_INTERNAL_SERVER_ERROR                = 500,
  GST_RTSP_STS_NOT_IMPLEMENTED                      = 501,
  GST_RTSP_STS_BAD_GATEWAY                          = 502,
  GST_RTSP_STS_SERVICE_UNAVAILABLE                  = 503,
  GST_RTSP_STS_GATEWAY_TIMEOUT                      = 504,
  GST_RTSP_STS_RTSP_VERSION_NOT_SUPPORTED           = 505,
  GST_RTSP_STS_OPTION_NOT_SUPPORTED                 = 551
} GstRTSPStatusCode;



/**
 * GstRTSPResult:
 * @GST_RTSP_OK: no error
 * @GST_RTSP_ERROR: some unspecified error occured
 * @GST_RTSP_EINVAL: invalid arguments were provided to a function
 * @GST_RTSP_EINTR: an operation was canceled
 * @GST_RTSP_ENOMEM: no memory was available for the operation
 * @GST_RTSP_ERESOLV: a host resolve error occured
 * @GST_RTSP_ENOTIMPL: function not implemented
 * @GST_RTSP_ESYS: a system error occured, errno contains more details
 * @GST_RTSP_EPARSE: a persing error occured
 * @GST_RTSP_EWSASTART: windows networking could not start
 * @GST_RTSP_EWSAVERSION: windows networking stack has wrong version
 * @GST_RTSP_EEOF: end-of-file was reached
 * @GST_RTSP_ENET: a network problem occured, h_errno contains more details
 * @GST_RTSP_ENOTIP: the host is not an IP host
 * @GST_RTSP_ETIMEOUT: a timeout occured
 * @GST_RTSP_ETGET: the tunnel GET request has been performed
 * @GST_RTSP_ETPOST: the tunnel POST request has been performed
 * @GST_RTSP_ELAST: last error
 *
 * Result codes from the RTSP functions.
 */
typedef enum {
  GST_RTSP_OK          =  0,
  /* errors */
  GST_RTSP_ERROR       = -1,
  GST_RTSP_EINVAL      = -2,
  GST_RTSP_EINTR       = -3,
  GST_RTSP_ENOMEM      = -4,
  GST_RTSP_ERESOLV     = -5,
  GST_RTSP_ENOTIMPL    = -6,
  GST_RTSP_ESYS        = -7,
  GST_RTSP_EPARSE      = -8,
  GST_RTSP_EWSASTART   = -9,
  GST_RTSP_EWSAVERSION = -10,
  GST_RTSP_EEOF        = -11,
  GST_RTSP_ENET        = -12,
  GST_RTSP_ENOTIP      = -13,
  GST_RTSP_ETIMEOUT    = -14,
  GST_RTSP_ETGET       = -15,
  GST_RTSP_ETPOST      = -16,

  GST_RTSP_ELAST       = -17
} GstRTSPResult;




/**
 * GstRTSPMessage:
 * @type: the message type
 *
 * An RTSP message containing request, response or data messages. Depending on
 * the @type, the appropriate structure may be accessed.
 */
typedef struct _GstRTSPMessage
{
  GstRTSPMsgType    type;

  union {
    struct {
      GstRTSPMethod      method;
      char             *uri;
      GstRTSPVersion     version;
    } request;
    struct {
      GstRTSPStatusCode  code;
      char             *reason;
      GstRTSPVersion     version;
    } response;
    struct {
      uint8_t             channel;
    } data;
  } type_data;

  /*< private >*/
//  vector        *hdr_fields;

  uint8_t        *body;
  uint            body_size;

//  gpointer _gst_reserved[GST_PADDING];
}GstRTSPMessage;

enum {
    RTSP_OK = 0,
    RTSP_NO_LINK,
    RTSP_NO_DATA
};

class RTSP:public NCbk_Poll,public TCP_CLIENT
{
public:
    RTSP():NCbk_Poll(1),TCP_CLIENT()
    {
        udprtp = NULL;
        h264depay = NULL;
        link = NULL;
        ethlink = 0;
        state = RTSP_OK;
        m_isplay = 0;
        m_fp = NULL;

        sem_init(&m_imagesem, 0, 0);
        sem_init(&m_restartsem, 0, 0);
    }
    RTSP(char * ip):NCbk_Poll(1),TCP_CLIENT(554,ip)
    {
        ;
    }
    virtual ~RTSP();
    int rtsp_init(string ip);
    int rtsp_connect_init(string ip);
    int rtsp_run(void);
    int rtsp_stop(void);
    int rtsp_restart(string ip);
    string message_to_string (GstRTSPMessage * message);
    void setup_message_parse (char * buf, size_t n);
    void link_state_image_process(void);
    void change_rtsp_state(int state);
public:
    string url;
    string ipaddr;
    int cseq ;
    int initport;

    sem_t  m_imagesem;
    sem_t  m_restartsem;
    string session;
    RTP * udprtp;
    H264Depay * h264depay;
    NetlinkStatus * link;
    int ethlink;
    int state;
    int m_isplay;
    FILE * m_fp;
    int m_rtsprun;
    void run();

};

#endif // RTSP_H
