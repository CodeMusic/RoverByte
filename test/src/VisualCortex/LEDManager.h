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
    static const int LED_NUM_MODES = 5;
    static const int NUM_RAINBOW_COLORS = 7;
    static const int STEP_DELAY = 100;
    static const int MONTH_DIM = 128;

    

    class LEDManager {
    public:
        static void init();
        static void updateLEDs();
        static void startLoadingAnimation();
        static void stopLoadingAnimation();
        static void nextMode();
        static void setMode(Mode mode);
        static void setFestiveTheme(FestiveTheme theme);
        static void setLED(int index, CRGB color);
        static void syncLEDsForDay();
        static void showLEDs();
        static void scaleLED(int index, uint8_t scale);
        static Mode getMode() { return currentMode; }
        static void updateLoadingAnimation();
        static bool isLoadingComplete();
        static void flashSuccess();
        static void flashLevelUp();
        static Mode currentMode;
        static FestiveTheme currentTheme;
        static void displayCardPattern(const uint8_t* uid, uint8_t uidLength);
        static void update();
        static void setPattern(Pattern pattern);
        static Pattern getPattern() { return currentPattern; }
        static void handleMessage(LEDMessage message);
        static void displayNote(uint16_t frequency, uint8_t position = 0);
        static void clearNoteDisplay();
        static void setErrorLED(bool state);
        static void setErrorPattern(uint32_t errorCode, bool isFatal);
        static void clearErrorPattern();
        static void updateErrorPattern();
        static void checkAndSetFestiveMode();
        static void setEncodingMode(EncodingModes mode);
        static EncodingModes currentEncodingMode;
        

    private:
        static CRGB leds[PinDefinitions::WS2812_NUM_LEDS];
        static Mode previousMode;
        static uint8_t currentPosition;
        static uint8_t currentColorIndex;
        static uint8_t completedCycles;
        static uint8_t filledPositions;
        static uint8_t activeTrails;
        static unsigned long lastStepTime;
        static bool isLoading;
        static bool tickTock;
        
        // New animation variables
        static uint8_t animationStep;
        static uint8_t fadeValue;
        static bool fadeDirection;
        static CRGB previousColors[PinDefinitions::WS2812_NUM_LEDS];
        
        static void updateFullMode();
        static void updateWeekMode();
        static void updateTimerMode();
        static void updateFestiveMode();
        static void updateCustomMode();
        static void updateMenuMode();
        static void updateRoverEmotionMode();

        static CRGB getRainbowColor(uint8_t index);
        static Pattern currentPattern;
        static void updateIRBlastPattern();
        static void updateSlotsPattern();
        static void updateNFCScanPattern();
        static NoteState currentNotes[PinDefinitions::WS2812_NUM_LEDS];
        static CRGB getNoteColor(uint16_t frequency);
        static bool isSharpNote(uint16_t frequency);
        static CRGB winningColor;
        static bool transitioningColor;
        static uint8_t currentFadeIndex;
        static unsigned long lastUpdate;
        static CRGB targetColor;
        static const uint8_t fadeSequence[];
        static bool readyForMelody;
        static constexpr uint8_t ERROR_LED_INDEX = 0;
        static constexpr uint8_t ERROR_LED_COUNT = 8;

        // Boot stage colors
        static const CRGB HARDWARE_INIT_COLOR;
        static const CRGB SYSTEM_START_COLOR;
        static const CRGB NETWORK_PREP_COLOR;
        static const CRGB FINAL_PREP_COLOR;

        static uint8_t loadingPosition;
        static const uint8_t LEDS_PER_STEP = 3;


    }; 
}