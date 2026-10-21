/*
 * Gemini OS v2 - Waveshare ESP32-S3 Touch LCD 3.49" V2
 * Driver Ecran: AXS15231B QSPI (172x640)
 * Expander I2C: TCA9554 (Backlight, Power Hold, Audio PA)
 * Touchscreen:  AXS15231B Capacitive I2C
 */

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include <Wire.h>
#include <time.h>
#include <ArduinoJson.h>
#include <Arduino_GFX_Library.h>
#include "esp_sleep.h"

// --- DEFINITII CULORI RGB565 ---
#define BLACK     0x0000
#define WHITE     0xFFFF
#define RED       0xF800
#define GREEN     0x07E0
#define BLUE      0x001F
#define CYAN      0x07FF
#define YELLOW    0xFFE0
#define DARKGRAY  0x2104
#define LIGHTGRAY 0x8410

// --- PINI ECRAN QSPI (AXS15231B) ---
#define TFT_CS    11
#define TFT_DC    8
#define TFT_RST   9
#define TFT_SCLK  12
#define TFT_D0    10
#define TFT_D1    14
#define TFT_D2    15
#define TFT_D3    13

// --- PINI I2C & PERIFERICE ---
#define I2C_SDA         2
#define I2C_SCL         1
#define TCA9554_ADDR    0x20
#define TOUCH_ADDR      0x3B
#define BATTERY_ADC_PIN 4
#define BOOT_BUTTON_PIN 0

// --- OBIECTE HARDWARE ---
Arduino_DataBus *bus = new Arduino_ESP32QSPI(TFT_CS, TFT_SCLK, TFT_D0, TFT_D1, TFT_D2, TFT_D3);
Arduino_GFX *gfx = new Arduino_AXS15231B(bus, TFT_RST, 0, true);
Preferences prefs;

// --- STARE SISTEM ---
bool isStandby = false;
int systemVolume = 80;       // 0 - 100%
int systemBrightness = 100;  // 0 - 100%
String currentStatus = "Gata de interactiune";
unsigned long lastTopBarUpdate = 0;

// --- COMUNICARE TCA9554 EXPANDER ---
void tca9554_write(uint8_t reg, uint8_t val) {
    Wire.beginTransmission(TCA9554_ADDR);
    Wire.write(reg);
    Wire.write(val);
    Wire.endTransmission();
}

uint8_t tca9554_read(uint8_t reg) {
    Wire.beginTransmission(TCA9554_ADDR);
    Wire.write(reg);
    Wire.endTransmission();
    Wire.requestFrom((uint8_t)TCA9554_ADDR, (uint8_t)1);
    return Wire.available() ? Wire.read() : 0;
}

void initTCA9554() {
    tca9554_write(0x03, 0x00); // Toti pinii ca iesire (Output)
    // EXIO1 = Backlight, EXIO7 = PWR HOLD, EXIO3 = Audio PA Enable
    tca9554_write(0x01, 0xFF); // Pornire completa ecran, lumina si audio
}

void setDisplayPower(bool state) {
    uint8_t val = tca9554_read(0x01);
    if (state) val |= 0x02;  // Backlight ON (EXIO1)
    else val &= ~0x02;       // Backlight OFF
    tca9554_write(0x01, val);
}

// --- DETECTIE TOUCH (AXS15231B) ---
struct TouchEvent {
    bool pressed;
    int16_t x;
    int16_t y;
};

TouchEvent getTouch() {
    TouchEvent ev = {false, 0, 0};
    Wire.beginTransmission(TOUCH_ADDR);
    Wire.write(0x00);
    if (Wire.endTransmission() != 0) return ev;

    Wire.requestFrom((uint8_t)TOUCH_ADDR, (uint8_t)6);
    if (Wire.available() >= 6) {
        uint8_t status = Wire.read();
        uint8_t fingers = Wire.read();
        uint8_t x_hi = Wire.read();
        uint8_t x_lo = Wire.read();
        uint8_t y_hi = Wire.read();
        uint8_t y_lo = Wire.read();

        if (fingers > 0 || (status & 0x0F) > 0) {
            ev.pressed = true;
            ev.x = ((x_hi & 0x0F) << 8) | x_lo;
            ev.y = ((y_hi & 0x0F) << 8) | y_lo;
        }
    }
    return ev;
}

// --- NIVEL BATERIE & CEAS ---
int getBatteryPercent() {
    uint32_t raw = analogRead(BATTERY_ADC_PIN);
    float mv = (raw / 4095.0) * 3.3 * 2.0 * 1000.0;
    int pct = (int)((mv - 3300.0) / (4200.0 - 3300.0) * 100.0);
    return constrain(pct, 0, 100);
}

String getFormattedTime() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) return "--:--";
    char buf[8];
    strftime(buf, sizeof(buf), "%H:%M", &timeinfo);
    return String(buf);
}

// --- DESENARE INTERFATA GRAFICA (172x640) ---
void drawTopBar() {
    gfx->fillRect(0, 0, 172, 28, DARKGRAY);
    gfx->setTextColor(WHITE);
    gfx->setTextSize(1);

    // Wi-Fi Status
    gfx->setCursor(4, 10);
    if (WiFi.status() == WL_CONNECTED) gfx->print(WiFi.SSID().substring(0, 6));
    else gfx->print("No-WiFi");

    // Ceas central
    gfx->setCursor(72, 10);
    gfx->print(getFormattedTime());

    // Nivel Baterie
    gfx->setCursor(132, 10);
    gfx->printf("%d%%", getBatteryPercent());

    gfx->drawFastHLine(0, 28, 172, LIGHTGRAY);
}

void drawMainUI() {
    gfx->fillScreen(BLACK);
    drawTopBar();

    // Fereastra Chat / Asistent
    gfx->drawRoundRect(6, 36, 160, 360, 6, LIGHTGRAY);
    gfx->setCursor(12, 48);
    gfx->setTextColor(CYAN);
    gfx->setTextSize(1);
    gfx->println("Gemini Assistant");

    gfx->setCursor(12, 70);
    gfx->setTextColor(WHITE);
    gfx->println(currentStatus);

    // Buton Voce / Microfon (Y: 410 -> 470)
    gfx->fillRoundRect(10, 410, 152, 55, 8, BLUE);
    gfx->setTextColor(WHITE);
    gfx->setTextSize(2);
    gfx->setCursor(35, 428);
    gfx->println("VORBESTE");

    // Buton Tastatura Text (Y: 475 -> 530)
    gfx->fillRoundRect(10, 475, 152, 50, 8, DARKGRAY);
    gfx->setCursor(30, 492);
    gfx->println("TASTATURA");

    // Butoane Volum & Luminozitate (Y: 535 -> 580)
    gfx->fillRoundRect(10, 535, 72, 40, 6, LIGHTGRAY);
    gfx->setTextSize(1);
    gfx->setCursor(20, 550);
    gfx->printf("VOL: %d%%", systemVolume);

    gfx->fillRoundRect(90, 535, 72, 40, 6, LIGHTGRAY);
    gfx->setCursor(102, 550);
    gfx->printf("LUM: %d%%", systemBrightness);

    // Buton Standby / Power Off (Y: 585 -> 625)
    gfx->fillRoundRect(10, 585, 152, 40, 6, RED);
    gfx->setTextColor(WHITE);
    gfx->setTextSize(1);
    gfx->setCursor(45, 600);
    gfx->println("STANDBY / OPRIRE");
}

// --- COMUTARE STANDBY & OPRIRE ---
void enterStandby() {
    isStandby = true;
    currentStatus = "In Standby. Apasa BOOT.";
    setDisplayPower(false); // Opreste lumina ecranului
}

void exitStandby() {
    isStandby = false;
    currentStatus = "Sistem activat.";
    setDisplayPower(true);  // Porneste lumina ecranului
    drawMainUI();
}

void powerOffSystem() {
    gfx->fillScreen(BLACK);
    gfx->setTextColor(RED);
    gfx->setTextSize(2);
    gfx->setCursor(30, 300);
    gfx->println("OPRIRE...");
    delay(1000);
    setDisplayPower(false);
    tca9554_write(0x01, 0x00); // Opreste complet alimentarea prin expander
    esp_deep_sleep_start();
}

void setup() {
    Serial.begin(115200);
    pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);

    // Initializare magistrala I2C (SDA=2, SCL=1)
    Wire.begin(I2C_SDA, I2C_SCL);
    initTCA9554(); // Porneste lumina si cipurile expander

    // Initializare ecran LCD
    gfx->begin();
    setDisplayPower(true);
    gfx->fillScreen(BLACK);

    // Sincronizare Ceas Romania (GMT+2 cu ora de vara)
    configTime(7200, 3600, "pool.ntp.org");

    drawMainUI();
}

void loop() {
    // 1. GESTIONARE BUTON FIZIC BOOT (GPIO 0)
    if (digitalRead(BOOT_BUTTON_PIN) == LOW) {
        unsigned long pressStart = millis();
        while (digitalRead(BOOT_BUTTON_PIN) == LOW) delay(10);
        unsigned long pressDuration = millis() - pressStart;

        if (pressDuration > 2000) {
            powerOffSystem(); // Apasare lunga (> 2 sec): Oprire completa
        } else {
            // Apasare scurta: Toggle Standby (Ecran oprit / pornit)
            if (isStandby) exitStandby();
            else enterStandby();
        }
    }

    if (isStandby) {
        delay(50);
        return;
    }

    // 2. ACTUALIZARE BARA DE STARE LA FIECARE 10 SECUNDE
    if (millis() - lastTopBarUpdate > 10000) {
        drawTopBar();
        lastTopBarUpdate = millis();
    }

    // 3. DETECTIE TOUCHSCREEN
    TouchEvent t = getTouch();
    if (t.pressed) {
        // Buton Standby / Oprire (Y: 585 - 625)
        if (t.y >= 585 && t.y <= 625) {
            enterStandby();
            delay(300);
        }
        // Buton Vorbeste / Microfon (Y: 410 - 465)
        else if (t.y >= 410 && t.y <= 465) {
            currentStatus = "Ascult vocea...";
            drawMainUI();
            delay(1000);
            currentStatus = "Procesare Gemini...";
            drawMainUI();
            delay(1000);
            currentStatus = "Gemini: Salut! Cu ce te pot ajuta astazi?";
            drawMainUI();
        }
        // Buton Volum (Y: 535 - 575, X: 10 - 82)
        else if (t.y >= 535 && t.y <= 575 && t.x <= 82) {
            systemVolume = (systemVolume >= 100) ? 20 : systemVolume + 20;
            drawMainUI();
            delay(200);
        }
        // Buton Luminozitate (Y: 535 - 575, X: 90 - 162)
        else if (t.y >= 535 && t.y <= 575 && t.x >= 90) {
            systemBrightness = (systemBrightness >= 100) ? 25 : systemBrightness + 25;
            drawMainUI();
            delay(200);
        }
        delay(150);
    }

    delay(20);
}
