const int Pin1 = 3;
const int Pin2 = 4;
const int Pin3 = 5;
const int Pin4 = 6;
const int buttonPin = 2;

void setup() {
  pinMode(Pin1, OUTPUT);
  pinMode(Pin2, OUTPUT);
  pinMode(Pin3, OUTPUT);
  pinMode(Pin4, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
}

void loop() {
  // "I LOVE EE"
  if (digitalRead(buttonPin) == LOW) {
    firstWord();
    delay(1500);
    secondWord();
    delay(1500);
    thirdWord();
    delay(1500);
    fourthWord();
  }

}

// "I": · ·
void firstWord() {
  dot(Pin1);
  dot(Pin1);
}

// "LOVE": · — · ·  — — —  · · · ·  ·
void secondWord() {
  dot(Pin2);
  dash(Pin2);
  dot(Pin2);
  dot(Pin2);
  delay(750); // 0.75 s gap between words

  dash(Pin2);
  dash(Pin2);
  dash(Pin2);
  delay(750);

  dot(Pin2);
  dot(Pin2);
  dot(Pin2);
  dot(Pin2);
  delay(750);

  dot(Pin2);
}

// "E": ·
void thirdWord() {
  dot(Pin3);
}

// "E": ·
void fourthWord() {
  dot(Pin4);
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