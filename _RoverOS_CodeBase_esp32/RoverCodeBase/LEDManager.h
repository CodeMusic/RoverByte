#pragma once
#include <FastLED.h>
#include "ColorUtilities.h"
#include "DisplayConfig.h"
#include "utilities.h"

class LEDManager {
public:
    enum Mode {
        FULL_MODE,
        WEEK_MODE,
        TIMER_MODE
    };

    static void init();
    static void initializeLEDs();
    static void updateLEDs();
    static void setMode(Mode newMode);
    static Mode getMode() { return currentMode; }
    static void nextMode();
    
    // Loading animation functions
    static void startLoadingAnimation();
    static void updateLoadingAnimation();
    static bool isLoadingComplete();
    static void stopLoadingAnimation();
    
    static void clearLEDs() {
        FastLED.clear(true);
        FastLED.show();
    }
    
private:
    static void updateFullMode();
    static void updateWeekMode();
    static void updateTimerMode();
    static void updateTimerAnimation();

    // Brightness constants
    static const uint8_t PAST_BRIGHTNESS = 0;      // 0%
    static const uint8_t FUTURE_BRIGHTNESS = 178;  // 70%
    static const uint8_t TODAY_BRIGHTNESS = 232;   // 91%
    static const uint8_t MONTH_DIM = 64;          // 25%

    static Mode currentMode;
    static CRGB leds[WS2812_NUM_LEDS];

    // Animation constants and state
    static const uint8_t NUM_RAINBOW_COLORS = 7;
    static const unsigned long STEP_DELAY = 125;  // Used for both timer and loading
    
    static uint8_t currentColorIndex;    // Current rainbow color (0-6)
    static uint8_t currentPosition;      // Current LED position (0-7)
    static uint8_t filledPositions;      // How many LEDs are filled with current color
    static uint8_t completedCycles;      // Number of white LEDs (completed cycles)
    static uint8_t activeTrails;         // Number of active trails in timer mode
    static unsigned long lastStepTime;   // Last animation step time
    static bool isLoading;
    
    static CRGB getRainbowColor(uint8_t index);
}; 