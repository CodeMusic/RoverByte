#pragma once
#include <FastLED.h>
#include "../PrefrontalCortex/utilities.h"
#include "../MotorCortex/PinDefinitions.h"
#include "../GameCortex/SlotsManager.h"
#include "../PsychicCortex/IRManager.h"

static const int LED_NUM_MODES = 5;
static const int NUM_RAINBOW_COLORS = 7;
static const int STEP_DELAY = 100;
static const int MONTH_DIM = 128;

enum class Mode {
    FULL_MODE,
    WEEK_MODE,
    TIMER_MODE,
    OFF_MODE,
    FESTIVE_MODE
};

enum class FestiveTheme {
    NEW_YEAR,        // January 1
    VALENTINES,      // February 14
    ST_PATRICK,      // March 17
    EASTER,          // March/April (variable)
    CANADA_DAY,      // July 1
    HALLOWEEN,       // October 31
    CHRISTMAS        // December 25
};

enum class Pattern {
    NONE,           // No app pattern active, use selected Mode
    SLOTS_GAME,     // Slots app is controlling LEDs
    IR_BLAST,       // IR Blaster is controlling LEDs
    NFC_SCAN        // NFC scanning pattern
};

enum class LEDMessage {
    NONE,
    SLOTS_WIN,
    SLOTS_LOSE,
    IR_SUCCESS,
    IR_FAIL,
    NFC_DETECTED,
    NFC_ERROR
};

struct NoteState {
    CRGB color1;
    CRGB color2;  // For sharps/flats
    bool isSharp;
    uint8_t position;
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
    static void displayCardPattern(uint8_t* uid, uint8_t length);
    static void update();
    static void setPattern(Pattern pattern);
    static Pattern getPattern() { return currentPattern; }
    static void handleMessage(LEDMessage msg, CRGB color = CRGB::Black);
    static void displayNote(uint16_t frequency, uint8_t position = 0);
    static void clearNoteDisplay();
    static void setErrorLED(bool state);
    static void setErrorPattern(uint32_t errorCode, bool isFatal);
    static void clearErrorPattern();
    static void updateErrorPattern();

private:
    static CRGB leds[WS2812_NUM_LEDS];
    static Mode previousMode;
    static uint8_t currentPosition;
    static uint8_t currentColorIndex;
    static uint8_t completedCycles;
    static uint8_t filledPositions;
    static uint8_t activeTrails;
    static unsigned long lastStepTime;
    static bool isLoading;
    
    // New animation variables
    static uint8_t animationStep;
    static uint8_t fadeValue;
    static bool fadeDirection;
    static CRGB previousColors[WS2812_NUM_LEDS];
    
    static void updateFullMode();
    static void updateWeekMode();
    static void updateTimerMode();
    static void updateFestiveMode();
    static CRGB getRainbowColor(uint8_t index);
    static Pattern currentPattern;
    static void updateIRBlastPattern();
    static void updateSlotsPattern();
    static void updateNFCScanPattern();
    static NoteState currentNotes[WS2812_NUM_LEDS];
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