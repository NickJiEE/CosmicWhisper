/*
  Cosmic Whisper: An Educational Adventure through Light and Logic
  ---------------------------------------------------------------
  A hands-on, multi-stage learning game combining the mysteries of optics
  with the elegance of circuitry. Designed to engage, educate, and excite
  learners through interactive puzzles, sensor-driven challenges, and
  embedded logic.

  Created by: Nick Ji
  Project Date: January - August 2025
  Platform: ESP32-S3 with TFT display, sensors, and actuators

  “When science whispers, curiosity listens.”

  License: MIT
  Repository: https://github.com/NickJiEE/CosmicWhisper
*/

// ========== LIBRARIES ==========
#include "WiFi.h"
#include <Wire.h>
#include <Keypad.h>
#include <Arduino.h>
#include <arduinoFFT.h>
#include <HTTPClient.h>
#include <LiquidCrystal_I2C.h>
#include "Adafruit_TCS34725.h"

// ========== Google Sheet Communications ==========
#define DEVICE_NAME "game_6"

// Wi-Fi Credentials
const char* ssid = "UCSD-GUEST"; // Wi-Fi SSID
const char* password = ""; // Wi-Fi Password (empty if none)
unsigned long lastWiFiCheckTime = 0;
const unsigned long wifiCheckInterval = 10000; // Check every 10 seconds

void connectToWiFi(bool verbose = true);

// Apps Script URL for Sheet
String Web_App_URL = "https://script.google.com/macros/s/AKfycby6tQuxwJNh96bDOGEywhGtjb037TwChgm3f7RJI5lEb7PyBRkS998mO30AnnCaW5f9rg/exec";

// ========== GAME 1 PINS ==========
// keypad PINs
#define R1 19
#define R2 20
#define R3 21
#define R4 47
#define C1 48
#define C2 45
#define C3 0

// linear actuator PINs
enum { RPWM = 11, LPWM = 12, R_EN = 13, L_EN = 14 };

// ========== GAME 2 PINS ==========
// Morse Code PINs Config
const int LED_1 = 1;
const int LED_2 = 2;
const int LED_3 = 42;
const int LED_4 = 41;
const int LED_5 = 40;
const int buttonPin = 39;

// ========== GAME 3 PINS ==========
// mic on ADC pin
#define MIC_PIN 4

// RGB LED PINs (PWM)
#define RED_PIN   5
#define GREEN_PIN 6
#define BLUE_PIN  7

// TCS PINs
#define TCS_SDA 16
#define TCS_SCL 15

// LCD PINs
#define LCD_SDA 3
#define LCD_SCL 8

// 16x02 LCD configuration
int lcdColumns = 16;
int lcdRows = 2;

LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);  

// TCS RGB sensor
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
TwoWire WireTCS = TwoWire(1);

// keypad Configuration
const byte ROWS = 4;
const byte COLS = 3;

char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

byte rowPins[ROWS] = {R1, R2, R3, R4};
byte colPins[COLS] = {C1, C2, C3};

Keypad* keypadPtr = nullptr;

// FFT configuration
const uint16_t samples = 256;
const double samplingFrequency = 4000;  // Hz

double vReal[samples], vImag[samples];

ArduinoFFT<double> FFT = ArduinoFFT<double>(vReal, vImag, samples, samplingFrequency);

double envelopePeak = 0;

// passwords for stages 1 and 2 (first two are for unlocking stage 2)
const String stagePasswords[3] = {"2681", "0143", "87808"};
int currentStage = 0;
int warningCount = 0;
String inputBuffer = "";

bool actuatorDone = 0; // monitors actuator status

bool game2Started = false;
bool game3Started = false;
bool gameStatus = 0; // haven't won

// function prototypes
void showMessage(const String &msg, int row = 0, bool clearFirst = true, unsigned long waitMs = 0);
void scrollMessage(const String &msg, int row = 0, unsigned long delayMs = 300);
void setupGame1Display();
void processKeypad();

void setup() {
  Serial.begin(115200);

  // Wi-Fi connection
  connectToWiFi();

  // ========== LCD SETUP ==========
  Wire.begin(LCD_SDA, LCD_SCL);

  // initialize
  lcd.init();  
  lcd.backlight();

  setupGame1Display();

  // ========== GAME 1 SETUP ==========
  keypadPtr = new Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
  actuatorDone = 0;
  pinMode(RPWM, OUTPUT);
  pinMode(LPWM, OUTPUT);
  pinMode(R_EN, OUTPUT);
  pinMode(L_EN, OUTPUT);

  digitalWrite(R_EN, HIGH);
  digitalWrite(L_EN, HIGH);
  
  // ========== GAME 2 SETUP ==========
  // Morse Code LEDs setup
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  pinMode(LED_3, OUTPUT);
  pinMode(LED_4, OUTPUT);
  pinMode(LED_5, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  // ========== GAME 3 SETUP ==========
  WireTCS.begin(TCS_SDA, TCS_SCL);
  if (!tcs.begin(0x29, &WireTCS)) { showMessage("TCS34725 not found!", 0, true, 0); while(1); }

  /*
  if (tcs.begin(0x29, &WireTCS)) {
    Serial.println("TCS34725 sensor found.");
  } else {
    Serial.println("TCS34725 not found. Check wiring!");
    while (1);  // halt
  }
  */

  tcs.setInterrupt(true); // turns off LED

  analogReadResolution(12);
  
  // setup RGB LED
  ledcAttach(RED_PIN,   5000, 8);
  ledcAttach(GREEN_PIN, 5000, 8);
  ledcAttach(BLUE_PIN,  5000, 8);

  // Serial.println("Enter Stage 1 Password:");
}

void loop() {
  // background Wi-Fi check
  if (millis() - lastWiFiCheckTime >= wifiCheckInterval) {
    lastWiFiCheckTime = millis();

    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi disconnected. Reconnecting...");
      connectToWiFi(false);
    }
  }

  // ========== GAME 1 LOOP ==========
  if (currentStage < 3) {
    processKeypad();
  }
  
  // ========== GAME 2 LOOP ==========
  static unsigned long last = 0;
  if (currentStage == 2 && !game2Started) {
    morseCodeIndicator();
    showMessage("Decypher the", 0, true, 0);
    showMessage("Signal Code", 1, false, 2500);
    showMessage("Press button to", 0, true, 0);
    showMessage("display the code", 1, false, 2500);
    showMessage("Security Wall 3:", 0, true, 0);
    game2Started = true;
  }
  // display Morse Code
  if (currentStage == 2 && game2Started) {
    if (digitalRead(buttonPin)==LOW) {
      if (millis() - last > 1000) {
        last = millis();

        letterX(LED_1);
        delay(1500);
        letterW(LED_2);
        delay(1500);
        letterX(LED_3);
        delay(1500);
        letterZ(LED_4);
        delay(1500);
        letterX(LED_5);
      }
    }
  }
  else if (currentStage <= 1 && digitalRead(buttonPin) == LOW) {
    // Serial.println("Please complete stage 1 & 2 first!");
    showMessage("Need Access of", 0, true, 0);
    showMessage("Walls 1 & 2!", 1, false, 1500);
    showMessage("Security Wall " + String(currentStage + 1) + ":", 0, true, 0);
    processKeypad();
  } 
  else if (currentStage == 3 && digitalRead(buttonPin) == LOW && !gameStatus) {
    // Serial.println("You've already completed this part!");
    showMessage("You have the", 0, true, 0);
    showMessage("access already!", 1, false, 1500);
    processKeypad();
  }

  // ========== GAME 3 LOOP ==========
  if (currentStage == 3 && !gameStatus) {
    // destroy the keypad pointer
    delete keypadPtr;
    keypadPtr = nullptr;

    if (!game3Started) {
      // Serial.println("Stage 3 unlocked! Adjust the frequency and volume to match the target.");
      // delay(2000);
      showMessage("Match the signal", 0, true, 2500);
      showMessage("Adjust frequency", 0, true, 0);
      showMessage("and brightness", 1, false, 1000);
      game3Started = true;
    }

    envelopePeak = 0;

    // checking memory before FFT
    // Serial.printf("Free heap: %d, Free PSRAM: %d\n", ESP.getFreeHeap(), ESP.getFreePsram());

    // Serial.println("Starting sample collection...");
    // collect samples
    for (int i = 0; i < samples; i++) {
      int raw = analogRead(MIC_PIN);
      vReal[i] = raw - 2048;
      if (abs(vReal[i]) > envelopePeak) {
        envelopePeak = abs(vReal[i]);
      }
      vImag[i] = 0;
      delayMicroseconds(1000000.0 / samplingFrequency);
    }

    // --- FFT ---
    FFT.windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.compute(FFT_FORWARD);
    FFT.complexToMagnitude();

    // find peak frequency
    double peak = 0;
    int peakIndex = 0;
    double freq;

    int lowerBin = (int)(300.0 * samples / samplingFrequency);
    int upperBin = (int)(1700.0 * samples / samplingFrequency);
    for (int i = lowerBin; i < upperBin; i++) {
      if (vReal[i] > peak) {
        peak = vReal[i];
        peakIndex = i;
      }
    }

    // calculate total energy
    double totalEnergy = 0;
    for (int i = 1; i < (samples/2); i++) {
      totalEnergy += vReal[i];
    }

    // if the total energy is too low, treat as silence
    if (totalEnergy < 5000) {
      freq = 0;
    } else {
      // find peak frequency as usual
      double peak = 0;
      int peakIndex = 0;
      for (int i = 1; i < (samples/2); i++) {
        if (vReal[i] > peak) {
          peak = vReal[i];
          peakIndex = i;
        }
      }
      freq = (peakIndex * samplingFrequency) / samples;
    }
    
    if (peak < 50) {
      freq = 0; // treat as silence
    }

    if (freq == 1843.75) freq = 0;

    static double smoothedFreq = 0;
    double alpha = 0.2;  // smoothing factor
    smoothedFreq = alpha * freq + (1 - alpha) * smoothedFreq;
    freq = smoothedFreq;

    // print peak frequency
    // Serial.print("Peak frequency: ");
    // Serial.println(freq);

    // --- RGB LED color ---
    if (freq < 300) freq = 300;
    if (freq > 1700) freq = 1700;

    double norm = (freq - 300) / (1700 - 300);
    if (norm < 0) norm = 0;
    if (norm > 1) norm = 1;

    uint8_t r, g, b;

    int scaledNorm = (int)(norm * 100);

    if (scaledNorm < 20) {  // red to orange
      r = 255;
      g = map(scaledNorm, 0, 20, 0, 128);
      b = 0;
    } else if (scaledNorm < 40) {  // orange to yellow
      r = 255;
      g = 255;
      b = 0;
    } else if (scaledNorm < 60) {   // yellow to green
      r = map(scaledNorm, 40, 60, 255, 0);
      g = 255;
      b = 0;
    } else if (scaledNorm < 75) {  // green to blue
      r = 0;
      g = map(scaledNorm, 60, 75, 255, 0);
      b = map(scaledNorm, 60, 75, 0, 255);
    } else {  // blue to violet
      r = map(scaledNorm, 75, 100, 0, 127);
      g = 0;
      b = 255;
    }

    // failsafe
    if (r == 0 && g == 0 && b == 0) r = 20;

    // --- envelope reading ---
    float envelope = envelopePeak / 2048.0;

    // scale overall brightness
    uint8_t brightness = (uint8_t)(envelope * 255); // scale to 0–255

    // apply brightness to LED colors
    uint8_t r_bright = (r * brightness) / 255;
    uint8_t g_bright = (g * brightness) / 255;
    uint8_t b_bright = (b * brightness) / 255;

    ledcWrite(RED_PIN,   r_bright / 4);   // PWM 8-bit (0–255), dividing to limit intensity
    ledcWrite(GREEN_PIN, g_bright / 4);
    ledcWrite(BLUE_PIN,  b_bright / 4);

    // print envelope level
    // Serial.print(" Envelope level: ");
    // Serial.println(envelope);

    // --- TCS RGB sensor reading ---
    uint16_t r_raw, g_raw, b_raw, c_raw;
    tcs.getRawData(&r_raw, &g_raw, &b_raw, &c_raw);

    // Normalize color values to brightness (prevent ambient light from skewing)
    float r_norm = (float)r_raw / (float)c_raw;
    float g_norm = (float)g_raw / (float)c_raw;
    float b_norm = (float)b_raw / (float)c_raw;

    // Thresholds to detect "bright green"
    bool isBright = c_raw > 100; // around 0.7 envelope level
    // bool isGreenish = (g_norm > 0.45 && r_norm < 0.3 && b_norm < 0.3); // 900-1kHz color
    bool isBluish = (b_norm > 0.45 && r_norm < 0.3 && g_norm < 0.3);

    // debugging
    /*
    Serial.print("Raw RGB: ");
    Serial.print(r_raw); Serial.print(", ");
    Serial.print(g_raw); Serial.print(", ");
    Serial.print(b_raw); Serial.print(" | Clear: ");
    Serial.println(c_raw);

    Serial.print("Normalized RGB: ");
    Serial.print(r_norm, 2); Serial.print(", ");
    Serial.print(g_norm, 2); Serial.print(", ");
    Serial.println(b_norm, 2);
    */

    if (isBright && isBluish) {
      // Serial.println("Bright green detected!");
      // Serial.println("Winning message and stuff.");
      gameStatus = 1;
      showMessage("Congrats! You", 0, true, 0);
      showMessage("saved the world!", 1, false, 2500);
      sendDataToGoogleSheet();
      displayNextGame();
    }
  }
}

// ========== Google Sheet Communication FUNCTIONS ==========
// Wi-Fi function
void connectToWiFi(bool verbose) {
  if (verbose) {
    // Serial.println("WIFI mode : STA");
  }
  WiFi.mode(WIFI_STA);

  while (WiFi.status() != WL_CONNECTED) {
    if (verbose) {
      // Serial.print("Connecting to ");
      // Serial.println(ssid);
    }
    WiFi.begin(ssid, password);
    unsigned long startAttemptTime = millis();

    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
      if (verbose) {
        showMessage("Connecting...", 0, true, 0);
      }
      delay(500);
      // if (verbose) Serial.print(".");
    }

    if (WiFi.status() != WL_CONNECTED) {
      if (verbose) {
        // Serial.println("\nFailed to connect. Retrying...");
        showMessage("Retrying Wifi...", 0, true, 2000);
      }
    }
  }

  if (verbose) {
    showMessage("WiFi connected!", 0, true, 2000);
    // Serial.println("WiFi connected");
  } else {
    // Serial.println("WiFi reconnected (silent).");
  }
}

// send data to Google Sheets
void sendDataToGoogleSheet() {
  if (WiFi.status() == WL_CONNECTED) {
    String Send_Data_URL = Web_App_URL + "?action=espwrite&game=" + String(DEVICE_NAME);

    /*
    Serial.println("Send data to Google Spreadsheet...");
    Serial.print("URL : ");
    Serial.println(Send_Data_URL);
    */

    showMessage("Sending data...", 0, true, 2000);

    HTTPClient http;
    http.begin(Send_Data_URL.c_str());
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    int httpCode = http.GET();

    // Serial.print("HTTP Status Code : ");
    // Serial.println(httpCode);

    if (httpCode > 0) {
      String payload = http.getString();
      // Serial.println("Payload : " + payload);
    } else {
      // Serial.println("Failed to send data");
    }
    
    http.end();
    showMessage("Data sent!", 0, true, 2000);
  } else {
    // Serial.println("WiFi not connected");
    showMessage("WiFi not connected!", 0, true, 0);
  }
}

// receive data and display next game
void displayNextGame() {
  if (WiFi.status() == WL_CONNECTED) {
    // construct the request URL
    String nextGameUrl = Web_App_URL + "?action=espread&game=" + String(DEVICE_NAME);

    /*
    Serial.println("Receiving data from Google Spreadsheet...");
    Serial.print("URL : ");
    Serial.println(nextGameUrl);
    */

    showMessage("Loading data...", 0, true, 0);
    
    HTTPClient http;
    http.setConnectTimeout(5000);  // 5 seconds
    http.begin(nextGameUrl.c_str());
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    int httpCode = http.GET();  // make the GET request
    // Serial.print("HTTP Status Code : ");
    // Serial.println(httpCode);

    if (httpCode > 0) {
      String payload = http.getString();  // get the response from Apps Script
      Serial.println("Payload : " + payload);  // print the payload

      // display next game
      showMessage("Next Game: ", 0, true, 0);
      showMessage(payload, 1, false, 5000);

      showMessage("Press the button", 0, true, 0);
      showMessage("to see magic ;)", 1, false, 0);
    } else {
      // Serial.println("Failed to receive data");
      showMessage("Data fetch failed!", 0, true, 0);
      showMessage("Please contact", 0, true, 0);
      showMessage("a staff!", 1, false, 5000);
      
      showMessage("Press the button", 0, true, 0);
      showMessage("to see magic ;)", 1, false, 0);
    }
    
    http.end();  // close the HTTP connection
    
    // wait for user to press button
    while (digitalRead(buttonPin) == HIGH) {
      delay(10);  // debounce / prevent lock-up
    }
    ESP.restart();

  } else {
    // Serial.println("WiFi not connected");
    showMessage("WiFi not connected!", 0, true, 0);
  }
}

// ========== LCD FUNCTIONS ==========
// display helper
void showMessage(const String &msg, int row, bool clearFirst, unsigned long waitMs) {
  if (clearFirst) lcd.clear();
  lcd.setCursor(0, row);
  if (msg.length() <= lcdColumns) {
    lcd.print(msg);
  } else {
    scrollMessage(msg, row);
  }
  if (waitMs) delay(waitMs);
}

// scroll long messages
void scrollMessage(const String &msg, int row, unsigned long delayMs) {
  int len = msg.length();
  int total = len + lcdColumns;
  for (int offset = 0; offset <= total; offset++) {
    lcd.setCursor(0, row);
    for (int i = 0; i < lcdColumns; i++) {
      int idx = offset + i - lcdColumns;
      if (idx >= 0 && idx < len) lcd.print(msg[idx]); else lcd.print(' ');
    }
    delay(delayMs);
  }
}

// initial prompt
void setupGame1Display() {
  showMessage("Hello, Scientists ;)", 0, true, 500);
  showMessage("Save the world!", 0, true, 1500);
  showMessage("Security Wall 1:", 0, true, 1000);
}

// ========== Keypad FUNCTION ==========
void processKeypad() {
  char key = keypadPtr->getKey();
  if (!key) return;

  if (key == '#') {
    if (inputBuffer == stagePasswords[currentStage]) {
      currentStage++;
      inputBuffer = "";
      if (currentStage == 1 && !actuatorDone) {
        showMessage("Access Granted:", 0, true, 0);
        // actuator sequence
        // retracting
        showMessage("Opening...", 1, false, 0);
        analogWrite(RPWM, 0); analogWrite(LPWM, 200); delay(3000);
        analogWrite(RPWM, 0); analogWrite(LPWM, 0); delay(10000);

        // extending
        showMessage("Closing...", 1, false, 0);
        analogWrite(RPWM, 200); analogWrite(LPWM, 0); delay(3000);
        analogWrite(RPWM, 0); analogWrite(LPWM, 0); delay(1000);
        actuatorDone = true;
        showMessage("Security Wall 2:", 0, true, 0);
      } else if (currentStage < 3) {
        showMessage("Access Granted:", 0, true, 0);
        showMessage("Decyphering", 1, false, 2000);
      } else {
        showMessage("Complete Access", 0, true, 0);
        showMessage("Granted!", 1, false, 2750);
        // move to Game2
      }
    } else {
      showMessage("Nah uh!", 0, true, 0);
      showMessage("Try again.", 1, false, 1500);
      showMessage("Security Wall " + String(currentStage + 1) + ":", 0, true, 0);
      inputBuffer = "";
    }
  }
  else if (key == '*') {
    inputBuffer = "";
    showMessage("Input cleared", 0, true, 1000);
    showMessage("Security Wall " + String(currentStage + 1) + ":", 0, true, 0);
  } else {
    inputBuffer += key;
    lcd.setCursor(inputBuffer.length() - 1, 1);
    lcd.print(key);
  }
}

// ========== GAME 2 FUNCTIONS ==========
// indicates Morse Code part begins
void morseCodeIndicator() {
  const int leds[] = {LED_5, LED_4, LED_3, LED_2, LED_1};
  const int numLeds = sizeof(leds) / sizeof(leds[0]);

  for (int i = 0; i < numLeds; i++) {
    digitalWrite(leds[i], HIGH);
    delay(250); // 0.25 seconds
    digitalWrite(leds[i], LOW);
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
void letterZ(int pin) {
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
