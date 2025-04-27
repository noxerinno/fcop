#include "fcop/rtcManager.h"

// Initialize RTC from NTP value retreive from internet
tm setRTC() {
	// WiFi debug 
	if (DEBUG_WIFI) {
		Serial.println("[DEBUG_WIFI] Connecting to Wi-Fi...");
		WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

		int attempts = 0;
		while (WiFi.status() != WL_CONNECTED && attempts < 20) {
			delay(500);
			Serial.print(".");
			attempts++;
		}

		if (WiFi.status() == WL_CONNECTED) {
			Serial.println("\n✅ Wi-Fi connected !");
			Serial.print("📡 SSID : "); Serial.println(WiFi.SSID());
			Serial.print("📶 local IP : "); Serial.println(WiFi.localIP());
		} else {
			Serial.println("\n❌ Connexion failed !");
			Serial.print("Check SSID / password : ");
			Serial.print(WIFI_SSID);
			Serial.print(" / ");
			Serial.println(WIFI_PASSWORD);
		}
	}

	// If RTC is not set, connect to Wi-Fi and retreive NTP
	WiFi.begin(WIFI_SSID, WIFI_PASSWORD);		// Set WiFi connection

	while (WiFi.status() != WL_CONNECTED) {		// Try to connect to WiFi
		delay(500);
		if (DEBUG_RTC) {Serial.println("[DEBUG_RTC] Connecting to WiFi...");}
	}
	if (DEBUG_RTC) {Serial.println("[DEBUG_RTC] Connected");}

	configTime(NTP_TIME_ZONE, NTP_DAY_LIGHT_OFFSET, NTP_SERVER);

	// Wait for the time to be synchronized with NTP
    int attempts = 0;
    struct tm timeInfo;
    while (attempts < 10) {  // Retry up to 10 times
        if (getLocalTime(&timeInfo)) {
            if (DEBUG_RTC) {Serial.println("[DEBUG_RTC] NTP time synchronized!");}
            break;
        }
        delay(500);  // Wait 0.5 second before retrying
        attempts++;
    }

    if (attempts == 10) {
        if (DEBUG_RTC) {Serial.println("[DEBUG_RTC] Failed to sync time with NTP!");}
    }

	WiFi.disconnect();
	WiFi.mode(WIFI_OFF);

	return timeInfo;
}