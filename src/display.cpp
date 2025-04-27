#include "fcop/display.h"

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);		// Creating screen object (with I2C interface)

// Initialize screen
void initializeScreen() {
	Wire.begin();
	delay(100);

	if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    	Serial.println(F("Unable to initialize OLED screen"));
   		while (true);
  	}
}


// Display welcome message
void displayWelcomeMessage() {
	display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_BLACK);
	display.clearDisplay();
	display.setTextSize(2);             						    // Text size
	display.setTextColor(SSD1306_WHITE); 		// Text color
	display.setCursor(10, 25);									 // Text position
	display.println("Bienvenue");				 	   // Text to display
	display.display();

	delay(2000);
	// display.clearDisplay();
	// display.display();
	// display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_BLACK);
	display.ssd1306_command(SSD1306_DISPLAYOFF);
}

// Display mesurement on screen
void displayMesurement(SensorData mesurement) {
	display.clearDisplay();
	display.setTextSize(2);
	display.setTextColor(SSD1306_WHITE);
	display.setCursor(0, 0);
	display.printf("Tem: %.1fC", mesurement.temperature);
	display.setCursor(0, 20);
	display.printf("Hum: %d%%", mesurement.humidity);
	display.setCursor(0, 40);
	display.printf("Lum: %d%%", mesurement.luminosity);
	display.display();

	delay(MESUREMENT_DISPLAY_DURATION);
	display.ssd1306_command(SSD1306_DISPLAYOFF);
}