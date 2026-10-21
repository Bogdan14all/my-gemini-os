/*
 * Gemini OS v3 - Waveshare ESP32-S3 Touch LCD 3.49" V2
 * Ecran: AXS15231B QSPI (172x640) | Touch: I2C (0x3B)
 * Expander: TCA9554 (0x20) -> Backlight, Power, Speaker PA
 * Audio: I2S Difuzor (ES8311) & Microfon (ES7210)
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

// --- PINI I2C, TOUCH & EXPANDER ---
#define I2C_SDA         2
#define I2C_SCL         1
#define TCA9554_ADDR    0x20
#define TOUCH_ADDR      0x3B
#define BATTERY_ADC_PIN 4
#define BOOT_BUTTON_PIN 0

// --- PINI AUDIO I2S (DIFUZOR & MICROFON) ---
#define I2S_NUM         I2S_NUM_0
#define I2S_BCK_IO      46
#define I2S_WS_IO       45
#define I2S_DO_IO       42
#define I2S_DI_IO       41

// --- OBIECTE HARDWARE ---
Arduino_DataBus *bus = new Arduino_ESP32QSPI(TFT_CS, TFT_SCLK, TFT_D0, TFT_D1, TFT_D2, TFT_D3);
Arduino_GFX *gfx = new Arduino_AXS15231B(bus, TFT_RST, 0, true);
Preferences prefs;

// --- STARE SISTEM ---
bool isStandby = false;
int systemVolume = 80;
int systemBrightness = 100;
String currentStatus = "Apasati VORBESTE";
String lastAiResponse = "Salut! Sunt asistentul tau Gemini.";
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

void setSpeakerPower(bool enable) {
    uint8_t val = tca9554_read(0x01);
    if (enable) val |= 0x08;  // EXIO3 = Activare Amplificator Difuzor (PA)
    else val &= ~0x08;
    tca9554_write(0x01, val);
}

void setDisplayBacklight(bool enable) {
    uint8_t val = tca9554_read(0x01);
    if (enable) val |= 0x02;  // EXIO1 = Backlight ON
    else val &= ~0x02;
    tca9554_write(0x01, val);
}

void initTCA9554() {
    tca9554_write(0x03, 0x00); // Configurare toti pinii ca iesire
    tca9554_write(0x01, 0xFF); // Activare Backlight, Power Hold si Difuzor PA
}

// --- INITIALIZARE & REDARE SUNET DIFUZOR (I2S) ---
void initAudio() {
    i2s_config_t i2s_config = {
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
    
    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_BCK_IO,
        .ws_io_num = I2S_WS_IO,
        .data_out_num = I2S_DO_IO,
        .data_in_num = I2S_DI_IO
    };

    i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM, &pin_config);
    setSpeakerPower(true);
}

// Redare semnal sonor in difuzor (feedback acustic)
void playTone(int freqHz, int durationMs) {
    int samples = (16000 * durationMs) / 1000;
    int16_t sample_val;
    size_t bytes_written;

    for (int i = 0; i < samples; i++) {
        sample_val = (int16_t)(sin(2 * 3.14159 * freqHz * i / 16000) * 10000.0 * (systemVolume / 100.0));
        int16_t stereo[2] = { sample_val, sample_val };
        i2s_write(I2S_NUM, stereo, sizeof(stereo), &bytes_written, portMAX_DELAY);
    }
}

// --- DETECTIE TOUCH CAPACITIV ---
bool getTouch(int16_t *x, int16_t *y) {
    Wire.beginTransmission(TOUCH_ADDR);
    Wire.write(0x00);
    if (Wire.endTransmission() != 0) return false;

    Wire.requestFrom((uint8_t)TOUCH_ADDR, (uint8_t)6);
    if (Wire.available() >= 6) {
        uint8_t status = Wire.read();
        uint8_t fingers = Wire.read();
        uint8_t x_hi = Wire.read();
        uint8_t x_lo = Wire.read();
        uint8_t y_hi = Wire.read();
        uint8_t y_lo = Wire.read();

        if (fingers > 0 || (status & 0x0F) > 0) {
            *x = ((x_hi & 0x0F) << 8) | x_lo;
            *y = ((y_hi & 0x0F) << 8) | y_lo;
            return true;
        }
    }
    return false;
}

// --- NIVEL BATERIE SI CEAS ---
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

// --- INTERFATA GRAFICA (172x640) ---
void drawTopBar() {
    gfx->fillRect(0, 0, 172, 28, DARKGRAY);
    gfx->setTextColor(WHITE);
    gfx->setTextSize(1);

    gfx->setCursor(4, 10);
    if (WiFi.status() == WL_CONNECTED) gfx->print(WiFi.SSID().substring(0, 6));
    else gfx->print("No-WiFi");

    gfx->setCursor(72, 10);
    gfx->print(getFormattedTime());

    gfx->setCursor(132, 10);
    gfx->printf("%d%%", getBatteryPercent());

    gfx->drawFastHLine(0, 28, 172, LIGHTGRAY);
}

void drawMainUI() {
    gfx->fillScreen(BLACK);
    drawTopBar();

    // Caseta conversatie AI
    gfx->drawRoundRect(6, 36, 160, 360, 6, LIGHTGRAY);
    gfx->setCursor(12, 46);
    gfx->setTextColor(CYAN);
    gfx->setTextSize(1);
    gfx->println("Conversatie Gemini:");

    gfx->setCursor(12, 65);
    gfx->setTextColor(YELLOW);
    gfx->println(currentStatus);

    gfx->setCursor(12, 95);
    gfx->setTextColor(WHITE);
    gfx->setTextSize(1);
    gfx->println(lastAiResponse);

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

    // Buton Standby / Oprire (Y: 585 -> 625)
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
    setSpeakerPower(false);
    setDisplayBacklight(false);
}

void exitStandby() {
    isStandby = false;
    currentStatus = "Sistem activat.";
    setDisplayBacklight(true);
    setSpeakerPower(true);
    drawMainUI();
    playTone(880, 150); // Chime pornire
}

void powerOffSystem() {
    gfx->fillScreen(BLACK);
    gfx->setTextColor(RED);
    gfx->setTextSize(2);
    gfx->setCursor(30, 300);
    gfx->println("OPRIRE...");
    playTone(440, 300);
    delay(500);
    setSpeakerPower(false);
    setDisplayBacklight(false);
    tca9554_write(0x01, 0x00);
    esp_deep_sleep_start();
}

// --- CONVERSATIE: CAPTARE VOCE SI REDARE RASPUNS ---
void handleVoiceConversation() {
    currentStatus = "Ascult la microfon...";
    drawMainUI();
    playTone(950, 120); // Bip start inregistrare

    // Buffer citire esantioane din microfon I2S
    int16_t micBuffer[512];
    size_t bytesRead;
    for (int i = 0; i < 40; i++) {
        i2s_read(I2S_NUM, micBuffer, sizeof(micBuffer), &bytesRead, 50 / portTICK_PERIOD_MS);
    }

    currentStatus = "Gandesc la raspuns...";
    drawMainUI();

    // Simulare raspuns primit de la Gemini API
    lastAiResponse = "Am receptionat comanda. Redau raspunsul vocal in difuzorul integrat.";
    currentStatus = "Redau in difuzor...";
    drawMainUI();

    // Redare secventa audio in difuzor (chime melodie + notificare)
    playTone(523, 100);
    delay(50);
    playTone(659, 100);
    delay(50);
    playTone(784, 180);

    currentStatus = "Gata de interactiune";
    drawMainUI();
}

void setup() {
    Serial.begin(115200);
    pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);

    Wire.begin(I2C_SDA, I2C_SCL);
    initTCA9554();

    gfx->begin();
    setDisplayBacklight(true);
    gfx->fillScreen(BLACK);

    initAudio();
    configTime(7200, 3600, "pool.ntp.org");

    drawMainUI();
    playTone(1000, 150); // Sunet de confirmare boot
}

void loop() {
    // 1. BUTON FIZIC BOOT (GPIO 0)
    if (digitalRead(BOOT_BUTTON_PIN) == LOW) {
        unsigned long pressStart = millis();
        while (digitalRead(BOOT_BUTTON_PIN) == LOW) delay(10);
        unsigned long pressDuration = millis() - pressStart;

        if (pressDuration > 2000) {
            powerOffSystem();
        } else {
            if (isStandby) exitStandby();
            else enterStandby();
        }
    }

    if (isStandby) {
        delay(50);
        return;
    }

    // 2. ACTUALIZARE STATUS LA 10 SECUNDE
    if (millis() - lastTopBarUpdate > 10000) {
        drawTopBar();
        lastTopBarUpdate = millis();
    }

    // 3. DETECTIE TOUCH
    int16_t touchX, touchY;
    if (getTouch(&touchX, &touchY)) {
        // Buton Standby / Oprire (Y: 585 - 625)
        if (touchY >= 585 && touchY <= 625) {
            enterStandby();
            delay(300);
        }
        // Buton Vorbeste / Conversatie (Y: 410 - 465)
        else if (touchY >= 410 && touchY <= 465) {
            handleVoiceConversation();
        }
        // Buton Volum (Y: 535 - 575, X: 10 - 82)
        else if (touchY >= 535 && touchY <= 575 && touchX <= 82) {
            systemVolume = (systemVolume >= 100) ? 20 : systemVolume + 20;
            drawMainUI();
            playTone(600 + (systemVolume * 4), 100);
            delay(200);
        }
        // Buton Luminozitate (Y: 535 - 575, X: 90 - 162)
        else if (touchY >= 535 && touchY <= 575 && touchX >= 90) {
            systemBrightness = (systemBrightness >= 100) ? 25 : systemBrightness + 25;
            drawMainUI();
            delay(200);
        }
        delay(150);
    }

    delay(20);
}
