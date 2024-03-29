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

NetlinkStatus::NetlinkStatus(string ethname)
{
    int skfd;
    int err;
    struct ifreq ifr;
    struct ethtool_value edata;


    edata.cmd = ETHTOOL_GLINK;
    edata.data = 0;
    linkstate = 0;
    nl_sock = 0;
    netlinkfater = NULL;

    eth = ethname;

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
         linkstate = edata.data;
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

    if(nl_sock >0)
    {
        close(nl_sock);
        nl_sock = 0;
    }
    netlinkfater = NULL;
    netlinkcb = NULL;

}

int NetlinkStatus::getLinkstate(void)
{
    return linkstate;
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

    nl_sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (nl_sock < 0) {
        zprintf1("netlink nl sock error!\n");
        perror("socket");
        exit(EXIT_FAILURE);
    }

    if (bind(nl_sock, (struct sockaddr *)&nladdr, sizeof(nladdr)) < 0) {
        zprintf1("netlink bind error!\n");
        perror("bind");
        close(nl_sock);
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

        status = recvmsg(nl_sock, &msg, 0);
        if (status < 0) {
            zprintf1("netlinkstatus recvmsg error!\n");
            perror("recvmsg");
            close(nl_sock);
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
                close(nl_sock);
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

                            if(eth == ethname)
                            {
                                if (iface_msg->ifi_flags & IFF_UP) {

                                    if(iface_msg->ifi_flags & IFF_RUNNING)
                                    {
                                        zprintf1("网线已连接\n");
                                        printf("网线已连接\n");
                                        linkstate = 1;

                                    }
                                     else {
                                        zprintf1("网线已断开\n");
                                        printf("网线已断开\n");
                                        linkstate = 0;

                                    }
                                    if(netlinkcb != NULL)
                                    {
                                        this->netlinkcb(this, linkstate);
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
    close(nl_sock);
    nl_sock = 0;
    return;
}

