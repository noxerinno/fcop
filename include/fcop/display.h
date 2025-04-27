#ifndef DISPLAY_h
#define DISPLAY_H

#include <Adafruit_SSD1306.h>
// #include <Adafruit_GFX.h>
#include <Wire.h>

#include "fcop/sensors.h"


// Screen configuration 
#define MESUREMENT_DISPLAY_DURATION 5000
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDRESS 0x3C		// I2C screen address
#define OLED_RESET -1

void initializeScreen();
void displayWelcomeMessage();
void displayMesurement(SensorData mesurement);

#endif  // DISPLAY_H