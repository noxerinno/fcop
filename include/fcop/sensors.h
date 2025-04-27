#ifndef SENSORS_DATA_H
#define SENSORS_DATA_H

#include "DHT.h"

#include "fcop/debug.h"
#include "fcop/logger.h"


// DHT sensor confiration
#define DHT_PIN 17          // DHT data pin (choosen)
#define DHT_TYPE DHT11      // DHT type

// LDR sensor configuration
#define LDR_PIN 35          // LDR data pin (choosen)
#define LDR_MAX_VALUE 4096  // LDR max data (observed, not found in doc)

// Touch wake up configuration
#define TOUCH_WAKE_UP_PAD T0
#define TOUCH_WAKE_UP_THRESHOLD 40


// Sensor data structure definiton
struct SensorData {
    char timestamp[9];      // Format: HH:MM:SS (8 caractères + '\0')
    float temperature;
    int humidity, luminosity;

    // Return a string from 
    const char* toString() const {
        static char buffer[23];             // Static buffer. Worst case scenario : double digit negative temp make a 23 char long string
        
        std::sprintf(buffer, "%s,%.1f,%d,%d", timestamp, temperature, humidity, luminosity);  // String formatting
        
        return buffer; 
    }
};


void sensorsInitialization();
SensorData mesure(int hours, int minutes, int seconds);


#endif  // SENSORS_DATA_H