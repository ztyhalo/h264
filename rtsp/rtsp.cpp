#include "rtsp.h"
#include "date/com_date.h"

const char * g_h264file[2] = {"/opt/config/h264/nolink","/opt/config/h264/nortp"};


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

      string cseqstring =  "CSeq: " + std::to_string(cseq) +"\r\n";

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
          string setupstr = "Transport: RTP/AVP;unicast;client_port=" + std::to_string(initport) + "-"
                  + std::to_string(initport+1) +"\r\n";
          str += setupstr;
          zprintf4("set %s!\n", str.c_str());
      }
      else if(message->type_data.request.method == GST_RTSP_PLAY)
      {
          string range = "Range: npt=0-\r\n";
          str += range;
          str += session;
          str += "\r\n";

          zprintf4("play %s!\n", str.c_str());
      }
      else if(message->type_data.request.method == GST_RTSP_TEARDOWN)
      {
          str += session;
          str += "\r\n";

          zprintf4("trardown %s!\n", str.c_str());
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

    return GST_RTSP_OK;
}
RTSP::~RTSP()
{
    zprintf1("rtsp delete!\n");

    if(h264depay != NULL && h264depay->vpudec != NULL)
    {
        h264depay->vpudec->vpu_stop();
    }
    // stop();

    close_fd();
    if(udprtp != NULL)
    {
        delete udprtp;
        udprtp = NULL;
    }

    if(link != NULL)
    {
        delete link;
        link = NULL;
    }

    stop();

    if(h264depay != NULL)
    {
        delete h264depay;
        h264depay = NULL;
    }

    sem_destroy(&m_imagesem);
    sem_destroy(&m_restartsem);
    if(m_fp != NULL)
    {
        fclose(m_fp);
        m_fp = NULL;
    }
}

void RTSP::setup_message_parse (char * buf, size_t n)
{
    (void)n;
    string seeionstr = buf;

    size_t pos = 0;
    int posend = 0;

    pos = seeionstr.find("Session:");


    if(pos != string::npos) //find
    {

        posend = seeionstr.find("\r\n", pos);;

        session = seeionstr.substr(pos, posend - pos);
        zprintf2("rtsp session: %s!\n", session.c_str());
    }

}

/***********************************************************************************
 * 函数名：rtp网络状态回调函数
 * 功能：rtp网络状态回调函数
 *
 ***********************************************************************************/
int rtp_netlink_callback(RTP * pro, int s)
{
    RTSP * midpro = (RTSP *) pro->net_father;

    if(midpro != NULL)
    {
        if(s == 1)  //rtp有数据
        {
            zprintf1("rtp up callback!\n");
            // if(midpro->h264depay->vpudec)
            //     midpro->h264depay->vpudec->vpu_change_mode(VPU_V_AVC);
            midpro->state = RTSP_OK;
        }
        else  //rtp无数据
        {
            zprintf1("rtp down callback!\n");
            if(midpro->udprtp != NULL)
            {
                midpro->udprtp->rtp_run_stop();
            }

            if(midpro->h264depay != NULL)
            {
                // midpro->h264depay->vpudec->vpu_change_mode(VPU_V_MJPG);
                midpro->change_rtsp_state(RTSP_NO_DATA);
                sem_post(&midpro->m_restartsem);
            }
        }
    }
    return 0;
}

int eth_netlink_callback(NetlinkStatus * pro, int s)
{

    RTSP * midpro = (RTSP *) (RTSP *) pro->netlinkfater;

    if(midpro != NULL)
    {
        if(s == 1)  //网络连接
        {
            zprintf1("eth up callback!\n");
            sem_post(&midpro->m_restartsem);
        }
        else  //网络断开
        {
            zprintf1("eth down callback!\n");

            if(midpro->udprtp != NULL)
            {
                midpro->rtsp_stop();
            }
            // if(midpro->h264depay != NULL)
            //     midpro->h264depay->vpudec->vpu_change_mode(VPU_V_MJPG);
            midpro->change_rtsp_state(RTSP_NO_LINK);

        }
    }
    return 0;
}



void RTSP::link_state_image_process(void)
{
    int  id = 0;
    int  err;
    int  frameid = 0;

    VPUDataType datatype;

    datatype.m_seq = 0;
    datatype.m_datatype = H264_DATA_TYPE;


    while(running)
    {
        if(state == RTSP_OK)
            sem_wait(&m_imagesem);
        if(state == RTSP_NO_LINK)
            id = 0;
        else if(state == RTSP_NO_DATA)
            id = 1;
        else
            continue;
        m_fp = fopen(g_h264file[id], "rb");

        if(NULL == m_fp)
        {
            continue;
        }
        fseek(m_fp, 0, SEEK_SET);
        while(1)
        {
            err = h264depay->vpudec->vpu_write_data_from_file(m_fp,  datatype);
            if(err < 0)
            {
                // zprintf4("read errorrrr!\n");
                break;
            }
            if(frameid < 3)
                frameid++;
            else
                usleep(32000);
            if(state == RTSP_OK)
            {
                zprintf1("state ok!\n");
                break;
            }

        }

        fclose(m_fp);
        m_fp = NULL;
    }

    return ;
}


void RTSP::run()
{
    link_state_image_process();
}

void RTSP::change_rtsp_state(int st)
{
    if(st != state)
    {
        state= st;
        sem_post(&m_imagesem);
    }
}

int RTSP::rtsp_init(string ip)
{
    GstRTSPMessage msg;
    string msg_str;
    char buf[2048] = {0};
    int ret;
    int err = -1;
    cseq = 1;
    if(initport != 51160)
        initport = 51160;
    else
        initport = 51168;

    memset(&msg, 0x00, sizeof(msg));

//    url = "rtsp://169.254.1.168/0";
    url = "rtsp://" + ip + "/0";
    ipaddr = ip;
    zprintf2("rtsp start display %s!\n", url.c_str());

    if(h264depay == NULL)
    {
        h264depay = new H264Depay;
        h264depay->rtp_h264_init();
    }

    h264depay->rtp_restart_process();


    if(running ==0)
    {
        this->start("LinkImage_pthread");
    }



    if(link == NULL)
    {

        link = new NetlinkStatus("eth1");

        link->netlinkcb = eth_netlink_callback;
        link->netlinkfater = this;

        link->start("linkstate_pthread");
    }

    if(link->getLinkstate() != 1)
    {
        zprintf4("no link!\n");
        // h264depay->vpudec->vpu_change_mode(VPU_V_MJPG);
        state = RTSP_NO_LINK;
        sem_post(&m_imagesem);
        return err;
    }


    tcp_client_init(554, ip.c_str());
    ret = tcp_client_connect();
    err--;
    if(ret < 0)
    {
        goto RTSP_ERROR;
    }

//    if(m_isplay == 1)
//    {
//        gst_rtsp_message_init_request(&msg, GST_RTSP_TEARDOWN, url.c_str());
//        msg_str = message_to_string(&msg);
//        ret = tcp_write((void *)msg_str.c_str(), msg_str.length());
//        err--;
//        if(ret < 0)
//        {
//            goto RTSP_ERROR;
//        }

//        ret = tcp_recv(buf, sizeof(buf));
//        err--;
//        if (ret > 0) {
//            printf("h264 stop play!\n");

//        } else if (ret < 0) {
//            printf("%s: errno:%d\n", __FUNCTION__, errno);
//            goto RTSP_ERROR;

//        } else if (0 == ret) {
//            printf("server fd[%d] disconnect\n", socket_fd);
//            goto RTSP_ERROR;
//        }
//        m_isplay = 0;
//    }


    gst_rtsp_message_init_request(&msg, GST_RTSP_OPTIONS, url.c_str()); //options 请求

    msg_str = message_to_string(&msg);

    zprintf3("optinos %s!\n", msg_str.c_str());

    ret = tcp_write((void *)msg_str.c_str(), msg_str.length());
    err--;
    if(ret < 0)
    {
        goto RTSP_ERROR;
    }

    ret = tcp_recv( buf, sizeof(buf));
    err--;
    if (ret < 0) {
        zprintf1("%s: errno:%d\n", __FUNCTION__, errno);
        goto RTSP_ERROR;
    } else if (0 == ret) {
        zprintf1("server fd[%d] disconnect\n", socket_fd);
        goto RTSP_ERROR;
    }
     //parse options

   gst_rtsp_message_init_request(&msg, GST_RTSP_DESCRIBE, url.c_str());
   msg_str = message_to_string(&msg);

   ret = tcp_write((void *)msg_str.c_str(), msg_str.length());
   err--;
   if(ret < 0)
   {
       goto RTSP_ERROR;
   }

    ret = tcp_recv(buf, sizeof(buf));
    err--;
    if (ret < 0)
    {
        zprintf1("%s: errno:%d\n", __FUNCTION__, errno);
        goto RTSP_ERROR;
    } else if (0 == ret) {
        zprintf1("server fd[%d] disconnect\n", socket_fd);
        goto RTSP_ERROR;
    }

   zprintf3("zty sdp len %d!\n", ret);

   ret = tcp_recv(buf, sizeof(buf));   //两次接收 由于tcp分包
   err--;
   if (ret > 0) {
//       printf("recv data[%s] from server\n", buf);
   } else if (ret < 0) {
       zprintf1("%s: errno:%d\n", __FUNCTION__, errno);
   } else if (0 == ret) {
       zprintf1("server fd[%d] disconnect\n", socket_fd);
   }
   zprintf3("zty sdp len %d!\n", ret);


   udprtp = new RTP;

   udprtp->set_protocol(h264depay);
   udprtp->rxcallback = h264_pro_rxdata_callback;
   udprtp->rtp_init(session, initport);

   udprtp->net_father = this;
   udprtp->netstatecb = rtp_netlink_callback;
   udprtp->start("rtpclient_pthread");

   gst_rtsp_message_init_request(&msg, GST_RTSP_SETUP, url.c_str());
   msg_str = message_to_string(&msg);

   ret = tcp_write((void *)msg_str.c_str(), msg_str.length());
   err--;
   if(ret < 0)
   {
       goto RTSP_ERROR;
   }

   ret = tcp_recv(buf, sizeof(buf));
   err--;
   if (ret > 0) {
        setup_message_parse(buf, ret);

   } else if (ret < 0) {
       zprintf1("%s: errno:%d\n", __FUNCTION__, errno);
       goto RTSP_ERROR;

   } else if (0 == ret) {
       zprintf1("server fd[%d] disconnect\n", socket_fd);
       goto RTSP_ERROR;
   }


   gst_rtsp_message_init_request(&msg, GST_RTSP_PLAY, url.c_str());
   msg_str = message_to_string(&msg);
   ret = tcp_write((void *)msg_str.c_str(), msg_str.length());
   err--;
   if(ret < 0)
   {
       goto RTSP_ERROR;
   }

   ret = tcp_recv(buf, sizeof(buf));
   err--;
   if (ret > 0) {
       zprintf1("h264 start play!\n");

   } else if (ret < 0) {
       zprintf1("%s: errno:%d\n", __FUNCTION__, errno);
       goto RTSP_ERROR;

   } else if (0 == ret) {
       zprintf1("server fd[%d] disconnect\n", socket_fd);
       goto RTSP_ERROR;
   }
   m_isplay = 1;

   return 0;

RTSP_ERROR:
    if(h264depay != NULL)
    {
        // if(state != RTSP_NO_DATA)
        // {
        //     // h264depay->vpudec->vpu_close();
        //     // h264depay->vpudec->vpu_open(VPU_V_MJPG);
        // }
        change_rtsp_state(RTSP_NO_DATA);
    }
   zprintf1("rtsp init error %d!\n", err);
   return ret;

}

int RTSP::rtsp_run(void)
{

    while(1)
    {
        while(state != RTSP_OK)
        {
            sleep(1);
            rtsp_stop();

            zprintf3("init rstp!\n");
            if(rtsp_init(ipaddr) == 0)
                break;
        }
        sem_wait(&m_restartsem);
    }
    return 0;
}

int  RTSP::rtsp_stop(void)
{
    zprintf2("rtsp stop!\n");

    close_fd();

    if(udprtp != NULL)
    {
        delete udprtp;
        udprtp = NULL;
    }
    return 0;
}

