#ifndef IR_MANAGER_H
#define IR_MANAGER_H

#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include "../CorpusCallosum/SynapticPathways.h"  // Replace individual includes with central nervous system
#include "../MotorCortex/PinDefinitions.h"

namespace PsychicCortex 
{
    namespace PSY = PsychicCortex;  // Add namespace alias
    using namespace CorpusCallosum;  // Access to all type systems

    class IRManager 
    {
    public:
        // Core perception methods
        static void init();
        static void update();

        // IR blast control methods
        static void startBlast();
        static void stopBlast();
        static bool isBlasting() { return blasting; }

        // Input handling methods
        static void handleRotaryTurn(int direction);
        static void handleRotaryPress();
        
    private:
        // Timing constants for psychic perception
        static const uint16_t SEND_DELAY_MS = 100;
        static const uint16_t LED_UPDATE_MS = 50;

        // State tracking variables
        static unsigned long lastSendTime;
        static unsigned long lastLEDUpdate;
        static uint16_t currentCode;
        static uint8_t currentRegion;
        static uint8_t currentLEDPosition;
        static bool animationDirection;
        static IRsend irsend;
        static bool blasting;

        // Internal methods
        static void sendCode(uint16_t code);
        static void setupIROutput();
        static void updateLEDAnimation();
    };
}

#endif