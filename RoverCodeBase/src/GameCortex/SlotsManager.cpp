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
    
    // Initialize slots
    for(int i = 0; i < 4; i++) {
        slotLocked[i] = false;
        slotColors[i] = getRainbowColor(random(7));
        LEDManager::setLED(i*2, slotColors[i]);
        LEDManager::setLED(i*2+1, slotColors[i]);
    }
    LEDManager::showLEDs();
    
    animationTimer = millis();
    MenuManager::hide(); // Hide menu after initialization
}

void SlotsManager::update() {
    if (!gameActive) return;
    
    if (showingResult) {
        // Flash winning slots
        if (millis() - animationTimer > 250) {
            static bool flashState = false;
            FastLED.clear();
            
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
        
        // End game after showing result
        if (millis() - animationTimer > 3000) {
            gameActive = false;
            MenuManager::show();
        }
    } else {
        // Regular slot spinning animation
        if (millis() - animationTimer > 50) {
            FastLED.clear();
            
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
    
    // Lock current slot pair
    slotLocked[activeSlotPair] = true;
    SoundFxManager::playTone(1000 + (activeSlotPair * 200), 100);
    
    // Move to next slot pair
    activeSlotPair++;
    
    // Check if all slots are locked
    if (activeSlotPair >= 4) {
        checkResult();
    }
}

void SlotsManager::checkResult() {
    // Check if any adjacent pairs match
    bool hasMatch = false;
    for(int i = 0; i < 3; i++) {
        if (slotColors[i] == slotColors[i+1]) {
            hasMatch = true;
            break;
        }
    }
    showResult(hasMatch);
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

void SlotsManager::startGame() {
    gameActive = true;
    showingResult = false;
    activeSlotPair = 0;
    
    // Initialize slots
    for(int i = 0; i < 4; i++) {
        slotLocked[i] = false;
        slotColors[i] = getRainbowColor(random(7));
        LEDManager::setLED(i*2, slotColors[i]);
        LEDManager::setLED(i*2+1, slotColors[i]);
    }
    LEDManager::showLEDs();
    
    animationTimer = millis();
    MenuManager::hide();
    RoverViewManager::showNotification("SLOTS", "Press to lock reels!", "GAME", 2000);
} 