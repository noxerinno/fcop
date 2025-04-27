#include "fcop/sensors.h"

DHT dht(DHT_PIN, DHT_TYPE);     // Creating DHT object

// Sensors initialization
void sensorsInitialization() {
	// DHT setup
	dht.begin();

	// LDR setup
	analogReadResolution(12);   // Resolution over 12 bits (0-4095)
}


// Take a mesurement from the sensor and return a SensorData structure
SensorData mesure(int hours, int minutes, int seconds) {
	// Allocate memory for sensor data struct
	SensorData mesurement = { 0 };
	
	// LDR data read
	int ldrValue = LDR_MAX_VALUE - analogRead(LDR_PIN); 
	int ldrPercent = map(ldrValue, 0, LDR_MAX_VALUE, 0, 100);
	
	// DHT11 data read 
	float temperature = dht.readTemperature();		// in °C
	int humidity = (int)dht.readHumidity();       			// in %

	if (isnan(temperature) || isnan(humidity)) {		  // Check every DHT record validity
		Serial.println("Erreur de lecture !");
    	return mesurement;
	}

	// Initialize sensor data structure
	snprintf(mesurement.timestamp, sizeof(mesurement.timestamp), "%02d:%02d:%02d",
			hours,
            minutes,
            seconds);
	mesurement.temperature = temperature;
	mesurement.humidity = humidity;
	mesurement.luminosity = ldrPercent;

	if (DEBUG_SENSOR_DATA) {Serial.printf("[DEBUG_SENSOR_DATA] New log entry : %s\t(%s)\n", mesurement.toString(), LOG_FILE_HEADER);}

	return mesurement;
}