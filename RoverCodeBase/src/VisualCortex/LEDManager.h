#pragma once
#include <FastLED.h>
#include "../PrefrontalCortex/utilities.h"

static const int LED_NUM_MODES = 4;

enum class Mode {
    FULL_MODE,
    PULSE_MODE,
    RAINBOW_MODE,
    OFF_MODE
};

class LEDManager {
public:
    static void init();
    static void updateLEDs();
    static void startLoadingAnimation();
    static void stopLoadingAnimation();
    static void nextMode();
    static void setLED(int index, CRGB color);
    static void showLEDs();
    static void scaleLED(int index, uint8_t scale);
    static Mode getMode() { return currentMode; }
    static void updateLoadingAnimation();
    
    static Mode currentMode;

private:
    static CRGB leds[WS2812_NUM_LEDS];
    static int currentPosition;
    static int currentColorIndex;
    static int completedCycles;
    static int filledPositions;
    static int activeTrails;
    static unsigned long lastStepTime;
    static bool isLoading;
}; 