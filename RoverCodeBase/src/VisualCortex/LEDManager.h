#pragma once
#include <FastLED.h>
#include "../PrefrontalCortex/utilities.h"
#include "../MotorCortex/PinDefinitions.h"
#include "../GameCortex/SlotsManager.h"
#include "../PsychicCortex/IRManager.h"
using namespace MotorCortex;
using namespace CorpusCallosum;

namespace VisualCortex 
{
    static const int LED_NUM_MODES = 5;
    static const int NUM_RAINBOW_COLORS = 7;
    static const int STEP_DELAY = 100;
    static const int MONTH_DIM = 128;

    enum class Mode {
        OFF_MODE,
        ENCODING_MODE,      // Main mode for encoding
        FESTIVE_MODE,
        ROVER_EMOTION_MODE
    };

    enum class EncodingModes {
        FULL_MODE,
        WEEK_MODE,
        TIMER_MODE,
        CUSTOM_MODE,
        MENU_MODE
    };

    enum class FestiveTheme {
        NEW_YEAR,        // January 1
        VALENTINES,      // February 14
        ST_PATRICK,      // March 17
        EASTER,          // March/April (variable)
        CANADA_DAY,      // July 1
        HALLOWEEN,       // October 31
        CHRISTMAS,       // December 25
        THANKSGIVING,    // Fourth Thursday in November (USA)
        INDEPENDENCE_DAY,// July 4 (USA)
        DIWALI,          // Date varies (Hindu festival of lights)
        RAMADAN,         // Date varies (Islamic holy month)
        CHINESE_NEW_YEAR,// Date varies (Lunar New Year)
        MARDI_GRAS,      // Date varies (Fat Tuesday)
        LABOR_DAY,       // First Monday in September (USA)
        MEMORIAL_DAY,    // Last Monday in May (USA)
        FLAG_DAY         // June 14 (USA)
    };

    enum class Pattern {
        NONE,
        RAINBOW,
        SOLID,
        PULSE,
        CHASE,
        SLOTS_GAME,
        IR_BLAST,
        NFC_SCAN,
        TIMER,
        MENU,
        CUSTOM
    };

    enum class LEDMessage {
        NONE,
        SLOTS_WIN,
        IR_SUCCESS,
        NFC_DETECTED,
        NFC_ERROR
    };

    struct NoteState {
        CRGB color1;
        CRGB color2;  // For sharps/flats
        bool isSharp;
        uint8_t position;
        
        NoteState() : color1(CRGB::Black), color2(CRGB::Black), isSharp(false), position(0) {}
    };

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