#ifndef RTC_MANAGER_H
#define RTC_MANAGER_H

#include "time.h"
#include <WiFi.h>

#include "fcop/credentials.h"
#include "fcop/debug.h"

// NTP configuration
#define NTP_SERVER "pool.ntp.org"       // NTP server 
#define NTP_TIME_ZONE 3600              // NTP timezone = UTC+1
#define NTP_DAY_LIGHT_OFFSET 3600       // NTP day ligth offset = summer hour
#define NTP_MAX_CONNECTION_ATTEMPTS	10  // NTP max connection atempts (10 = 5 sec)

tm setRTC();

#endif  // RTC_MANAGER_H