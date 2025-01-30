#ifndef IR_MANAGER_H
#define IR_MANAGER_H

#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <string>
#include "../MotorCortex/PinDefinitions.h"

// Forward declarations
namespace PrefrontalCortex { class Utilities; }
namespace VisualCortex { class RoverViewManager; }

namespace PsychicCortex 
{

    class IRManager {
    public:
        static void init();
        static void startBlast();
        static void stopBlast();
        static void update();
        static bool isBlasting() { return blasting; }
        static void handleRotaryTurn(int direction);
        static void handleRotaryPress();
        
    private:
        static unsigned long lastSendTime;
        static unsigned long lastLEDUpdate;
        static uint16_t currentCode;
        static uint8_t currentRegion;
        static uint8_t currentLEDPosition;
        static bool animationDirection;
        static const uint16_t SEND_DELAY_MS = 100;
        static const uint16_t LED_UPDATE_MS = 50;
        static IRsend irsend;
        
        static void sendCode(uint16_t code);
        static void setupIROutput();
        static void updateLEDAnimation();
        static bool blasting;
        
    };

}

#endif