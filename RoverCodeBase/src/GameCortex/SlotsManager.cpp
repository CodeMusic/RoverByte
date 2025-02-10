/**
 * @brief Implementation of the SlotsManager pattern recognition system
 * 
 * Handles the implementation of:
 * - Visual pattern sequencing
 * - Reward feedback loops
 * - Cross-modal sensory integration
 * - Temporal pattern processing
 */

#include "SlotsManager.h"
#include "../CorpusCallosum/SynapticPathways.h"
#include "../VisualCortex/LEDManager.h"
#include "../VisualCortex/RoverViewManager.h"
#include "../AuditoryCortex/SoundFxManager.h"
#include "../SomatosensoryCortex/MenuManager.h"

namespace GameCortex
{
    using namespace CorpusCallosum;
    using PC::GameTypes::GameState;
    using PC::GameTypes::SlotSymbol;
    using PC::GameTypes::GameScore;
    using VC::LEDManager;
    using VC::RoverViewManager;
    using AC::SoundFxManager;
    using SC::MenuManager;

    // Initialize neural state variables
    uint8_t SlotsManager::activeSlotPair = 0;
    bool SlotsManager::slotLocked[4] = {false};
    CRGB SlotsManager::slotColors[4];
    unsigned long SlotsManager::animationTimer = 0;
    bool SlotsManager::showingResult = false;
    bool SlotsManager::gameActive = false;
    unsigned long SlotsManager::lockTimer = 0;

    void SlotsManager::init() {
        gameActive = true;
        // Reset game state
        activeSlotPair = 0;
        showingResult = false;
        animationTimer = millis();
        for (int i = 0; i < 4; i++) {
            slotLocked[i] = false;
            slotColors[i] = CRGB::Black;
        }
    }

    void SlotsManager::handleRotaryTurn(int direction) {
        update();
    }

    void SlotsManager::handleRotaryPress() {
        spin();
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
                
                // Check if it's time to lock the next reel
                if (!slotLocked[activeSlotPair] && millis() >= lockTimer) {
                    slotLocked[activeSlotPair] = true;
                    SoundFxManager::playTone(1000 + (activeSlotPair * 200), 100);
                    
                    activeSlotPair++;
                    if (activeSlotPair < 4) {
                        // Set next lock timer
                        lockTimer = millis() + random(200, 1000);
                    } else {
                        checkResult();
                    }
                }
            }
        }
    }

    void SlotsManager::spin() {
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
            SoundFxManager::playErrorSound(PC::AudioTypes::ErrorSoundType::PLAYBACK);
            RoverViewManager::showNotification("SLOTS", "Try Again!", "GAME", 3000);
        }
    }

    CRGB SlotsManager::getRainbowColor(uint8_t index) {
        // Use VisualSynesthesia's color perception system
        return PC::ColorPerceptionTypes::BASE_8_COLORS[index];
    }

    void SlotsManager::reset() {
        gameActive = false;
        showingResult = false;
        FastLED.clear();
        FastLED.show();
    }

    void SlotsManager::startGame() {
        init();
        lockTimer = millis() + 1000; // Initial delay before first lock
    }
}