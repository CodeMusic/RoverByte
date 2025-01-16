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
#include "src/VisualCortex/DisplayConfig.h"
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
#include "src/SomatosensoryCortex/UIManager.h"
#include "src/PrefrontalCortex/RoverBehaviorManager.h"
#include "src/SomatosensoryCortex/MenuManager.h"
#include "src/MotorCortex/PinDefinitions.h"

// Core configuration
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
    
    // Initialize core managers with try-catch blocks
    try {
        RoverBehaviorManager::init();
    } catch (const std::exception& e) {
        Serial.println("ERROR: Failed to initialize RoverBehaviorManager");
    }
    
    try {
        PowerManager::init();
    } catch (const std::exception& e) {
        Serial.println("ERROR: Failed to initialize PowerManager");
    }
    
    try {
        LEDManager::init();
    } catch (const std::exception& e) {
        Serial.println("ERROR: Failed to initialize LEDManager");
    }
    
    RoverViewManager::init();
    UIManager::init();
    
    try {
        MenuManager::init();
    } catch (const std::exception& e) {
        Serial.println("ERROR: Failed to initialize MenuManager");
    }
    
    // Initialize audio before other systems
    SoundFxManager::init();
    
    // Initialize non-critical systems
    SDManager::init();
    if (!SPIFFS.begin()) {
        Serial.println("ERROR: Failed to initialize SPIFFS");
    }
    
    // Start background processes last
    WiFiManager::init();
}

void loop() {
    static unsigned long lastDraw = 0;
    const unsigned long DRAW_INTERVAL = 50;  // 20fps
    static bool soundStarted = false;
    
    // Handle encoder direction
    encoder.tick();
    // Swap pins in encoder initialization instead of trying to negate the direction
    
    // Play startup sound once
    if (!soundStarted) {
        SoundFxManager::playStartupSound();
        soundStarted = true;
    }
    
    // Add try-catch block around UI updates
    try {
        UIManager::update();  // Remove direction parameter since it's not accepted
        LEDManager::updateLEDs();
    } catch (const std::exception& e) {
        Serial.println("ERROR in UI/LED update: " + String(e.what()));
    }
    
    // Add error checking for behavior updates
    if (millis() - lastDraw >= DRAW_INTERVAL) {
        try {
            RoverBehaviorManager::update();
        } catch (const std::exception& e) {
            Serial.println("ERROR in behavior update: " + String(e.what()));
        }
        lastDraw = millis();
    }
}


