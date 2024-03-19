#ifndef COM_DATE_H
#define COM_DATE_H
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <string>
#include <iostream>
#include <sys/time.h>
using namespace std;
class Com_Date
{

public:
    Com_Date();
    void gen_date_string (string & date_string);
public:
    time_t now;
    struct tm tm;

};

#endif // COM_DATE_H
