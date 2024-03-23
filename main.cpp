#include <iostream>
//#include <string>
#include "rtsp/rtsp.h"
#ifdef ARM
#include "vpudec/vpudec.h"
#endif //ARM
using namespace std;

#ifdef ARM
VpuDec * vpudec;
#endif //ARM

#ifdef BREAKPAD
#include "exception_handler.h"
google_breakpad::ExceptionHandler* eh = NULL;
static bool dumpCallback(const google_breakpad::MinidumpDescriptor& descriptor, void* context, bool succeeded)
{
//    Q_UNUSED(context);
//    QString str = "/opt/bin/breakpad/dump.sh BusinessLogic " + QString(descriptor.path());
//    system(str.toStdString().c_str());
//    qDebug("commend: %s\n", str.toStdString().c_str());
    (void)context;
    printf("Dump path: %s\n", descriptor.path());
    string path =  descriptor.path();
    string dumpcmd = "/opt/bin/breakpad/dump.sh h264 " + path;
    system(dumpcmd.c_str());
    return succeeded;
}
#endif

#define H264_VERSION "2.0"

#include "netlinkstatus/netlinkstatus.h"
#include "zprint/zprint.h"

NetlinkStatus * glink = NULL;
//#define SPEAKER_CONFIG_FILE "/opt/config/voip/speaker"
//int speaker_val;
//int speaker_time = 1000;
//static int hn_open_speaker_config(void)
//{
//    char buf[8];
//    int i = 0;
//    FILE *fp;

//    fp = fopen(SPEAKER_CONFIG_FILE, "r");
//    if(fp == NULL)
//    {
//        printf("hn speaker value is no!\n");
//        speaker_val = 0;
//        return -1;
//    }
//    memset(buf, 0x00, 8);

//    while(fgets(buf, 8, fp) != NULL)
//    {
//        i++;
//        printf("line%d: %s", i, buf);
//        if(i == 1)
//            speaker_val = atoi(buf);
//        else if(i == 2)
//        {
//            if(strlen(buf) != 0 && buf[0] != '\n')
//            {
//                printf("%d %d!\n", buf[0], strlen(buf));
//                speaker_time = atoi(buf);
//            }
//        }
//    }
//    fclose(fp);
//    printf("zty speaker val %d speaker_time %d!\n", speaker_val, speaker_time);
//    return 0;
//}

void SignalFunc(int var)
{
    ::printf("<DeviceMng signal1 exit %d val!>\n", var);

    if (glink != NULL)
    {
        delete glink;
    }
#ifdef BREAKPAD
    if (eh != nullptr)
    {
        delete eh;
        eh = nullptr;
    }
#endif
    exit(0);
}
//void crash() { volatile int* a = (int*)(NULL); *a = 1; }
int main(int argc, char* argv[])
{
    string ipaddr = "169.254.1.168";

    if (argc == 2)
    {
        string ver = "version";
        string::size_type idx;
        ipaddr = argv[1];
        idx = ipaddr.find(ver);
        if(idx != string::npos)
        {
            printf("h264 version: %s!\n", H264_VERSION);
            return 0;
        }

    }
//    hn_open_speaker_config();
//    return 0;
    signal(SIGINT, SignalFunc);
    signal(SIGTERM, SignalFunc);

#ifdef BREAKPAD
    system("mkdir -p /opt/h264Logs/crashlog");
    google_breakpad::MinidumpDescriptor descriptor("/opt/h264Logs/crashlog");
    eh = new google_breakpad::ExceptionHandler(descriptor, NULL, dumpCallback, NULL, true, -1);
//    crash();
#endif

    PRINTF_CLASS::getInstance()->printf_class_init("/opt/h264Logs/");
    zprintf3(".......................................................\n");
    zprintf1("H264-%s start:\n", H264_VERSION);



    glink = new NetlinkStatus("eth1");

    glink->start();

//    while
//    while (1) {

//        sleep(1);

//    }


    RTSP rtsp;
     cout << "ip " << ipaddr << " !" <<endl;
     rtsp.rtsp_init(ipaddr);
//    rtsp.rtsp_init("169.254.1.168");
#ifdef ARM
//    vpudec = new VpuDec;
//    vpudec->vpu_init();

//    delete vpudec;
#endif //ARM
    return 0;
}
