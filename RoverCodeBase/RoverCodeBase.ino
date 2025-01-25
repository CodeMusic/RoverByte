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
#include "src/PrefrontalCortex/SPIManager.h"

// Core configuration
#define CLOCK_PIN 45    

// Global objects
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);
RotaryEncoder encoder(ENCODER_INA, ENCODER_INB, RotaryEncoder::LatchMode::TWO03);

bool hasSDCard = false;
void setup() {
    Serial.begin(115200);
    LOG_PROD("Starting setup...");

    // Initialize SPI bus and chip selects first
    SPIManager::initSPI();
    
    LOG_DEBUG("Starting display initialization...");
    tft.init();
    LOG_DEBUG("TFT init successful");
    
    tft.setRotation(0);
    LOG_DEBUG("Rotation set");
    
    tft.writecommand(TFT_SLPOUT);
    LOG_DEBUG("Sleep out command sent");
    
    delay(120);
    
    tft.writecommand(TFT_DISPON);
    LOG_DEBUG("Display on command sent");
    
    if (!spr.createSprite(SCREEN_WIDTH, SCREEN_HEIGHT)) {
        throw std::runtime_error("Sprite creation failed");
    }
    LOG_DEBUG("Sprite created successfully");
    
    spr.setTextDatum(MC_DATUM);

    // Initialize core managers
    try {
        // esp_task_wdt_init(10, true);  // 10 second timeout, panic on timeout
        // esp_task_wdt_add(NULL);       // Add current thread to WDT watch
        RoverBehaviorManager::init();
    } catch (const std::exception& e) {
        RoverBehaviorManager::triggerFatalError(
            static_cast<uint32_t>(RoverBehaviorManager::StartupErrorCode::CORE_INIT_FAILED),
            e.what()
        );
        return;
    }

    // esp_task_wdt_reset();  // Final watchdog reset after successful setup
    Serial.println("Setup complete!");

    // After successful initialization
    spr.fillSprite(TFT_BLACK);
    spr.pushSprite(0, 0);
    delay(100); 
}

void loop() {
    static unsigned long lastDraw = 0;
    const unsigned long DRAW_INTERVAL = 50;  // 20fps
    static bool soundStarted = false;

    if (RoverBehaviorManager::getCurrentState() != RoverBehaviorManager::BehaviorState::LOADING) {
        LEDManager::stopLoadingAnimation();
        LEDManager::setMode(Mode::FULL_MODE);
    }    

    UIManager::update(); // Handle update to user input and update the UI
    LEDManager::update(); // Update LED patterns
    RoverBehaviorManager::update(); // The main state of RoverOS

    // Play startup sound once
    if (SoundFxManager::isInitialized() && !SoundFxManager::isPlaying() && !soundStarted) {
        Serial.println("Playing startup sound...");
        SoundFxManager::playStartupSound();
        soundStarted = true;
    }
    
    // Handle display updates at fixed interval
    unsigned long currentMillis = millis();
    if (currentMillis - lastDraw >= DRAW_INTERVAL) {
        lastDraw = currentMillis;
        if (RoverBehaviorManager::getCurrentState() == RoverBehaviorManager::BehaviorState::LOADING) {
            RoverViewManager::drawLoadingScreen(RoverBehaviorManager::getStatusMessage());
        } else {
            RoverViewManager::drawCurrentView();
        }
    }
}


