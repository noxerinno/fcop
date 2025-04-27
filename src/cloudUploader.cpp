#include "fcop/cloudUploader.h"

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
            int quoteStart = response.indexOf("\"", tokenKeyIndex + 15);
            int quoteEnd = response.indexOf("\"", quoteStart + 1);

            if (quoteStart != -1 && quoteEnd != -1) {
                accessToken = response.substring(quoteStart + 1, quoteEnd);
            }
        }

		if (DEBUG_CLOUD) {
			Serial.println("[DEBUG_CLOUD] Server response and generated token :");
			Serial.println(response);
			Serial.println(accessToken);
			Serial.println();
		}
    } else {
        if (DEBUG_CLOUD) {
            Serial.printf("[DEBUG_CLOUD] HTTP error during token refresh: %s\n", http.errorToString(httpResponseCode).c_str());
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
		if (DEBUG_CLOUD) {Serial.println("[DEBUG_CLOUD] Connecting to WiFi...");}
	}
	if (DEBUG_CLOUD) {Serial.println("[DEBUG_CLOUD] Connected");}

	// Read log file
	File logFile = SPIFFS.open(rtcLogFilename, "r");
	if (!logFile) {
		if (DEBUG_CLOUD) {Serial.println("[DEBUG_CLOUD] Error : unable to  find log file");}
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
    "  \"name\": \"" + String(rtcLogFilename).substring(1) + "\",\r\n" // Nom du fichier sans "/"
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

	if (DEBUG_CLOUD) {
		if (httpResponseCode > 0) {
			Serial.printf("[DEBUG_CLOUD] HTTP Response: %d\n", httpResponseCode);
			Serial.println(http.getString());
		} else {
			Serial.printf("[DEBUG_CLOUD] HTTP error: %s\n", http.errorToString(httpResponseCode).c_str());
		}
	}
	
	http.end();

	WiFi.disconnect();
	WiFi.mode(WIFI_OFF);
}