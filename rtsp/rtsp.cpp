#include "rtsp.h"
#include "date/com_date.h"


static const char *rtsp_methods[] = {
  "DESCRIBE",
  "ANNOUNCE",
  "GET_PARAMETER",
  "OPTIONS",
  "PAUSE",
  "PLAY",
  "RECORD",
  "REDIRECT",
  "SETUP",
  "SET_PARAMETER",
  "TEARDOWN",
  "GET",
  "POST",
  NULL
};

const char *
gst_rtsp_method_as_text (GstRTSPMethod method)
{
  int i;
  int tmp = method;

  if (method == GST_RTSP_INVALID)
    return NULL;

  i = 0;
  while ((tmp & 1) == 0) {
    i++;
    tmp >>= 1;
  }
  return rtsp_methods[i];
}

const char *
gst_rtsp_version_as_text (GstRTSPVersion version)
{
  switch (version) {
    case GST_RTSP_VERSION_1_0:
      return "1.0";

    case GST_RTSP_VERSION_1_1:
      return "1.1";

    default:
      return "0.0";
  }
}





string
RTSP::message_to_string (GstRTSPMessage * message)
{
  string str;
  string dateStr;


  switch (message->type) {
    case GST_RTSP_MESSAGE_REQUEST:
    {
      /* create request string, add CSeq */
      string method(gst_rtsp_method_as_text(message->type_data.request.method));
      string uri(message->type_data.request.uri);
//      string cseq("CSeq: ");
      string cseqstring =  "CSeq: " + std::to_string(cseq) +"\r\n";
//        string cseqstring =  "CSeq: "  + "\r\n";
      if(message->type_data.request.method == GST_RTSP_SETUP)
          uri += "/track1";
      str = method +" "+ uri +" RTSP/1.0\r\n" + cseqstring;
      cseq++;
      if(message->type_data.request.method == GST_RTSP_DESCRIBE)
      {
          string acc = "Accept: application/sdp\r\n";
          str += acc;
      }
      else if(message->type_data.request.method == GST_RTSP_SETUP)
      {
          string setupstr = "Transport: RTP/AVP;unicast;client_port=51160-51161\r\n";
          str += setupstr;
      }
      else if(message->type_data.request.method == GST_RTSP_PLAY)
      {
          string range = "Range: npt=0-\r\n";
          str += range;
          str += session;
          str += "\r\n";

          cout << "zty play " << str <<endl;
      }

      break;
    }
    case GST_RTSP_MESSAGE_RESPONSE:
      /* create response string */
//      g_string_append_printf (str, "RTSP/1.0 %d %s\r\n",
//          message->type_data.response.code, message->type_data.response.reason);
      break;
    case GST_RTSP_MESSAGE_HTTP_REQUEST:
      /* create request string */
//      g_string_append_printf (str, "%s %s HTTP/%s\r\n",
//          gst_rtsp_method_as_text (message->type_data.request.method),
//          message->type_data.request.uri,
//          gst_rtsp_version_as_text (message->type_data.request.version));


      break;
    case GST_RTSP_MESSAGE_HTTP_RESPONSE:
      /* create response string */
//      g_string_append_printf (str, "HTTP/%s %d %s\r\n",
//          gst_rtsp_version_as_text (message->type_data.request.version),
//          message->type_data.response.code, message->type_data.response.reason);
      break;
    case GST_RTSP_MESSAGE_DATA:
    {
//      guint8 data_header[4];

//      /* prepare data header */
//      data_header[0] = '$';
//      data_header[1] = message->type_data.data.channel;
//      data_header[2] = (message->body_size >> 8) & 0xff;
//      data_header[3] = message->body_size & 0xff;

//      /* create string with header and data */
//      str = g_string_append_len (str, (gchar *) data_header, 4);
//      str =
//          g_string_append_len (str, (gchar *) message->body,
//          message->body_size);
      break;
    }
    default:
//      g_string_free (str, TRUE);
//      g_return_val_if_reached (NULL);
      break;
  }

  /* append headers and body */
  if (message->type != GST_RTSP_MESSAGE_DATA) {
    Com_Date data;
    data.gen_date_string(dateStr);
    str += "Date: ";
    str += dateStr;
    str += "\r\n\r\n";
  }
  return str;
}


GstRTSPResult
gst_rtsp_message_init_request (GstRTSPMessage * msg, GstRTSPMethod method,
    const char * uri)
{

  msg->type = GST_RTSP_MESSAGE_REQUEST;
  msg->type_data.request.method = method;
  msg->type_data.request.uri = (char *) (uri);
  msg->type_data.request.version = GST_RTSP_VERSION_1_0;
//  msg->hdr_fields = g_array_new (FALSE, FALSE, sizeof (RTSPKeyValue));

  return GST_RTSP_OK;
}
RTSP::~RTSP()
{
    cout << "rtsp delete!" << endl;

    stop();

    if(udprtp != NULL)
    {
        delete udprtp;
        udprtp = NULL;
    }
    cout << "delete udprtp ok!" <<endl;

    if(h264depay != NULL)
    {
        delete h264depay;
        h264depay = NULL;
    }
    cout << "delete h264ok" << endl;

    cout << "rtsp delete end!" << endl;
    if(link != NULL)
    {
        delete link;
        link = NULL;
    }

    sem_destroy(&netlinksem);
}

void RTSP::setup_message_parse (char * buf, size_t n)
{
    (void)n;
    string seeionstr = buf;

    size_t pos = 0;
    int posend = 0;

    cout << "zty " << seeionstr <<endl;
    pos = seeionstr.find("Session:");


    if(pos != string::npos) //find
    {
        cout << "zty pos " << pos << endl;
        posend = seeionstr.find("\r\n", pos);;

        cout << "zty posend " << posend  << endl;

        session = seeionstr.substr(pos, posend - pos);
        cout << "zty val" << session  << " endl" << endl;
    }

}

/***********************************************************************************
 * 函数名：rtp网络状态回调函数
 * 功能：rtp网络状态回调函数
 *
 ***********************************************************************************/
int rtp_netlink_callback(RTP * pro, int s)
{
    (void)s;
    RTSP * midpro = (RTSP *) pro->net_father;

    if(midpro != NULL)
        sem_post(&midpro->netlinksem);
    return 0;
}

int eth_netlink_callback(NetlinkStatus * pro, int s)
{
    (void)s;
    RTSP * midpro = (RTSP *) pro->netlinkfater;

    if(midpro != NULL)
        sem_post(&midpro->netlinksem);
    return 0;
}

void RTSP::run()
{
    while(1)
    {
        sem_wait(&netlinksem);  //等待网络变化信号
        if(link->getLinkstate() != ethlink) //网线变化
        {
            cout << "eth net change!" <<endl;
            ethlink = link->getLinkstate();
            if(ethlink == 1)
            {
                cout << "eth up!" << endl;
                rtsp_stop();
                rtsp_restart(url);

            }
            else
            {
                cout << "eth down!" << endl;
            }
        }
        else
        {
            cout << "rtp net change!" <<endl;
            if(udprtp->state == 1)
            {
                cout << "rtp up!" << endl;
            }
            else
            {
                cout << "rtp down!" << endl;
            }
        }

    }
}

int RTSP::rtsp_init(string ip)
{
    GstRTSPMessage msg;
    string msg_str;
    char buf[2048] = {0};
    int ret;
    cseq = 1;
    initport = 51160;

    memset(&msg, 0x00, sizeof(msg));
//    url = "rtsp://169.254.1.168/0";
    url = "rtsp://" + ip + "/0";
    ipaddr = ip;
    cout << url << endl;

    link = new NetlinkStatus("eth1");

    link->start();

    while(link->getLinkstate() != 1)
    {
        zprintf1("hndz h264 eth1 is no link!\n");
        sleep(1);
    }

    ethlink = 1;
    link->netlinkcb = eth_netlink_callback;
    link->netlinkfater = this;


    tcp_client_init(554, ip.c_str());
    tcp_client_connect();

    gst_rtsp_message_init_request(&msg, GST_RTSP_OPTIONS, url.c_str());

    msg_str = message_to_string(&msg);

    cout << msg_str << endl;

    tcp_write((void *)msg_str.c_str(), msg_str.length());

     ret = tcp_recv( buf, sizeof(buf));
       if (ret > 0) {
//           printf("recv data[%s] from server\n", buf);
       } else if (ret < 0) {
           printf("%s: errno:%d\n", __FUNCTION__, errno);
       } else if (0 == ret) {
           printf("server fd[%d] disconnect\n", socket_fd);
       }
     //parse options

   gst_rtsp_message_init_request(&msg, GST_RTSP_DESCRIBE, url.c_str());
   msg_str = message_to_string(&msg);
   tcp_write((void *)msg_str.c_str(), msg_str.length());

   ret = tcp_recv(buf, sizeof(buf));
   if (ret > 0) {
//       printf("recv data[%s] from server\n", buf);
   } else if (ret < 0) {
       printf("%s: errno:%d\n", __FUNCTION__, errno);
   } else if (0 == ret) {
       printf("server fd[%d] disconnect\n", socket_fd);
   }
   printf("zty sdp len %d!\n", ret);

   ret = tcp_recv(buf, sizeof(buf));
   if (ret > 0) {
//       printf("recv data[%s] from server\n", buf);
   } else if (ret < 0) {
       printf("%s: errno:%d\n", __FUNCTION__, errno);
   } else if (0 == ret) {
       printf("server fd[%d] disconnect\n", socket_fd);
   }
   printf("zty sdp len %d!\n", ret);

  //parse discribe

   h264depay = new H264Depay;

   udprtp = new RTP;

   udprtp->set_protocol(h264depay);
   h264depay->rtp_h264_init(udprtp);
   udprtp->rtp_init(session, initport);
   h264depay->start();

   udprtp->net_father = this;
   udprtp->netstatecb = rtp_netlink_callback;
   udprtp->start();
   gst_rtsp_message_init_request(&msg, GST_RTSP_SETUP, url.c_str());
   msg_str = message_to_string(&msg);
   tcp_write((void *)msg_str.c_str(), msg_str.length());

   ret = tcp_recv(buf, sizeof(buf));
   if (ret > 0) {
        setup_message_parse(buf, ret);

   } else if (ret < 0) {
       printf("%s: errno:%d\n", __FUNCTION__, errno);
   } else if (0 == ret) {
       printf("server fd[%d] disconnect\n", socket_fd);
   }


   gst_rtsp_message_init_request(&msg, GST_RTSP_PLAY, url.c_str());
   msg_str = message_to_string(&msg);
   tcp_write((void *)msg_str.c_str(), msg_str.length());

   return 0;

}

int RTSP::rtsp_run(void)
{
    while(1)
    {
        sem_wait(&netlinksem);  //等待网络变化信号
        if(link->getLinkstate() != ethlink) //网线变化
        {
            cout << "eth net change!" <<endl;
            zprintf1("eth net change!\n");
            ethlink = link->getLinkstate();
            if(ethlink == 1)
            {
                cout << "eth up!" << endl;
                zprintf1("eth up!\n");
                rtsp_stop();
                rtsp_restart(ipaddr);
            }
            else
            {
                cout << "eth down!" << endl;
                zprintf1("eth down!\n");
            }
        }
        else
        {
            cout << "rtp net change!" <<endl;
            zprintf1("rtp net change!\n");
            if(udprtp->state == 1)
            {
                cout << "rtp up!" << endl;
                zprintf1("rtp up!\n");
            }
            else
            {
                cout << "rtp down!" << endl;
                zprintf1("rtp down!\n");
            }
        }

    }
    return 0;
}

int  RTSP::rtsp_stop(void)
{
    cout << "rtsp stop!" << endl;

    stop();
    close_fd();

    if(udprtp != NULL)
    {
        delete udprtp;
        udprtp = NULL;
    }
    cout << "delete udprtp ok!" <<endl;

    if(h264depay != NULL)
    {
        delete h264depay;
        h264depay = NULL;
    }
    cout << "delete h264ok" << endl;

    cout << "rtsp delete end!" << endl;
    if(link != NULL)
    {
        delete link;
        link = NULL;
    }

    sem_destroy(&netlinksem);
    return 0;
}


int RTSP::rtsp_restart(string ip)
{
    sem_init(&netlinksem, 0, 0);
    return rtsp_init(ip);
//    return 0;
}
