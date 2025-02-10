/**
 * @brief Visual perception system for LED pattern generation and control
 * 
 * Manages the LED-based visual feedback system:
 * - Pattern generation and perception
 * - Emotional state visualization
 * - Cross-modal sensory integration
 * - Visual feedback coordination
 * - Boot sequence visualization
 */

#pragma once
#include <FastLED.h>
#include "../PrefrontalCortex/Utilities.h"
#include "../MotorCortex/PinDefinitions.h"
#include "../GameCortex/SlotsManager.h"
#include "../PsychicCortex/IRManager.h"
#include "../CorpusCallosum/SynapticPathways.h"

using namespace MotorCortex;
using namespace CorpusCallosum;
using namespace PrefrontalCortex::VisualTypes;  // Import all visual types

namespace VisualCortex 
{
    namespace VC = VisualCortex;
    
    // Replace individual using declarations with namespace import
    using namespace PrefrontalCortex::VisualTypes;

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
        /**
         * @brief Initialize visual processing pathways
         * Sets up LED hardware and initial perceptual state
         */
        static void init();

        /**
         * @brief Process ongoing visual patterns
         * Updates LED states based on current cognitive state
         */
        static void updateLEDs();

        /**
         * @brief Initiate loading animation sequence
         * Begins temporal pattern generation for loading feedback
         */
        static void startLoadingAnimation();

        /**
         * @brief Terminate loading animation sequence
         * Ends temporal pattern generation for loading feedback
         */
        static void stopLoadingAnimation();

        // Core perception methods
        static void update();
        static void showLEDs();

        // Mode management
        static void setMode(VisualMode mode);
        static void nextMode();
        static VisualMode getMode() { return currentMode; }
        static void setPattern(VisualPattern pattern);
        static VisualPattern getPattern() { return currentPattern; }

        // Animation control
        static void updateLoadingAnimation();
        static bool isLoadingComplete();

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
        /**
         * @brief Generate card-specific visual pattern
         * @param uid Card identifier bytes
         * @param uidLength Length of identifier
         */
        static void displayCardPattern(const uint8_t* uid, uint8_t uidLength);
        /**
         * @brief Visualize musical frequency
         * @param frequency Note frequency in Hz
         * @param position LED position for visualization
         */
        static void displayNote(uint16_t frequency, uint8_t position = 0);
        static void clearNoteDisplay();

        // State management
        static void handleMessage(VisualMessage message);
        static void setFestiveTheme(FestiveTheme theme);
        static void checkAndSetFestiveMode();
        static void setEncodingMode(EncodingModes mode);

        // Public state tracking
        static VisualMode currentMode;
        static FestiveTheme currentTheme;
        static EncodingModes currentEncodingMode;

        // Add new methods
        static void displayChromatic(const PrefrontalCortex::ColorPerceptionTypes::ChromaticContext& context);
        static void displayEmotional(const PrefrontalCortex::ColorPerceptionTypes::EmotionalColor& emotion);
        static void setIntensity(const PrefrontalCortex::ColorPerceptionTypes::ColorIntensity& intensity);

        static bool isInitialized() {
            return initialized;
        }

    private:
        // LED Arrays
        static CRGB leds[MC::PinDefinitions::VisualPathways::WS2812_NUM_LEDS];
        static CRGB previousColors[MC::PinDefinitions::VisualPathways::WS2812_NUM_LEDS];
        static NoteState currentNotes[MC::PinDefinitions::VisualPathways::WS2812_NUM_LEDS];
        static VisualPattern currentPattern;

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

        static bool initialized;
    }; 
}