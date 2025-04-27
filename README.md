# FCOP - Fait Chaud ou Pas ?

🌡️ A lightweight ESP32-based weather station

## About

**FCOP** (short for *Fait Chaud ou Pas ?*) is a mini weather station built using an ESP32 microcontroller.  
It measures temperature, humidity, and light intensity at regular intervals while optimizing power consumption by using deep sleep modes.  
Perfect for experimenting with sensors, low-power IoT design, and cloud data logging.

## Features

- **Sensors**:
  - DHT11: Temperature and humidity
  - LDR: Ambient light (luminosity)
- **Data Collection**:
  - Takes a new measurement precisely every 30 seconds
- **Power Management**:
  - Deep Sleep Mode between each measurement to save battery
  - Wake-up either by timer or via capacitive touch (Touch Wake-Up)
- **Display**:
  - Displays data on a 0.96" OLED screen when woken up by touch
- **Logging**:
  - Stores measurements in a local log file (SPIFFS filesystem)
- **Cloud Integration**:
  - Automatically uploads log files every night at midnight to Google Drive
  - Refreshes OAuth2 access token dynamically before upload
  - Clears local storage after successful upload to save space

---

## Hardware Requirements

- ESP32 Development Board
- DHT11 Sensor (Temperature & Humidity)
- LDR Sensor (Light Intensity)
- 0.96" OLED Display (I2C)
- 3.7V 820mAh battery
- Optional: Capacitive touch input (for manual wake-up)

---

## Software Stack

- PlatformIO / Arduino framework (C++/C)
- Libraries:
  - Adafruit SSD1306
  - DHT sensor library
  - ESP32 touch functionality
  - HTTPClient for web communication
  - SPIFFS for file system operations
- Google Drive API (for file upload)

---

## Setup

1. Clone the repository:
   ```bash
   git clone https://github.com/noxerinno/fcop.git
   ```

2. Create your ```.env``` file in root following this model:
   ```bash
   WIFI_SSID=
   WIFI_PASSWORD=
   DRIVE_FOLDER_ID=
   DRIVE_CLIENT_ID=
   DRIVE_CLIENT_SECRET=
   DRIVE_REFRESH_TOKEN=
   ```