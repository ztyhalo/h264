#ifndef __MUTEX_CLASS_H__
#define __MUTEX_CLASS_H__
#include <stdio.h>
#include <pthread.h>
#include <iostream>

using namespace std;


class MUTEX_CLASS
{
private:
    pthread_mutex_t   mut;
public:
    MUTEX_CLASS()
    {
        pthread_mutex_init(&mut, NULL);
    }
    ~MUTEX_CLASS()
    {
        pthread_mutex_destroy(&mut);
    }
    int lock(void){
        return pthread_mutex_lock(&mut);
    }
    int unlock(void){
        return pthread_mutex_unlock(&mut);
    }

};

#endif //__MUTEX_CLASS_H__
