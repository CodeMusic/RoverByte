#pragma once
#include <FastLED.h>
#include "../PrefrontalCortex/utilities.h"

static const int LED_NUM_MODES = 3;
static const int NUM_RAINBOW_COLORS = 7;
static const int STEP_DELAY = 100;
static const int MONTH_DIM = 128;

enum class Mode {
    FULL_MODE,
    WEEK_MODE,
    TIMER_MODE
};

class LEDManager {
public:
    static void init();
    static void updateLEDs();
    static void startLoadingAnimation();
    static void stopLoadingAnimation();
    static void nextMode();
    static void setMode(Mode newMode);
    static void setLED(int index, CRGB color);
    static void showLEDs();
    static void scaleLED(int index, uint8_t scale);
    static Mode getMode() { return currentMode; }
    static void updateLoadingAnimation();
    static bool isLoadingComplete();
    static void flashSuccess();
    static Mode previousMode;
    static Mode currentMode;


private:
    static CRGB leds[WS2812_NUM_LEDS];
    static uint8_t currentPosition;
    static uint8_t currentColorIndex;
    static uint8_t completedCycles;
    static uint8_t filledPositions;
    static uint8_t activeTrails;
    static unsigned long lastStepTime;
    static bool isLoading;
    
    static void updateFullMode();
    static void updateWeekMode();
    static void updateTimerMode();
    static CRGB getRainbowColor(uint8_t index);
}; 