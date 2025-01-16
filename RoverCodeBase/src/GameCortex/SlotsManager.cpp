#include "SlotsManager.h"
#include "../VisualCortex/LEDManager.h"
#include "../VisualCortex/RoverViewManager.h"
#include "../AuditoryCortex/SoundFxManager.h"
#include "../SomatosensoryCortex/MenuManager.h"

// Initialize static members
uint8_t SlotsManager::activeSlotPair = 0;
bool SlotsManager::slotLocked[4] = {false};
CRGB SlotsManager::slotColors[4];
unsigned long SlotsManager::animationTimer = 0;
bool SlotsManager::showingResult = false;
bool SlotsManager::gameActive = false;

void SlotsManager::init() {
    gameActive = true;
    showingResult = false;
    activeSlotPair = 0;
    for(int i = 0; i < 4; i++) {
        slotLocked[i] = false;
        slotColors[i] = getRainbowColor(random(7));
    }
    animationTimer = millis();
}

void SlotsManager::update() {
    if (!gameActive) return;
    
    if (showingResult) {
        // Flash winning slots
        if (millis() - animationTimer > 250) { // Faster animation
            static bool flashState = false;
            for(int i = 0; i < 4; i++) {
                if (slotLocked[i]) {
                    CRGB color = flashState ? slotColors[i] : CRGB::Black;
                    LEDManager::setLED(i*2, color);
                    LEDManager::setLED(i*2+1, color);
                }
            }
            LEDManager::showLEDs();
            flashState = !flashState;
            animationTimer = millis();
        }
        
        if (millis() - animationTimer > 3000) {
            gameActive = false;
            MenuManager::show();
            return;
        }
    } else {
        // Regular slot spinning animation
        if (millis() - animationTimer > 50) { // Faster spinning
            for(int i = 0; i < 4; i++) {
                if (!slotLocked[i]) {
                    slotColors[i] = getRainbowColor(random(7));
                }
                LEDManager::setLED(i*2, slotColors[i]);
                LEDManager::setLED(i*2+1, slotColors[i]);
            }
            LEDManager::showLEDs();
            animationTimer = millis();
        }
    }
}

void SlotsManager::handleButtonPress() {
    if (!gameActive || showingResult) return;
    
    slotLocked[activeSlotPair] = true;
    activeSlotPair++;
    
    if (activeSlotPair >= 4) {
        checkResult();
    }
}

void SlotsManager::checkResult() {
    // Check for matches (3 or more same colors)
    int colorCounts[7] = {0}; // Count of each color
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 7; j++) {
            if(slotColors[i] == getRainbowColor(j)) {
                colorCounts[j]++;
                break;
            }
        }
    }
    
    // Check if any color appears 3 or more times
    bool won = false;
    for(int i = 0; i < 7; i++) {
        if(colorCounts[i] >= 3) {
            won = true;
            break;
        }
    }
    
    showResult(won);
}

void SlotsManager::showResult(bool won) {
    showingResult = true;
    animationTimer = millis();
    
    if (won) {
        SoundFxManager::playStartupSound();
        RoverViewManager::showNotification("SLOTS", "Winner!", "GAME", 3000);
    } else {
        SoundFxManager::playErrorSound(1);
        RoverViewManager::showNotification("SLOTS", "Try Again!", "GAME", 3000);
    }
}

CRGB SlotsManager::getRainbowColor(uint8_t index) {
    switch(index) {
        case 0: return CRGB::Red;
        case 1: return CRGB::Orange;
        case 2: return CRGB::Yellow;
        case 3: return CRGB::Green;
        case 4: return CRGB::Blue;
        case 5: return CRGB::Purple;
        default: return CRGB::White;
    }
}

void SlotsManager::reset() {
    gameActive = false;
    showingResult = false;
    FastLED.clear();
    FastLED.show();
} 