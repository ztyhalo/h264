#include "zprint.h"
#include <assert.h>
#include <stdarg.h>
#include <time.h>




//PRINTF_CLASS *debug_p = new PRINTF_CLASS(DEBUG_F_DIR, 1);
PRINTF_CLASS * PRINTF_CLASS::m_pSelf = NULL;
PRINTF_CLASS *debug_p = PRINTF_CLASS::getInstance();

static int hn_open_level_config(const char * file)
{
    char buf[8];
    int i = 0;
    FILE *fp;

    fp = fopen(file, "r");
    if(fp == NULL)
    {
        zprintf1("hn level config no!\n");
        return -1;
    }
    memset(buf, 0x00, 8);

    if(fgets(buf, 8, fp) != NULL)
    {
        i = atoi(buf);
    }

    fclose(fp);


    return i;
}

PRINTF_CLASS::PRINTF_CLASS()
{
    pfd = stdout;
    mark = 0;
    level = PRINT_PRO;
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
                int val;
                gettimeofday(&tv, NULL);
                p = localtime(&tv.tv_sec);
                memset(buf, 0x00, sizeof(buf));
                sprintf(buf,"%02d-%02d_%02d_%02d_%02d.log",
                       1+p->tm_mon, p->tm_mday,
                       p->tm_hour, p->tm_min, p->tm_sec);
                string log = buf;

                string dirlog = dir + log;
                string lev    = dir + "level";

                val = hn_open_level_config(lev.c_str());
                if(val > 0)
                {
                    level = val;
                }


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
    if(mark == 0 || level < 3) return;
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

void PRINTF_CLASS::hprintf(const char * format, ...)
{
    va_list args;
    if(mark == 0 || level < 4) return;

    lock();
    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args);
    fflush(stdout);
    unlock();

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
    if(mark == 0 || level < 2) return;
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
    if(mark == 0 || level < 1) return;
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

