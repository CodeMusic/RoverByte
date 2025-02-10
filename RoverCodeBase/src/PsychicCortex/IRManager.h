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

    /**
     * @brief Manages IR communication and device control
     * 
     * Provides:
     * - IR signal transmission
     * - Device control sequences
     * - Visual feedback during transmission
     * - Interactive control via rotary input
     * - Timed transmission patterns
     */
    class IRManager 
    {
    public:
        /**
         * @brief Initialize IR hardware and pins
         */
        static void init();

        /**
         * @brief Process IR transmission state and LED animations
         */
        static void update();

        /**
         * @brief Begin IR code transmission sequence
         */
        static void startBlast();

        /**
         * @brief Stop IR code transmission
         */
        static void stopBlast();

        /**
         * @brief Check if currently transmitting
         */
        static bool isBlasting() { return blasting; }

        /**
         * @brief Handle rotary encoder rotation events
         * @param direction Positive or negative rotation value
         */
        static void handleRotaryTurn(int direction);

        /**
         * @brief Handle rotary encoder button press
         */
        static void handleRotaryPress();
        
    private:
        // Timing constants
        static const uint16_t SEND_DELAY_MS = 100;   // Delay between code transmissions
        static const uint16_t LED_UPDATE_MS = 50;    // LED animation update interval

        // State tracking
        static bool blasting;                // Currently transmitting
        static unsigned long lastSendTime;   // Last transmission timestamp
        static unsigned long lastLEDUpdate;  // Last LED update timestamp
        static uint16_t currentCode;         // Current IR code
        static uint8_t currentRegion;        // Current LED region
        static uint8_t currentLEDPosition;   // Current LED animation position
        static bool animationDirection;      // LED animation direction
        static IRsend irsend;               // IR transmitter object

        // Internal methods
        static void sendCode(uint16_t code);
        static void setupIROutput();
        static void updateLEDAnimation();
    };
}

#endif