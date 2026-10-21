/*
 * Gemini OS - Standalone Firmware (Zero Hardcoding / Tastatura Touch)
 * Placa: Waveshare ESP32-S3 Touch LCD 3.49" V2 (AXS15231B 172x640)
 */

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include <Arduino_GFX_Library.h>

#define TFT_CS   11
#define TFT_DC   8
#define TFT_RST  9
#define TFT_SCLK 12
#define TFT_D0   10
#define TFT_D1   14
#define TFT_D2   15
#define TFT_D3   13

#define TOUCH_I2C_SDA 2
#define TOUCH_I2C_SCL 1

Arduino_DataBus *bus = new Arduino_ESP32QSPI(TFT_CS, TFT_SCLK, TFT_D0, TFT_D1, TFT_D2, TFT_D3);
Arduino_GFX *gfx = new Arduino_AXS15231B(bus, TFT_RST, 0, true);
Preferences prefs;

void setup() {
    Serial.begin(115200);
    prefs.begin("gemini_os", false);

    gfx->begin();
    gfx->fillScreen(BLACK);
    gfx->setTextColor(CYAN);
    gfx->setTextSize(2);
    gfx->setCursor(10, 40);
    gfx->println("Gemini OS");
    gfx->setTextSize(1);
    gfx->setTextColor(WHITE);
    gfx->setCursor(10, 80);
    gfx->println("Initializare hardware...");

    Wire.begin(TOUCH_I2C_SDA, TOUCH_I2C_SCL);
}

void loop() {
    delay(100);
}
