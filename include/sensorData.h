#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H

// Sensor data structure definiton
struct SensorData {
    char timestamp[9];    // Format: HH:MM:SS (8 caractères + '\0')
    float temperature;
    int humidity, luminosity;

    // Return a string from 
    const char* toString() const {
        static char buffer[23];             // Static buffer. Worst case scenario : double digit negative temp make a 23 char long string
        
        std::sprintf(buffer, "%s,%.1f,%d,%d", timestamp, temperature, humidity, luminosity);  // String formatting
        
        return buffer; 
    }
};

#endif // SENSOR_DATA_H
