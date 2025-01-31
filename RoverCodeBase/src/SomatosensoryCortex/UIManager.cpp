#include "UIManager.h"
#include "../VisualCortex/RoverViewManager.h"
#include "../VisualCortex/RoverManager.h"
#include "../PrefrontalCortex/PowerManager.h"
#include "../VisualCortex/LEDManager.h"
#include "../AuditoryCortex/SoundFxManager.h"
#include "../PrefrontalCortex/RoverBehaviorManager.h"
#include "../SomatosensoryCortex/MenuManager.h"
#include "../MotorCortex/PinDefinitions.h"
#include "../PrefrontalCortex/utilities.h"

namespace SomatosensoryCortex 
{
    namespace SC = SomatosensoryCortex;
    using namespace CorpusCallosum;
    
    // Import specific types we need
    using PC::Utilities;
    using PC::InputTypes::InputState;
    using VC::RoverViewManager;
    using VC::RoverManager;
    using AC::SoundFxManager;
    using BehaviorState = PC::RoverTypes::BehaviorState;

    // Initialize static members
    RotaryEncoder* UIManager::encoder = nullptr;
    int UIManager::lastEncoderPosition = 0;
    bool UIManager::rotaryPressed = false;
    bool UIManager::sideButtonPressed = false;
    unsigned long UIManager::lastDebounceTime = 0;
    InputState UIManager::initState = InputState::NOT_STARTED;

    void UIManager::init() 
    {
        if (encoder != nullptr) 
        {
            delete encoder;  // Clean up if already initialized
        }
        
        initState = InputState::INITIALIZING;
        encoder = new RotaryEncoder(ENCODER_INA, ENCODER_INB, RotaryEncoder::LatchMode::TWO03);
        
        // Configure input pins with internal pullups
        pinMode(BOARD_USER_KEY, INPUT_PULLUP);
        pinMode(ENCODER_KEY, INPUT_PULLUP);
        
        // Add interrupt handlers for encoder
        attachInterrupt(digitalPinToInterrupt(ENCODER_INA), []() 
        { 
            if (encoder) encoder->tick(); 
        }, CHANGE);
        attachInterrupt(digitalPinToInterrupt(ENCODER_INB), []() 
        { 
            if (encoder) encoder->tick(); 
        }, CHANGE);
        
        lastEncoderPosition = encoder->getPosition();
        Utilities::LOG_DEBUG("UIManager initialized with interrupts");
        initState = InputState::READY;
    }

    void UIManager::update() 
    {
        if (initState != InputState::READY) 
        {
            init();
            return;
        }
        updateEncoder();
        updateSideButton();
    }

    void UIManager::updateEncoder() 
    {
        if (!encoder) return;  // Guard against null encoder
        
        int newPos = encoder->getPosition();
        if (newPos != lastEncoderPosition) 
        {
            Utilities::LOG_DEBUG("Encoder position changed: %d", newPos);
            PC::PowerManager::updateLastActivityTime();
            
            // Handle encoder movement based on context
            if (MenuManager::isVisible()) 
            {
                // In menu - navigate menu items (inverted the direction)
                MenuManager::handleRotaryTurn(newPos < lastEncoderPosition ? 1 : -1);
            } 
            else 
            {
                // On home screen - change views
                if (newPos > lastEncoderPosition) 
                {
                    RoverViewManager::nextView();
                } 
                else 
                {
                    RoverViewManager::previousView();
                }
            }
            lastEncoderPosition = newPos;
        }
        
        // Check encoder button with debouncing
        static bool lastButtonState = HIGH;
        bool currentButtonState = digitalRead(ENCODER_KEY);
        
        if (currentButtonState != lastButtonState && 
            (millis() - lastDebounceTime) > DEBOUNCE_DELAY) 
        {
            lastButtonState = currentButtonState;
            if (currentButtonState == LOW)  // Button pressed
            {
                if (!MenuManager::isVisible()) 
                {
                    MenuManager::show();
                    SoundFxManager::playMenuOpenSound();
                } 
                else 
                {
                    MenuManager::handleMenuSelect();
                    SoundFxManager::playMenuSelectSound();
                }
            }
            lastDebounceTime = millis();
        }
    }

    void UIManager::handleRotaryTurn(int direction) 
    {
        if (MenuManager::isVisible()) 
        {
            // Invert the direction for menu navigation
            MenuManager::handleRotaryTurn(-direction);
        } 
        else 
        {
            if (direction > 0) 
            {
                RoverViewManager::nextView();
            } 
            else 
            {
                RoverViewManager::previousView();
            }
        }
    }

    void UIManager::handleRotaryPress() 
    {
        static bool lastButtonState = HIGH;
        bool currentButtonState = digitalRead(ENCODER_KEY);
        
        if (currentButtonState == LOW && lastButtonState == HIGH) 
        {
            if (RoverViewManager::hasActiveNotification()) 
            {
                RoverViewManager::clearNotification();
            }
            else if (RoverManager::showTime) 
            {
                MenuManager::handleMenuSelect();
            } 
            else 
            {
                RoverManager::setShowTime(true);
            }
            PC::PowerManager::updateLastActivityTime();
        }
        lastButtonState = currentButtonState;
    }

    void UIManager::updateSideButton() 
    {
        static bool lastButtonState = HIGH;
        bool currentButtonState = digitalRead(BOARD_USER_KEY);
        
        if (currentButtonState != lastButtonState && 
            (millis() - lastDebounceTime) > DEBOUNCE_DELAY) 
        {
            lastButtonState = currentButtonState;
            if (currentButtonState == LOW)  // Button pressed
            {
                PC::PowerManager::updateLastActivityTime();
                
                if (MenuManager::isVisible()) 
                {
                    MenuManager::hide();
                    SoundFxManager::playMenuCloseSound();
                }
                RoverManager::setEarsPerked(true);
            } 
            else 
            {
                RoverManager::setEarsPerked(false);
            }
            lastDebounceTime = millis();
        }
    }
}