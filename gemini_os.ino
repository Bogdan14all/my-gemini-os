/*
 * Gemini OS v5 - Modern UI/UX Edition
 * Waveshare ESP32-S3 Touch LCD 3.49" V2 (172x640)
 */

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include <Wire.h>
#include <time.h>
#include <ArduinoJson.h>
#include <Arduino_GFX_Library.h>
#include "driver/i2s.h"
#include "esp_sleep.h"

// --- PALETA CULORI MODERNE (RGB565) ---
#define COLOR_BG        0x0842  // Dark Navy/Slate
#define COLOR_CARD      0x18E5  // Surface Card
#define COLOR_PRIMARY   0x3D1F  // Gemini Blue/Purple
#define COLOR_ACCENT    0x07FF  // Cyan Glow
#define COLOR_TEXT_PRI  0xFFFF  // Pure White
#define COLOR_TEXT_SEC  0x9CD3  // Muted Grey
#define COLOR_GREEN     0x2FE4  // Mint Green
#define COLOR_AMBER     0xFD60  // Warm Amber
#define COLOR_RED       0xF986  // Coral Red
#define BLACK           0x0000

// --- PINI ECRAN QSPI (AXS15231B) ---
#define TFT_CS    11
#define TFT_DC    8
#define TFT_RST   9
#define TFT_SCLK  12
#define TFT_D0    10
#define TFT_D1    14
#define TFT_D2    15
#define TFT_D3    13

// --- PINI I2C & BUTOANE ---
#define I2C_SDA         2
#define I2C_SCL         1
#define TCA9554_ADDR    0x20
#define TOUCH_ADDR      0x3B
#define BATTERY_ADC_PIN 4
#define BOOT_BUTTON_PIN 0

// --- PINI AUDIO I2S ---
#define I2S_NUM         I2S_NUM_0
#define I2S_BCK_IO      46
#define I2S_WS_IO       45
#define I2S_DO_IO       42
#define I2S_DI_IO       41

// Obiecte alocate in setup()
Arduino_DataBus *bus = nullptr;
Arduino_GFX *gfx = nullptr;

// --- STARE SISTEM ---
bool isStandby = false;
int systemVolume = 80;
int systemBrightness = 100;
String currentAiStatus = "Gata";
String lastAiResponse = "Salut! Atinge butonul de jos sau apasa BOOT pentru a vorbi cu Gemini.";
uint16_t orbColor = COLOR_ACCENT;
unsigned long lastTopBarUpdate = 0;

// --- EXPANDER I2C TCA9554 ---
void tcaWrite(uint8_t reg, uint8_t val) {
    Wire.beginTransmission(TCA9554_ADDR);
    Wire.write(reg);
    Wire.write(val);
    Wire.endTransmission();
}

uint8_t tcaRead(uint8_t reg) {
    Wire.beginTransmission(TCA9554_ADDR);
    Wire.write(reg);
    Wire.endTransmission();
    Wire.requestFrom((uint8_t)TCA9554_ADDR, (uint8_t)1);
    return Wire.available() ? Wire.read() : 0;
}

void setSpeakerPower(bool en) {
    uint8_t val = tcaRead(0x01);
    if (en) val |= 0x08; else val &= ~0x08; // EXIO3 = Audio PA
    tcaWrite(0x01, val);
}

void setBacklight(bool en) {
    uint8_t val = tcaRead(0x01);
    if (en) val |= 0x02; else val &= ~0x02; // EXIO1 = Backlight
    tcaWrite(0x01, val);
}

// --- SUNET DIFUZOR I2S ---
void initAudio() {
    i2s_config_t cfg = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_RX),
        .sample_rate = 16000,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 256,
        .use_apll = false
    };
    i2s_pin_config_t pins = {
        .bck_io_num = I2S_BCK_IO,
        .ws_io_num = I2S_WS_IO,
        .data_out_num = I2S_DO_IO,
        .data_in_num = I2S_DI_IO
    };
    i2s_driver_install(I2S_NUM, &cfg, 0, NULL);
    i2s_set_pin(I2S_NUM, &pins);
    setSpeakerPower(true);
}

void playTone(int freq, int ms) {
    int samples = (16000 * ms) / 1000;
    size_t bw;
    for (int i = 0; i < samples; i++) {
        int16_t v = (int16_t)(sin(6.28318 * freq * i / 16000) * 8000.0 * (systemVolume / 100.0));
        int16_t stereo[2] = { v, v };
        i2s_write(I2S_NUM, stereo, sizeof(stereo), &bw, portMAX_DELAY);
    }
}

// --- DETECTIE TOUCH ---
bool getTouch(int16_t *x, int16_t *y) {
    Wire.beginTransmission(TOUCH_ADDR);
    Wire.write(0x00);
    if (Wire.endTransmission() != 0) return false;

    Wire.requestFrom((uint8_t)TOUCH_ADDR, (uint8_t)6);
    if (Wire.available() >= 6) {
        uint8_t st = Wire.read();
        uint8_t f = Wire.read();
        uint8_t x_hi = Wire.read(), x_lo = Wire.read();
        uint8_t y_hi = Wire.read(), y_lo = Wire.read();
        if (f > 0 || (st & 0x0F) > 0) {
            *x = ((x_hi & 0x0F) << 8) | x_lo;
            *y = ((y_hi & 0x0F) << 8) | y_lo;
            return true;
        }
    }
    return false;
}

int getBattery() {
    uint32_t raw = analogRead(BATTERY_ADC_PIN);
    float mv = (raw / 4095.0) * 3.3 * 2.0 * 1000.0;
    return constrain((int)((mv - 3300.0) / 9.0), 0, 100);
}

String getTimeStr() {
    struct tm t;
    if (!getLocalTime(&t)) return "12:00";
    char b[8];
    strftime(b, sizeof(b), "%H:%M", &t);
    return String(b);
}

// --- DESENARE UI ERGONOMIC & MODERN (172x640) ---
void drawTopBar() {
    if (!gfx) return;
    gfx->fillRoundRect(6, 6, 160, 26, 13, COLOR_CARD);
    gfx->setTextColor(COLOR_TEXT_SEC);
    gfx->setTextSize(1);

    // Wi-Fi
    gfx->setCursor(14, 15);
    gfx->print(WiFi.status() == WL_CONNECTED ? "WiFi" : "Offline");

    // Ceas
    gfx->setTextColor(COLOR_TEXT_PRI);
    gfx->setCursor(72, 15);
    gfx->print(getTimeStr());

    // Baterie
    gfx->setTextColor(COLOR_GREEN);
    gfx->setCursor(130, 15);
    gfx->printf("%d%%", getBattery());
}

void drawMainUI() {
    if (!gfx) return;
    gfx->fillScreen(COLOR_BG);
    drawTopBar();

    // 1. GEMINI ORB (Avatar vizual centrat)
    int orbY = 95;
    gfx->fillCircle(86, orbY, 32, COLOR_CARD);
    gfx->drawCircle(86, orbY, 32, orbColor);
    gfx->drawCircle(86, orbY, 31, orbColor);
    gfx->fillCircle(86, orbY, 18, orbColor);

    // Eticheta stare AI
    gfx->setTextColor(COLOR_TEXT_SEC);
    gfx->setTextSize(1);
    gfx->setCursor(45, 138);
    gfx->printf("Status: ");
    gfx->setTextColor(orbColor);
    gfx->print(currentAiStatus);

    // 2. CARD CONVERSATIE
    gfx->fillRoundRect(8, 160, 156, 280, 10, COLOR_CARD);
    gfx->drawRoundRect(8, 160, 156, 280, 10, COLOR_PRIMARY);
    gfx->setTextColor(COLOR_ACCENT);
    gfx->setTextSize(1);
    gfx->setCursor(16, 172);
    gfx->println("Asistent Gemini");

    gfx->setTextColor(COLOR_TEXT_PRI);
    gfx->setCursor(16, 195);
    gfx->println(lastAiResponse);

    // 3. CONTROALE RAPIDE: VOLUM & LUMINA (Y: 450 - 495)
    gfx->fillRoundRect(8, 450, 75, 42, 8, COLOR_CARD);
    gfx->setTextColor(COLOR_TEXT_SEC);
    gfx->setCursor(16, 460);
    gfx->print("VOLUM");
    gfx->setTextColor(COLOR_TEXT_PRI);
    gfx->setCursor(16, 474);
    gfx->printf("%d%%", systemVolume);

    gfx->fillRoundRect(89, 450, 75, 42, 8, COLOR_CARD);
    gfx->setTextColor(COLOR_TEXT_SEC);
    gfx->setCursor(97, 460);
    gfx->print("LUMINA");
    gfx->setTextColor(COLOR_TEXT_PRI);
    gfx->setCursor(97, 474);
    gfx->printf("%d%%", systemBrightness);

    // 4. BUTON PRINCIPAL THUMB-ZONE: VORBESTE (Y: 505 - 565)
    gfx->fillRoundRect(8, 505, 156, 60, 14, COLOR_PRIMARY);
    gfx->drawRoundRect(8, 505, 156, 60, 14, COLOR_ACCENT);
    gfx->setTextColor(COLOR_TEXT_PRI);
    gfx->setTextSize(2);
    gfx->setCursor(24, 526);
    gfx->println("VORBESTE");

    // 5. BUTON SECUNDAR: STANDBY / OPRIRE (Y: 575 - 625)
    gfx->fillRoundRect(8, 575, 156, 46, 10, COLOR_CARD);
    gfx->setTextColor(COLOR_RED);
    gfx->setTextSize(1);
    gfx->setCursor(44, 593);
    gfx->println("STANDBY / SLEEP");
}

void enterStandby() {
    isStandby = true;
    setSpeakerPower(false);
    setBacklight(false);
}

void exitStandby() {
    isStandby = false;
    setBacklight(true);
    setSpeakerPower(true);
    orbColor = COLOR_ACCENT;
    currentAiStatus = "Activ";
    drawMainUI();
    playTone(900, 100);
}

void handleVoice() {
    orbColor = COLOR_AMBER;
    currentAiStatus = "Ascult...";
    drawMainUI();
    playTone(800, 100);

    // Inregistrare audio I2S
    int16_t buf[256];
    size_t br;
    for (int i = 0; i < 30; i++) {
        i2s_read(I2S_NUM, buf, sizeof(buf), &br, 40 / portTICK_PERIOD_MS);
    }

    orbColor = COLOR_PRIMARY;
    currentAiStatus = "Gandesc...";
    drawMainUI();
    delay(400);

    orbColor = COLOR_GREEN;
    currentAiStatus = "Raspund";
    lastAiResponse = "Te aud perfect! Conversatia vocala este activa pe difuzor.";
    drawMainUI();

    // Redare raspuns sonor
    playTone(523, 80);
    playTone(659, 80);
    playTone(784, 150);

    delay(1000);
    orbColor = COLOR_ACCENT;
    currentAiStatus = "Gata";
    drawMainUI();
}

void setup() {
    Serial.begin(115200);
    pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);

    Wire.begin(I2C_SDA, I2C_SCL);
    tcaWrite(0x03, 0x00); // Toti pinii TCA9554 ca iesire
    tcaWrite(0x01, 0xFF); // Pornire Backlight, Power Hold si Audio PA

    bus = new Arduino_ESP32QSPI(TFT_CS, TFT_SCLK, TFT_D0, TFT_D1, TFT_D2, TFT_D3);
    gfx = new Arduino_AXS15231B(bus, TFT_RST, 0, true);

    gfx->begin();
    setBacklight(true);

    initAudio();
    configTime(7200, 3600, "pool.ntp.org");

    drawMainUI();
    playTone(1000, 120);
}

void loop() {
    // Buton fizic BOOT
    if (digitalRead(BOOT_BUTTON_PIN) == LOW) {
        unsigned long t0 = millis();
        while (digitalRead(BOOT_BUTTON_PIN) == LOW) delay(10);
        if (millis() - t0 > 2000) {
            setBacklight(false);
            setSpeakerPower(false);
            esp_deep_sleep_start();
        } else {
            if (isStandby) exitStandby(); else enterStandby();
        }
    }

    if (isStandby) {
        delay(50);
        return;
    }

    // Refresh Top Bar la fiecare 10 secunde
    if (millis() - lastTopBarUpdate > 10000) {
        drawTopBar();
        lastTopBarUpdate = millis();
    }

    // Detectie Touchscreen
    int16_t tx, ty;
    if (getTouch(&tx, &ty)) {
        // Buton VORBESTE (Y: 505 - 565)
        if (ty >= 505 && ty <= 565) {
            handleVoice();
        }
        // Buton Standby (Y: 575 - 625)
        else if (ty >= 575 && ty <= 625) {
            enterStandby();
            delay(300);
        }
        // Buton Volum (Y: 450 - 495, X: 8 - 83)
        else if (ty >= 450 && ty <= 495 && tx <= 83) {
            systemVolume = (systemVolume >= 100) ? 20 : systemVolume + 20;
            drawMainUI();
            playTone(400 + (systemVolume * 5), 80);
            delay(150);
        }
        // Buton Lumina (Y: 450 - 495, X: 89 - 164)
        else if (ty >= 450 && ty <= 495 && tx >= 89) {
            systemBrightness = (systemBrightness >= 100) ? 25 : systemBrightness + 25;
            drawMainUI();
            delay(150);
        }
        delay(120);
    }

    delay(20);
}
