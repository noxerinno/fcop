#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHT.h"

// Screen configuration 
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDRESS 0x3C		// I2C screen address

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);		// Creating screen object (with I2C interface)


// DHT sensor confiration
#define DHTPIN 17							// Data pin I choose
#define DHTTYPE DHT11				// DHT type

DHT dht(DHTPIN, DHTTYPE);		// Creating DHT object


void setup() {
	// Screen setup
	Serial.begin(115200);			// Screen framerate ?

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
}


void loop() {
	delay(2000);			// DHT sensor refresh rate

	float temperature = dht.readTemperature();		// in °C
	float humidity = dht.readHumidity();       				 // in %

	// Check every record validity
	if (isnan(temperature) || isnan(humidity)) {
    	Serial.println("Erreur de lecture !");
    	return;
	}

	// Display on the monitor tab 
	Serial.print("Temperature: ");
	Serial.print(temperature);
	Serial.print(" °C | Humidity: ");
	Serial.print(humidity);
	Serial.println(" %");

	// Display on screen
	display.clearDisplay();
	display.setCursor(0, 16);
	display.printf("Tem: %.1fC", temperature);
	display.setCursor(0, 42);
	display.printf("Hum: %.1f%%", humidity);
	display.display();
}