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
#include "src/PrefrontalCortex/Utilities.h"
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

// Replace or remove avr/interrupt.h
#ifdef ESP32
    #include "esp32-hal.h"
#endif






