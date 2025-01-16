#pragma once
#include <FastLED.h>
#include "../VisualCortex/LEDManager.h"
#include "../VisualCortex/RoverManager.h"

class SlotsManager {
public:
    static void init();
    static void update();
    static void handleButtonPress();
    static void reset();
    static bool isGameActive() { return gameActive; }

private:
    static uint8_t activeSlotPair;
    static bool slotLocked[4];
    static CRGB slotColors[4];
    static unsigned long animationTimer;
    static bool showingResult;
    static bool gameActive;
    static void checkResult();
    static void showResult(bool won);
    static CRGB getRainbowColor(uint8_t index);
}; 