const int Pin1 = 3;
const int Pin2 = 4;
const int Pin3 = 5;
const int Pin4 = 6;
const int Pin5;
const int buttonPin = 2;

void setup() {
  pinMode(Pin1, OUTPUT);
  pinMode(Pin2, OUTPUT);
  pinMode(Pin3, OUTPUT);
  pinMode(Pin4, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
}

void loop() {
  // Display Morse code
  if (digitalRead(buttonPin) == LOW) {
    letterX(Pin1);
    delay(1500);
    letterW(Pin2);
    delay(1500);
    letterX(Pin3);
    delay(1500);
    letterZ(Pin4);
    delay(1500);
    letterX(Pin5);
  }
}

// "W": · — —
void letterW(int pin) {
  dot(pin);
  dash(pin);
  dash(pin);
}

// "X": — · · —
void letterX(int pin) {
  dash(pin);
  dot(pin);
  dot(pin);
  dash(pin);
}

// "Z": — — · ·
void letterW(int pin) {
  dash(pin);
  dash(pin);
  dot(pin);
  dot(pin);
}

// Morse code
void dot(int pin) {
  digitalWrite(pin, HIGH);
  delay(250);
  digitalWrite(pin, LOW);
  delay(250);
}

void dash(int pin) {
  digitalWrite(pin, HIGH);
  delay(750);
  digitalWrite(pin, LOW);
  delay(250);
}
