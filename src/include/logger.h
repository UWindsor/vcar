#include <time.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>

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

  std::ofstream logFile;
  logFile.open(LOGFILE);

  if (logFile.fail())
    return -1;

  logFile << "\n[" << timeinfo->tm_mday << "/" << timeinfo->tm_mon + 1 << "/" << timeinfo->tm_year + 1900 << " "
    << timeinfo->tm_hour + 2 << ":" << timeinfo->tm_min << ":" << timeinfo->tm_sec << "] LOG: " << str;

  logFile.close();

  return 1;
}
