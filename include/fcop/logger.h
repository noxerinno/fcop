#ifndef LOGGER_H
#define LOGGER_H

#include "FS.h"
#include "SPIFFS.h"

#include "fcop/debug.h"

#define LOG_FILE_HEADER "TIMESTAMP,TEMPERATURE,HUMIDITY,LUMINOSITY"
extern char rtcLogFilename[14];  		// Allocate 20 char  in RTC memory (1 for the '/', 4 for the year, 2 for ther month, 2 for the day, 4 for '.log' and 1 for '\0')

void mountSPIFFS();
void setRtcLogFilename(int year, int month, int day);
void readLogFile();
void writeLog(const char* mesurementString);
void deleteCurrentLogFile();

#endif  // LOGGER_H