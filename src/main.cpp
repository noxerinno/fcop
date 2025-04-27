#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <Wire.h>
#include "credentials.h"
#include "DHT.h"
#include "FS.h"
#include "sensorData.h"
#include "SPIFFS.h"
#include "time.h"


// Mesurement interval (in seconds). Intervals smaller than 5s are not recommended 
#define MESURE_INTERVAL 30
#define MESUREMENT_DISPLAY_DURATION 5000
RTC_DATA_ATTR bool isFirstBoot = true; 
RTC_DATA_ATTR SensorData lastMesurement;


// Debbugging configuration
#define DEBUG_LOGS false
#define DEBUG_RTC false
#define DEBUG_SENSOR_DATA false
#define DEBUG_TOKEN true
#define DEBUG_WIFI false

#define MAX_CYCLES 5
RTC_DATA_ATTR int cycleCounter =  0;


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


// Logfile configuration
#define LOG_FILE_HEADER "TIMESTAMP,TEMPERATURE,HUMIDITY,LUMINOSITY"
RTC_DATA_ATTR char logFilename[14]= {0};  		// Allocate 20 char  in RTC memory (1 for the '/', 4 for the year, 2 for ther month, 2 for the day, 4 for '.log' and 1 for '\0')


// Touch wake up configuration
#define TOUCH_WAKE_UP_PAD T0
#define TOUCH_WAKE_UP_THRESHOLD 40


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


// Initialize RTC from NTP value retreive from internet
tm setRTC() {
	// If RTC is not set, connect to Wi-Fi and retreive NTP
	WiFi.begin(WIFI_SSID, WIFI_PASSWORD);		// Set WiFi connection

	while (WiFi.status() != WL_CONNECTED) {		// Try to connect to WiFi
		delay(500);
		if( DEBUG_RTC) {Serial.println("Connecting to WiFi...");}
	}
	if( DEBUG_RTC) {Serial.println("Connected");}

	configTime(NTP_TIME_ZONE, NTP_DAY_LIGHT_OFFSET, NTP_SERVER);

	// Wait for the time to be synchronized with NTP
    int attempts = 0;
    struct tm timeInfo;
    while (attempts < 10) {  // Retry up to 10 times
        if (getLocalTime(&timeInfo)) {
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

	return timeInfo;
}


// Sensors initialization
void sensorsInitialization() {
	// DHT setup
	dht.begin();

	// LDR setup
	analogReadResolution(12); // Resolution over 12 bits (0-4095)

	/// WiFi debug 
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

	if( DEBUG_SENSOR_DATA) {
		Serial.println(LOG_FILE_HEADER);
		Serial.println(mesurement.toString());
	}

	return mesurement;
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

	if (DEBUG_LOGS) {
		cycleCounter++;
		Serial.println();
		Serial.printf("CYCLES COUNTER  = %d", cycleCounter);
		Serial.println();

		if (cycleCounter >= MAX_CYCLES) {
			readLogFile();
			while(true) {}
		}
	}
}


// Setup log system
void mountSPIFFS() {
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
}


// Initialize log file
void setLogFilename(int year, int month, int day) {
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


// Delet log file
void deleteCurrentLogFile() {
	if (SPIFFS.exists(logFilename)) {
		if (SPIFFS.remove(logFilename)) {
				if(DEBUG_LOGS) {Serial.println("Log file successfully deleted");}
			} else {
				if(DEBUG_LOGS) {Serial.println("Unable to delete log file");}
			}
		} else {
			if(DEBUG_LOGS) {Serial.println("Unable to locate log file");}
		}
	}


// Display mesurement on screen
void displayMesurement() {
	display.clearDisplay();
	display.setTextSize(2);
	display.setTextColor(SSD1306_WHITE);
	display.setCursor(0, 0);
	display.printf("Tem: %.1fC", lastMesurement.temperature);
	display.setCursor(0, 20);
	display.printf("Hum: %d%%", lastMesurement.humidity);
	display.setCursor(0, 40);
	display.printf("Lum: %d%%", lastMesurement.luminosity);
	display.display();

	delay(MESUREMENT_DISPLAY_DURATION);
	display.ssd1306_command(SSD1306_DISPLAYOFF);
}


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


// Retreive access token from refresh token
String getAccessToken() {
    HTTPClient http;

    // Construire l'URL pour obtenir un nouveau token
    String url = "https://oauth2.googleapis.com/token";
    
    // Créer le corps de la requête avec les paramètres nécessaires
    String payload = "grant_type=refresh_token&refresh_token=" + String(DRIVE_REFRESH_TOKEN) +
                     "&client_id=" + String(DRIVE_CLIENT_ID) + 
                     "&client_secret=" + String(DRIVE_CLIENT_SECRET);

    // Configurer la requête HTTP
    http.begin(url);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    // Envoyer la requête et obtenir la réponse
    int httpResponseCode = http.POST(payload);

    String accessToken = "";
    if (httpResponseCode == 200) {
        String response = http.getString();
        
		// Access token extraction from response 
        int tokenKeyIndex = response.indexOf("\"access_token\":");
        if (tokenKeyIndex != -1) {
            int quoteStart = response.indexOf("\"", tokenKeyIndex + 15); // trouve le premier guillemet après "access_token":
            int quoteEnd = response.indexOf("\"", quoteStart + 1);       // trouve le second guillemet (fin du token)

            if (quoteStart != -1 && quoteEnd != -1) {
                accessToken = response.substring(quoteStart + 1, quoteEnd);
            }
        }

		if(DEBUG_TOKEN) {
			Serial.println(response);
			Serial.println(accessToken);
			Serial.println();
		}
    } else {
        if (DEBUG_TOKEN) {
            Serial.printf("HTTP error during token refresh: %s\n", http.errorToString(httpResponseCode).c_str());
        }
    }

    http.end();
    return accessToken;
}


// Send log file to my personnal drive
void sendLogFileToDrive() {
	// Connect to Wi-Fi to send log file to drive
	WiFi.begin(WIFI_SSID, WIFI_PASSWORD);		// Set WiFi connection

	while (WiFi.status() != WL_CONNECTED) {		// Try to connect to WiFi
		delay(500);
		if( DEBUG_TOKEN) {Serial.println("Connecting to WiFi...");}
	}
	if( DEBUG_TOKEN) {Serial.println("Connected");}

	// Read log file
	File logFile = SPIFFS.open(logFilename, "r");
	if (!logFile) {
		if( DEBUG_TOKEN) {Serial.println("Error : unable to  find log file");}
	  return;
	}
	
	String logFileContent = "";
	while (logFile.available()) {
	  logFileContent += (char)logFile.read();
	}
	logFile.close();

	// Request creation
	HTTPClient http;
	http.begin("https://www.googleapis.com/upload/drive/v3/files?uploadType=multipart");

	http.addHeader("Authorization", String("Bearer ") + getAccessToken());
	http.addHeader("Content-Type", "multipart/related; boundary=foo_bar_baz");

	//  Metadata creation
	String metadata = 
    "--foo_bar_baz\r\n"
    "Content-Type: application/json; charset=UTF-8\r\n\r\n"
    "{\r\n"
    "  \"name\": \"" + String(logFilename).substring(1) + "\",\r\n" // Nom du fichier sans "/"
    "  \"parents\": [\"" + String(DRIVE_FOLDER_ID) + "\"]\r\n"
    "}\r\n";

	// Media creation
	String media = 
    "--foo_bar_baz\r\n"
    "Content-Type: text/plain\r\n\r\n" +
    logFileContent + "\r\n" +
    "--foo_bar_baz--";

	String body = metadata + media;
	int httpResponseCode = http.POST((uint8_t*)body.c_str(), body.length());

	if (DEBUG_TOKEN) {
		if (httpResponseCode > 0) {
			Serial.printf("HTTP Response: %d\n", httpResponseCode);
			Serial.println(http.getString());
		} else {
			Serial.printf("HTTP error: %s\n", http.errorToString(httpResponseCode).c_str());
		}
	}
	
	http.end();

	WiFi.disconnect();
	WiFi.mode(WIFI_OFF);
}


void setup() {
	// Initialize "debugger"
	Serial.begin(115200);

	struct tm timeInfo;						// Time information precise to the second (date to create log filename one first boot & HH:MM:SS to timestamp the log entries )

	
	// First boot handeling
	if (isFirstBoot) {
		// initializeScreen();
		// displayWelcomeMessage();

		timeInfo = setRTC();				// RTC setup
		setLogFilename(timeInfo.tm_year, timeInfo.tm_mon, timeInfo.tm_mday);
		Serial.println("FIRST BOOT !");
	}
	
	
	getLocalTime(&timeInfo); 		// Retreive RTC value 
	Serial.printf("WOKE UP AT %02d:%02d:%02d", timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec);
	Serial.println();


	// On timer wakeup, take mesurement and log it memory 
	if (isFirstBoot || esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER) {
		// Sensor initialization
		sensorsInitialization();

		// SPIFFS & Log setup
		mountSPIFFS();		// Mount the file system

		// Take a mesurment
		SensorData mesurement = mesure(timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec);
		lastMesurement = mesurement;

		// Writing data in log file
		writeLog(mesurement.toString());
	}


	// On touch wake up, display last mesurement on screen for 5sec
	if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TOUCHPAD) {
		Serial.println("WOKE UP FROM TOUCHPAD");

		// Screen setup
		initializeScreen();

		// Display on mesurement on screen
		displayMesurement();
	}


	// Log file cloud saving setup
	if (timeInfo.tm_hour == 0 && timeInfo.tm_min == 0 && timeInfo.tm_sec == 0) {
		mountSPIFFS();
		sendLogFileToDrive();
		deleteCurrentLogFile();
		setLogFilename(timeInfo.tm_year, timeInfo.tm_mon, timeInfo.tm_mday);
	}


	// Deep sleep mode setup 
	uint64_t interval = calculateNextInterval();

	if (isFirstBoot) {isFirstBoot = false;}	
	touchSleepWakeUpEnable(TOUCH_WAKE_UP_PAD, TOUCH_WAKE_UP_THRESHOLD);		// Touch wake up configuration
	esp_sleep_enable_timer_wakeup(interval);

	getLocalTime(&timeInfo); 
	Serial.printf("GOING TO SLEEP AT %02d:%02d:%02d FOR %llu\n", timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec, interval);

	esp_deep_sleep_start();
}


void loop() {
}