#pragma once
#include <FastLED.h>
#include "../PrefrontalCortex/utilities.h"
#include "../MotorCortex/PinDefinitions.h"
#include "../GameCortex/SlotsManager.h"
#include "../PsychicCortex/IRManager.h"
#include "../CorpusCallosum/SynapticPathways.h"

using namespace MotorCortex;
using namespace CorpusCallosum;

namespace VisualCortex 
{
    namespace VC = VisualCortex;  // Add namespace alias
    using PC::VisualTypes::VisualPattern;  // Use ProtoPerceptions type
    using PC::VisualTypes::VisualMode;     // Use ProtoPerceptions type
    using PC::VisualTypes::VisualMessage;  // Use ProtoPerceptions type
    using PC::VisualTypes::FestiveTheme;   // Use ProtoPerceptions type
    using PC::VisualTypes::NoteState;      // Use ProtoPerceptions type
    using PC::VisualTypes::EncodingModes;  // Use ProtoPerceptions type

    // Visual perception constants
    namespace VisualConstants
    {
        constexpr int LED_NUM_MODES = 5;
        constexpr int NUM_RAINBOW_COLORS = 7;
        constexpr int STEP_DELAY = 100;
        constexpr int MONTH_DIM = 128;
    }

    class LEDManager 
    {
    public:
        // Core perception methods
        static void init();
        static void update();
        static void showLEDs();

        // Mode management
        static void setMode(VisualMode mode);
        static void nextMode();
        static VisualMode getMode() { return currentMode; }
        static void setPattern(PC::VisualPattern pattern);
        static PC::VisualTypes::VisualPattern getPattern() { return currentPattern; }

        // Animation control
        static void startLoadingAnimation();
        static void stopLoadingAnimation();
        static void updateLoadingAnimation();
        static bool isLoadingComplete();
        static void updateLEDs();

        // Visual feedback
        static void flashSuccess();
        static void flashLevelUp();
        static void setErrorLED(bool state);
        static void setErrorPattern(uint32_t errorCode, bool isFatal);
        static void clearErrorPattern();
        static void updateErrorPattern();

        // LED manipulation
        static void setLED(int index, CRGB color);
        static void scaleLED(int index, uint8_t scale);
        static void syncLEDsForDay();

        // Special effects
        static void runInitializationTest();
        static void displayCardPattern(const uint8_t* uid, uint8_t uidLength);
        static void displayNote(uint16_t frequency, uint8_t position = 0);
        static void clearNoteDisplay();

        // State management
        static void handleMessage(PC::VisualMessage message);
        static void setFestiveTheme(FestiveTheme theme);
        static void checkAndSetFestiveMode();
        static void setEncodingMode(EncodingModes mode);

        // Public state tracking
        static VisualMode currentMode;
        static FestiveTheme currentTheme;
        static EncodingModes currentEncodingMode;

    private:
        // LED state management
        static CRGB leds[PinDefinitions::WS2812_NUM_LEDS];
        static CRGB previousColors[PinDefinitions::WS2812_NUM_LEDS];
        static NoteState currentNotes[PinDefinitions::WS2812_NUM_LEDS];
        static PC::VisualTypes::VisualPattern currentPattern;

        // Mode tracking
        static VisualMode previousMode;
        static CRGB winningColor;
        static CRGB targetColor;
        static bool transitioningColor;

        // Animation state
        static uint8_t currentPosition;
        static uint8_t currentColorIndex;
        static uint8_t completedCycles;
        static uint8_t filledPositions;
        static uint8_t activeTrails;
        static uint8_t animationStep;
        static uint8_t fadeValue;
        static uint8_t currentFadeIndex;
        static bool fadeDirection;
        static bool isLoading;
        static bool tickTock;
        static bool readyForMelody;

        // Timing
        static unsigned long lastStepTime;
        static unsigned long lastUpdate;

        // Mode update methods
        static void updateFullMode();
        static void updateWeekMode();
        static void updateTimerMode();
        static void updateFestiveMode();
        static void updateCustomMode();
        static void updateMenuMode();
        static void updateRoverEmotionMode();
        static void updateIRBlastPattern();
        static void updateSlotsPattern();
        static void updateNFCScanPattern();

        // Utility methods
        static CRGB getRainbowColor(uint8_t index);
        static CRGB getNoteColor(uint16_t frequency);
        static bool isSharpNote(uint16_t frequency);

        // Boot sequence colors
        static const CRGB HARDWARE_INIT_COLOR;
        static const CRGB SYSTEM_START_COLOR;
        static const CRGB NETWORK_PREP_COLOR;
        static const CRGB FINAL_PREP_COLOR;
        static const uint8_t fadeSequence[];

        // Constants
        static constexpr uint8_t ERROR_LED_INDEX = 0;
        static constexpr uint8_t ERROR_LED_COUNT = 8;
        static constexpr uint8_t LEDS_PER_STEP = 3;
        static uint8_t loadingPosition;
    }; 
}