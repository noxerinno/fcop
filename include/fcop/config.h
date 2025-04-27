#ifndef CONFIG_H
#define CONFIG_H

#include "fcop/sensors.h"

#define MESURE_INTERVAL 30      // Mesurement interval (in seconds). Intervals smaller than 5s are not recommended 

extern bool rtcIsFirstBoot; 
extern SensorData rtcLastMesurement;

#endif  // CONFIG_H