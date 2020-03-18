#include <time.h>
#include <stdio.h>
#include <string>


#define LOGFILE "virtualcar.log"     // all Log(); messages will be appended to this file

/**
 * Log message to a logfile
 * 
 */
int Log (std::string str, ...)
{
  time_t rawtime;
  struct tm * timeinfo;

  time(&rawtime);
  timeinfo = localtime(&rawtime);

  FILE * fp = fopen(LOGFILE, "a");

  if (fp == NULL)
    return -1;

  fprintf(fp, "\n[%i/%i/%i %i:%i:%i] LOG: %s", 
        timeinfo->tm_mday, 
        timeinfo->tm_mon + 1, 
        timeinfo->tm_year + 1900, 
        timeinfo->tm_hour + 2, 
        timeinfo->tm_min, 
        timeinfo->tm_sec, str);

  fclose(fp);

  return 1;
}
