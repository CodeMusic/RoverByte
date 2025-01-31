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

    class UIManager {
    public:
        static void init();
        static void update();
        
        // Input states
        static bool isRotaryPressed() { return rotaryPressed; }
        static bool isSideButtonPressed() { return sideButtonPressed; }
        static int getEncoderPosition() { return lastEncoderPosition; }
        static bool isInitialized() { return initState == InputState::READY; }
    private:
        static RotaryEncoder* encoder;
        static int lastEncoderPosition;
        static bool rotaryPressed;
        static bool sideButtonPressed;
        static InputState initState;
        static unsigned long lastDebounceTime;
        static const unsigned long DEBOUNCE_DELAY = 50;
        
        static void handleRotaryTurn(int direction);
        static void handleRotaryPress();
        static void updateEncoder();
        static void updateSideButton();
    };

}

#endif