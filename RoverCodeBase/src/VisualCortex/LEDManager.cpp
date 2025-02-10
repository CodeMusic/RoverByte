/**
 * @brief Visual perception system implementation
 * 
 * Implements core visual processing functions:
 * - Pattern generation algorithms
 * - Temporal sequence processing
 * - Cross-modal sensory integration
 * - Emotional state visualization
 * - Boot sequence coordination
 */

#include "../PrefrontalCortex/ProtoPerceptions.h"
#include "../CorpusCallosum/SynapticPathways.h"
#include "../PrefrontalCortex/Utilities.h"
#include "../MotorCortex/PinDefinitions.h"
#include "../PrefrontalCortex/PowerManager.h"  
#include "LEDManager.h"
#include "VisualSynesthesia.h"
#include "../AuditoryCortex/SoundFxManager.h"
#include "../GameCortex/AppManager.h"
#include "../AuditoryCortex/PitchPerception.h"
#include "../PsychicCortex/NFCManager.h"
#include "../PrefrontalCortex/RoverBehaviorManager.h"
#include "../GameCortex/SlotsManager.h"
#include "../PsychicCortex/IRManager.h"
#include "../SomatosensoryCortex/MenuManager.h"
#include "../VisualCortex/RoverViewManager.h"  
#include "../VisualCortex/FastLEDConfig.h"

namespace VisualCortex 
{
    // Namespace aliases for better readability
    namespace VC = VisualCortex;
    using namespace CorpusCallosum;
    
    // Cortex-specific namespace aliases
    namespace PC = PrefrontalCortex;
    namespace AC = AuditoryCortex;
    namespace GC = GameCortex;
    namespace PSY = PsychicCortex;
    namespace SC = SomatosensoryCortex;
    
    // Manager-specific using declarations
    using AC::SoundFxManager;
    using PSY::NFCManager;
    using SC::MenuManager;
    using PM = PC::PowerManager;
    
    // Visual types from PrefrontalCortex
    using PC::VisualTypes::VisualPattern;
    using PC::VisualTypes::VisualMode;
    using PC::VisualTypes::VisualMessage;
    using PC::VisualTypes::FestiveTheme;
    using PC::VisualTypes::NoteState;
    using PC::VisualTypes::EncodingModes;
    
    // Other PrefrontalCortex types
    using PC::Utilities;
    using PC::AudioTypes::Tone;

    // Visual constants from FastLEDConfig
    using namespace VC::FastLEDConfig;
    using VC::AnimationTiming;
    using VC::PatternConfig;
    using VC::BootConfig;

    // Boot sequence color constants
    const CRGB LEDManager::HARDWARE_INIT_COLOR = CRGB::Blue;
    const CRGB LEDManager::SYSTEM_START_COLOR = CRGB::Green;
    const CRGB LEDManager::NETWORK_PREP_COLOR = CRGB::Purple;
    const CRGB LEDManager::FINAL_PREP_COLOR = CRGB::Gold;

    // Static member initialization - LED Arrays
    CRGB LEDManager::leds[MC::PinDefinitions::VisualPathways::WS2812_NUM_LEDS];
    CRGB LEDManager::previousColors[MC::PinDefinitions::VisualPathways::WS2812_NUM_LEDS];
    NoteState LEDManager::currentNotes[MC::PinDefinitions::VisualPathways::WS2812_NUM_LEDS];

    // Visual state tracking
    PC::VisualTypes::VisualPattern LEDManager::currentPattern = PC::VisualTypes::VisualPattern::NONE;
    VisualMode LEDManager::currentMode = VisualMode::ENCODING_MODE;
    VisualMode LEDManager::previousMode = VisualMode::ENCODING_MODE;
    FestiveTheme LEDManager::currentTheme = FestiveTheme::NONE;
    EncodingModes LEDManager::currentEncodingMode = EncodingModes::FULL_MODE;

    // Animation state tracking
    uint8_t LEDManager::currentPosition = 0;
    uint8_t LEDManager::currentColorIndex = 0;
    uint8_t LEDManager::completedCycles = 0;
    uint8_t LEDManager::filledPositions = 0;
    uint8_t LEDManager::activeTrails = 0;
    uint8_t LEDManager::animationStep = 0;
    uint8_t LEDManager::fadeValue = 128;
    uint8_t LEDManager::currentFadeIndex = 0;
    uint8_t LEDManager::loadingPosition = 0;

    // Color state tracking
    CRGB LEDManager::winningColor = CRGB::Green;
    CRGB LEDManager::targetColor = CRGB::Blue;
    bool LEDManager::transitioningColor = false;

    // Timing and state flags
    unsigned long LEDManager::lastStepTime = 0;
    unsigned long LEDManager::lastUpdate = 0;
    bool LEDManager::isLoading = false;
    bool LEDManager::fadeDirection = true;
    bool LEDManager::tickTock = false;
    bool LEDManager::readyForMelody = false;

    // Animation sequences
    const uint8_t LEDManager::fadeSequence[] = {0, 1, 2, 3, 4, 5, 6, 7};

    // Festive calendar structure
    struct FestiveDay 
    {
        int month;
        int day;
        FestiveTheme theme;
    };

    // Festive days configuration
    static const FestiveDay festiveDays[] = {
        {1, 1, FestiveTheme::NEW_YEAR},        // January 1
        {2, 14, FestiveTheme::VALENTINES},     // February 14
        {3, 17, FestiveTheme::ST_PATRICK},     // March 17
        {4, 1, FestiveTheme::EASTER},          // Easter (approximate)
        {7, 1, FestiveTheme::CANADA_DAY},      // July 1
        {10, 31, FestiveTheme::HALLOWEEN},     // October 31
        {12, 25, FestiveTheme::CHRISTMAS},     // December 25
        {11, 26, FestiveTheme::THANKSGIVING},  // Fourth Thursday in November (USA)
        {7, 4, FestiveTheme::INDEPENDENCE_DAY},// July 4 (USA)
        {6, 14, FestiveTheme::FLAG_DAY},       // June 14 (USA)
        {5, 31, FestiveTheme::MEMORIAL_DAY},   // Last Monday in May (USA)
        {9, 1, FestiveTheme::LABOR_DAY},       // First Monday in September (USA)
        {2, 1, FestiveTheme::DIWALI},          // Date varies
        {9, 1, FestiveTheme::MARDI_GRAS},      // Date varies
        {4, 1, FestiveTheme::RAMADAN},         // Date varies
        {1, 29, FestiveTheme::CHINESE_NEW_YEAR}// Date varies
    };

    bool LEDManager::initialized = false;

    /**
     * @brief Initialize visual processing pathways
     * Sets up LED hardware and initial perceptual state
     */
    void LEDManager::init() {
        Utilities::LOG_SCOPE("init()");
        Utilities::LOG_DEBUG("Starting LED Manager initialization...");
        
        try {
            // Initialize FastLED with hardware SPI configuration
            FastLED.addLeds<LED_TYPE,  // Changed from LEDConfig::LED_TYPE
                           MC::PinDefinitions::VisualPathways::WS2812_DATA_PIN, 
                           MC::PinDefinitions::VisualPathways::WS2812_COLOR_ORDER>(
                leds, 
                MC::PinDefinitions::VisualPathways::WS2812_NUM_LEDS
            ).setCorrection(FastLEDConfig::COLOR_CORRECTION);  // Changed from LEDConfig::COLOR_CORRECTION
            
            FastLED.setBrightness(FastLEDConfig::MAX_BRIGHTNESS);  // Changed from LEDConfig::MAX_BRIGHTNESS
            FastLED.setMaxRefreshRate(FastLEDConfig::DEFAULT_FPS);  // Changed from LEDConfig::DEFAULT_FPS
            FastLED.clear(true);
            
            // Initialize boot sequence colors
            fill_solid(leds, MC::PinDefinitions::VisualPathways::WS2812_NUM_LEDS, HARDWARE_INIT_COLOR);
            FastLED.show();
            
            Utilities::LOG_DEBUG("LED Manager initialized successfully");
        } catch (const std::exception& e) {
            Utilities::LOG_ERROR("LED Manager init failed: %s", e.what());
            throw;
        }
        
        startLoadingAnimation();
    }

    void LEDManager::runInitializationTest()
    {
        Utilities::LOG_SCOPE("runInitializationTest()");
        fill_solid(leds, MC::PinDefinitions::VisualPathways::WS2812_NUM_LEDS, HARDWARE_INIT_COLOR);
        FastLED.show();
        delay(100);
        FastLED.clear(true);
    }

    void LEDManager::stopLoadingAnimation() 
    {
        Utilities::LOG_SCOPE("stopLoadingAnimation()");
        if (!isLoading) return;
        
        isLoading = false;
        currentMode = VisualMode::ENCODING_MODE;
        currentEncodingMode = EncodingModes::FULL_MODE;
        // Initialize FULL_MODE pattern
        for (int i = 0; i < MC::PinDefinitions::VisualPathways::WS2812_NUM_LEDS; i++) {
            leds[i] = CRGB::Blue;  // Start with blue
            previousColors[i] = CRGB::Black;
        }
        FastLED.show();

        
        Utilities::LOG_DEBUG("LED Manager: Transitioned to FULL_MODE with patterns");
    }

    /**
     * @brief Process ongoing visual patterns
     * Updates LED states based on current cognitive state
     */
    void LEDManager::updateLEDs() {
        Utilities::LOG_SCOPE("updateLEDs()");
        if (GC::AppManager::isAppActive()) {
            switch (currentPattern) {  // Using class member variable
                case PC::VisualTypes::VisualPattern::SLOTS_GAME:
                    updateSlotsPattern();
                    break;
                case PC::VisualTypes::VisualPattern::IR_BLAST:
                    updateIRBlastPattern();
                    break;
                case PC::VisualTypes::VisualPattern::NFC_SCAN:
                    updateNFCScanPattern();
                    break;
                case PC::VisualTypes::VisualPattern::NONE:
                    // Do nothing for NONE pattern
                    break;
                default:
                    break;
            }
            return;
        }
        
        // Check and set festive mode based on the current date
        checkAndSetFestiveMode();

        if (isLoading) {
            updateLoadingAnimation();
            return;
        }

        switch (currentMode) {
            case VisualMode::ENCODING_MODE:
                switch(currentEncodingMode) {
                    case EncodingModes::FULL_MODE:
                        updateFullMode();
                        break;
                    case EncodingModes::WEEK_MODE:
                        updateWeekMode();
                        break;
                    case EncodingModes::TIMER_MODE:
                        updateTimerMode();
                        break;
                }
                break;
            case VisualMode::OFF_MODE:
                FastLED.clear();
                FastLED.show();
                break;
            case VisualMode::FESTIVE_MODE:
                updateFestiveMode();
                break;
        }
    }

    void LEDManager::setMode(VisualMode newMode) {
        Utilities::LOG_SCOPE("setMode(VisualMode)", String(static_cast<int>(newMode)));
        currentMode = newMode;
        FastLED.clear();
        updateLEDs();
    }

    void LEDManager::nextMode() {
        Utilities::LOG_SCOPE("nextMode()");
        currentMode = static_cast<VisualMode>((static_cast<int>(currentMode) + 1) % VisualConstants::LED_NUM_MODES);
        FastLED.clear();
        updateLEDs();
    }

    void LEDManager::updateFullMode() {
        Utilities::LOG_SCOPE("updateFullMode()");
        static unsigned long lastUpdate = 0;
        unsigned long currentTime = millis();

        if (currentTime - lastUpdate >= 100) {  // Update every 100ms
            lastUpdate = currentTime;

            time_t now = time(nullptr);
            struct tm* timeInfo = localtime(&now);
            
            // Create a 3-state blink cycle (0, 1, 2) using integer division of seconds
            int blinkState = (timeInfo->tm_sec % 3);
            
            // LED 0: Current day color (1-7, where 1 is Sunday)
            CRGB dayColor = VisualSynesthesia::getDayColor(timeInfo->tm_wday + 1);
            leds[0] = dayColor;
            
            // LED 1: Week number (base 5)
            int weekNum = (timeInfo->tm_mday + 6) / 7;
            switch(weekNum) {
                case 1: leds[1] = CRGB::Red; break;
                case 2: leds[1] = CRGB::Orange; break;
                case 3: leds[1] = CRGB::Yellow; break;
                case 4: leds[1] = CRGB::Green; break;
                default: leds[1] = CRGB::Blue; break;
            }
            
            // LED 2: Month (base 12)
            CRGB monthColor1, monthColor2;
            VisualSynesthesia::getMonthColors(timeInfo->tm_mon + 1, monthColor1, monthColor2);
            if (monthColor1 == monthColor2) {
                leds[2] = monthColor1;
            } else {
                switch(blinkState) {
                    case 0: leds[2] = monthColor1; break;
                    case 1: leds[2] = monthColor2; break;
                    case 2: leds[2] = CRGB::Black; break;
                }
            }

            // LED 3: Hours (base 12)
            int hour12 = timeInfo->tm_hour % 12;
            if (hour12 == 0) hour12 = 12;
            CRGB hourColor1, hourColor2;
            VisualSynesthesia::getHourColors(hour12, hourColor1, hourColor2);
            if (hourColor1 == hourColor2) {
                leds[3] = hourColor1;
            } else {
                switch(blinkState) {
                    case 0: leds[3] = hourColor1; break;
                    case 1: leds[3] = hourColor2; break;
                    case 2: leds[3] = CRGB::Black; break;
                }
            }

            // LED 4-5: Minutes (base 8)
            int minutes = timeInfo->tm_min;
            int minTens = minutes / 8;
            int minOnes = minutes % 8;
            leds[4] = VisualSynesthesia::getBase8Color(minTens);
            leds[5] = VisualSynesthesia::getBase8Color(minOnes);
            
            // LED 6-7: Day of month (base 8)
            int day = timeInfo->tm_mday;
            int dayTens = day / 8;
            int dayOnes = day % 8;
            leds[6] = VisualSynesthesia::getBase8Color(dayOnes);
            leds[7] = VisualSynesthesia::getBase8Color(dayTens);

            FastLED.show();  // Update the LED strip
        }
    }

    void LEDManager::updateWeekMode() {
        Utilities::LOG_SCOPE("updateWeekMode()");
        time_t now = time(nullptr);
        struct tm* timeInfo = localtime(&now);
        
        // Create a common blink state for both LEDs
        bool shouldBlink = (timeInfo->tm_sec % 2 == 0);
        
        // LED 0: Month color with alternating pattern
        CRGB monthColor1, monthColor2;
        VisualSynesthesia::getMonthColors(timeInfo->tm_mon + 1, monthColor1, monthColor2);
        
        // Month LED should always blink
        if (shouldBlink) {
            monthColor1.nscale8(VisualConstants::MONTH_DIM);
            leds[0] = monthColor1;
        } else {
            leds[0] = CRGB::Black;
        }
        
        // Days of week
        for (int i = 1; i <= 7; i++) {
            CRGB dayColor = VisualSynesthesia::getDayColor(i);
            
            if (i - 1 < timeInfo->tm_wday) {
                leds[i] = CRGB::Black;  // Past days are off
            } else if (i - 1 == timeInfo->tm_wday) {
                // Current day should always blink
                if (shouldBlink) {
                    dayColor.nscale8(184);  // Bright when on
                    leds[i] = dayColor;
                } else {
                    leds[i] = CRGB::Black;  // Off during blink
                }
            } else {
                dayColor.nscale8(77);  // Future days are dimmed
                leds[i] = dayColor;
            }
        }
    }

    void LEDManager::updateTimerMode() {
        Utilities::LOG_SCOPE("updateTimerMode()");
        static const CRGB timerColors[] = {
            CRGB::Red, CRGB::Orange, CRGB::Yellow, 
            CRGB::Green, CRGB::Blue, CRGB::Indigo, 
            CRGB::Purple, CRGB::White, CRGB::Black
        };
        static const int NUM_TIMER_COLORS = sizeof(timerColors) / sizeof(timerColors[0]);
        static CRGB backgroundColors[MC::PinDefinitions::VisualPathways::WS2812_NUM_LEDS] = {CRGB::Black};
        static bool isMoving = false;
        
        unsigned long currentTime = millis();
        if (currentTime - lastStepTime >= 125)  // 125ms between moves
        {
            lastStepTime = currentTime;
            
            if (!isMoving) 
            {
                // Start new drop at position 0
                leds[0] = timerColors[currentColorIndex];
                currentPosition = 0;
                isMoving = true;
            } 
            else 
            {
                // Clear current position
                leds[currentPosition] = backgroundColors[currentPosition];
                
                // Move to next position if possible
                if (currentPosition < MC::PinDefinitions::VisualPathways::WS2812_NUM_LEDS - 1 && 
                    leds[currentPosition + 1] == backgroundColors[currentPosition + 1]) 
                {
                    currentPosition++;
                    leds[currentPosition] = timerColors[currentColorIndex];
                } 
                else 
                {
                    // Drop has reached its final position
                    leds[currentPosition] = timerColors[currentColorIndex];
                    AC::SoundFxManager::playToneFx(PC::AudioTypes::Tone::TIMER_DROP);
                    isMoving = false;
                    
                    // Check if we completed this color's cycle
                    bool allFilled = true;
                    for (int i = 0; i < MC::PinDefinitions::VisualPathways::WS2812_NUM_LEDS; i++) 
                    {
                        if (leds[i] == backgroundColors[i]) 
                        {
                            allFilled = false;
                            break;
                        }
                    }
                    
                    if (allFilled) 
                    {
                        // Update background for next color
                        for (int i = 0; i < MC::PinDefinitions::VisualPathways::WS2812_NUM_LEDS; i++) 
                        {
                            backgroundColors[i] = timerColors[currentColorIndex];
                        }
                        currentColorIndex = (currentColorIndex + 1) % NUM_TIMER_COLORS;
                        if (currentColorIndex == 0) 
                        {
                            // Reset background when starting over
                            for (int i = 0; i < MC::PinDefinitions::VisualPathways::WS2812_NUM_LEDS; i++) 
                            {
                                backgroundColors[i] = CRGB::Black;
                            }
                        }
                    }
                }
            }
            FastLED.show();
        }
    }

    CRGB LEDManager::getRainbowColor(uint8_t index) 
    {
        Utilities::LOG_SCOPE("getRainbowColor(uint8_t)", String(index));
        static const CRGB rainbowColors[] = {
            CRGB::Red,
            CRGB::Orange,
            CRGB::Yellow,
            CRGB::Green,
            CRGB::Blue,
            CRGB(75, 0, 130),   // Indigo
            CRGB(148, 0, 211)   // Violet
        };
        return rainbowColors[index % VisualConstants::NUM_RAINBOW_COLORS];
    }

    void LEDManager::startLoadingAnimation() 
    {
        Utilities::LOG_SCOPE("startLoadingAnimation()");
        currentColorIndex = 0;
        currentPosition = 0;
        filledPositions = 0;
        completedCycles = 0;
        lastStepTime = 0;
        isLoading = true;
        FastLED.clear();
        FastLED.show();
    }

    void LEDManager::updateLoadingAnimation() {
        Utilities::LOG_SCOPE("updateLoadingAnimation()");
        if (millis() - lastUpdate < AnimationTiming::LOADING_DELAY) return;
        lastUpdate = millis();
        
        unsigned long currentTime = millis();
        if (currentTime - lastStepTime < AnimationTiming::LOADING_DELAY) return;
        
        lastStepTime = currentTime;
        int bootStep = PC::RoverBehaviorManager::getCurrentBootStep();
        static int lastBootStep = -1;

        // Only initialize new LEDs when boot step changes
        if (bootStep != lastBootStep) 
        {
            lastBootStep = bootStep;
            loadingPosition = bootStep * LEDS_PER_STEP;
        }

        // Select color based on current boot step
        CRGB currentColor;
        switch(bootStep) 
        {
            case 0: currentColor = HARDWARE_INIT_COLOR; break;
            case 1: currentColor = SYSTEM_START_COLOR; break;
            case 2: currentColor = NETWORK_PREP_COLOR; break;
            case 3: currentColor = FINAL_PREP_COLOR; break;
            default: currentColor = HARDWARE_INIT_COLOR;
        }

        // Light up one LED at a time within current step's section
        if (loadingPosition < (bootStep + 1) * LEDS_PER_STEP) 
        {
            leds[loadingPosition] = currentColor;
            loadingPosition++;
        }
        FastLED.show();
    }

    bool LEDManager::isLoadingComplete() 
    {
        Utilities::LOG_SCOPE("isLoadingComplete()");
        return completedCycles >= MC::PinDefinitions::VisualPathways::WS2812_NUM_LEDS;
    }

    void LEDManager::setLED(int index, CRGB color) 
    {
        Utilities::LOG_SCOPE("setLED(int, CRGB)", String(index));
        if (index >= 0 && index < MC::PinDefinitions::VisualPathways::WS2812_NUM_LEDS) {
            leds[index] = color;
        }
    }

    void LEDManager::showLEDs() 
    {
        Utilities::LOG_SCOPE("showLEDs()");
        FastLED.show();
    }

    void LEDManager::scaleLED(int index, uint8_t scale) 
    {
        Utilities::LOG_SCOPE("scaleLED(int, uint8_t)", 
            String(index),
            String(scale)
        );
        if (index >= 0 && index < MC::PinDefinitions::VisualPathways::WS2812_NUM_LEDS) {
            leds[index].nscale8(scale);
        }
    }

    void LEDManager::flashSuccess() 
    {
        Utilities::LOG_SCOPE("flashSuccess()");
        // Save current LED state
        CRGB savedState[MC::PinDefinitions::VisualPathways::WS2812_NUM_LEDS];
        memcpy(savedState, leds, sizeof(CRGB) * MC::PinDefinitions::VisualPathways::WS2812_NUM_LEDS);
        
        // Flash green
        fill_solid(leds, MC::PinDefinitions::VisualPathways::WS2812_NUM_LEDS, CRGB::Green);
        FastLED.show();
        delay(BootConfig::SUCCESS_FLASH_DURATION);
        
        // Restore previous state
        memcpy(leds, savedState, sizeof(CRGB) * MC::PinDefinitions::VisualPathways::WS2812_NUM_LEDS);
        FastLED.show();
    }

    void LEDManager::flashLevelUp() 
    {
        Utilities::LOG_SCOPE("flashLevelUp()");
        // First pass - clockwise light up in gold/yellow
        for (int i = 0; i < MC::PinDefinitions::VisualPathways::WS2812_NUM_LEDS; i++) {
            leds[i] = CRGB::Gold;
            FastLED.show();
            delay(50);
        }
        
        // Flash all bright white
        for (int i = 0; i < MC::PinDefinitions::VisualPathways::WS2812_NUM_LEDS; i++) {
            leds[i] = CRGB::White;
        }
        FastLED.show();
        delay(100);
        
        // Sparkle effect
        for (int j = 0; j < 3; j++) {  // Do 3 sparkle cycles
            for (int i = 0; i < MC::PinDefinitions::VisualPathways::WS2812_NUM_LEDS; i++) {
                if (random(2) == 0) {  // 50% chance for each LED
                    leds[i] = CRGB::Gold;
                } else {
                    leds[i] = CRGB::White;
                }
            }
            FastLED.show();
            delay(100);
        }
        
        // Fade out
        for (int i = 0; i < MC::PinDefinitions::VisualPathways::WS2812_NUM_LEDS; i++) {
            leds[i] = CRGB::Black;
            FastLED.show();
            delay(30);
        }
    }

    void LEDManager::displayCardPattern(const uint8_t* uid, uint8_t length) 
    {
        Utilities::LOG_SCOPE("displayCardPattern(uint8_t*, uint8_t)", String(length));
        static unsigned long lastUpdate = 0;
        static uint8_t step = 0;
        
        if (millis() - lastUpdate < 50) return;
        lastUpdate = millis();
        
        // Use card UID to create unique patterns
        for (int i = 0; i < MC::PinDefinitions::VisualPathways::WS2812_NUM_LEDS; i++) {
            uint8_t hue = (uid[i % length] + step) % 255;
            leds[i] = CHSV(hue, 255, 255);
        }
        
        step = (step + 1) % 255;
        showLEDs();
    }

    void LEDManager::syncLEDsForDay() 
    {
        Utilities::LOG_SCOPE("syncLEDsForDay()");
        time_t now = time(nullptr);
        struct tm* timeInfo = localtime(&now);
        int currentDay = timeInfo->tm_wday;
        int currentHour = timeInfo->tm_hour % 12;
        if (currentHour == 0) currentHour = 12;
        
        static const CRGB hourColors[] = {
            CRGB::Red, CRGB(255, 69, 0), CRGB::Orange,
            CRGB(255, 165, 0), CRGB::Yellow, CRGB::Green,
            CRGB::Blue, CRGB(75, 0, 130), CRGB(75, 0, 130),
            CRGB(75, 0, 130), CRGB(148, 0, 211), CRGB::Purple
        };

        setLED(0, hourColors[currentHour - 1]);
        scaleLED(0, 178);  // 70% brightness
        
        for (int i = 1; i <= 7; i++) {
            setLED(i, CRGB::White);
            scaleLED(i, i <= currentDay ? 128 : 28);
        }
        
        showLEDs();
    }

    void LEDManager::update() {
        Utilities::LOG_SCOPE("update()");
        // Check and set festive mode based on the current date
        LEDManager::checkAndSetFestiveMode();

        if (isLoading) {
            LEDManager::updateLoadingAnimation();
            return;
        }

        switch (currentMode) {
            case VisualMode::ENCODING_MODE:
                switch(currentEncodingMode) {
                    case EncodingModes::FULL_MODE:
                        LEDManager::updateFullMode();
                        break;
                    case EncodingModes::WEEK_MODE:
                        LEDManager::updateWeekMode();
                        break;
                    case EncodingModes::TIMER_MODE:
                        LEDManager::updateTimerMode();
                        break;
                    case EncodingModes::MENU_MODE:
                        LEDManager::updateMenuMode();
                        break;
                    case EncodingModes::CUSTOM_MODE:
                        LEDManager::updateCustomMode();
                        break;
                }
                break;
            case VisualMode::OFF_MODE:
                FastLED.clear();
                FastLED.show();
                break;
            case VisualMode::FESTIVE_MODE:
                LEDManager::updateFestiveMode();
                break;
            case VisualMode::ROVER_EMOTION_MODE:
                LEDManager::updateRoverEmotionMode();
                break;
        }
    }

    void LEDManager::updateMenuMode() {
        Utilities::LOG_SCOPE("updateMenuMode()");
        int selectedIndex = SC::MenuManager::getSelectedIndex();
        for (int i = 0; i < MC::PinDefinitions::VisualPathways::WS2812_NUM_LEDS; i++) {
            if (i <= selectedIndex) {
                setLED(i, VisualSynesthesia::getBase8Color(i));
                delay(50);
                showLEDs();
            } else {
                setLED(i, CRGB::Black);
            }
        }
        showLEDs();
    }

    void LEDManager::updateRoverEmotionMode() 
    {
        Utilities::LOG_SCOPE("updateRoverEmotionMode()");
        Utilities::LOG_DEBUG("< Pending Implementation // updateRoverEmotionMode() >");
        // TODO: Implement rover emotion mode
    }

    unsigned long tickTalkTime = 0;
    void LEDManager::updateCustomMode() {
        Utilities::LOG_SCOPE("updateCustomMode()");
        unsigned long currentTime = millis();

        if (currentTime - tickTalkTime >= 10000) {
            tickTalkTime = currentTime;
            tickTock = !tickTock;
        }

        if (tickTock) {
            int batteryLevel8 = PM::getBatteryPercentage() % 8;
            int uptime8 = PM::getUpTime() % 8;

            CRGB color = VisualSynesthesia::getBase8Color(batteryLevel8);

            for (int i = 0; i < MC::PinDefinitions::VisualPathways::WS2812_NUM_LEDS; i++) {
                if (i <= uptime8) {
                    setLED(i, color);
                    delay(50);
                    showLEDs();
                } else {
                    setLED(i, CRGB::Black);
                }
            }
        } else {
            for (int i = 0; i < MC::PinDefinitions::VisualPathways::WS2812_NUM_LEDS; i++) {
                leds[i] = CHSV(((i * 256 / MC::PinDefinitions::VisualPathways::WS2812_NUM_LEDS) + currentTime/10) % 256, 255, 255);
            }
        }

        showLEDs();
    }

    void LEDManager::updateFestiveMode() {
        Utilities::LOG_SCOPE("updateFestiveMode()");
        unsigned long currentTime = millis();
        if (currentTime - lastStepTime >= 50) {  // 50ms animation interval
            lastStepTime = currentTime;

            switch (currentTheme) {
                case FestiveTheme::NEW_YEAR:
                    // Fireworks effect with bright colors
                    for (int i = 0; i < MC::PinDefinitions::VisualPathways::WS2812_NUM_LEDS; i++) {
                        leds[i] = CRGB::White; // Bright white for fireworks
                        if (random8() < 20) { // 20% chance to add color
                            leds[i] = CRGB(random8(255), random8(255), random8(255));
                        }
                    }
                    break;

                case FestiveTheme::VALENTINES:
                    // Pulsing hearts effect with red and pink
                    fadeValue = fadeValue + (fadeDirection ? 5 : -5);
                    if (fadeValue >= 250) fadeDirection = false;
                    if (fadeValue <= 50) fadeDirection = true;

                    for (int i = 0; i < MC::PinDefinitions::VisualPathways::WS2812_NUM_LEDS; i++) {
                        CRGB color = (i % 2 == 0) ? CRGB::Red : CRGB::Pink;
                        color.nscale8(fadeValue);
                        leds[i] = color;
                    }
                    break;

                case FestiveTheme::ST_PATRICK:
                    // Rotating shamrock effect with green shades
                    for (int i = 0; i < MC::PinDefinitions::VisualPathways::WS2812_NUM_LEDS; i++) {
                        int adjustedPos = (i + animationStep) % MC::PinDefinitions::VisualPathways::WS2812_NUM_LEDS;
                        switch (adjustedPos % 3) {
                            case 0: leds[i] = CRGB::Green; break;
                            case 1: leds[i] = CRGB(0, 180, 0); break;
                            case 2: leds[i] = CRGB(0, 100, 0); break;
                        }
                    }
                    animationStep = (animationStep + 1) % MC::PinDefinitions::VisualPathways::WS2812_NUM_LEDS;
                    break;

                case FestiveTheme::EASTER:
                    // Soft pastel fade between colors
                    if (++animationStep >= 255) {
                        animationStep = 0;
                        currentColorIndex = (currentColorIndex + 1) % 4;
                    }

                    CRGB targetColor;
                    switch (currentColorIndex) {
                        case 0: targetColor = CRGB::Pink; break;
                        case 1: targetColor = CRGB(255, 255, 150); break;
                        case 2: targetColor = CRGB(150, 255, 255); break;
                        case 3: targetColor = CRGB(200, 255, 200); break;
                    }

                    for (int i = 0; i < MC::PinDefinitions::VisualPathways::WS2812_NUM_LEDS; i++) {
                        leds[i] = blend(previousColors[i], targetColor, animationStep);
                        previousColors[i] = leds[i];
                    }
                    break;

                case FestiveTheme::CANADA_DAY:
                    // Waving flag effect with red and white
                    animationStep = (animationStep + 1) % 255;
                    for (int i = 0; i < MC::PinDefinitions::VisualPathways::WS2812_NUM_LEDS; i++) {
                        int wave = sin8(animationStep + (i * 32));
                        if (i == 0 || i == 4) {
                            CRGB red = CRGB::Red;
                            red.nscale8(wave);
                            leds[i] = red;
                        } else {
                            CRGB white = CRGB::White;
                            white.nscale8(wave);
                            leds[i] = white;
                        }
                    }
                    break;

                case FestiveTheme::HALLOWEEN:
                    // Spooky fade between orange and purple
                    fadeValue = fadeValue + (fadeDirection ? 5 : -5);
                    if (fadeValue >= 250) fadeDirection = false;
                    if (fadeValue <= 50) fadeDirection = true;

                    for (int i = 0; i < MC::PinDefinitions::VisualPathways::WS2812_NUM_LEDS; i++) {
                        CRGB color = (i % 2 == 0) ? CRGB::Orange : CRGB::Purple;
                        color.nscale8(fadeValue);
                        leds[i] = color;
                    }
                    break;

                case FestiveTheme::THANKSGIVING:
                    // Autumn colors fading effect
                    for (int i = 0; i < MC::PinDefinitions::VisualPathways::WS2812_NUM_LEDS; i++) {
                        leds[i] = (i % 2 == 0) ? CRGB::Orange : CRGB::Brown; // Alternating colors
                    }
                    break;

                case FestiveTheme::INDEPENDENCE_DAY:
                    // Red, white, and blue flashing
                    for (int i = 0; i < MC::PinDefinitions::VisualPathways::WS2812_NUM_LEDS; i++) {
                        leds[i] = (i % 3 == 0) ? CRGB::Red : (i % 3 == 1) ? CRGB::White : CRGB::Blue;
                    }
                    break;

                case FestiveTheme::DIWALI:
                    // Colorful lights effect
                    for (int i = 0; i < MC::PinDefinitions::VisualPathways::WS2812_NUM_LEDS; i++) {
                        leds[i] = CRGB(random8(255), random8(255), random8(255)); // Random colors
                    }
                    break;

                case FestiveTheme::RAMADAN:
                    // Soft white and gold glow
                    for (int i = 0; i < MC::PinDefinitions::VisualPathways::WS2812_NUM_LEDS; i++) {
                        leds[i] = (i % 2 == 0) ? CRGB::White : CRGB::Gold; // Alternating colors
                    }
                    break;

                case FestiveTheme::CHINESE_NEW_YEAR:
                    // Red and gold flashing
                    for (int i = 0; i < MC::PinDefinitions::VisualPathways::WS2812_NUM_LEDS; i++) {
                        leds[i] = (i % 2 == 0) ? CRGB::Red : CRGB::Gold; // Alternating colors
                    }
                    break;

                case FestiveTheme::MARDI_GRAS:
                    // Purple, green, and gold flashing
                    for (int i = 0; i < MC::PinDefinitions::VisualPathways::WS2812_NUM_LEDS; i++) {
                        leds[i] = (i % 3 == 0) ? CRGB::Purple : (i % 3 == 1) ? CRGB::Green : CRGB::Gold; // Alternating colors
                    }
                    break;

                case FestiveTheme::LABOR_DAY:
                    // Red and white stripes
                    for (int i = 0; i < MC::PinDefinitions::VisualPathways::WS2812_NUM_LEDS; i++) {
                        leds[i] = (i % 2 == 0) ? CRGB::Red : CRGB::White; // Alternating colors
                    }
                    break;

                case FestiveTheme::MEMORIAL_DAY:
                    // Red, white, and blue stripes
                    for (int i = 0; i < MC::PinDefinitions::VisualPathways::WS2812_NUM_LEDS; i++) {
                        leds[i] = (i % 3 == 0) ? CRGB::Red : (i % 3 == 1) ? CRGB::White : CRGB::Blue; // Alternating colors
                    }
                    break;

                case FestiveTheme::FLAG_DAY:
                    // Red and white stripes with blue at the ends
                    for (int i = 0; i < MC::PinDefinitions::VisualPathways::WS2812_NUM_LEDS; i++) {
                        if (i < 2) {
                            leds[i] = CRGB::Blue; // Blue at the ends
                        } else {
                            leds[i] = (i % 2 == 0) ? CRGB::Red : CRGB::White; // Alternating colors
                        }
                    }
                    break;
            }
            FastLED.show();
        }
    }

    void LEDManager::setFestiveTheme(FestiveTheme theme) 
    {
        Utilities::LOG_SCOPE("setFestiveTheme(FestiveTheme)", String(static_cast<int>(theme)));
        currentTheme = theme;
        currentMode = VisualMode::FESTIVE_MODE;
        FastLED.clear();
        LEDManager::updateLEDs();
    }

    void LEDManager::updateIRBlastPattern() {
        Utilities::LOG_SCOPE("updateIRBlastPattern()");
        static uint8_t currentLEDPosition = 0;
        static bool animationDirection = true;
        
        FastLED.clear();
        
        if (animationDirection) {
            // Moving outward from center
            setLED(4, CRGB::Red);  // Always show center
            if (currentLEDPosition < 4) {
                setLED(4 - currentLEDPosition, CRGB::Red);
                setLED(4 + currentLEDPosition, CRGB::Red);
            }
            
            currentLEDPosition++;
            if (currentLEDPosition >= 4) {
                animationDirection = false;
                currentLEDPosition = 4;
                SoundFxManager::playTone(1000, 200);
            }
        } else {
            // Moving inward to center
            setLED(4, CRGB::Red);  // Always show center
            if (currentLEDPosition > 0) {
                setLED(4 - currentLEDPosition, CRGB::Red);
                setLED(4 + currentLEDPosition, CRGB::Red);
            }
            
            currentLEDPosition--;
            if (currentLEDPosition == 0) {
                animationDirection = true;
            }
        }
        
        showLEDs();
    }

    void LEDManager::updateSlotsPattern() {
        Utilities::LOG_SCOPE("updateSlotsPattern()");
        // Move slots LED code from SlotsManager
        // But keep using LEDManager's methods
    }

    void LEDManager::updateNFCScanPattern() {
        Utilities::LOG_SCOPE("updateNFCScanPattern()");
        if (millis() - lastUpdate < AnimationTiming::LOADING_DELAY) return;
        lastUpdate = millis();

        if (transitioningColor) {
            if (currentFadeIndex < sizeof(fadeSequence)) {
                // Fade sequence from blue to green
                uint8_t ledIndex = fadeSequence[currentFadeIndex];
                leds[ledIndex] = blend(CRGB::Blue, CRGB::Green, fadeValue);
                fadeValue += PatternConfig::FADE_INCREMENT;
                
                if (fadeValue >= AnimationTiming::MAX_FADE) {
                    fadeValue = 0;
                    currentFadeIndex++;
                }
            } else if (!readyForMelody) {
                readyForMelody = true;
                AC::SoundFxManager::playCardMelody(PSY::NFCManager::getLastCardId());
            }
        } else {
            // Normal blue pulse
            fadeValue += (fadeDirection ? PatternConfig::FADE_INCREMENT : -PatternConfig::FADE_INCREMENT);
            
            if (fadeValue <= AnimationTiming::MIN_FADE) fadeDirection = true;
            if (fadeValue >= AnimationTiming::MAX_FADE) fadeDirection = false;
            
            fill_solid(leds, MC::PinDefinitions::VisualPathways::WS2812_NUM_LEDS, CRGB::Blue);
            fadeToBlackBy(leds, MC::PinDefinitions::VisualPathways::WS2812_NUM_LEDS, 255 - fadeValue);
        }
        FastLED.show();
    }

    void LEDManager::setPattern(PC::VisualPattern pattern) 
    {
        Utilities::LOG_SCOPE("setPattern(VisualPattern)", String(static_cast<int>(pattern)));
        currentPattern = pattern;
    }

    void LEDManager::handleMessage(PC::VisualMessage message) 
    {
        Utilities::LOG_SCOPE("handleMessage(VisualMessage)", String(static_cast<int>(message)));
        switch(message) 
        {
            case PC::VisualMessage::SLOTS_WIN:
                LEDManager::currentPattern = PC::VisualPattern::SLOTS_GAME;
                winningColor = CRGB::Green;
                break;
                
            case PC::VisualMessage::IR_SUCCESS:
                flashSuccess();
                break;
                
            case PC::VisualMessage::NFC_DETECTED:
                LEDManager::currentPattern = PC::VisualPattern::NFC_SCAN;
                LEDManager::transitioningColor = true;
                LEDManager::fadeValue = 0;
                LEDManager::currentFadeIndex = 0;
                LEDManager::readyForMelody = false;
                break;
                
            case PC::VisualMessage::NFC_ERROR:
                LEDManager::currentPattern = PC::VisualPattern::NFC_SCAN;
                LEDManager::targetColor = CRGB::Red;
                LEDManager::fadeValue = 0;
                LEDManager::transitioningColor = true;
                break;

            case PC::VisualMessage::SUCCESS:
                LEDManager::currentPattern = PC::VisualPattern::RAINBOW;
                LEDManager::fadeValue = 0;
                LEDManager::transitioningColor = true;
                break;

            case PC::VisualMessage::ERROR:
                LEDManager::currentPattern = PC::VisualPattern::PULSE;
                LEDManager::targetColor = CRGB::Red;
                LEDManager::fadeValue = 0;
                LEDManager::transitioningColor = true;
                break;

            case PC::VisualMessage::WARNING:
                LEDManager::currentPattern = PC::VisualPattern::CHASE;
                LEDManager::targetColor = CRGB::Yellow;
                LEDManager::fadeValue = 0;
                LEDManager::transitioningColor = true;
                break;

            case PC::VisualMessage::SYSTEM_ERROR:
                LEDManager::currentPattern = PC::VisualPattern::SOLID;
                LEDManager::targetColor = CRGB::Red;
                LEDManager::fadeValue = 255; // Full brightness
                LEDManager::transitioningColor = false;
                break;

            case PC::VisualMessage::POWER_LOW:
                LEDManager::currentPattern = PC::VisualPattern::TIMER;
                LEDManager::targetColor = CRGB::Orange;
                LEDManager::fadeValue = 0;
                LEDManager::transitioningColor = true;
                break;

            case PC::VisualMessage::LEVEL_UP:
                LEDManager::currentPattern = PC::VisualPattern::CUSTOM;
                LEDManager::targetColor = CRGB::Gold;
                LEDManager::fadeValue = 0;
                LEDManager::transitioningColor = true;
                LEDManager::readyForMelody = false;
                break;

            case PC::VisualMessage::BOOT_COMPLETE:
                LEDManager::currentPattern = PC::VisualPattern::MENU;
                LEDManager::targetColor = CRGB::Blue;
                LEDManager::fadeValue = 0;
                LEDManager::transitioningColor = true;
                break;

            case PC::VisualMessage::NONE:
            case PC::VisualMessage::INFO:
            case PC::VisualMessage::CARD_SCANNED: // Handled by NFC_DETECTED
            case PC::VisualMessage::NETWORK_STATUS:
            default:
                LEDManager::currentPattern = PC::VisualPattern::NONE;
                break;
        }
    }

    CRGB LEDManager::getNoteColor(uint16_t frequency) 
    {
        Utilities::LOG_SCOPE("getNoteColor(uint16_t)", String(frequency));
        return VisualSynesthesia::getColorForFrequency(frequency);
    }

    void LEDManager::displayNote(uint16_t frequency, uint8_t position) {
        Utilities::LOG_SCOPE("displayNote(uint16_t, uint8_t)", 
            String(frequency),
            String(position)
        );
        position = position % MC::PinDefinitions::VisualPathways::WS2812_NUM_LEDS;
        
        NoteInfo info = AC::PitchPerception::getNoteInfo(frequency);
        LEDManager::currentNotes[position].isSharp = info.isSharp;
        LEDManager::currentNotes[position].position = position;
        
        if (info.isSharp) {
            // For sharp/flat notes, get colors of adjacent natural notes
            uint16_t lowerFreq = AC::PitchPerception::getStandardFrequency(frequency - 10);
            uint16_t upperFreq = AC::PitchPerception::getStandardFrequency(frequency + 10);
            currentNotes[position].color1 = LEDManager::getNoteColor(lowerFreq);
            currentNotes[position].color2 = LEDManager::getNoteColor(upperFreq);
        } else {
            CRGB noteColor = LEDManager::getNoteColor(frequency);
            LEDManager::currentNotes[position].color1 = noteColor;
            LEDManager::currentNotes[position].color2 = noteColor;
        }

        leds[position] = tickTock ? LEDManager::currentNotes[position].color1 : LEDManager::currentNotes[position].color2;
        tickTock = !tickTock;
        showLEDs();
    }

    void LEDManager::setErrorLED(bool state) 
    {
        Utilities::LOG_SCOPE("setErrorLED(bool)", String(state));
        if (state) {
            leds[ERROR_LED_INDEX] = CRGB::Red;
        } else {
            leds[ERROR_LED_INDEX] = CRGB::Black;
        }
        FastLED.show();
    }

    void LEDManager::setErrorPattern(uint32_t errorCode, bool isFatal) 
    {
        Utilities::LOG_SCOPE("setErrorPattern(uint32_t, bool)", 
            String(errorCode),
            String(isFatal)
        );
        // Clear existing pattern first
        FastLED.clear();
        
        // Make error more visible - use first 16 LEDs
        CRGB errorColor = isFatal ? CRGB::Red : CRGB::Yellow;
        
        // Set all error indicator LEDs
        for (uint8_t i = 0; i < ERROR_LED_COUNT * 2; i++) {
            leds[i] = errorColor;
        }
        
        // Encode error in binary using brighter LEDs
        for (uint8_t i = 0; i < ERROR_LED_COUNT; i++) {
            if (errorCode & (1 << i)) {
                leds[i].maximizeBrightness();
            }
        }
        
        FastLED.setBrightness(isFatal ? 255 : 128);  // Full brightness for fatal errors
        FastLED.show();
        
        // Debug output
        Serial.printf("Error pattern set: code=0x%08X, fatal=%d\n", errorCode, isFatal);
    }

    void LEDManager::clearErrorPattern() {
        Utilities::LOG_SCOPE("clearErrorPattern()");
        // Clear error LEDs
        fill_solid(leds + ERROR_LED_INDEX, ERROR_LED_COUNT * 2, CRGB::Black);
        FastLED.show();
    }

    void LEDManager::updateErrorPattern() {
        Utilities::LOG_SCOPE("updateErrorPattern()");
        // Only update for fatal errors (pulsing effect)
        if (RoverViewManager::isFatalError) {
            // Update fade value
            if (fadeDirection) {
                fadeValue = std::min<uint8_t>(255, fadeValue + PatternConfig::FADE_INCREMENT);
                if (fadeValue >= AnimationTiming::MAX_FADE) fadeDirection = false;
            } else {
                fadeValue = std::max<uint8_t>(AnimationTiming::ERROR_MIN_FADE, 
                                            fadeValue - PatternConfig::FADE_INCREMENT);
                if (fadeValue <= AnimationTiming::ERROR_MIN_FADE) fadeDirection = true;
            }
            
            // Apply fade to error LEDs
            for (uint8_t i = 0; i < PatternConfig::ERROR_LED_COUNT; i++) {
                if (leds[PatternConfig::ERROR_LED_INDEX + i].r > 0) // Only fade red LEDs (fatal errors)
                { 
                    leds[PatternConfig::ERROR_LED_INDEX + i].fadeToBlackBy(255 - fadeValue);
                }
            }
            
            FastLED.show();
        }
    }

    void LEDManager::checkAndSetFestiveMode() {
        Utilities::LOG_SCOPE("checkAndSetFestiveMode()");
        time_t now = time(nullptr);
        struct tm* timeInfo = localtime(&now);

        // Get the current month and day
        int month = timeInfo->tm_mon + 1; // tm_mon is 0-based
        int day = timeInfo->tm_mday;

        // Check for festive days
        for (const auto& festiveDay : festiveDays) {
            if (festiveDay.month == month && festiveDay.day == day) {
                setFestiveTheme(festiveDay.theme);
                return; // Exit after setting the festive theme
            }
        }

        // Reset to default mode if no festive day
        setMode(VisualMode::ENCODING_MODE);
    }

    void LEDManager::setEncodingMode(EncodingModes mode) {
        Utilities::LOG_SCOPE("setEncodingMode(EncodingModes)", String(static_cast<int>(mode)));
        currentMode = VisualMode::ENCODING_MODE; // Set the main mode to ENCODING_MODE
        currentEncodingMode = mode; // Set the specific encoding mode
        // Additional logic to handle the encoding mode can be added here
    }

    /**
     * @brief Process chromatic visual perception
     * @param context Chromatic sensory context for visualization
     */
    void LEDManager::displayChromatic(const PC::ColorPerceptionTypes::ChromaticContext& context) 
    {
        Utilities::LOG_SCOPE("displayChromatic(ChromaticContext)");
        // Set primary color on odd LEDs
        for(int i = 0; i < MC::PinDefinitions::VisualPathways::WS2812_NUM_LEDS; i += 2) 
        {
            leds[i] = context.primary;
            leds[i].nscale8(context.intensity);
        }
        
        // Set secondary color on even LEDs
        for(int i = 1; i < MC::PinDefinitions::VisualPathways::WS2812_NUM_LEDS; i += 2) 
        {
            leds[i] = context.secondary;
            leds[i].nscale8(context.intensity);
        }
        
        FastLED.show();
    }

    void LEDManager::displayEmotional(const PC::ColorPerceptionTypes::EmotionalColor& emotion) 
    {
        Utilities::LOG_SCOPE("displayEmotional(EmotionalColor)");
        // Alternate between primary and accent colors
        for(int i = 0; i < MC::PinDefinitions::VisualPathways::WS2812_NUM_LEDS; i++) 
        {
            leds[i] = (i % 2 == 0) ? emotion.primaryColor : emotion.accentColor;
        }
        FastLED.show();
    }
}