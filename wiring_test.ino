#include <ESP8266WiFi.h>

// Define pins using numeric GPIO values:
// LED on GPIO5 (D1 on the board)
// Button on GPIO4 (D2 on the board)
const int ledPin = 5;    // GPIO5
const int buttonPin = 4; // GPIO4

void setup() {
  Serial.begin(115200);
  delay(10);

  // Initialize LED and button pins.
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP); // Button wired to ground when pressed

  // Ensure LED starts off.
  digitalWrite(ledPin, LOW);
  Serial.println("LED initialized to OFF.");
}

void loop() {
  // Blink the LED on and off every second.
  digitalWrite(ledPin, HIGH);
  delay(1000);
  digitalWrite(ledPin, LOW);
  delay(1000);

  // Read the button state and print it to the serial monitor.
  int buttonState = digitalRead(buttonPin);
  Serial.print("Button state: ");
  Serial.println(buttonState);
}