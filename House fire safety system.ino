int rgbLedRedPin = A2;
int rgbLedGreenPin = A3;
int rgbLedBluePin = A4;
int buzzerPin = 2; // PWM (~) pin
int relayPin = 6;  // Digital pin to control the relay
int gasPin = A0;
int flamePin = 3;
int gasSensorThres = 17;

// Push-button variables
const int buttonPin = 4; // Digital pin for the push-button
const int passwordSequence[] = {HIGH, LOW, HIGH}; // Define your desired password sequence
const int sequenceLength = sizeof(passwordSequence) / sizeof(passwordSequence[0]);
int buttonState = LOW;
int buttonPresses = 0;
int lastButtonState = LOW;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 10;

// State machine variables
enum State {IDLE, DETECTED, PASSWORD_CHECK};
State currentState = IDLE;

void setup() {
  pinMode(rgbLedRedPin, OUTPUT);
  pinMode(rgbLedGreenPin, OUTPUT);
  pinMode(rgbLedBluePin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(relayPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP); // Use internal pull-up resistor

  digitalWrite(relayPin, LOW); // Initialize relay to OFF state
  currentState = IDLE;
  Serial.begin(9600);
}

void loop() {
  int gasSensor = analogRead(gasPin);
  int flameSensor = digitalRead(flamePin);

  int reading = digitalRead(buttonPin);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;

      if (buttonState == HIGH) { // Button released
        if (currentState == PASSWORD_CHECK) {
          currentState = IDLE; // Reset state to IDLE after successful password entry
          buttonPresses = 0; // Reset button press counter
          noTone(buzzerPin); // Stop the beeping
        }
      } else { // Button pressed
        if (currentState == IDLE) {
          currentState = DETECTED;
        }
      }
    }
  }

  lastButtonState = reading;

  switch (currentState) {
    case IDLE:
      // Normal sensor operation
      sensorOperation(gasSensor, flameSensor);
      break;
    case DETECTED:
      // Sequence detected; increment buttonPresses
      buttonPresses++;
      if (buttonPresses == sequenceLength) {
        currentState = PASSWORD_CHECK;
        buttonPresses = 0; // Reset button press counter
        tone(buzzerPin, 1500, 300); // Play a short beep to indicate successful password entry
      }
      break;
    case PASSWORD_CHECK:
      // Password sequence entered; wait for button release (handled in buttonState HIGH section)
      break;
  }
}

// Function to control the RGB LED
void rgbLedControl(int redValue, int greenValue, int blueValue) {
  analogWrite(rgbLedRedPin, redValue);
  analogWrite(rgbLedGreenPin, greenValue);
  analogWrite(rgbLedBluePin, blueValue);
}

// Function to play the buzzer alert
void buzzerAlert(int onDuration, int offDuration) {
  tone(buzzerPin, 1000); // Set the buzzer sound frequency to 1000 Hz
  delay(onDuration);
  noTone(buzzerPin);
  delay(offDuration);
}

// Function to handle normal sensor operation
void sensorOperation(int gasSensor, int flameSensor) {
  static bool buzzerActive = false; // Keep track of the buzzer status

  Serial.print("gasPin Value: ");
  Serial.println(gasSensor);
  Serial.print("flamePin Value: ");
  Serial.println(flameSensor);
  delay(100);

  if (gasSensor > gasSensorThres && flameSensor == LOW) {
    // Both gas and flame detected - RGB LED: Purple (Red + Blue)
    rgbLedControl(255, 0, 255);
    if (!buzzerActive) {
      buzzerActive = true; // Turn the buzzer on only once
      tone(buzzerPin, 1000); // Set the buzzer sound frequency to 1000 Hz
    }
    digitalWrite(relayPin, HIGH); // Activate the relay
  } else if (gasSensor > gasSensorThres) {
    // Only gas detected - RGB LED: Red
    rgbLedControl(255, 0, 0);
    if (!buzzerActive) {
      buzzerActive = true; // Turn the buzzer on only once
      tone(buzzerPin, 1000); // Set the buzzer sound frequency to 1000 Hz
    }
    digitalWrite(relayPin, LOW); // Deactivate the relay
  } else if (flameSensor == HIGH) {
    // Only flame detected - RGB LED: Blue
    rgbLedControl(0, 0, 255);
    if (!buzzerActive) {
      buzzerActive = true; // Turn the buzzer on only once
      tone(buzzerPin, 1000); // Set the buzzer sound frequency to 1000 Hz
    }
    digitalWrite(relayPin, HIGH); // Activate the relay
  } else {
    // No gas and flame detected - RGB LED: Green
    rgbLedControl(0, 255, 0);
    if (buzzerActive) {
      buzzerActive = false; // Turn the buzzer off only once
      noTone(buzzerPin);
    }
    digitalWrite(relayPin, LOW); // Deactivate the relay
  }
}