#include "zprint.h"
#include <assert.h>
#include <stdarg.h>
#include <time.h>




//PRINTF_CLASS *debug_p = new PRINTF_CLASS(DEBUG_F_DIR, 1);
PRINTF_CLASS * PRINTF_CLASS::m_pSelf = NULL;
PRINTF_CLASS *debug_p = PRINTF_CLASS::getInstance();

PRINTF_CLASS::PRINTF_CLASS()
{
    pfd = stdout;
    mark = 0;
}

PRINTF_CLASS * PRINTF_CLASS::getInstance(void)
{
    if(m_pSelf == NULL)
    {
        m_pSelf = new PRINTF_CLASS();
    }
    return m_pSelf;
}


void PRINTF_CLASS::printf_class_init(string dir)
{
    struct tm *p;
    struct timeval tv;
    char buf[48];


    if(!dir.empty())
    {
        if(dir == "stdout")
        {
            mark = 1;
        }
        else
        {
             if (access(dir.c_str(), F_OK) == 0) // dir exist
             {

                gettimeofday(&tv, NULL);
                p = localtime(&tv.tv_sec);
                memset(buf, 0x00, sizeof(buf));
                sprintf(buf,"%02d-%02d_%02d_%02d_%02d.log",
                       1+p->tm_mon, p->tm_mday,
                       p->tm_hour, p->tm_min, p->tm_sec);
                string log = buf;

                string dirlog = dir + log;

                pfd =  fopen(dirlog.c_str(), "a+");
                if(pfd == NULL)
                {
                    printf("file %s open fail!\n", dirlog.c_str());
                }
                else
                    mark = 1;
             }
        }
    }
    else
        mark = 0;
}

void PRINTF_CLASS::zprintf(const char * format, ...)
{
    va_list args;
    if(mark == 0) return;
    if(pfd != NULL)
    {
        lock();
        va_start(args, format);
        vfprintf(pfd, format, args);
        va_end(args);
        fflush(pfd);
        unlock();
    }
}

void PRINTF_CLASS::printf_init(const char * name, int fd)
{
    if(fd == 1)          //标准输出
    {
        pfd = stdout;
    }
    else
    {
        if(pfd != NULL && pfd != stdout)
        {
            fclose(pfd);
        }
        pfd = NULL;
        if(name != NULL)
        {
           // if(remove(name) == 0 )
           //      printf("Removed %s.", name);
           // else
           //      printf("Removed %s. failed!", name);

            pfd = fopen(name, "a+");
            if(pfd == NULL)
            {
                printf("file %s open fail\n", name);
            }
        }
    }
}

void PRINTF_CLASS::timeprintf(const char * format, ...)
{
    va_list args;
    if(mark == 0) return;
     lock();
    if(pfd != NULL)
    {
        struct tm *p;
        struct timeval tv;
        va_start(args, format);

        gettimeofday(&tv, NULL);
        p = localtime(&tv.tv_sec);

        fprintf(pfd,"%d-%02d-%02d %02d:%03d:%02d ",
               1900+p->tm_year, 1+p->tm_mon, p->tm_mday,
               p->tm_hour, p->tm_min, p->tm_sec);
        vfprintf(pfd, format, args);
        va_end(args);
        fflush(pfd);

    }
    unlock();
}

void PRINTF_CLASS::timemsprintf(const char * format, ...)
{
    va_list args;
    if(mark == 0) return;
    lock();
    if(pfd != NULL)
    {
        struct tm *p;
        struct timeval tv;
        va_start(args, format);

        gettimeofday(&tv, NULL);
        p = localtime(&tv.tv_sec);

        fprintf(pfd,"%d-%02d-%02d %02d:%02d:%02d.%06ld ",
               1900+p->tm_year, 1+p->tm_mon, p->tm_mday,
               p->tm_hour, p->tm_min, p->tm_sec, tv.tv_usec);
        vfprintf(pfd, format, args);
        va_end(args);
        fflush(pfd);

    }
    unlock();
}

