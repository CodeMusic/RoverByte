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
        static void init();
        static void update();
        static void spin();
        static void reset();
        
        static uint8_t activeSlotPair;
        static bool slotLocked[4];
        static CRGB slotColors[4];
        static unsigned long animationTimer;
        static bool showingResult;
        static bool gameActive;
        static void checkResult();
        static void showResult(bool won);
        static CRGB getRainbowColor(uint8_t index);
        static unsigned long lockTimer;
        static bool isGameActive() { return gameActive; }
        static void startGame();
        static void handleRotaryTurn(int direction);
        static void handleRotaryPress();
    }; 
}