#include "fcop/logger.h"

RTC_DATA_ATTR char rtcLogFilename[14]= {0};

// Setup log system
void mountSPIFFS() {
	// Mount SPIFFS
	if (!SPIFFS.begin(true)) {
        if (DEBUG_LOGS) {Serial.println("[DEBUG_LOGS] Unable to mount SPIFFS, trying to format...");}
		if (!SPIFFS.format()) {
			if (DEBUG_LOGS) {Serial.println("[DEBUG_LOGS] Unable to format SPIFFS");}
		} else {
			if (DEBUG_LOGS) {Serial.println("[DEBUG_LOGS] Successfully formated SPIFFS");}
		}
    } else {
		if (DEBUG_LOGS) {Serial.println("[DEBUG_LOGS] SPIFFS initialized");}
	}
}

// Initialize log file
void setRtcLogFilename(int year, int month, int day) {
	snprintf(rtcLogFilename, sizeof(rtcLogFilename), "/%04d%02d%02d.log",
		year + 1900,
		month + 1,
		day);

	if (DEBUG_LOGS) {
		Serial.print("[DEBUG_LOGS] Logfile : ");
		Serial.println(rtcLogFilename);
	}
	
	// Write csv header
	delay(500);
	writeLog(LOG_FILE_HEADER);
}

// Read current log file
void readLogFile() {
    File file = SPIFFS.open(rtcLogFilename, "r");  // Ouvre en mode "read"

    if (!file) {
        if (DEBUG_LOGS) {Serial.println("[DEBUG_LOGS] Could not open log file");}
        return;
    }

	Serial.println("===================== LOG FILE =====================");
	delay(1000);
    // Lire et afficher le contenu du fichier ligne par ligne
    while (file.available()) {
        String line = file.readStringUntil('\n');
        Serial.println(line);  // Affiche chaque ligne
    }
	Serial.println("=================================================="); 

    file.close();
} 


// Write a new data line in log file
void writeLog(const char* line) {
	File file = SPIFFS.open(rtcLogFilename, "a");  // Ouvre en mode "append"

    if (!file) {
        if (DEBUG_LOGS) {Serial.println("[DEBUG_LOGS] Could not open log file");}
        return;
    }

    // Écriture dans le fichier
    file.println(line);  // Ajoute un saut de ligne après chaque entrée

    // Fermer le fichier
    file.close();

	if (DEBUB_LIMIT_CICLES) {
		rtcDebugCycleCounter++;
		Serial.printf("[DEBUG_MAX_CYCLES] cycles counter = %d\n", rtcDebugCycleCounter);

		if (rtcDebugCycleCounter >= DEBUG_MAX_CYCLES) {
			Serial.printf("[DEBUG_MAX_CYCLES] Maximum cycles reach\n\n", rtcDebugCycleCounter);
			readLogFile();
			while(true) {}
		}
	}
}

// Delet log file
void deleteCurrentLogFile() {
	if (SPIFFS.exists(rtcLogFilename)) {
		if (SPIFFS.remove(rtcLogFilename)) {
			if (DEBUG_LOGS) {Serial.println("[DEBUG_LOGS] Log file successfully deleted");}
		} else {
			if (DEBUG_LOGS) {Serial.println("[DEBUG_LOGS] Unable to delete log file");}
		}
	} else {
		if (DEBUG_LOGS) {Serial.println("[DEBUG_LOGS] Unable to locate log file");}
	}
}