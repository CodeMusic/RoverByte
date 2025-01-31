/*
 * FastLEDConfig.h
 * 
 * Core LED configuration for the visual cortex's nervous system.
 * Defines hardware settings, timing constants, and animation parameters
 * used across the LED control system.
 * 
 * Note: This must be included before FastLED.h in the main sketch
 * to properly configure the ESP32 hardware settings.
 */

#ifndef FASTLED_CONFIG_H
#define FASTLED_CONFIG_H

#include "../PrefrontalCortex/ProtoPerceptions.h"
#include "../CorpusCallosum/SynapticPathways.h"

namespace VisualCortex 
{
    using namespace CorpusCallosum;
    using PC::ConfigTypes:: LEDConfig;
    using PC::VisualTypes::VisualPattern;

    // Core FastLED ESP32 Hardware Configuration
    namespace FastLEDHardware 
    {
        constexpr bool INTERNAL = true;
        constexpr uint8_t ESP32_I2S = 0;
        constexpr bool ALLOW_INTERRUPTS = false;
        constexpr bool ESP32_FLASH_LOCK = true;
        constexpr uint8_t RMT_MAX_CHANNELS = 2;
        constexpr bool ESP32_RMT = true;
    }

    // LED Strip Configuration
    namespace FastLEDConfig 
    {
        constexpr uint8_t MAX_BRIGHTNESS = 255;
        constexpr uint8_t DEFAULT_FPS = 60;
        constexpr auto COLOR_CORRECTION = TypicalLEDStrip;
        #define LED_TYPE WS2812B
    }

    // Animation Timing Constants
    namespace AnimationTiming 
    {
        constexpr uint16_t LOADING_DELAY = 100;
        constexpr uint16_t FLASH_DURATION = 100;
        constexpr uint8_t FADE_STEP = 5;
        constexpr uint8_t MIN_FADE = 50;
        constexpr uint8_t MAX_FADE = 250;
        constexpr uint8_t ERROR_MIN_FADE = 64;
    }

    // Pattern Configuration
    namespace PatternConfig 
    {
        constexpr uint8_t LEDS_PER_STEP = 3;
        constexpr uint8_t ERROR_LED_INDEX = 0;
        constexpr uint8_t ERROR_LED_COUNT = 8;
        constexpr uint8_t FADE_INCREMENT = 5;
        constexpr uint16_t ANIMATION_DELAY = 50;
    }

    // Boot Sequence Configuration
    namespace BootConfig 
    {
        constexpr uint16_t STEP_DELAY = 100;
        constexpr uint16_t SUCCESS_FLASH_DURATION = 100;
        constexpr uint16_t ERROR_FLASH_DURATION = 200;
    }
}

// Required FastLED configuration defines
#define FASTLED_INTERNAL               VisualCortex::FastLEDHardware::INTERNAL
#define FASTLED_ESP32_I2S             VisualCortex::FastLEDHardware::ESP32_I2S
#define FASTLED_ALLOW_INTERRUPTS      VisualCortex::FastLEDHardware::ALLOW_INTERRUPTS
#define FASTLED_ESP32_FLASH_LOCK      VisualCortex::FastLEDHardware::ESP32_FLASH_LOCK
#define FASTLED_RMT_MAX_CHANNELS      VisualCortex::FastLEDHardware::RMT_MAX_CHANNELS
#define FASTLED_ESP32_RMT             VisualCortex::FastLEDHardware::ESP32_RMT

#endif // FASTLED_CONFIG_H 