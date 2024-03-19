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

int main(int argc, char* argv[])
{
    RTSP rtsp;
    string ipaddr = "169.254.1.168";


    if (argc == 2)
    {
        ipaddr = argv[1];
    }
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
