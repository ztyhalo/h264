
#include "rtsp/rtsp.h"
#include "zprint/zprint.h"

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
    (void)context;
    printf("Dump path: %s\n", descriptor.path());
    string path =  descriptor.path();
    string dumpcmd = "/opt/bin/breakpad/dump.sh h264 " + path;
    system(dumpcmd.c_str());
    return succeeded;
}
//void crash() { volatile int* a = (int*)(NULL); *a = 1; }
#endif

#define H264_VERSION "2.0.3"

RTSP          * gRTSP = NULL;


void SignalFunc(int var)
{
    ::printf("<h264 signal1 exit %d val!>\n", var);

    zprintf1("h264 app stop: ");

    if(gRTSP != NULL)
    {
        delete gRTSP;
        gRTSP = NULL;
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


    gRTSP = new RTSP;

//    RTSP rtsp;
//    rtsp.rtsp_init(ipaddr);
//    rtsp.rtsp_run();

    zprintf4("ip %s!\n", ipaddr.c_str());
    gRTSP->rtsp_init(ipaddr);
    gRTSP->rtsp_run();




#ifdef ARM
//    vpudec = new VpuDec;
//    vpudec->vpu_init();

//    delete vpudec;
#endif //ARM
    return 0;
}
