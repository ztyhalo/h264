/*
 * File:   timer_poll.h
 * Author: Administrator
 *
 * 文件监管库V1.1
 */
 
#ifndef TIMER_POLL_H
#define TIMER_POLL_H
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/epoll.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/timerfd.h>
#include <unistd.h>
#include <pthread.h>
#include <map>
#include <errno.h>
#include <iostream>
#include "zprint/zprint.h"

#define MAXFDS 4
#define EVENTS 100

class z_poll
{
public:
    z_poll(int max_num= MAXFDS)
    {
        epfd = epoll_create(max_num);
        if(epfd == -1)
        {
            zprintf1("epoll creat failed!\n");
            eposize = 0;
            active = 0;
            return;
        }
        active = 1;
        eposize = max_num;
    }

    int e_poll_add(int fd)
    {
        if(setNonBlock(fd) == false)
            return -1;
        int err = 0;
        struct epoll_event ev;
        memset(&ev, 0x00, sizeof(struct epoll_event));
        ev.data.fd = fd;
        ev.events = EPOLLIN | EPOLLET;
        err = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
        if(err == -1)
        {
            zprintf1("%s\n",strerror(errno));
            return err;
        }
        return 0;
    }

    int e_poll_del(int fd)
    {
        struct epoll_event ev;
        ev.data.fd = fd;
        ev.events = EPOLLIN | EPOLLET;
        int err = epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &ev);
        if(err == -1)
        {
            zprintf1("%s\n",strerror(errno));
            return err;
        }

        return 0;
    }
    int e_poll_deactive()
    {
        active = 0;
        return 0;
    }
     int get_epoll_size(){
        return eposize;
    }
    bool setNonBlock (int fd)
    {
         int flags = fcntl (fd, F_GETFL, 0);
         flags |= O_NONBLOCK;
         if (-1 == fcntl (fd, F_SETFL, flags))
         {
             zprintf1("fd%d set non block failed!\n", fd);
             return false;
         }

         return true;
    }

    int wait_fd_change(int time)
    {
         struct epoll_event events[get_epoll_size()];
         memset(&events, 0, sizeof(events));
         int nfds = epoll_wait(epfd, events, get_epoll_size(), time);
         //printf("zc   nfd ====== %x\r\n",nfds);
         if(nfds > 0)
         {
             return nfds;
         }
         else
             return -1;
    }
    ~ z_poll()
    {
        zprintf4("destory zpoll!\n");
        if(active)
            close(epfd);
    }
public:
    int epfd;
    int active;
    int eposize;
};

class Pth_Class
{
private:
    pthread_t pid;
private:
     static void * start_thread(void * arg){
            int res = pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,   NULL);   //设置立即取消
            if (res != 0)
            {
                perror("Thread pthread_setcancelstate failed");
                exit(EXIT_FAILURE);
            }
          ((Pth_Class *)arg)->run();
         return NULL;
     }
public:
     Pth_Class(){
         pid = 0;
     }
     ~Pth_Class(){
         zprintf4("destory Pth_Class!\n");
         if(pid > 0){
            pthread_cancel(pid);
            pthread_join(pid, NULL);
            pid = 0;
         }
         zprintf4("destory Pth_Class delete over!\n");
     }
//     void pth_class_exit(void){
//          if(pid > 0){
//             pthread_cancel(pid);
//             pthread_join(pid, NULL);
//             pid = 0;
//          }
//          zprintf3("destory Pth_Class delete over!\n");
//     }

     int start(){
         if(pid == 0)
         {
             if(pthread_create(&pid, NULL, start_thread,this) != 0)
             {
                 zprintf1("creat pthread failed!\n");
                 return -1;
             }
             else
             {
                 return 0;
             }
         }
         zprintf1("pid %d have creat\n",(int)pid);
         return -1;
     }

     virtual void run() = 0;


};

class NCbk_Poll:public z_poll,public Pth_Class
{
public:
     NCbk_Poll(int max):z_poll(max){
     }
};

class Cbk_Poll:public z_poll
{
private:
    pthread_t pid;
private:
     static void * start_thread(void * arg){
          ((Cbk_Poll *)arg)->run();
         return NULL;
     }

public:
     Cbk_Poll(int max):z_poll(max){
         pid = 0;
     }

     int start(){
         if(pid == 0)
         {
             if(pthread_create(&pid, NULL, start_thread,this) != 0)
             {
                 zprintf1("creat pthread failed!\n");
                 return -1;
             }
             else
                 return 0;
         }
     }

     virtual void run() = 0;
};


 
#endif  /* TIMER_POLL_H */
