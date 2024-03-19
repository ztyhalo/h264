#include "zprint.h"
#include <assert.h>
#include <stdarg.h>
#include <time.h>




PRINTF_CLASS *debug_p = new PRINTF_CLASS(DEBUG_F_DIR, 1);



void PRINTF_CLASS::zprintf(const char * format, ...)
{
    va_list args;

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
           if(remove(name) == 0 )
                printf("Removed %s.\n", name);
           else
                printf("Removed %s. failed!\n", name);

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

