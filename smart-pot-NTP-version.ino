#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <TimeLib.h>
#include <EEPROM.h>

// WiFi credentials
const char* ssid = "your_SSID";
const char* password = "your_PASSWORD";

// NTP client setup
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000); // Update every 60 seconds

// Pin definitions
const int ledPin = 5;    // GPIO5 (D1 on the board)
const int buttonPin = 4; // GPIO4 (D2 on the board)

// EEPROM addresses
const int ledStateAddress = 0;
const int lastResetTimeAddress = 4;

bool ledState = false; // Track whether LED is on or off
time_t lastResetTime;

void flashLED(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(ledPin, HIGH);
    delay(200);
    digitalWrite(ledPin, LOW);
    delay(200);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP); // Enable internal pull-up resistor
  digitalWrite(ledPin, LOW); // Ensure LED starts off

  // Initialize EEPROM
  EEPROM.begin(512);

  // Load saved state from EEPROM
  ledState = EEPROM.read(ledStateAddress);
  EEPROM.get(lastResetTimeAddress, lastResetTime);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Flash LED 3 times to confirm WiFi connection
  flashLED(3);

  // Initialize NTP client
  timeClient.begin();
  timeClient.update();

  // Set the initial time
  setTime(timeClient.getEpochTime());

  // Flash LED 3 times to confirm NTP synchronization
  flashLED(3);

  // Update LED state based on saved state
  if (ledState) {
    digitalWrite(ledPin, HIGH);
  }
}

void loop() {
  timeClient.update();
  setTime(timeClient.getEpochTime());
  time_t currentTime = now();

  // Check if it's 8 AM on Monday
  if (weekday(currentTime) == 2 && hour(currentTime) == 8 && minute(currentTime) == 0 && second(currentTime) == 0) {
    digitalWrite(ledPin, HIGH);
    ledState = true; // Mark LED as ON
    EEPROM.write(ledStateAddress, ledState);
    EEPROM.put(lastResetTimeAddress, currentTime);
    EEPROM.commit();
  }

  // Reset timer only if LED is ON and button is pressed
  if (ledState && digitalRead(buttonPin) == LOW) {
    lastResetTime = currentTime;  // Reset timer
    digitalWrite(ledPin, LOW); // Turn off LED
    ledState = false; // Mark LED as OFF
    EEPROM.write(ledStateAddress, ledState);
    EEPROM.put(lastResetTimeAddress, lastResetTime);
    EEPROM.commit();
    delay(200); // Debounce delay
  }
}