// Libraries
#include <Keypad.h>

// Number Pad PINs Config (Rows and Columns)
#define R1 12
#define R2 14
#define R3 27
#define R4 26
#define C1 25
#define C2 33
#define C3 32

// Morse Code PINs Config
const int LED_1 = 2;
const int LED_2 = 4;
const int LED_3 = 5;
const int LED_4 = 18;
const int LED_5 = 19;
const int buttonPin = 15;

// Number Pad Configuration
const byte ROWS = 4;
const byte COLS = 3;

char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

byte rowPins[ROWS] = {12, 14, 27, 26};
byte colPins[COLS] = {25, 33, 32};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Passwords for stages 1/2/3
const String stagePasswords[3] = {"3681", "0143", "87808"};
int currentStage = 0;
int warningCount = 0;
String inputBuffer = "";

void setup() {
  Serial.begin(115200);
  delay(100);

  Serial.println("Enter the correct S-code:");

  // Morse Code LEDs setup
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  pinMode(LED_3, OUTPUT);
  pinMode(LED_4, OUTPUT);
  pinMode(LED_5, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
}

void loop() {
  char key = keypad.getKey();

  // Number Pad Password Stages
  if (key && currentStage < 3) {
    if (key == '#') {
      if (inputBuffer == stagePasswords[currentStage]) {
        Serial.println();
        switch (currentStage) {
          case 0:
            Serial.println("Stage 1 unlocked! Proceed to the next clue.");
            break;
          case 1:
            Serial.println("Stage 2 unlocked! Press the button to receive Morse code.");
            break;
          case 2:
            Serial.println("Final password accepted! You win!");
            winning();
            break;
        }
        inputBuffer = "";
        currentStage++;
        if (currentStage < 3) {
          Serial.print("Enter Stage ");
          Serial.print(currentStage + 1);
          Serial.println(" Password:");
        }
      } else {
        Serial.println("\nIncorrect password. Try again.");
        inputBuffer = "";
      }
    } else if (key == '*') {
      inputBuffer = "";
      Serial.println("Input cleared.");
    } else {
      inputBuffer += key;
      Serial.print(key); // Show typed key (can hide if needed)
    }
  } 
  
  // After final stage is passed
  else if (key && currentStage >= 3) {
    if (warningCount < 3) {
      Serial.println("You won, congratulations!");
      warningCount++;
    } else {
      Serial.println("Really, you won already. Chill out.");
    }
  }

  // Display Morse Code after Stage 2 is completed
  if (currentStage == 2 && digitalRead(buttonPin) == LOW) {
    letterX(LED_1);
    delay(1500);
    letterW(LED_2);
    delay(1500);
    letterX(LED_3);
    delay(1500);
    letterZ(LED_4);
    delay(1500);
    letterX(LED_5);
  } else {
    Serial.println("Hold up, finish the previous part first!");
  }
}

// Morse code letters
void letterW(int pin) {
  dot(pin);
  dash(pin);
  dash(pin);
}

void letterX(int pin) {
  dash(pin);
  dot(pin);
  dot(pin);
  dash(pin);
}

void letterZ(int pin) {
  dash(pin);
  dash(pin);
  dot(pin);
  dot(pin);
}

// Morse code units
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

// LEDs Win Animation
void winning() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_1, HIGH);
    delay(200);
    digitalWrite(LED_1, LOW);
    digitalWrite(LED_2, HIGH);
    delay(200);
    digitalWrite(LED_2, LOW);
    digitalWrite(LED_3, HIGH);
    delay(400);
    digitalWrite(LED_3, LOW);
    digitalWrite(LED_4, HIGH);
    delay(200);
    digitalWrite(LED_4, LOW);
    digitalWrite(LED_5, HIGH);
    delay(600);
    digitalWrite(LED_5, LOW);
    delay(500);
  }
}
