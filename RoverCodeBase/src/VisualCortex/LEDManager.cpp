#include "LEDManager.h"
#include <time.h>
#include "ColorUtilities.h"
#include "../PrefrontalCortex/utilities.h"
#include <FastLED.h>
// Static member initialization
CRGB LEDManager::leds[WS2812_NUM_LEDS];
Mode LEDManager::currentMode = Mode::FULL_MODE;
int LEDManager::currentPosition = 0;
int LEDManager::currentColorIndex = 0;
int LEDManager::completedCycles = 0;
int LEDManager::filledPositions = 0;
int LEDManager::activeTrails = 0;
unsigned long LEDManager::lastStepTime = 0;
bool LEDManager::isLoading = false;

void LEDManager::init() {
    FastLED.addLeds<WS2812, WS2812_DATA_PIN, GRB>(leds, WS2812_NUM_LEDS);
    FastLED.setBrightness(50);
    FastLED.clear();
    FastLED.show();
}

void LEDManager::updateLEDs() {
    if (isLoading) {
        updateLoadingAnimation();
    }
    FastLED.show();
}

void LEDManager::startLoadingAnimation() {
    isLoading = true;
    FastLED.clear();
}

void LEDManager::stopLoadingAnimation() {
    isLoading = false;
    FastLED.clear();
    FastLED.show();
}

void LEDManager::nextMode() {
    currentMode = static_cast<Mode>((static_cast<int>(currentMode) + 1) % LED_NUM_MODES);
    FastLED.clear();
}

void LEDManager::setLED(int index, CRGB color) {
    if (index >= 0 && index < WS2812_NUM_LEDS) {
        leds[index] = color;
    }
}

void LEDManager::showLEDs() {
    FastLED.show();
}

void LEDManager::scaleLED(int index, uint8_t scale) {
    if (index >= 0 && index < WS2812_NUM_LEDS) {
        leds[index].nscale8(scale);
    }
}

void LEDManager::updateLoadingAnimation() {
    unsigned long currentTime = millis();
    if (currentTime - lastStepTime > 50) { // Update every 50ms
        FastLED.clear();
        
        // Create a rotating dot pattern
        int pos = currentPosition % WS2812_NUM_LEDS;
        leds[pos] = CRGB::White;
        
        // Update position for next frame
        currentPosition++;
        lastStepTime = currentTime;
    }
} 