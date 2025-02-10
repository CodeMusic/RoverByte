/**
 * @brief Core cognitive system integration and neural pathway coordination
 * 
 * Manages primary neural processing systems:
 * - Sensory Integration
 *   - Visual processing (LED, Display)
 *   - Auditory processing (Sound, Music)
 *   - Tactile processing (Touch, Buttons)
 *   - Proprioceptive feedback.           
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
#include "VisualCortex/FastLEDConfig.h"

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
#include "CorpusCallosum/SynapticPathways.h"
#include "MotorCortex/PinDefinitions.h"

// Use base namespace
using namespace CorpusCallosum;

// Include all managers after namespace declaration
#include "PrefrontalCortex/Utilities.h"
#include "PrefrontalCortex/SPIManager.h"
#include "PrefrontalCortex/RoverBehaviorManager.h"
#include "VisualCortex/DisplayConfig.h"
#include "VisualCortex/RoverViewManager.h"
#include "VisualCortex/LEDManager.h"
#include "VisualCortex/RoverManager.h"
#include "SomatosensoryCortex/UIManager.h"
#include "SomatosensoryCortex/MenuManager.h"
#include "PrefrontalCortex/PowerManager.h"
#include "PrefrontalCortex/SDManager.h"
#include "VisualCortex/VisualSynesthesia.h"
#include "AuditoryCortex/SoundFxManager.h"
#include "PsychicCortex/WiFiManager.h"
#include "PsychicCortex/IRManager.h"
#include "PsychicCortex/NFCManager.h"
#include "GameCortex/SlotsManager.h"

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

// Replace or remove avr/interrupt.h
#ifdef ESP32
    #include "esp32-hal.h"
#endif

#include "main.h"

// Define the global encoder instance
RotaryEncoder encoder(ENCODER_INA, ENCODER_INB, RotaryEncoder::LatchMode::TWO03);

// Function declarations
void setup();
void loop();



// Implementation
void setup() {
    Utilities::LOG_SCOPE("setup()");
    Serial.begin(115200);
    esp_log_level_set("*", ESP_LOG_VERBOSE);
    Utilities::LOG_DEBUG("Starting with verbose logging...");

    // Enable watchdog
    esp_task_wdt_init(30, true); // 30 second timeout
    esp_task_wdt_add(NULL);

    // Monitor initial memory state
    Utilities::LOG_DEBUG("Initial heap: %d, Free: %d, Min Free: %d", 
        ESP.getHeapSize(),
        ESP.getFreeHeap(),
        ESP.getMinFreeHeap());

    // Check PSRAM if available
    if(psramFound()) {
        Utilities::LOG_DEBUG("PSRAM size: %d, Free: %d", 
            ESP.getPsramSize(),
            ESP.getFreePsram());
    }

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
        RoverViewManager::genericErrorMessage,
        RoverViewManager::detailedErrorMessage,
        RoverViewManager::isFatalError
    );
    RoverViewManager::drawLoadingScreen(RoverBehaviorManager::getStatusMessage());
    delay(100);
    if (!RoverBehaviorManager::isInitialized()) 
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
    //Utilities::LOG_SCOPE("Main::loop()");
    static unsigned long lastDraw = 0;
    const unsigned long DRAW_INTERVAL = 50;  // 20fps
    static bool soundStarted = false;

    // Add watchdog reset
    esp_task_wdt_reset();

    // Protect against null pointer access
    if (!RoverBehaviorManager::isInitialized()) {
        Utilities::LOG_ERROR("RoverBehaviorManager not initialized!");
        delay(1000);
        return;
    }

    // Handle critical updates first with error checking
    try {
        if (RoverBehaviorManager::isValid()) {
            RoverBehaviorManager::update();
            delay(1);

            // Only update UI and LED if we're not in LOADING state
            if (RoverBehaviorManager::getCurrentState() != PC::BehaviorState::LOADING) {
                if (UIManager::isInitialized()) {
                    UIManager::update();
                    delay(1);
                }
                
                if (LEDManager::isInitialized()) {
                    LEDManager::update();
                    delay(1);
                }
                
                // Handle sound initialization with null check
                if (!soundStarted && 
                    SoundFxManager::isInitialized() && 
                    SoundFxManager::isValid() && 
                    !SoundFxManager::isPlaying()) {
                    SoundFxManager::playStartupSound();
                    soundStarted = true;
                    delay(1);
                }
            }
        }
        
        // Handle display updates at fixed interval
        unsigned long currentMillis = millis();
        if (currentMillis - lastDraw >= DRAW_INTERVAL && RoverViewManager::isInitialized()) 
        {
            lastDraw = currentMillis;
            
            // Clear sprite first
            RoverViewManager::clearSprite();
            
            // Handle different cognitive states with additional validation
            if (RoverViewManager::isError || RoverViewManager::isFatalError) 
            {
                // Only draw error screen in error state
                RoverViewManager::drawErrorScreen(
                    RoverViewManager::errorCode,
                    RoverViewManager::genericErrorMessage,
                    RoverViewManager::detailedErrorMessage,
                    RoverViewManager::isFatalError
                );
            }
            else if (RoverBehaviorManager::getCurrentState() == PC::BehaviorState::LOADING) 
            {
                // Draw loading screen during initialization
                RoverViewManager::drawLoadingScreen(
                    RoverBehaviorManager::getStatusMessage()
                );
            }
            else if (RoverViewManager::isValid())  // Additional validation
            {
                // Normal operation - draw current view and rover
                RoverViewManager::drawCurrentView();
                
                if (!RoverViewManager::isError && 
                    !RoverViewManager::isFatalError &&
                    RoverManager::isInitialized()) 
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
            if (RoverViewManager::isValid()) {
                RoverViewManager::pushSprite();
                delay(1);
            }
        }
    } catch (const std::exception& e) {
        Utilities::LOG_ERROR("Loop error: %s", e.what());
        // Add error recovery attempt
        RoverBehaviorManager::attemptRecovery();
        delay(100);
    } catch (...) {
        Utilities::LOG_ERROR("Unknown error in main loop");
        delay(100);
    }
    
    // Final yield with slightly longer delay for stability
    delay(5);
}


