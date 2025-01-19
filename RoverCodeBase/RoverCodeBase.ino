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
#include "src/VisualCortex/VisualSynesthesia.h"
#include "src/VisualCortex/DisplayConfig.h"
#include "src/AuditoryCortex/SoundFxManager.h"
#include "src/VisualCortex/LEDManager.h"
#include "src/PsychicCortex/WiFiManager.h"
#include "src/PrefrontalCortex/SDManager.h"
#include "src/SomatosensoryCortex/UIManager.h"
#include "src/PrefrontalCortex/RoverBehaviorManager.h"
#include "src/SomatosensoryCortex/MenuManager.h"
#include "src/MotorCortex/PinDefinitions.h"
#include "src/GameCortex/SlotsManager.h"
#include "src/PsychicCortex/IRManager.h"
#include "src/PsychicCortex/NFCManager.h"
#include <esp_task_wdt.h>

// Core configuration
#define CLOCK_PIN 45    

// Global objects
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);
RotaryEncoder encoder(ENCODER_INA, ENCODER_INB, RotaryEncoder::LatchMode::TWO03);

bool hasSDCard = false;
void setup() {
    Serial.begin(115200);
    Serial.println("Starting setup...");

    // Initialize display first
    try {
        tft.init();
        tft.setRotation(0);
        tft.writecommand(TFT_SLPOUT);
        delay(120);
        tft.writecommand(TFT_DISPON);
        spr.createSprite(SCREEN_WIDTH, SCREEN_HEIGHT);
        spr.setTextDatum(MC_DATUM);
    } catch (const std::exception& e) {
        RoverBehaviorManager::triggerFatalError(
            static_cast<uint32_t>(RoverBehaviorManager::StartupErrorCode::DISPLAY_INIT_FAILED),
            "Display initialization failed"
        );
        return;
    }
    
    // Initialize core managers
    try {
        // esp_task_wdt_init(10, true);  // 10 second timeout, panic on timeout
        // esp_task_wdt_add(NULL);       // Add current thread to WDT watch
        RoverBehaviorManager::init();
    } catch (const std::exception& e) {
        RoverBehaviorManager::triggerFatalError(
            static_cast<uint32_t>(RoverBehaviorManager::StartupErrorCode::CORE_INIT_FAILED),
            "Core system initialization failed"
        );
        return;
    }
    
    try {
        // esp_task_wdt_init(10, true);  // 10 second timeout, panic on timeout
        // esp_task_wdt_add(NULL);       // Add current thread to WDT watch
        PowerManager::init();
    } catch (const std::exception& e) {
        RoverBehaviorManager::triggerFatalError(
            static_cast<uint32_t>(RoverBehaviorManager::StartupErrorCode::POWER_INIT_FAILED),
            "Power system initialization failed"
        );
        return;
    }
    
    try {
        LEDManager::init();
    } catch (const std::exception& e) {
        RoverBehaviorManager::triggerFatalError(
            static_cast<uint32_t>(RoverBehaviorManager::StartupErrorCode::LED_INIT_FAILED),
            "LED system initialization failed"
        );
        return;
    }
    
    try {
        RoverViewManager::init();
        UIManager::init();
    } catch (const std::exception& e) {
        RoverBehaviorManager::triggerFatalError(
            static_cast<uint32_t>(RoverBehaviorManager::StartupErrorCode::UI_INIT_FAILED),
            "UI system initialization failed"
        );
        return;
    }
    
    try {
        MenuManager::init();
    } catch (const std::exception& e) {
        RoverBehaviorManager::triggerFatalError(
            static_cast<uint32_t>(RoverBehaviorManager::StartupErrorCode::MENU_INIT_FAILED),
            "Menu system initialization failed"
        );
        return;
    }
    
    try {
        SoundFxManager::init();
    } catch (const std::exception& e) {
        RoverBehaviorManager::triggerFatalError(
            static_cast<uint32_t>(RoverBehaviorManager::StartupErrorCode::AUDIO_INIT_FAILED),
            "Audio system initialization failed"
        );
        return;
    }
    
    // Initialize storage systems
    if (!SPIFFS.begin()) {
        RoverBehaviorManager::triggerError(
            static_cast<uint32_t>(RoverBehaviorManager::StartupErrorCode::STORAGE_INIT_FAILED),
            "Storage init failed",
            RoverBehaviorManager::ErrorType::WARNING
        );
    }
    
    // Initialize WiFi last
    try {
        WiFiManager::init();
    } catch (const std::exception& e) {
        RoverBehaviorManager::triggerError(
            static_cast<uint32_t>(RoverBehaviorManager::StartupErrorCode::WIFI_INIT_FAILED),
            "WiFi init failed",
            RoverBehaviorManager::ErrorType::FATAL
        );
    }

    // esp_task_wdt_reset();  // Final watchdog reset after successful setup
    Serial.println("Setup complete!");
}

void loop() {
    static unsigned long lastDraw = 0;
    const unsigned long DRAW_INTERVAL = 50;  // 20fps
    static bool soundStarted = false;
    
    // Handle encoder direction
    encoder.tick();
    
    // Play startup sound once
    if (!soundStarted) {
        Serial.println("Playing startup sound...");
        SoundFxManager::playStartupSound();
        soundStarted = true;
    }
    
    // Update LED animation
    LEDManager::updateLoadingAnimation();
    
    // Add try-catch block around UI updates
    try {
        UIManager::update();
        SlotsManager::update();
        IRManager::update();
        NFCManager::update();
    } catch (const std::exception& e) {
        Serial.println("ERROR in update: ");
        Serial.println(e.what());
    }
    
    // Add error checking for behavior updates
    if (millis() - lastDraw >= DRAW_INTERVAL) {
        try {
            RoverBehaviorManager::update();
        } catch (const std::exception& e) {
            Serial.println("ERROR in behavior update: ");
            Serial.println(e.what());
        }
        lastDraw = millis();
    }
}


