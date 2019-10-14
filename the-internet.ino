#include "ESP8266WiFi.h"
#include <WiFiClient.h>

//#########################################
//# Defines
//#########################################
// LED defines
#define R_PIN
#define G_PIN
#define B_PIN
#define ONBOARD_LED_PIN 16 //D6 on board

// Sensor defines
#define SENSOR_PIN 12

// Wifi defines
#define SCAN_PERIOD 5000

//#########################################
//# Globals
//#########################################
// LED vars
long lastBlinkMillis;
boolean ledState;

// Scan vars
long lastScanMillis;
int networksFound = 1;

// Sensor vars
long lastTriggerMillis;
int sensorState = HIGH;  // This is where we record our shock measurement
boolean sensorTriggered = false;
int sensorTriggerCount = 0;
int sensorDebounceTime = 100; // Number of milli seconds to keep the shock alarm high

// Mode vars
int currentMode = 0;

//#########################################
//# Setup
//#########################################
void setup()
{
  Serial.begin(115200);
  Serial.println();

  pinMode(ONBOARD_LED_PIN, OUTPUT);
  pinMode(SENSOR_PIN, INPUT);

  Serial.println(WiFi.macAddress());
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
}

//#########################################
//# Loop
//#########################################
void loop()
{
  long currentMillis = millis();

  // handle
  sensorState = digitalRead(SENSOR_PIN);
  if (sensorState == LOW) { // If we're in an alarm state
    lastTriggerMillis = currentMillis; // record the time of the shock
    if (!sensorTriggered) {
      Serial.println("Shock Alarm");
      sensorTriggerCount++;
      sensorTriggered = true;
    }
  } else {
    if ((currentMillis - lastTriggerMillis) > sensorDebounceTime  &&  sensorTriggered) {
      Serial.println("no alarm");
      Serial.println(sensorTriggerCount);
      sensorTriggered = false;
    }
  }

  // blink LED
  if (currentMillis - lastBlinkMillis > 1000 / networksFound)
  {
    digitalWrite(ONBOARD_LED_PIN, ledState);
    ledState = !ledState;
    lastBlinkMillis = currentMillis;
  }

  // trigger Wi-Fi network scan
  if (currentMillis - lastScanMillis > SCAN_PERIOD)
  {
    WiFi.scanNetworks(true);
    Serial.print("\nScan start ... ");
    lastScanMillis = currentMillis;
  }

  // print out Wi-Fi network scan result uppon completion
  int n = WiFi.scanComplete();
  if (n >= 0)
  {
    Serial.printf("%d network(s) found\n", n);
    networksFound = n;
    for (int i = 0; i < n; i++)
    {
      Serial.printf("%d: %s, Ch:%d (%ddBm) %s\n", i + 1, WiFi.SSID(i).c_str(), WiFi.channel(i), WiFi.RSSI(i), WiFi.encryptionType(i) == ENC_TYPE_NONE ? "open" : "");
    }
    WiFi.scanDelete();
  }
}