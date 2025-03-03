#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <TimeLib.h>
#include <EEPROM.h>
#include "arduino_secrets.h" // Include the secrets file

// NTP client setup
WiFiUDP ntpUDP;
const long utcOffsetInSeconds = -5 * 3600; // EST (UTC-5 hours)
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds, 300000); // Update every 5 minutes (300000 ms)

// Pin definitions
const int ledPin = 5;    // GPIO5 (D1 on the board)
const int buttonPin = 4; // GPIO4 (D2 on the board)

// EEPROM addresses
const int ledStateAddress = 0;
const int buttonPressedAddress = 1;

bool ledState = false; // Track whether LED is on or off
bool buttonPressed = false; // Track whether the button was pressed to turn off the LED
unsigned long previousMillis = 0; // Store the last time the NTP update was checked
const unsigned long interval = 300000; // Interval for NTP update (5 minutes)

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
  buttonPressed = EEPROM.read(buttonPressedAddress);

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
  unsigned long currentMillis = millis();

  // Check if it's time to update the NTP time
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    timeClient.update();
    setTime(timeClient.getEpochTime());
  }

  time_t currentTime = now();

  // Print current time and button state for debugging
  Serial.print("Current time: ");
  Serial.println(currentTime);
  Serial.print("Button state: ");
  Serial.println(digitalRead(buttonPin));
  Serial.print("LED state: ");
  Serial.println(ledState);

  // Check if it's after 8:45 AM on Monday and the LED is not already on
  if (weekday(currentTime) == 2 && hour(currentTime) >= 8 && minute(currentTime) >= 45 && !ledState && !buttonPressed) {
    digitalWrite(ledPin, HIGH);
    ledState = true; // Mark LED as ON
    EEPROM.write(ledStateAddress, ledState);
    EEPROM.commit();
    Serial.println("LED turned ON");
  }

  // Reset timer only if LED is ON and button is pressed
  int buttonState = digitalRead(buttonPin);
  if (ledState && buttonState == LOW) {
    Serial.println("Button pressed, turning off LED");
    digitalWrite(ledPin, LOW); // Turn off LED
    ledState = false; // Mark LED as OFF
    buttonPressed = true; // Mark button as pressed
    EEPROM.write(ledStateAddress, ledState);
    EEPROM.write(buttonPressedAddress, buttonPressed);
    EEPROM.commit();
    Serial.println("LED turned OFF by button press");
    delay(200); // Debounce delay
    // Wait for button release
    while (digitalRead(buttonPin) == LOW) {
      delay(10);
    }
  }

  // Reset buttonPressed flag at the start of a new week
  if (weekday(currentTime) == 2 && hour(currentTime) == 0 && minute(currentTime) == 0) {
    buttonPressed = false;
    EEPROM.write(buttonPressedAddress, buttonPressed);
    EEPROM.commit();
  }
}