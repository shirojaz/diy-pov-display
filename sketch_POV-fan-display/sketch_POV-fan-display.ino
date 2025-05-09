// Blynk configuration
#define BLYNK_TEMPLATE_ID "TMPL6lmHoF70W"
#define BLYNK_TEMPLATE_NAME "POV Fan Display"
#define BLYNK_AUTH_TOKEN "ceQOsxT5W2r8kgMFg1uypjDRhix7jQ6t"

#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <TimeLib.h>       // For hour(), minute()
#include <WidgetRTC.h>     // Blynk's RTC time support

// ===== Hardware Pins and Settings =====
#define LED_PIN 13
#define NUM_LEDS 8
#define HALL_SENSOR 14
#define TOTAL_COLUMNS 16

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// WiFi Credentials
const char* ssid = "realme GT Master Edition";
const char* password = "password";

// Timing & display rotation vars
volatile unsigned long lastPulse = 0, currentPulse = 0;
volatile float rotationTime = 0, interval = 0;

// ===== State Variables =====
char message[32] = "TIME: 00:00";  // Default message
int displayMode = 0; // 0: time, 1: weather, 2: custom
char weatherDesc[32] = "Sunny";
char temperature[8] = "30C";
char customText[32] = "HELLO!";
uint32_t textColor = strip.Color(255, 255, 255); // default white
int brightness = 100;

WidgetRTC blynkRtc;
bool isBlynkConnected = false;

// ====== Basic Font: 5x7 for A–Z and 0–9 ======
const uint8_t font[][5] = {
  {0x7C, 0x12, 0x11, 0x12, 0x7C}, // A
  {0x7F, 0x49, 0x49, 0x49, 0x36}, // B
  {0x3E, 0x41, 0x41, 0x41, 0x22}, // C
  {0x7F, 0x41, 0x41, 0x22, 0x1C}, // D
  {0x7F, 0x49, 0x49, 0x49, 0x41}, // E
  {0x7F, 0x09, 0x09, 0x09, 0x01}, // F
  {0x3E, 0x41, 0x49, 0x49, 0x7A}, // G
  {0x7F, 0x08, 0x08, 0x08, 0x7F}, // H
  {0x00, 0x41, 0x7F, 0x41, 0x00}, // I
  {0x20, 0x40, 0x41, 0x3F, 0x01}, // J
  {0x7F, 0x08, 0x14, 0x22, 0x41}, // K
  {0x7F, 0x40, 0x40, 0x40, 0x40}, // L
  {0x7F, 0x02, 0x0C, 0x02, 0x7F}, // M
  {0x7F, 0x04, 0x08, 0x10, 0x7F}, // N
  {0x3E, 0x41, 0x41, 0x41, 0x3E}, // O
  {0x7F, 0x09, 0x09, 0x09, 0x06}, // P
  {0x3E, 0x41, 0x51, 0x21, 0x5E}, // Q
  {0x7F, 0x09, 0x19, 0x29, 0x46}, // R
  {0x46, 0x49, 0x49, 0x49, 0x31}, // S
  {0x01, 0x01, 0x7F, 0x01, 0x01}, // T
  {0x3F, 0x40, 0x40, 0x40, 0x3F}, // U
  {0x1F, 0x20, 0x40, 0x20, 0x1F}, // V
  {0x3F, 0x40, 0x38, 0x40, 0x3F}, // W
  {0x63, 0x14, 0x08, 0x14, 0x63}, // X
  {0x07, 0x08, 0x70, 0x08, 0x07}, // Y
  {0x61, 0x51, 0x49, 0x45, 0x43}, // Z
  {0x1C, 0x22, 0x49, 0x52, 0x1C}, // 0
  {0x00, 0x20, 0x7F, 0x20, 0x00}, // 1
  {0x62, 0x51, 0x49, 0x45, 0x42}, // 2
  {0x42, 0x41, 0x49, 0x55, 0x22}, // 3
  {0x0C, 0x14, 0x24, 0x7F, 0x04}, // 4
  {0x4F, 0x49, 0x49, 0x49, 0x31}, // 5
  {0x1E, 0x29, 0x49, 0x49, 0x06}, // 6
  {0x40, 0x47, 0x48, 0x50, 0x60}, // 7
  {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
  {0x30, 0x49, 0x49, 0x4A, 0x3C}  // 9
};

// ===== Interrupt: Hall sensor pulse for rotation sync =====
void hallSensorTrigger() {
  lastPulse = currentPulse;
  currentPulse = micros();
  rotationTime = currentPulse - lastPulse;
}

// ===== Setup: WiFi, Blynk, Interrupts, LEDs =====
void setup() {
  strip.begin();
  strip.setBrightness(brightness);
  strip.show();

  pinMode(HALL_SENSOR, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(HALL_SENSOR), hallSensorTrigger, FALLING);

  Serial.begin(115200);
  WiFi.begin(ssid, password);

  // Attempt Blynk connection (blocking)
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
  blynkRtc.begin();
}

// ===== Main Loop =====
void loop() {
  // Check Blynk connection once per loop
  if (Blynk.connected()) {
    isBlynkConnected = true;
    Blynk.run();
  } else {
    isBlynkConnected = false;
  }

  if (rotationTime > 0) {
    interval = rotationTime / TOTAL_COLUMNS;

    // Pick mode
    switch (displayMode) {
      case 0:  updateTime(); break;
      case 1:  sprintf(message, "%s %s", weatherDesc, temperature); break;
      case 2:  strncpy(message, customText, sizeof(message)); break;
    }

    displayArcText(message);
  }
}

// ====== Draw characters centered in the rotation ======
void displayArcText(char *text) {
  int textLength = strlen(text);
  int totalTextWidth = textLength * 5;
  int startOffset = (TOTAL_COLUMNS - totalTextWidth) / 2;

  delayMicroseconds(interval * startOffset);  // Centering

  for (int i = 0; i < textLength; i++) {
    char c = text[i];
    const uint8_t* charFont = nullptr;

    if (c >= 'A' && c <= 'Z')      charFont = font[c - 'A'];
    else if (c >= '0' && c <= '9') charFont = font[c - '0' + 26];
    else                           continue;

    for (int col = 0; col < 5; col++) {
      for (int row = 0; row < 7; row++) {
        int ledIndex = (row * NUM_LEDS / 7);
        bool pixelOn = charFont[col] & (1 << row);
        strip.setPixelColor(ledIndex, pixelOn ? textColor : 0);
      }
      strip.show();
      delayMicroseconds(interval);
    }
  }
}

// ===== Time mode display message =====
void updateTime() {
  if (year() < 2020) return;  // Wait for sync
  sprintf(message, "TIME: %02d:%02d", hour(), minute());
  if (isBlynkConnected) Blynk.virtualWrite(V0, message); // Optional V0 display
}

// ====== BLYNK HANDLERS ======
// Brightness Slider (V1)
BLYNK_WRITE(V1) {
  brightness = param.asInt();
  strip.setBrightness(brightness);
}

// Color Picker (V2)
BLYNK_WRITE(V2) {
  textColor = param.asInt(); // RGB 24-bit color from picker
}

// Mode Selector (V3): 0 = time, 1 = weather, 2 = custom
BLYNK_WRITE(V3) {
  displayMode = param.asInt();
}

// Default mode button (V4)
BLYNK_WRITE(V4) {
  displayMode = 0;
}

// Weather description (V5)
BLYNK_WRITE(V5) {
  strncpy(weatherDesc, param[0].c_str(), sizeof(weatherDesc));
}

// Temperature string (V6)
BLYNK_WRITE(V6) {
  strncpy(temperature, param[0].c_str(), sizeof(temperature));
}

// Custom text (V7)
BLYNK_WRITE(V7) {
  strncpy(customText, param[0].c_str(), sizeof(customText));
}
