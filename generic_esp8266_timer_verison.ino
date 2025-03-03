// Pin definitions
const int ledPin = 5;    // GPIO5 (D1 on the board)
const int buttonPin = 4; // GPIO4 (D2 on the board)

// Timer variables
const unsigned long dayMillis = 86400000UL; // 1 day in milliseconds
const unsigned long reminderDays = 7;      // LED will turn on after 7 days
unsigned long lastResetTime;
bool ledState = false; // Track whether LED is on or off

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP); // Enable internal pull-up resistor
  digitalWrite(ledPin, LOW); // Ensure LED starts off
  lastResetTime = millis(); // Start timer
}

void loop() {
  unsigned long elapsedTime = millis() - lastResetTime;
  unsigned long timeLeft = (elapsedTime >= dayMillis * reminderDays) ? 0 : (dayMillis * reminderDays - elapsedTime);

  // Turn on LED when the countdown reaches zero
  if (timeLeft == 0 && !ledState) {
    digitalWrite(ledPin, HIGH);
    ledState = true; // Mark LED as ON
  }

  // Reset timer only if LED is ON and button is pressed
  if (ledState && digitalRead(buttonPin) == LOW) {
    lastResetTime = millis();  // Reset timer
    digitalWrite(ledPin, LOW); // Turn off LED
    ledState = false; // Mark LED as OFF
    delay(200); // Debounce delay
  }
}
