#include "netlinkstatus.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <net/if.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <sys/ioctl.h>
#include <linux/ethtool.h>
#include <linux/sockios.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netdb.h>

NetlinkStatus::NetlinkStatus(string ethname):m_netlinkcb(NULL),m_linkFather(NULL),m_linkState(0),m_eth(ethname),
    m_nlSock(0)
{
    int skfd;
    int err;
    struct ifreq ifr;
    struct ethtool_value edata;


    edata.cmd = ETHTOOL_GLINK;
    edata.data = 0;


    memset(&ifr, 0x00, sizeof(ifr));
    memcpy(ifr.ifr_name, ethname.c_str(), ethname.length());
    ifr.ifr_data = (char *) &edata;

    if (( skfd = socket( AF_INET, SOCK_DGRAM, 0 )) == 0)
    {
        zprintf1("netlinkstatus skfd socket fail!\n");
        return;
    }
    err = ioctl(skfd,  SIOCETHTOOL, &ifr);
    if(!err){
         zprintf1("zty edata.data %d!\n", edata.data);
         m_linkState = edata.data;
    }
    else
    {
        zprintf1("zty ioctl error!\n");
        perror("ioctl:");
    }
    close(skfd);

}

NetlinkStatus::~NetlinkStatus()
{
    zprintf3("delete netlinkstatus!\n");
    stop();

    if(m_nlSock >0)
    {
        close(m_nlSock);
        m_nlSock = 0;
    }
    m_linkFather = NULL;
    m_netlinkcb = NULL;

}

int NetlinkStatus::getNetState(string name, struct  NetInfo & val)
{
    int skfd;
    int err;
    struct ifreq ifr;
    struct ethtool_value edata;
    struct ethtool_cmd   espeed;

    memset(&ifr, 0x00, sizeof(ifr));
    memcpy(ifr.ifr_name, name.c_str(), name.length());
    edata.cmd = ETHTOOL_GLINK;
    ifr.ifr_data = (char *) &edata;

    if (( skfd = socket( AF_INET, SOCK_DGRAM, 0 )) == 0)
    {
        zprintf1("netlinkstatus skfd socket fail!\n");
        return -1;
    }
    err = ioctl(skfd,  SIOCETHTOOL, &ifr);
    if(!err){
        zprintf1("zty state %d!\n", edata.data);
        val.m_status = edata.data;
    }
    else
    {
        zprintf1("zty ioctl error!\n");
        perror("ioctl:");
        return -2;
    }


    espeed.cmd = ETHTOOL_GSET;
    ifr.ifr_data = (char *) &espeed;

    err = ioctl(skfd,  SIOCETHTOOL, &ifr);
    if(!err){
        zprintf1("zty speed %d!\n", espeed.speed);
        val.m_speed = espeed.speed;
    }
    else
    {
        zprintf1("zty ioctl error!\n");
        perror("ioctl:");
        return -2;
    }
    close(skfd);
    return 0;
}

int NetlinkStatus::getLinkstate(void)
{
    return m_linkState;
}


int NetlinkStatus::getNetInfo(void)
{
    struct ifaddrs *ifaddr, *ifa;
    struct NetInfo tmpInfo;
    memset(&tmpInfo, 0x00, sizeof(tmpInfo));
    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        return -1;
    }

    int idx = 0;
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) {
            continue;
        }

        int family = ifa->ifa_addr->sa_family;
        if (family == AF_INET)
        {
            struct sockaddr_in *addr = (struct sockaddr_in *)(ifa->ifa_addr);
            struct sockaddr_in *mask = (struct sockaddr_in *)(ifa->ifa_netmask);
            struct sockaddr_in *gw = (struct sockaddr_in *)(ifa->ifa_dstaddr);

            if (strcmp(ifa->ifa_name, "lo") != 0)
            {
                strncpy(tmpInfo.m_name, ifa->ifa_name, sizeof(tmpInfo.m_name));
                inet_ntop(AF_INET, &addr->sin_addr, tmpInfo.m_ip, sizeof(tmpInfo.m_ip));
                inet_ntop(AF_INET, &mask->sin_addr, tmpInfo.m_netmask, sizeof(tmpInfo.m_netmask));
                inet_ntop(AF_INET, &gw->sin_addr, tmpInfo.m_gw, sizeof(tmpInfo.m_gw));
                printf("net name %s ip %s!\n", tmpInfo.m_name, tmpInfo.m_ip);
                strcpy(tmpInfo.m_dns, "8.8.8.8"); // DNS地址可以使用Google的公共DNS
                tmpInfo.m_speed = 100; // 网络速度初始值为100
                tmpInfo.m_status = 0; // 网络状态初始值为0
                // string tmpName = ;
                getNetState(tmpInfo.m_name, tmpInfo);

                m_netInfo.push_back(tmpInfo);
                /* 添加其他关键信息，如网络速度、状态等 */
                idx++;
            }
        }
    }

    freeifaddrs(ifaddr);

    return idx;
}

void NetlinkStatus::run()
{
    struct sockaddr_nl nladdr;
    int status;
    char buf[2048];
    struct iovec iov;
    struct msghdr msg;
    struct rtattr *attr;
    int len;

//    int err;
//    struct ifreq ifr;

    memset(&nladdr, 0, sizeof(nladdr));
    nladdr.nl_family = AF_NETLINK;
    nladdr.nl_groups = RTNLGRP_LINK;

    m_nlSock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (m_nlSock < 0) {
        zprintf1("netlink nl sock error!\n");
        perror("socket");
        exit(EXIT_FAILURE);
    }

    if (bind(m_nlSock, (struct sockaddr *)&nladdr, sizeof(nladdr)) < 0) {
        zprintf1("netlink bind error!\n");
        perror("bind");
        close(m_nlSock);
        exit(EXIT_FAILURE);
    }

    iov.iov_base = buf;
    iov.iov_len = sizeof(buf);
    memset(&msg, 0, sizeof(msg));
    msg.msg_name = &nladdr;
    msg.msg_namelen = sizeof(nladdr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    while (running) {
//        iov.iov_base = buf;
//        iov.iov_len = sizeof(buf);
//        memset(&msg, 0, sizeof(msg));
//        msg.msg_name = &nladdr;
//        msg.msg_namelen = sizeof(nladdr);
//        msg.msg_iov = &iov;
//        msg.msg_iovlen = 1;

        status = recvmsg(m_nlSock, &msg, 0);
        if (status < 0) {
            zprintf1("netlinkstatus recvmsg error!\n");
            perror("recvmsg");
            close(m_nlSock);
            exit(EXIT_FAILURE);
        }
//        printf("receive mes len %d!\n", status);
//        printf("receive msg is %s!\n", msg.msg_iov->iov_base);
        struct nlmsghdr *h;
        for (h = (struct nlmsghdr *)buf; NLMSG_OK(h, (unsigned int)status); h = NLMSG_NEXT(h, status)) {
            if (h->nlmsg_type == NLMSG_DONE) {
                break;
            }
            if (h->nlmsg_type == NLMSG_ERROR) {
                fprintf(stderr, "Error received in netlink message.\n");
                close(m_nlSock);
                exit(EXIT_FAILURE);
            }
            // 处理接口变化事件
            if (h->nlmsg_type == RTM_NEWLINK) {
                struct ifinfomsg *iface_msg = (struct ifinfomsg *)NLMSG_DATA(h);
//                zprintf1("iface_msg->ifi_flags 0x%x, iface_msg->ifi_index %u!\n", iface_msg->ifi_flags, iface_msg->ifi_index);
//                if (iface_msg->ifi_flags & IFF_UP) {

//                    if(iface_msg->ifi_flags & IFF_RUNNING)
//                    {
//                        zprintf1("网线已连接\n");
//                        linkstate = 1;
//                    }
//                     else {
//                        zprintf1("网线已断开\n");
//                        linkstate = 0;
//                    }

                    attr = (struct rtattr*)(((char*)h) + NLMSG_SPACE(sizeof(*iface_msg)));
                    len = h->nlmsg_len - NLMSG_SPACE(sizeof(*iface_msg));
                    for (; RTA_OK(attr, len); attr = RTA_NEXT(attr, len))
                    {
                        if (attr->rta_type == IFLA_IFNAME)
                        {
                            zprintf1("ifname %s !\n", (char*)RTA_DATA(attr));
                            string ethname = (char*)RTA_DATA(attr);

                            if(m_eth == ethname)
                            {
                                if (iface_msg->ifi_flags & IFF_UP) {

                                    if(iface_msg->ifi_flags & IFF_RUNNING)
                                    {
                                        zprintf1("网线已连接\n");
                                        printf("网线已连接\n");
                                        m_linkState = 1;

                                    }
                                     else {
                                        zprintf1("网线已断开\n");
                                        printf("网线已断开\n");
                                        m_linkState = 0;

                                    }
                                    if(m_netlinkcb != NULL)
                                    {
                                        this->m_netlinkcb(this, m_linkState);
                                    }
                            }

                            break;
                        }
                    }
                }
            }
        }
    }
    zprintf1("zty netlink end!\n");
    close(m_nlSock);
    m_nlSock = 0;
    return;
}

