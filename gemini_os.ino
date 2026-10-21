#include <WiFi.h>
#include <Wire.h>
#include <Arduino_GFX_Library.h>
#include "esp_sleep.h"

// Pini ecran
#define TFT_CS    11
#define TFT_DC    8
#define TFT_RST   9
#define TFT_SCLK  12
#define TFT_D0    10
#define TFT_D1    14
#define TFT_D2    15
#define TFT_D3    13

// I2C
#define I2C_SDA 2
#define I2C_SCL 1
#define BOOT_BUTTON_PIN 0

Arduino_DataBus *bus = nullptr;
Arduino_GFX *gfx = nullptr;

void setup() {
    Serial.begin(115200);
    pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);
    
    // Stabilizare boot
    delay(500);

    // Initializare alimentare expander (TCA9554)
    Wire.begin(I2C_SDA, I2C_SCL);
    Wire.beginTransmission(0x20); Wire.write(0x03); Wire.write(0x00); Wire.endTransmission(); // Output
    Wire.beginTransmission(0x20); Wire.write(0x01); Wire.write(0xFF); Wire.endTransmission(); // Power ON
    delay(200);

    // Initializare ecran
    bus = new Arduino_ESP32QSPI(TFT_CS, TFT_SCLK, TFT_D0, TFT_D1, TFT_D2, TFT_D3);
    gfx = new Arduino_AXS15231B(bus, TFT_RST, 0, true);
    
    if(gfx->begin()) {
        gfx->fillScreen(0x0000); // Negru
        gfx->fillRoundRect(10, 100, 152, 200, 10, 0x18E5);
        gfx->setTextColor(0x07FF);
        gfx->setTextSize(2);
        gfx->setCursor(25, 140);
        gfx->println("OS ONLINE");
        
        gfx->fillRoundRect(10, 400, 152, 60, 10, 0x3D1F);
        gfx->setTextColor(0xFFFF);
        gfx->setCursor(30, 420);
        gfx->println("VORBESTE");
    }
}

void loop() {
    // Butonul BOOT fizic intra in Sleep pentru a testa repornirea
    if (digitalRead(BOOT_BUTTON_PIN) == LOW) {
        delay(100);
        Wire.beginTransmission(0x20); Wire.write(0x01); Wire.write(0x00); Wire.endTransmission(); // Power OFF
        esp_deep_sleep_start();
    }
    delay(100);
}
