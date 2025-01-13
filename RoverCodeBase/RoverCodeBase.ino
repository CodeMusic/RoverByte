#include <SPI.h>
#include <Wire.h>
#define FASTLED_ESP32_SPI_BUS FSPI
#define FASTLED_ALL_PINS_HARDWARE_SPI
#define FASTLED_ESP32_SPI_CLOCK_DIVIDER 16

#include <FastLED.h>
#include "TFT_eSPI.h"
#include <RotaryEncoder.h>
#include <time.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include "Audio.h"
#include <XPowersLib.h>
#include "driver/i2s.h"

#include "src/PrefrontalCortex/utilities.h"
#include "src/PrefrontalCortex/PowerManager.h"
#include "src/VisualCortex/RoverViewManager.h"
#include "src/VisualCortex/RoverManager.h"
#include "src/VisualCortex/ColorUtilities.h"
#include "src/VisualCortex/DisplayConfig.h"
#include "src/AuditoryCortex/SoundFxManager.h"
#include "src/VisualCortex/LEDManager.h"
#include "src/PsychicCortex/WiFiManager.h"
#include "src/PrefrontalCortex/SDManager.h"
#include "src/PsychicCortex/NFCManager.h"
#include "src/SomatosensoryCortex/UIManager.h"
#include "src/PrefrontalCortex/RoverBehaviorManager.h"

// Core configuration
#define SCREEN_CENTER_X 85
#define CLOCK_PIN 45

// Global objects
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);
RotaryEncoder encoder(ENCODER_INA, ENCODER_INB, RotaryEncoder::LatchMode::TWO03);

bool hasSDCard = false;
void setup() {
    Serial.begin(115200);
    delay(100); // Give serial time to initialize
    
    // Initialize display first
    tft.init();
    tft.setRotation(0);
    tft.writecommand(TFT_SLPOUT);
    delay(120);
    tft.writecommand(TFT_DISPON);
    spr.createSprite(SCREEN_WIDTH, SCREEN_HEIGHT);
    spr.setTextDatum(MC_DATUM);
    
    // Initialize core managers first
    RoverBehaviorManager::init();
    PowerManager::init();
    LEDManager::init();
    RoverViewManager::init();
    UIManager::init();
    
    // Initialize audio before other systems
    SoundFxManager::init();
    
    // Initialize non-critical systems
    SDManager::init();
    if (!SPIFFS.begin()) {
        Serial.println("ERROR: Failed to initialize SPIFFS");
    }
    
    // Start background processes last
    NFCManager::startBackgroundInit();
    WiFiManager::init();
}

void loop() {
    static unsigned long lastDraw = 0;
    const unsigned long DRAW_INTERVAL = 50;  // 20fps
    static bool soundStarted = false;
    
    // Play startup sound once
    if (!soundStarted) {
        SoundFxManager::playStartupSound();
        soundStarted = true;
    }
    
    // Update UI first
    UIManager::update();
    
    // Update LED animations
    LEDManager::updateLEDs();
    
    // Other updates
    if (millis() - lastDraw >= DRAW_INTERVAL) {
        RoverBehaviorManager::update();
        lastDraw = millis();
    }
}


