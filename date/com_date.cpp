#include "com_date.h"



Com_Date::Com_Date()
{
    now = time(&now);
    localtime_r(&now, &tm);
}
 void
Com_Date::gen_date_string (string & date_string)
{

  static const char wkdays[7][4] =
      { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
  static const char months[12][4] =
      { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct",
    "Nov", "Dec"
  };

  char date[100];


  sprintf (date, "%s, %02d %s %04d %02d:%02d:%02d GMT",
      wkdays[tm.tm_wday], tm.tm_mday, months[tm.tm_mon], tm.tm_year + 1900,
      tm.tm_hour, tm.tm_min, tm.tm_sec);
  string tmp(date);

  date_string = date;
}
