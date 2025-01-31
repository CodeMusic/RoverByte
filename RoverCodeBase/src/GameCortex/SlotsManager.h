/**
 * @brief SlotsManager handles pattern recognition and reward-based engagement
 * 
 * This class manages the slots mini-game which provides:
 * - Visual pattern recognition training through color matching
 * - Reward-based learning through win/loss feedback
 * - Cross-modal sensory integration (visual, auditory, tactile)
 * - Temporal pattern sequencing through slot timing
 */

#pragma once
#include <FastLED.h>
#include "../VisualCortex/LEDManager.h"
#include "../VisualCortex/RoverManager.h"
#include "../MotorCortex/PinDefinitions.h"
#include "../PrefrontalCortex/ProtoPerceptions.h"

namespace GameCortex
{
    using PC::GameTypes::GameState;
    using PC::GameTypes::SlotSymbol;
    using PC::GameTypes::GameScore;

    class SlotsManager {
    public:
        /**
         * @brief Initializes the pattern recognition system
         */
        static void init();

        /**
         * @brief Updates the pattern recognition state
         */
        static void update();

        /**
         * @brief Initiates pattern spinning sequence
         */
        static void spin();

        /**
         * @brief Resets the pattern recognition system
         */
        static void reset();

        /**
         * @brief Begins a new pattern recognition session
         */
        static void startGame();

        /**
         * @brief Processes rotary input for pattern selection
         * @param direction Cognitive input direction (-1 or 1)
         */
        static void handleRotaryTurn(int direction);

        /**
         * @brief Processes selection confirmation
         */
        static void handleRotaryPress();

        /**
         * @brief Checks if pattern recognition game is active
         * @return True if game is in progress
         */
        static bool isGameActive() { return gameActive; }

        /**
         * @brief Evaluates pattern match results
         */
        static void checkResult();

        /**
         * @brief Displays pattern recognition outcome
         * @param won True if pattern successfully matched
         */
        static void showResult(bool won);

        /**
         * @brief Gets chromatic perception for slot display
         * @param index Cognitive color index
         * @return CRGB color value
         */
        static CRGB getRainbowColor(uint8_t index);

        // Neural state variables
        static uint8_t activeSlotPair;
        static bool slotLocked[4];
        static CRGB slotColors[4];
        static unsigned long animationTimer;
        static bool showingResult;
        static bool gameActive;
        static unsigned long lockTimer;
    }; 
}