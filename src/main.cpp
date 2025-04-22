#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include "credentials.h"
#include "DHT.h"
#include "FS.h"
#include "sensorData.h"
#include "SPI.h"
#include "SPIFFS.h"
#include "time.h"


// Debbugging configuration
#define DEBUG_LOGS true
#define DEBUG_RTC false
#define DEBUG_SENSOR_DATA false
#define DEBUG_WIFI false


// Screen configuration 
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDRESS 0x3C		// I2C screen address
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);		// Creating screen object (with I2C interface)


// DHT sensor confiration
#define DHT_PIN 17							// DHT data pin (choosen)
#define DHT_TYPE DHT11				// DHT type

DHT dht(DHT_PIN, DHT_TYPE);		// Creating DHT object


// LDR sensor configuration
#define LDR_PIN 35						   		// LDR data pin (choosen)
#define LDR_MAX_VALUE 4096  	 // LDR max data (observed, not found in doc)


// NTP configuration
#define NTP_SERVER "pool.ntp.org"			  			 	 // NTP server 
#define NTP_TIME_ZONE 3600									 	 // NTP timezone = UTC+1
#define NTP_DAY_LIGHT_OFFSET 3600						// NTP day ligth offset = summer hour
#define NTP_MAX_CONNECTION_ATTEMPTS	10	   // NTP max connection atempts (10 = 5 sec)
RTC_DATA_ATTR bool isRTCSetup = false;   				 // To get NTP and load RTC only on the first boot


// Logfile configuration
#define LOG_FILE_HEADER "TIMESTAMP,TEMPERATURE,HUMIDITY,LUMINOSITY"
RTC_DATA_ATTR char logFilename[14]= {0};  		// Allocate 20 char  in RTC memory (1 for the '/', 4 for the year, 2 for ther month, 2 for the day, 4 for '.log' and 1 for '\0')


// Initialize RTC from NTP value retreive from internet
void setRTC() {
	// If RTC is not set, connect to Wi-Fi and retreive NTP
	WiFi.begin(WIFI_SSID, WIFI_PASSWORD);		// Set WiFi connection

	while (WiFi.status() != WL_CONNECTED) {		// Try to connect to WiFi
		delay(500);
		if( DEBUG_RTC) {Serial.println("Connecting NTP server...");}
	}
	if( DEBUG_RTC) {Serial.println("Connected");}

	configTime(NTP_TIME_ZONE, NTP_DAY_LIGHT_OFFSET, NTP_SERVER);

	// Wait for the time to be synchronized with NTP
    int attempts = 0;
    struct tm timeinfo;
    while (attempts < 10) {  // Retry up to 10 times
        if (getLocalTime(&timeinfo)) {
            if( DEBUG_RTC) {Serial.println("NTP time synchronized!");}
            break;
        }
        delay(500);  // Wait 0.5 second before retrying
        attempts++;
    }

    if (attempts == 10) {
        if( DEBUG_RTC) {Serial.println("Failed to sync time with NTP!");}
    }

	WiFi.disconnect();
	WiFi.mode(WIFI_OFF);
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

	return mesurement;
}


// Write a new data line in log file
void writeLog(const char* mesurementString) {
	File file = SPIFFS.open(logFilename, "a");  // Ouvre en mode "append"

    if (!file) {
        if( DEBUG_LOGS) {Serial.println("Could not open log file");}
        return;
    }

    // Écriture dans le fichier
    file.println(mesurementString);  // Ajoute un saut de ligne après chaque entrée

    // Fermer le fichier
    file.close();
}


// Read current log file
void readLogFile() {
    File file = SPIFFS.open(logFilename, "r");  // Ouvre en mode "read"

    if (!file) {
        if( DEBUG_LOGS) {Serial.println("Could not open log file");}
        return;
    }

	Serial.println("===================== LOG FILE ====================="); 
    // Lire et afficher le contenu du fichier ligne par ligne
    while (file.available()) {
        String line = file.readStringUntil('\n');
        Serial.println(line);  // Affiche chaque ligne
    }
	Serial.println("=================================================="); 

    file.close();
} 


// Setup log system
void setupLogSystem(int year, int month, int day) {
	// Mount SPIFFS
	if (!SPIFFS.begin(true)) {
        if( DEBUG_LOGS) {Serial.println("Unable to mount SPIFFS, trying to format...");}
		if (!SPIFFS.format()) {
			if( DEBUG_LOGS) {Serial.println("Unable to format SPIFFS");}
		} else {
			if( DEBUG_LOGS) {Serial.println("Successfully formated SPIFFS");}
		}
    } else {
		if( DEBUG_LOGS) {Serial.println("SPIFFS initialized");}
	}
    
	// Initialize log file on first boot
	if (logFilename[0] == '\0') {
		snprintf(logFilename, sizeof(logFilename), "/%04d%02d%02d.log",
            year + 1900,
            month + 1,
            day);

		if(DEBUG_LOGS) {
			Serial.print("Logfile : ");
			Serial.println(logFilename);
		}
		
		// Write csv header
		writeLog(LOG_FILE_HEADER);
	}	
}


void setup() {
	// DHT setup
	dht.begin();

	// LDR setup
	analogReadResolution(12); // Resolution over 12 bits (0-4095)

	// Initialize "debugger"
	Serial.begin(115200);

	// WiFi debug 
	if( DEBUG_WIFI) {
		Serial.println("Connexion au Wi-Fi...");
		WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

		int attempts = 0;
		while (WiFi.status() != WL_CONNECTED && attempts < 20) {
			delay(500);
			Serial.print(".");
			attempts++;
		}

		if (WiFi.status() == WL_CONNECTED) {
			Serial.println("\n✅ Wi-Fi connecté !");
			Serial.print("📡 SSID : "); Serial.println(WiFi.SSID());
			Serial.print("📶 IP locale : "); Serial.println(WiFi.localIP());
		} else {
			Serial.println("\n❌ Connexion Wi-Fi échouée !");
			Serial.print("Vérifie le SSID / mot de passe : ");
			Serial.print(WIFI_SSID);
			Serial.print(" / ");
			Serial.println(WIFI_PASSWORD);
		}
	}

	// RTC setup
	if (!isRTCSetup) {
		setRTC();
		isRTCSetup = true;
	}

	struct tm timeinfo;
	getLocalTime(&timeinfo); 		// Retreive RTC value 

	// SPIFFS & Log setup
	setupLogSystem(timeinfo.tm_year, timeinfo.tm_mon, timeinfo.tm_mday);

	// Take a mesurment
	SensorData mesurement = mesure(timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
	if( DEBUG_SENSOR_DATA) {
		Serial.println(LOG_FILE_HEADER);
		Serial.println(mesurement.toString());
	}

	// Writing data in log file
	writeLog(mesurement.toString());
	if( DEBUG_LOGS) {readLogFile();}
	
	delay(2000);





	// // Screen setup
	// Wire.begin();

	// // Screen initialization
	// if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    // 	Serial.println(F("Échec de l'initialisation de l'écran OLED"));
   	// 	while (true); // Stop si échec
  	// }

	// display.clearDisplay();
	// display.setTextSize(2);             						    // Text size
	// display.setTextColor(SSD1306_WHITE); 		// Text color
	// display.setCursor(10, 25);									 // Text position
	// display.println(F("Bienvenue"));				 	   // Text to display
	// display.display();
	// display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_BLACK);
	// delay(2000);
}


void loop() {
	// float temperature = dht.readTemperature();		// in °C
	// int humidity = (int)dht.readHumidity();       			// in %

	// // Check every DHT record validity
	// if (isnan(temperature) || isnan(humidity)) {
	// 	Serial.println("Erreur de lecture !");
    // 	return;
	// }

	// // LDR data read
	// int ldrValue = LDR_MAX_VALUE - analogRead(LDR_PIN); 
	// int ldrPercent = map(ldrValue, 0, LDR_MAX_VALUE, 0, 100);
	
	// // Sensors data debug
	// if( DEBUG_SENSOR_DATA) {
	// 	Serial.print("Temperature: ");
	// 	Serial.print(temperature);
	// 	Serial.print(" °C | Humidity: ");
	// 	Serial.print(humidity);
	// 	Serial.print(" % | LDR value: ");
	// 	Serial.println(ldrPercent);
	// }
	// if( DEBUG_RTC) {
	// 	struct tm timeinfo;
	// 	if (!getLocalTime(&timeinfo)) {
	// 	  Serial.println("Time retrival error ! ");
	// 	  return;
	// 	}
	  
	// 	// Date and time display
	// 	Serial.print("Date/Heure: ");
	// 	Serial.print(timeinfo.tm_hour);
	// 	Serial.print(":");
	// 	Serial.print(timeinfo.tm_min);
	// 	Serial.print(":");
	// 	Serial.print(timeinfo.tm_sec);
	// 	Serial.print(" - ");
	// 	Serial.print(timeinfo.tm_mday);
	// 	Serial.print("/");
	// 	Serial.print(timeinfo.tm_mon + 1); 
	// 	Serial.print("/");
	// 	Serial.println(timeinfo.tm_year + 1900);  // Unix timestamp
	// }
	
	// // Display on screen
	// display.clearDisplay();
	// display.setCursor(0, 0);
	// display.printf("Tem: %.1fC", temperature);
	// display.setCursor(0, 20);
	// display.printf("Hum: %d%%", humidity);
	// display.setCursor(0, 40);
	// display.printf("Lum: %.1d%%", ldrPercent);
	// display.display();
	
	// delay(5000);			// DHT sensor refresh rate
}