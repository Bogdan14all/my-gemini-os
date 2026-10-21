#include <Arduino.h>
#include <Wire.h>
#include <Arduino_GFX_Library.h>

#define TFT_CS    11
#define TFT_DC    8
#define TFT_RST   9
#define TFT_SCLK  12
#define TFT_D0    10
#define TFT_D1    14
#define TFT_D2    15
#define TFT_D3    13
#define BOOT_BUTTON_PIN 0

Arduino_DataBus *bus = nullptr;
Arduino_GFX *gfx = nullptr;

void setup() {
    Serial.begin(115200);
    pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);
    delay(500); 

    Wire.setTimeOut(150);
    Wire.begin(2, 1);

    Wire.beginTransmission(0x20); Wire.write(0x03); Wire.write(0x00); Wire.endTransmission();
    Wire.beginTransmission(0x20); Wire.write(0x01); Wire.write(0xFF); Wire.endTransmission();
    delay(200);

    bus = new Arduino_ESP32QSPI(TFT_CS, TFT_SCLK, TFT_D0, TFT_D1, TFT_D2, TFT_D3);
    gfx = new Arduino_AXS15231B(bus, TFT_RST, 0, true);
    
    if(gfx->begin()) {
        gfx->fillScreen(0x0000); 
        gfx->fillRoundRect(10, 100, 152, 200, 10, 0x18E5);
        gfx->setTextColor(0x07FF);
        gfx->setTextSize(2);
        gfx->setCursor(20, 140);
        gfx->println("OS ONLINE");
        
        gfx->setTextColor(0xFFFF);
        gfx->setTextSize(1);
        gfx->setCursor(20, 180);
        gfx->println("Sistem functional");
    }
}

void loop() {
    if (digitalRead(BOOT_BUTTON_PIN) == LOW) {
        if(gfx) {
            gfx->fillScreen(0xF800);
            gfx->setTextColor(0xFFFF);
            gfx->setTextSize(2);
            gfx->setCursor(40, 300);
            gfx->println("BUTON OK");
        }
        delay(1000);
    }
    delay(100);
}
