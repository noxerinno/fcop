#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include "credentials.h"
#include "DHT.h"
#include "FS.h"
#include "SPI.h"
#include "time.h"


// Debbugging configuration
#define DEBUG_RTC true
#define DEBUG_SENSOR_DATA false
#define DEBUG_WIFI true


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
#define LDR_PIN 35						   // LDR data pin (choosen)
#define LDR_MAX_VALUE 4096  // LDR max data (observed, not found in doc)


// NTP configuration
#define NTP_SERVER "pool.ntp.org"			  // NTP server 
#define NTP_TIME_ZONE 3600						 // NTP timezone = UTC+1
#define NTP_DAY_LIGHT_OFFSET 3600		//NTP day ligth offset = summer hour
bool isRTCSetup = false;



void setRTC() {
	// If RTC is not set, connect to Wi-Fi and retreive NTP
	struct tm timeinfo;
	if (!getLocalTime(&timeinfo)) {
		WiFi.begin(WIFI_SSID, WIFI_PASSWORD);		// Set WiFi connection

		while (WiFi.status() != WL_CONNECTED) {		// Try to connect to WiFi
			delay(500);
			Serial.println("Connecting to Wi-Fi...");
		}
		Serial.println("Wi-Fi connected");

		configTime(NTP_TIME_ZONE, NTP_DAY_LIGHT_OFFSET, NTP_SERVER);
	}
}


void setup() {
	// WiFi debug 
	if(Serial && DEBUG_WIFI) {
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


	// Screen setup
	Wire.begin();
	Serial.begin(115200);			// Monitor refresh rate

	// Screen initialization
	if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    	Serial.println(F("Échec de l'initialisation de l'écran OLED"));
   		while (true); // Stop si échec
  	}

	display.clearDisplay();
	display.setTextSize(2);             						    // Text size
	display.setTextColor(SSD1306_WHITE); 		// Text color
	display.setCursor(10, 25);									 // Text position
	display.println(F("Bienvenue"));				 	   // Text to display
	display.display();
	display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_BLACK);
	delay(2000);


	// DHT setup
	dht.begin();


	// LDR setup
	analogReadResolution(12); // Resolution over 12 bits (0-4095)
}


void loop() {
	float temperature = dht.readTemperature();		// in °C
	float humidity = dht.readHumidity();       				 // in %

	// Check every DHT record validity
	if (isnan(temperature) || isnan(humidity)) {
		Serial.println("Erreur de lecture !");
    	return;
	}

	// LDR data read
	int ldrValue = LDR_MAX_VALUE - analogRead(LDR_PIN); 
	int ldrPercent = map(ldrValue, 0, LDR_MAX_VALUE, 0, 100);
	
	// Sensors data debug
	if(Serial && DEBUG_SENSOR_DATA) {
		Serial.print("Temperature: ");
		Serial.print(temperature);
		Serial.print(" °C | Humidity: ");
		Serial.print(humidity);
		Serial.print(" % | LDR value: ");
		Serial.println(ldrPercent);
	}
	if(Serial && DEBUG_RTC) {
		struct tm timeinfo;
		if (!getLocalTime(&timeinfo)) {
		  Serial.println("Time retrival error ! ");
		  return;
		}
	  
		// Date and time display
		Serial.print("Date/Heure: ");
		Serial.print(timeinfo.tm_hour);
		Serial.print(":");
		Serial.print(timeinfo.tm_min);
		Serial.print(":");
		Serial.print(timeinfo.tm_sec);
		Serial.print(" - ");
		Serial.print(timeinfo.tm_mday);
		Serial.print("/");
		Serial.print(timeinfo.tm_mon + 1); 
		Serial.print("/");
		Serial.println(timeinfo.tm_year + 1900);  // Unix timestamp
	}
	
	// Display on screen
	display.clearDisplay();
	display.setCursor(0, 0);
	display.printf("Tem: %.1fC", temperature);
	display.setCursor(0, 20);
	display.printf("Hum: %.1f%%", humidity);
	display.setCursor(0, 40);
	display.printf("Lum: %.1d%%", ldrPercent);
	display.display();
	
	delay(5000);			// DHT sensor refresh rate
}