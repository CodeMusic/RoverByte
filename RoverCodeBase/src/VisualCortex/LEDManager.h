#pragma once
#include <FastLED.h>
#include "../PrefrontalCortex/utilities.h"
#include "../MotorCortex/PinDefinitions.h"

static const int LED_NUM_MODES = 5;
static const int NUM_RAINBOW_COLORS = 7;
static const int STEP_DELAY = 100;
static const int MONTH_DIM = 128;

enum class Mode {
    FULL_MODE,
    WEEK_MODE,
    TIMER_MODE,
    OFF_MODE,
    FESTIVE_MODE
};

enum class FestiveTheme {
    NEW_YEAR,        // January 1
    VALENTINES,      // February 14
    ST_PATRICK,      // March 17
    EASTER,          // March/April (variable)
    CANADA_DAY,      // July 1
    HALLOWEEN,       // October 31
    CHRISTMAS        // December 25
};

class LEDManager {
public:
    static void init();
    static void updateLEDs();
    static void startLoadingAnimation();
    static void stopLoadingAnimation();
    static void nextMode();
    static void setMode(Mode newMode);
    static void setFestiveTheme(FestiveTheme theme);
    static void setLED(int index, CRGB color);
    static void syncLEDsForDay();
    static void showLEDs();
    static void scaleLED(int index, uint8_t scale);
    static Mode getMode() { return currentMode; }
    static void updateLoadingAnimation();
    static bool isLoadingComplete();
    static void flashSuccess();
    static void flashLevelUp();
    static Mode currentMode;
    static FestiveTheme currentTheme;
    static void displayCardPattern(uint8_t* uid, uint8_t length);
    static void update();

private:
    static CRGB leds[WS2812_NUM_LEDS];
    static Mode previousMode;
    static uint8_t currentPosition;
    static uint8_t currentColorIndex;
    static uint8_t completedCycles;
    static uint8_t filledPositions;
    static uint8_t activeTrails;
    static unsigned long lastStepTime;
    static bool isLoading;
    
    // New animation variables
    static uint8_t animationStep;
    static uint8_t fadeValue;
    static bool fadeDirection;
    static CRGB previousColors[WS2812_NUM_LEDS];
    
    static void updateFullMode();
    static void updateWeekMode();
    static void updateTimerMode();
    static void updateFestiveMode();
    static CRGB getRainbowColor(uint8_t index);
}; 