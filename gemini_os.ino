#include <WiFi.h>
#include <Wire.h>
#include <Arduino_GFX_Library.h>
#include "esp_sleep.h"

#define COLOR_BG        0x0842
#define COLOR_CARD      0x18E5
#define COLOR_PRIMARY   0x3D1F
#define COLOR_ACCENT    0x07FF
#define COLOR_TEXT_PRI  0xFFFF
#define COLOR_RED       0xF986
#define BLACK           0x0000

#define TFT_CS    11
#define TFT_DC    8
#define TFT_RST   9
#define TFT_SCLK  12
#define TFT_D0    10
#define TFT_D1    14
#define TFT_D2    15
#define TFT_D3    13

#define I2C_SDA         2
#define I2C_SCL         1
#define TCA9554_ADDR    0x20
#define TOUCH_ADDR      0x3B
#define BOOT_BUTTON_PIN 0

Arduino_DataBus *bus = nullptr;
Arduino_GFX *gfx = nullptr;

void initHardware() {
    Wire.begin(I2C_SDA, I2C_SCL);
    delay(100);

    Wire.beginTransmission(TCA9554_ADDR);
    Wire.write(0x03); 
    Wire.write(0x00);
    Wire.endTransmission();

    Wire.beginTransmission(TCA9554_ADDR);
    Wire.write(0x01); 
    Wire.write(0x0B); 
    Wire.endTransmission();
    delay(100);

    bus = new Arduino_ESP32QSPI(TFT_CS, TFT_SCLK, TFT_D0, TFT_D1, TFT_D2, TFT_D3);
    gfx = new Arduino_AXS15231B(bus, TFT_RST, 0, true);
    gfx->begin();
    gfx->fillScreen(COLOR_BG);
}

void drawUI() {
    gfx->fillRoundRect(8, 160, 156, 280, 10, COLOR_CARD);
    gfx->setTextColor(COLOR_ACCENT);
    gfx->setTextSize(2);
    gfx->setCursor(16, 200);
    gfx->println("OS INITIALIZAT");
    
    gfx->fillRoundRect(8, 505, 156, 60, 14, COLOR_PRIMARY);
    gfx->setTextColor(COLOR_TEXT_PRI);
    gfx->setCursor(24, 526);
    gfx->println("VORBESTE");

    gfx->fillRoundRect(8, 575, 156, 46, 10, COLOR_CARD);
    gfx->setTextColor(COLOR_RED);
    gfx->setTextSize(1);
    gfx->setCursor(44, 593);
    gfx->println("STANDBY / SLEEP");
}

void setup() {
    Serial.begin(115200);
    pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);
    delay(1000); 

    initHardware();
    drawUI();
}

void loop() {
    if (digitalRead(BOOT_BUTTON_PIN) == LOW) {
        delay(500); 
        Wire.beginTransmission(TCA9554_ADDR);
        Wire.write(0x01); 
        Wire.write(0x00); 
        Wire.endTransmission();
        esp_deep_sleep_start();
    }
    delay(100);
}
