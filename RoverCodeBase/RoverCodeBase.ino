#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <FastLED.h>
#include "TFT_eSPI.h"
#include <RotaryEncoder.h>
#include <time.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include "Audio.h"
#include <XPowersLib.h>
#include "driver/i2s.h"
#include <vector>

// Core system includes
#include "src/PrefrontalCortex/utilities.h"
#include "src/PrefrontalCortex/PowerManager.h"
#include "src/PrefrontalCortex/SPIManager.h"
#include "src/PrefrontalCortex/SDManager.h"
#include "src/PrefrontalCortex/RoverBehaviorManager.h"

// Sensory system includes
#include "src/VisualCortex/DisplayConfig.h"
#include "src/VisualCortex/RoverViewManager.h"
#include "src/VisualCortex/RoverManager.h"
#include "src/VisualCortex/VisualSynesthesia.h"
#include "src/VisualCortex/LEDManager.h"
#include "src/AuditoryCortex/SoundFxManager.h"
#include "src/SomatosensoryCortex/UIManager.h"
#include "src/SomatosensoryCortex/MenuManager.h"

// Communication includes
#include "src/PsychicCortex/WiFiManager.h"
#include "src/PsychicCortex/IRManager.h"
#include "src/PsychicCortex/NFCManager.h"

// Motor control includes
#include "src/MotorCortex/PinDefinitions.h"

// Game system includes
#include "src/GameCortex/SlotsManager.h"

#include <esp_task_wdt.h>

#include "src/CorpusCallosum/SynapticPathways.h"
using namespace CorpusCallosum;
using PC::Utilities;
using PC::SPIManager;
using PC::RoverBehaviorManager;
using SC::UIManager;
using VC::RoverViewManager;

// Global objects
RotaryEncoder encoder(static_cast<uint8_t>(ENCODER_INA), 
                     static_cast<uint8_t>(ENCODER_INB), 
                     RotaryEncoder::LatchMode::TWO03);

void setup() {
    Serial.begin(115200);
    Utilities::LOG_PROD("Starting setup...");

    // Check free heap memory before initialization
    //Utilities::LOG_DEBUG("Free heap before initialization: %d", ESP.getFreeHeap());

    // Initialize SPI bus and chip selects first
    SPIManager::init();
    Utilities::LOG_DEBUG("SPI Manager initialized.");

    try 
    {
        RoverViewManager::init();
        Utilities::LOG_DEBUG("Display initialized successfully.");
    } 
    catch (const std::exception& e) 
    {
        Utilities::LOG_ERROR("Display initialization failed: %s", e.what());
        return;
    }

    // After successful initialization
    VisualCortex::RoverViewManager::drawErrorScreen(
        VisualCortex::RoverViewManager::errorCode,
        VisualCortex::RoverViewManager::errorMessage,
        VisualCortex::RoverViewManager::isFatalError
    );
    VisualCortex::RoverViewManager::drawLoadingScreen(RoverBehaviorManager::getStatusMessage());
    delay(100);
    if (!RoverBehaviorManager::IsInitialized()) 
    {
        try 
        {
            Utilities::LOG_DEBUG("Starting RoverBehaviorManager...");
            PrefrontalCortex::RoverBehaviorManager::init();
            Utilities::LOG_DEBUG("RoverBehaviorManager started successfully.");
        } 
        catch (const std::exception& e) 
        {
            Utilities::LOG_ERROR("Initialization error: %s", e.what());
            PrefrontalCortex::RoverBehaviorManager::triggerFatalError(
                static_cast<uint32_t>(PrefrontalCortex::RoverBehaviorManager::StartupErrorCode::CORE_INIT_FAILED),
                e.what()
            );
            return;
        }
    }

    // Check free heap memory after initialization
    //Utilities::LOG_DEBUG("Free heap after initialization: %d", ESP.getFreeHeap());


}

void loop() {
    static unsigned long lastDraw = 0;
    const unsigned long DRAW_INTERVAL = 50;  // 20fps
    static bool soundStarted = false;

    // Handle critical updates first with error checking
    try {
        PrefrontalCortex::RoverBehaviorManager::update();
        delay(1);

        // Only update UI and LED if we're not in LOADING state
        if (PrefrontalCortex::RoverBehaviorManager::getCurrentState() != 
            PrefrontalCortex::RoverBehaviorManager::BehaviorState::LOADING) {
            UIManager::update();
            delay(1);
            
            VisualCortex::LEDManager::update();
            delay(1);
            
            // Handle sound initialization
            if (!soundStarted && AuditoryCortex::SoundFxManager::isInitialized() && 
                !AuditoryCortex::SoundFxManager::isPlaying()) {
                AuditoryCortex::SoundFxManager::playStartupSound();
                soundStarted = true;
                delay(1);
            }
        }
        
        // Handle display updates at fixed interval
        unsigned long currentMillis = millis();
        if (currentMillis - lastDraw >= DRAW_INTERVAL) 
        {
            lastDraw = currentMillis;
            
            // Clear sprite first
            VisualCortex::RoverViewManager::clearSprite();
            
            // Handle different cognitive states
            if (VisualCortex::RoverViewManager::isError || 
                VisualCortex::RoverViewManager::isFatalError) 
            {
                // Only draw error screen in error state
                VisualCortex::RoverViewManager::drawErrorScreen(
                    VisualCortex::RoverViewManager::errorCode,
                    VisualCortex::RoverViewManager::errorMessage,
                    VisualCortex::RoverViewManager::isFatalError
                );
            }
            else if (PrefrontalCortex::RoverBehaviorManager::getCurrentState() == 
                     PrefrontalCortex::RoverBehaviorManager::BehaviorState::LOADING) 
            {
                // Draw loading screen during initialization
                VisualCortex::RoverViewManager::drawLoadingScreen(
                    PrefrontalCortex::RoverBehaviorManager::getStatusMessage()
                );
            }
            else 
            {
                // Normal operation - draw current view and rover
                VisualCortex::RoverViewManager::drawCurrentView();
                
                if (!VisualCortex::RoverViewManager::isError && 
                    !VisualCortex::RoverViewManager::isFatalError) 
                {
                    VisualCortex::RoverManager::drawRover(
                        VisualCortex::RoverManager::getCurrentMood(),
                        VisualCortex::RoverManager::earsPerked,
                        !VisualCortex::RoverManager::showTime,
                        10,
                        VisualCortex::RoverManager::showTime ? 50 : 80
                    );
                }
            }
            
            // Push sprite to display with minimal delay
            VisualCortex::RoverViewManager::pushSprite();
            delay(1); // Reduced delay for better performance
        }
    } catch (const std::exception& e) {
        Utilities::LOG_ERROR("Loop error: %s", e.what());
        delay(100); // Give system time to recover
    }
    
    // Final yield
    delay(2);
}


