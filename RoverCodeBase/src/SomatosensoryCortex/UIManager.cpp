#include "UIManager.h"
#include "../VisualCortex/RoverViewManager.h"
#include "../VisualCortex/RoverManager.h"
#include "../PrefrontalCortex/PowerManager.h"
#include "../VisualCortex/LEDManager.h"
#include "../AuditoryCortex/SoundFxManager.h"
#include "../PrefrontalCortex/RoverBehaviorManager.h"
#include "../SomatosensoryCortex/MenuManager.h"
#include "../MotorCortex/PinDefinitions.h"


namespace SomatosensoryCortex 
{

    using BehaviorState = RoverBehaviorManager::BehaviorState;

    // Static member initialization
    RotaryEncoder* UIManager::encoder = nullptr;
    int UIManager::lastEncoderPosition = 0;
    bool UIManager::rotaryPressed = false;
    bool UIManager::sideButtonPressed = false;
    unsigned long UIManager::lastDebounceTime = 0;
    bool UIManager::_isInitialized = false;
    void UIManager::init() {
        if (encoder != nullptr) {
            delete encoder;  // Clean up if already initialized
        }
        
        encoder = new RotaryEncoder(ENCODER_INA, ENCODER_INB, RotaryEncoder::LatchMode::TWO03);
        
        // Configure input pins with internal pullups
        pinMode(BOARD_USER_KEY, INPUT_PULLUP);
        pinMode(ENCODER_KEY, INPUT_PULLUP);
        
        // Add interrupt handlers for encoder
        attachInterrupt(digitalPinToInterrupt(ENCODER_INA), []() { 
            if (encoder) encoder->tick(); 
        }, CHANGE);
        attachInterrupt(digitalPinToInterrupt(ENCODER_INB), []() { 
            if (encoder) encoder->tick(); 
        }, CHANGE);
        
        lastEncoderPosition = encoder->getPosition();
        Utilities::LOG_DEBUG("UIManager initialized with interrupts");
        _isInitialized = true;
    }

    void UIManager::update() {
        if (!_isInitialized) {
            UIManager::init();
        }
        updateEncoder();
        updateSideButton();
    }

    void UIManager::updateEncoder() {
        Utilities::LOG_DEBUG("Updating encoder");
        if (!encoder) return;  // Guard against null encoder
        
        int newPos = encoder->getPosition();
        if (newPos != lastEncoderPosition) {
            Utilities::LOG_DEBUG("Encoder position changed: %d", newPos);
            lastEncoderPosition = newPos;
            PowerManager::updateLastActivityTime();
            
            // Handle encoder movement based on context
            if (MenuManager::isVisible()) {
                // In menu - navigate menu items (inverted the direction)
                MenuManager::handleRotaryTurn(newPos < lastEncoderPosition ? 1 : -1);
            } else {
                // On home screen - change views
                if (newPos > lastEncoderPosition) {
                    RoverViewManager::nextView();
                } else {
                    RoverViewManager::previousView();
                }
            }
        }
        
        // Check encoder button
        static bool lastButtonState = HIGH;
        bool currentButtonState = digitalRead(ENCODER_KEY);
        
        if (currentButtonState != lastButtonState) {
            delay(50);  // Debounce
            currentButtonState = digitalRead(ENCODER_KEY);
            if (currentButtonState != lastButtonState) {
                lastButtonState = currentButtonState;
                if (currentButtonState == LOW) {  // Button pressed
                    if (!MenuManager::isVisible()) {
                        MenuManager::show();
                        SoundFxManager::playMenuOpenSound();
                    } else {
                        MenuManager::handleMenuSelect();
                        SoundFxManager::playMenuSelectSound();
                    }
                }
            }
        }
    }

    void UIManager::handleRotaryTurn(int direction) {
        if (MenuManager::isVisible()) {
            // Invert the direction for menu navigation
            MenuManager::handleRotaryTurn(-direction);
        } else {
            if (direction > 0) {
                RoverViewManager::nextView();
            } else {
                RoverViewManager::previousView();
            }
        }
    }

    void UIManager::handleRotaryPress() {
        Utilities::LOG_DEBUG("Rotary press");
        static bool lastButtonState = HIGH;
        bool currentButtonState = digitalRead(ENCODER_KEY);
        
        if (currentButtonState == LOW && lastButtonState == HIGH) 
        {
            if (RoverViewManager::hasActiveNotification()) 
            {
                RoverViewManager::clearNotification();
            }
            else if (RoverManager::showTime) { // Check if time is currently displayed
                MenuManager::handleMenuSelect(); // Open the menu if time is visible
            } 
            else 
            {
                RoverManager::setShowTime(true);
            }
            PowerManager::updateLastActivityTime();
        }
        
        lastButtonState = currentButtonState;
    }

    void UIManager::updateSideButton() {
        Utilities::LOG_DEBUG("Updating side button");
        static bool lastButtonState = HIGH;
        bool currentButtonState = digitalRead(BOARD_USER_KEY);
        
        if (currentButtonState != lastButtonState) {
            delay(50);  // Simple debounce
            currentButtonState = digitalRead(BOARD_USER_KEY);
            if (currentButtonState != lastButtonState) {
                lastButtonState = currentButtonState;
                if (currentButtonState == LOW) {  // Button pressed
                    Utilities::LOG_DEBUG("Side button pressed");
                    PowerManager::updateLastActivityTime();
                    
                    // If menu is visible, hide it and return to home
                    if (MenuManager::isVisible()) {
                        MenuManager::hide();
                        SoundFxManager::playMenuCloseSound();
                    }
                    RoverManager::setEarsPerked(true);  // Perk ears on side button press
                } else {
                    RoverManager::setEarsPerked(false);  // Un-perk ears when released
                }
            }
        }
    }
}