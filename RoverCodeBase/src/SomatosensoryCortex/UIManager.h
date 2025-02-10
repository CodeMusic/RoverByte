#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <RotaryEncoder.h>
#include <Arduino.h>
#include "../PrefrontalCortex/ProtoPerceptions.h"
#include "../CorpusCallosum/SynapticPathways.h"

namespace SomatosensoryCortex 
{
    using namespace CorpusCallosum;
    using PC::InputTypes::InputState;  // Add this to ProtoPerceptions

    /**
     * @brief Manages user interface interactions and input processing
     * 
     * Provides:
     * - Rotary encoder input processing
     * - Button state management
     * - Input debouncing and validation
     * - Multi-modal feedback coordination
     * - State transition handling
     * - User interaction patterns
     * 
     * The UIManager serves as the primary sensory interface for user input,
     * coordinating:
     * - Physical input detection
     * - Input state validation
     * - Feedback generation
     * - Menu system integration
     * - Application state updates
     */
    class UIManager {
    public:
        static void init();
        static void update();
        
        // Input states
        /**
        * @brief Check if the rotary encoder is pressed
        * @return True if the rotary encoder is pressed
        */
        static bool isRotaryPressed() { return rotaryPressed; }
        /**
        * @brief Check if the side button is pressed
        * @return True if the side button is pressed
        */
        static bool isSideButtonPressed() { return sideButtonPressed; }
        /**
        * @brief Get the encoder position
        * @return The position of the encoder
        */
        static int getEncoderPosition() { return lastEncoderPosition; }
        /**
        * @brief Check for input device availability
        * @return True if input systems are operational
        */
        static bool isInitialized() { return initState == InputState::READY; }
        /**
        * @brief Reset the state
        */
        static void resetState();
        
    private:
        static RotaryEncoder* encoder;
        static int lastEncoderPosition;
        static bool rotaryPressed;
        static bool sideButtonPressed;
        static InputState initState;
        static unsigned long lastDebounceTime;
        static const unsigned long DEBOUNCE_DELAY = 50;
        /**
        * @brief Handle the rotary encoder turn
        * @param direction The direction of the turn
        */
        static void handleRotaryTurn(int direction);
        /**
        * @brief Handle the rotary encoder press
        */
        static void handleRotaryPress();
        /**
        * @brief Update the encoder
        */
        static void updateEncoder();
        /**
        * @brief Update the side button
        */
        static void updateSideButton();
    };

}

#endif