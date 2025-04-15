#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Adresse I2C de l'écran (0x3C est le plus courant)
#define OLED_ADDRESS 0x3C

// Création de l'objet écran (avec interface I2C)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
	Serial.begin(115200);

	// Initialisation de l'écran
	if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    	Serial.println(F("Échec de l'initialisation de l'écran OLED"));
   		while (true); // Stop si échec
  	}

	display.clearDisplay();

	display.setTextSize(2);             // Taille du texte
	display.setTextColor(SSD1306_WHITE); // Couleur du texte
	display.setCursor(0, 0);             // Position du texte
	display.println(F("Hello, World!"));        // Texte à afficher
	display.display();                   // Rafraîchit l'écran
}


void loop() {
}