/**
 * @brief Core cognitive system integration and neural pathway coordination
 * 
 * Manages primary neural processing systems:
 * - Sensory Integration
 *   - Visual processing (LED, Display)
 *   - Auditory processing (Sound, Music)
 *   - Tactile processing (Touch, Buttons)
 *   - Proprioceptive feedback
 * 
 * - Cognitive Functions
 *   - Emotional state management
 *   - Behavioral pattern generation
 *   - Memory formation and recall
 *   - Learning and adaptation
 * 
 * - Cross-Modal Integration
 *   - Audio-visual synesthesia
 *   - Emotional-color mapping
 *   - Temporal-spatial coordination
 *   - Multi-sensory binding
 * 
 * - System Management
 *   - Power state regulation
 *   - Error detection/recovery
 *   - Resource allocation
 *   - Neural pathway optimization
 * 
 * @note Core system initialization must follow specific order:
 * 1. FastLED configuration
 * 2. Hardware interfaces
 * 3. Cognitive systems
 * 4. Behavioral patterns
 * 5. Interactive functions
 */

// FastLED configuration must come first
#include "src/VisualCortex/FastLEDConfig.h"

// Core system includes
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

// Include core configurations first
#include "src/CorpusCallosum/SynapticPathways.h"
#include "src/MotorCortex/PinDefinitions.h"

// Use base namespace
using namespace CorpusCallosum;

// Include all managers after namespace declaration
#include "src/PrefrontalCortex/utilities.h"
#include "src/PrefrontalCortex/SPIManager.h"
#include "src/PrefrontalCortex/RoverBehaviorManager.h"
#include "src/VisualCortex/DisplayConfig.h"
#include "src/VisualCortex/RoverViewManager.h"
#include "src/VisualCortex/LEDManager.h"
#include "src/VisualCortex/RoverManager.h"
#include "src/SomatosensoryCortex/UIManager.h"
#include "src/SomatosensoryCortex/MenuManager.h"
#include "src/PrefrontalCortex/PowerManager.h"
#include "src/PrefrontalCortex/SDManager.h"
#include "src/VisualCortex/VisualSynesthesia.h"
#include "src/AuditoryCortex/SoundFxManager.h"
#include "src/PsychicCortex/WiFiManager.h"
#include "src/PsychicCortex/IRManager.h"
#include "src/PsychicCortex/NFCManager.h"
#include "src/GameCortex/SlotsManager.h"

// Use specific namespaces after includes
using PC::Utilities;
using PC::SPIManager;
using PC::RoverBehaviorManager;
using PC::PowerManager;
using PC::SDManager;
using VC::RoverViewManager;
using VC::LEDManager;
using VC::RoverManager;
using VC::VisualSynesthesia;
using SC::UIManager;
using SC::MenuManager;
using AC::SoundFxManager;
using PSY::WiFiManager;
using PSY::IRManager;
using PSY::NFCManager;
using GC::SlotsManager;

#include <esp_task_wdt.h>

// Create encoder using pin definitions directly
RotaryEncoder encoder(ENCODER_INA, ENCODER_INB, RotaryEncoder::LatchMode::TWO03);

void setup() {
    Utilities::LOG_SCOPE("setup()");
    Serial.begin(115200);
    esp_log_level_set("*", ESP_LOG_VERBOSE);
    Utilities::LOG_DEBUG("Starting with verbose logging...");

    // Check free heap memory before initialization
    //LOG_DEBUG("Free heap before initialization: %d", ESP.getFreeHeap());

    // Initialize SPI bus and chip selects first
    SPIManager::init();
    Utilities::LOG_DEBUG("SPI Manager initialized.");

    try 
    {
        RoverViewManager::init();
        Utilities::LOG_DEBUG("Display initialized successfully.");
        
        // Draw initial loading screen instead of error screen
        RoverViewManager::drawLoadingScreen("Starting up...");
    } 
    catch (const std::exception& e) 
    {
        Utilities::LOG_ERROR("Display initialization failed: %s", e.what());
        return;
    }

    // After successful initialization
    RoverViewManager::drawErrorScreen(
        RoverViewManager::errorCode,
        RoverViewManager::errorMessage,
        RoverViewManager::isFatalError
    );
    RoverViewManager::drawLoadingScreen(RoverBehaviorManager::getStatusMessage());
    delay(100);
    if (!RoverBehaviorManager::IsInitialized()) 
    {
        try 
        {
            Utilities::LOG_DEBUG("Starting RoverBehaviorManager...");
            RoverBehaviorManager::init();
            Utilities::LOG_DEBUG("RoverBehaviorManager started successfully.");
        } 
        catch (const std::exception& e) 
        {
            Utilities::LOG_ERROR("Initialization error: %s", e.what());
            RoverBehaviorManager::triggerFatalError(
                static_cast<uint32_t>(PC::StartupErrorCode::CORE_INIT_FAILED),
                e.what()
            );
            return;
        }
    }

    // Check free heap memory after initialization
    //LOG_DEBUG("Free heap after initialization: %d", ESP.getFreeHeap());


}

void loop() {
    Utilities::LOG_SCOPE("loop()");
    static unsigned long lastDraw = 0;
    const unsigned long DRAW_INTERVAL = 50;  // 20fps
    static bool soundStarted = false;

    // Handle critical updates first with error checking
    try {
        RoverBehaviorManager::update();
        delay(1);

        // Only update UI and LED if we're not in LOADING state
        if (RoverBehaviorManager::getCurrentState() != PC::BehaviorState::LOADING) {
            UIManager::update();
            delay(1);
            
            LEDManager::update();
            delay(1);
            
            // Handle sound initialization
            if (!soundStarted && SoundFxManager::isInitialized() && 
                !SoundFxManager::isPlaying()) {
                SoundFxManager::playStartupSound();
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
            RoverViewManager::clearSprite();
            
            // Handle different cognitive states
            if (RoverViewManager::isError || 
                RoverViewManager::isFatalError) 
            {
                // Only draw error screen in error state
                RoverViewManager::drawErrorScreen(
                    RoverViewManager::errorCode,
                    RoverViewManager::errorMessage,
                    RoverViewManager::isFatalError
                );
            }
            else if (RoverBehaviorManager::getCurrentState() == 
                     PC::BehaviorState::LOADING) 
            {
                // Draw loading screen during initialization
                RoverViewManager::drawLoadingScreen(
                    RoverBehaviorManager::getStatusMessage()
                );
            }
            else 
            {
                // Normal operation - draw current view and rover
                RoverViewManager::drawCurrentView();
                
                if (!RoverViewManager::isError && 
                    !RoverViewManager::isFatalError) 
                {
                    RoverManager::drawRover(
                        RoverManager::getCurrentMood(),
                        RoverManager::earsPerked,
                        !RoverManager::showTime,
                        10,
                        RoverManager::showTime ? 50 : 80
                    );
                }
            }
            
            // Push sprite to display with minimal delay
            RoverViewManager::pushSprite();
            delay(1); // Reduced delay for better performance
        }
    } catch (const std::exception& e) {
        Utilities::LOG_ERROR("Loop error: %s", e.what());
        delay(100); // Give system time to recover
    }
    
    // Final yield
    delay(2);
}


