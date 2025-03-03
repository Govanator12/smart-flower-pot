#include <ESP8266WiFi.h>
#include <time.h>
#include "Arduino_secrets.h"  // Contains ssid and password

// Define pins using numeric GPIO values:
// LED on GPIO5 (D1 on the board)
// Button on GPIO4 (D2 on the board)
const int ledPin = 5;    // GPIO5
const int buttonPin = 4; // GPIO4

// Global state variables
bool ledReminderOn = false;    // True when LED is on (reminder active)
bool reminderTriggered = false; // True once this week's reminder has fired
unsigned long lastButtonPressTime = 0; // For button debouncing
const unsigned long debounceDelay = 50; // Debounce time in milliseconds

// Function to triple flash the LED upon successful WiFi connection
void tripleFlashLED() {
  Serial.println("Starting triple flash of LED for WiFi connection confirmation.");
  for (int i = 0; i < 3; i++) {
    Serial.printf("Flash %d: LED ON\n", i + 1);
    digitalWrite(ledPin, HIGH);
    delay(200);
    digitalWrite(ledPin, LOW);
    Serial.printf("Flash %d: LED OFF\n", i + 1);
    delay(200);
  }
  Serial.println("Triple flash complete.");
}

void setup() {
  Serial.begin(115200);
  delay(10);

  // Initialize LED and button pins.
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP); // Button wired to ground when pressed
  
  // Ensure LED starts off.
  digitalWrite(ledPin, LOW);
  Serial.println("LED initialized to OFF.");

  // Connect to WiFi using credentials from Arduino_secrets.h.
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected successfully!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  
  // Triple flash the LED to indicate a successful WiFi connection.
  tripleFlashLED();

  // Configure time via NTP.
  // Fixed EST: offset -5 hours (UTC-5) and DST offset 0.
  Serial.println("Configuring time with NTP for fixed EST (UTC-5)...");
  configTime(-5 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  
  // Wait for time to be set.
  time_t now = time(nullptr);
  int retry = 0;
  while (now < 8 * 3600 && retry < 10) { // if time isn't set (e.g., still near 1970)
    delay(500);
    now = time(nullptr);
    retry++;
    Serial.print("Waiting for time sync... Retry: ");
    Serial.println(retry);
  }
  if (retry < 10) {
    Serial.println("Time synchronized successfully.");
  } else {
    Serial.println("Time sync may have failed; check WiFi/NTP settings.");
  }
}

void loop() {
  // Obtain the current time.
  time_t now = time(nullptr);
  struct tm *timeinfo = localtime(&now);
  if (timeinfo == NULL) {
    Serial.println("Error: Failed to obtain time.");
    delay(1000);
    return;
  }
  
  // Log the current time for troubleshooting.
  Serial.print("Current time: ");
  Serial.print(asctime(timeinfo));  // asctime() returns a string that includes a newline

  // Check if today is Monday (tm_wday: 0=Sunday, 1=Monday, etc.).
  if (timeinfo->tm_wday == 1) {
    Serial.println("Today is Monday.");
    // Trigger the reminder exactly at 10:38 AM EST if it hasn't already been triggered.
    if (timeinfo->tm_hour == 10 && timeinfo->tm_min == 38 && !reminderTriggered) {
      Serial.println("Trigger condition met: It is 10:38 AM on Monday.");
      digitalWrite(ledPin, HIGH);  // Turn on the LED
      ledReminderOn = true;
      reminderTriggered = true;    // Mark that this week's reminder has fired
      Serial.println("Reminder triggered: LED turned ON (time to water the plants)!");
    } else {
      Serial.print("Not trigger time or reminder already triggered (Hour: ");
      Serial.print(timeinfo->tm_hour);
      Serial.print(", Minute: ");
      Serial.print(timeinfo->tm_min);
      Serial.println(")");
    }
  } else {
    // Reset reminder flag on any day other than Monday.
    if (reminderTriggered) {
      Serial.println("Resetting reminder flag (today is not Monday).");
    }
    reminderTriggered = false;
  }
  
  // Check the button (active LOW) with a simple debounce.
  int buttonState = digitalRead(buttonPin);
  Serial.print("Button state: ");
  Serial.println(buttonState);
  if (buttonState == LOW && ledReminderOn) {
    if (millis() - lastButtonPressTime > debounceDelay) {
      Serial.println("Button press detected: Turning LED OFF (plants watered).");
      digitalWrite(ledPin, LOW); // Turn off the LED
      ledReminderOn = false;
      lastButtonPressTime = millis();  // Update debounce timer
    }
  }
  
  // Wait a short while before checking again.
  delay(1000);
}
