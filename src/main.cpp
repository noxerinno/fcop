#include <Arduino.h>

#include "fcop/cloudUploader.h"
#include "fcop/config.h"
#include "fcop/credentials.h"
#include "fcop/debug.h"
#include "fcop/display.h"
#include "fcop/logger.h"
#include "fcop/rtcManager.h"
#include "fcop/sensors.h"


RTC_DATA_ATTR int rtcDebugCycleCounter =  0;
RTC_DATA_ATTR bool rtcIsFirstBoot = true; 
RTC_DATA_ATTR SensorData rtcLastMesurement;


// Calculate next deep sleep interval (to the micro seconds)
uint64_t calculateNextInterval() {
	struct timeval timeInterval;								 // Time interval precise to the microsecond to precisely calculate next deep sleep interval
	gettimeofday(&timeInterval, nullptr);	 		  // Retreive RTC value

	uint64_t currentSec = timeInterval.tv_sec;						// Get seconds 
	uint64_t currentMicroSec = timeInterval.tv_usec;	   // Get micro seconds

	uint64_t nextStep = ((currentSec / MESURE_INTERVAL) + 1) * MESURE_INTERVAL;
	uint64_t deltaInSec = nextStep - currentSec;

	return (deltaInSec * 1000000ULL) - currentMicroSec;
}


void setup() {
	// Initialize "debugger"
	Serial.begin(115200);

	struct tm timeInfo;						// Time information precise to the second (date to create log filename one first boot & HH:MM:SS to timestamp the log entries )

	
	// First boot handeling
	if (rtcIsFirstBoot) {
		initializeScreen();
		displayWelcomeMessage();

		timeInfo = setRTC();				// RTC setup
		mountSPIFFS();
		setRtcLogFilename(timeInfo.tm_year, timeInfo.tm_mon, timeInfo.tm_mday);
		if (DEBUG_SLEEP_CYCLES) {Serial.printf("\n\n==================== FIRST BOOT AT %02d:%02d:%02d ====================\n", timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec);}
	}
	
	
	getLocalTime(&timeInfo); 		// Retreive RTC value


	// On timer wakeup, take mesurement and log it memory 
	if (rtcIsFirstBoot || esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER) {
		if (DEBUG_SLEEP_CYCLES && !rtcIsFirstBoot) {Serial.printf("\n\n================ WOKE UP AT %02d:%02d:%02d FROM TIMER ================\n", timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec);}
		// Sensor initialization
		sensorsInitialization();

		// SPIFFS & Log setup
		mountSPIFFS();		// Mount the file system

		// Take a mesurment
		SensorData mesurement = mesure(timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec);
		rtcLastMesurement = mesurement;

		// Writing data in log file
		writeLog(mesurement.toString());
	}


	// On touch wake up, display last mesurement on screen for 5sec
	if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TOUCHPAD) {
		if (DEBUG_SLEEP_CYCLES) {Serial.printf("\n\n================ WOKE UP AT %02d:%02d:%02d FROM TOUCH ================\n", timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec);}

		// Screen setup
		initializeScreen();

		// Display on mesurement on screen
		displayMesurement(rtcLastMesurement);
	}


	// Log file cloud saving setup
	if ((timeInfo.tm_hour == 0 && timeInfo.tm_min == 0 && timeInfo.tm_sec == 0) || esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TOUCHPAD) {
		mountSPIFFS();
		sendLogFileToDrive();
		deleteCurrentLogFile();
		setRtcLogFilename(timeInfo.tm_year, timeInfo.tm_mon, timeInfo.tm_mday);
	}


	// Deep sleep mode setup 
	uint64_t interval = calculateNextInterval();

	if (rtcIsFirstBoot) {rtcIsFirstBoot = false;}
	touchSleepWakeUpEnable(TOUCH_WAKE_UP_PAD, TOUCH_WAKE_UP_THRESHOLD);		// Touch wake up configuration
	esp_sleep_enable_timer_wakeup(interval);

	if (DEBUG_SLEEP_CYCLES) {
		getLocalTime(&timeInfo); 
		Serial.printf("============ GOING TO SLEEP AT %02d:%02d:%02d FOR %llu ===========\n\n\n", timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec, interval);
	}

	esp_deep_sleep_start();
}


void loop() {
}