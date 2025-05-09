// ========== Blynk Credentials ==========
#define BLYNK_TEMPLATE_ID "TMPL6lmHoF70W"
#define BLYNK_TEMPLATE_NAME "POV Fan Display"
#define BLYNK_AUTH_TOKEN "ceQOsxT5W2r8kgMFg1uypjDRhix7jQ6t"

// ========== Libraries ==========
#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <TimeLib.h>
#include <WidgetRTC.h>

// ========== Hardware Configuration ==========
#define LED_PIN       13
#define NUM_LEDS      8
#define TOTAL_COLUMNS 16  // Number of virtual POV slices per revolution

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// ========== WiFi Credentials ==========
const char* ssid = "realme GT Master Edition";
const char* password = "password";

// ========== Blynk Virtual Pins ==========
#define VPIN_BRIGHTNESS V1
#define VPIN_COLOR      V2
#define VPIN_MODE       V3
#define VPIN_CLOCK_MODE V4
#define VPIN_WEATHER_DESC V5
#define VPIN_WEATHER_TEMP V6
#define VPIN_CUSTOM_TEXT V7

// ========== Global State ==========
WidgetRTC blynkRtc;
char message[30] = "TIME: 00:00";
char weatherDesc[20] = "Sunny";
char temperature[10] = "28C";
char customText[30] = "HELLO";
int mode = 0;  // 0 = Clock, 1 = Weather, 2 = Custom
uint32_t textColor = Adafruit_NeoPixel::Color(255, 255, 255);
uint8_t brightness = 50;

unsigned long prevColumnTime = 0;
unsigned long columnInterval = 50;  // Time in ms between "slices"
int columnIndex = 0;

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

// ========== Setup ==========
void setup() {
  Serial.begin(115200);
  strip.begin();
  strip.setBrightness(brightness);
  strip.show();

  WiFi.begin(ssid, password);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
  blynkRtc.begin();
}

// ========== Main Loop ==========
void loop() {
  Blynk.run();

  if (millis() - prevColumnTime >= columnInterval) {
    prevColumnTime = millis();

    switch (mode) {
      case 0: updateTime(); break;
      case 1: snprintf(message, sizeof(message), "%s %s", weatherDesc, temperature); break;
      case 2: strncpy(message, customText, sizeof(message)); break;
    }

    displayColumn(message, columnIndex);
    columnIndex = (columnIndex + 1) % TOTAL_COLUMNS;
  }
}

// ========== Display a Column ==========
void displayColumn(const char* text, int colIndex) {
  strip.clear();
  int textLen = strlen(text);
  int totalCols = textLen * 6 - 1;

  int offset = colIndex - (TOTAL_COLUMNS - totalCols) / 2;

  for (int i = 0; i < textLen; i++) {
    char c = text[i];
    int fontIndex = -1;
    if (c >= 'A' && c <= 'Z') fontIndex = c - 'A';
    else if (c >= '0' && c <= '9') fontIndex = c - '0' + 26;

    if (fontIndex >= 0) {
      for (int col = 0; col < 5; col++) {
        int displayCol = i * 6 + col;
        if (displayCol == offset) {
          for (int row = 0; row < 7; row++) {
            if (font[fontIndex][col] & (1 << row)) {
              int ledIndex = row * NUM_LEDS / 7;
              strip.setPixelColor(ledIndex, textColor);
            }
          }
        }
      }
    }
  }
  strip.show();
}

// ========== Update Time ==========
void updateTime() {
  if (year() > 2020) {
    sprintf(message, "TIME: %02d:%02d", hour(), minute());
    Blynk.virtualWrite(VPIN_CLOCK_MODE, message);
  }
}

// ========== Blynk Handlers ==========
BLYNK_WRITE(VPIN_BRIGHTNESS) {
  brightness = param.asInt();
  strip.setBrightness(brightness);
}

BLYNK_WRITE(VPIN_COLOR) {
  int r = param[0].asInt();
  int g = param[1].asInt();
  int b = param[2].asInt();
  textColor = strip.Color(r, g, b);
}

BLYNK_WRITE(VPIN_MODE) {
  mode = param.asInt();
}

BLYNK_WRITE(VPIN_WEATHER_DESC) {
  strncpy(weatherDesc, param.asStr(), sizeof(weatherDesc));
}

BLYNK_WRITE(VPIN_WEATHER_TEMP) {
  strncpy(temperature, param.asStr(), sizeof(temperature));
}

BLYNK_WRITE(VPIN_CUSTOM_TEXT) {
  strncpy(customText, param.asStr(), sizeof(customText));
}
