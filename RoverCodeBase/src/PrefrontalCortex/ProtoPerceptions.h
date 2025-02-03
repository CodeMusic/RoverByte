/*
 * ProtoPerceptions.h
 * 
 * Core type definitions and data structures used across the robot's cortices.
 * Organized in cognitive hierarchy from low-level to high-level processing:
 * 
 * 1. Basic Types - Fundamental type definitions (byte, etc.)
 * 2. Storage Types - Memory and persistence (SPIFFS, SD, etc.)
 * 3. Sensor Types - Hardware interfaces and readings
 * 4. Config Types - System configuration structures
 * 5. System Types - Core system state and logging
 * 6. Input/Output Types - UI, Menu, Display interactions
 * 7. Behavior Types - Robot personality and emotions
 * 8. Rover Types - High-level robot states and expressions
 * 9. Chakra/Virtue Types - Metaphysical/philosophical states
 */

#ifndef PROTO_PERCEPTIONS_H
#define PROTO_PERCEPTIONS_H

#include <Arduino.h>
#include <FastLED.h>
#include <vector>
#include <string>
#include <functional>

namespace PrefrontalCortex 
{
    // 1. Basic Types
    #ifdef byte
    #undef byte
    #endif
    typedef uint8_t byte;

    // 2. Storage Types
    namespace StorageTypes 
    {
        enum class StorageDevice 
        {
            SPIFFS,
            SD_CARD,
            EEPROM,
            FLASH
        };

        struct FileMetadata 
        {
            String name;
            uint32_t size;
            uint32_t lastModified;
            bool isDirectory;
        };

        struct StorageStats 
        {
            uint32_t totalSpace;
            uint32_t usedSpace;
            uint32_t freeSpace;
            bool isHealthy;
        };
    }

    // 3. Sensor Types
    namespace SensorTypes 
    {
        enum class SensorState 
        {
            INITIALIZING,
            READY,
            READING,
            ERROR,
            CALIBRATING
        };

        struct SensorReading 
        {
            uint32_t timestamp;
            float value;
            uint8_t sensorId;
            bool isValid;
        };

        struct CalibrationData 
        {
            float offset;
            float scale;
            uint32_t lastCalibration;
            bool isCalibrated;
        };
    }

    // 4. Config Types
    namespace ConfigTypes 
    {
        struct NetworkConfig 
        {
            String ssid;
            String password;
            bool isDhcp;
            String staticIp;
            uint16_t port;
        };

        struct DisplayConfig 
        {
            uint8_t brightness;
            uint8_t rotation;
            bool isEnabled;
            uint16_t backgroundColor;
            uint16_t textColor;
        };

        struct AudioConfig 
        {
            uint8_t volume;
            bool isMuted;
            uint8_t equalizer[5];
            uint16_t sampleRate;
        };

        struct LEDConfig 
        {
            uint8_t brightness;
            uint8_t pattern;
            CRGB primaryColor;
            CRGB secondaryColor;
            uint8_t speed;
        };
    }

    // Power Management Types
    namespace PowerTypes 
    {
        enum class PowerState 
        {
            AWAKE,
            DIM_DISPLAY,
            DISPLAY_OFF,
            DEEP_SLEEP
        };

        struct BatteryStatus 
        {
            float voltageLevel;
            int percentageCharge;
            bool isCharging;
            float temperature;
            uint32_t lastUpdateTime;
        };

        struct PowerConfig 
        {
            PowerState defaultState;
            uint32_t dimTimeout;
            uint32_t sleepTimeout;
            uint8_t dimBrightness;
            bool enableDeepSleep;
        };
    }

    // 5. System Types
    namespace SystemTypes 
    {
        // Renamed from BehaviorState to SystemState for clarity
        // Represents overall system states rather than specific behaviors
        enum class SystemState 
        {
            STARTUP,    // Keep existing values
            IDLE,
            ACTIVE,
            ERROR,
            MAINTENANCE,
            SHUTDOWN
        };

        enum class LogLevel
        {
            PRODUCTION,
            ERROR,
            DEBUG,
            INFO,
            WARNING,
            SCOPE
        };

        // Add this array of strings
        static const char* LOG_LEVEL_STRINGS[] = 
        {
            "PROD",
            "ERROR", 
            "DEBUG",
            "INFO",
            "WARNING"
        };

        struct SystemStatus 
        {
            PowerTypes::PowerState powerState;
            float batteryLevel;
            float temperature;
            uint32_t uptime;
            bool isCharging;
        };

        struct ErrorLog 
        {
            uint32_t timestamp;    // When the error occurred
            String message;        // Human-readable error message
            uint8_t severity;      // How serious the error is
            String component;      // Which part of the system had the error
        };

        struct ErrorState 
        {
            uint32_t code;         // Numeric error code
            const char* message;   // Error message (using const char* for memory efficiency)
            bool isFatal;          // Whether this error stops the system
            uint32_t timestamp;    // When the error occurred
            String context;        // Additional error context/details
        };

        enum class ErrorType 
        {
            WARNING,
            FATAL,
            SILENT
        };
    }

    // 6. Input/Output Types
    namespace InputTypes 
    {
        enum class InputState 
        {
            NOT_STARTED,
            INITIALIZING,
            READY,
            ERROR
        };
    }

    namespace UITypes 
    {
        enum class MenuState 
        {
            MAIN_MENU,
            SUB_MENU,
            SETTINGS,
            GAME_SELECT,
            APP_RUNNING
        };

        enum class ScrollDirection 
        {
            UP,
            DOWN,
            LEFT,
            RIGHT,
            NONE
        };

        struct MenuItem 
        {
            String label;
            uint8_t id;
            std::vector<MenuItem> subItems;
            bool isSelectable;
            bool hasSubMenu;
        };
    }

    // 7. Behavior Types
    namespace BehaviorTypes 
    {
        enum class EmotionalState 
        {
            HAPPY,
            SAD,
            EXCITED,
            CALM,
            CONFUSED,
            FOCUSED
        };

        enum class InteractionMode 
        {
            PASSIVE,
            RESPONSIVE,
            PROACTIVE,
            LEARNING
        };

        struct BehaviorProfile 
        {
            EmotionalState currentEmotion;
            InteractionMode mode;
            uint8_t energyLevel;
            uint8_t sociability;
        };

        struct Interaction 
        {
            uint32_t timestamp;
            String type;
            uint8_t intensity;
            bool wasSuccessful;
        };
    }

    // 8. Rover Types
    namespace RoverTypes 
    {
        enum class Expression 
        {
            NEUTRAL,
            HAPPY,
            LOOKING_LEFT,
            LOOKING_RIGHT,
            INTENSE,
            LOOKING_UP,
            LOOKING_DOWN,
            BIG_SMILE,
            EXCITED,
            NUM_EXPRESSIONS
        };

        enum class BehaviorState 
        {
            LOADING,        // Initial boot/loading state
            HOME,          // Main home screen
            MENU,          // Menu navigation
            APP,           // Running an application
            ERROR,         // Error state
            WARNING,       // Warning state
            FATAL_ERROR,   // Fatal error state
            IDLE,          // Idle state
            MENU_MODE,     // Menu display mode (for LED encoding)
            FULL_DISPLAY   // Full display mode (for LED encoding)
        };

        enum class LoadingPhase 
        {
            BOOTING,
            CONNECTING_WIFI,
            SYNCING_TIME
        };

        enum class StartupErrorCode 
        {
            NONE,
            WIFI_INIT_FAILED,
            TIME_SYNC_FAILED,
            SD_INIT_FAILED,
            DISPLAY_INIT_FAILED,
            UI_INIT_FAILED,
            APP_INIT_FAILED,
            CORE_INIT_FAILED
        };

        struct ErrorInfo 
        {
            uint32_t code;
            const char* message;
            SystemTypes::ErrorType type;
        };
    }

    // 9. Metaphysical Types
    namespace ChakraTypes 
    {
        enum class ChakraState 
        {
            ROOT,
            SACRAL,
            SOLAR_PLEXUS,
            HEART,
            THROAT,
            THIRD_EYE,
            CROWN
        };

        struct ChakraInfo 
        {
            const char* name;
            const char* description;
            uint16_t color;
            void (*drawFunction)(int, int, int);
        };
    }

    namespace VirtueTypes 
    {
        enum class VirtueState 
        {
            CHASTITY,
            TEMPERANCE,
            CHARITY,
            DILIGENCE,
            FORGIVENESS,
            KINDNESS,
            HUMILITY
        };

        struct VirtueInfo 
        {
            const char* name;
            const char* description;
            void (*drawSymbol)(int, int, int);
        };
    }

    // Common data structures
    struct SensoryInput 
    {
        uint32_t timestamp;
        byte sensorId;
        float value;
    };

    struct NeuralResponse 
    {
        bool isProcessed;
        std::string message;
        byte priority;
    };

    // Enums for system states
    enum class CognitionState 
    {
        IDLE,
        PROCESSING,
        ERROR,
        LEARNING
    };

    // Type aliases for common data structures
    using SensoryBuffer = std::vector<SensoryInput>;
    using ResponseQueue = std::vector<NeuralResponse>;

    // Auditory Perception Types
    namespace AudioTypes 
    {
        enum class TimeSignature 
        {
            TIME_2_2,
            TIME_4_4,
            TIME_3_4,
            TIME_6_8,
            TIME_12_16
        };

        enum class NoteIndex 
        {
            NOTE_C = 0,
            NOTE_CS = 1,
            NOTE_D = 2,
            NOTE_DS = 3,
            NOTE_E = 4,
            NOTE_F = 5,
            NOTE_FS = 6,
            NOTE_G = 7,
            NOTE_GS = 8,
            NOTE_A = 9,
            NOTE_AS = 10,
            NOTE_B = 11,
            REST = 12
        };

        enum class NoteType 
        {
            WHOLE,
            HALF,
            QUARTER,
            EIGHTH,
            SIXTEENTH,
            THIRTY_SECOND,
            SIXTY_FOURTH,
            HUNDRED_TWENTY_EIGHTH
        };

        struct NoteInfo 
        {
            NoteIndex note;
            uint8_t octave;
            NoteType type;
            bool isSharp;

            NoteInfo(NoteIndex n, uint8_t o, NoteType t) 
                : note(n), octave(o), type(t), isSharp(false) {}
        };

        struct ErrorTone 
        {
            uint16_t frequency;
            uint16_t duration;
        };

        struct WavHeaderInfo 
        {
            uint32_t fileSize;
            uint32_t byteRate;
            uint32_t sampleRate;
            uint16_t numChannels;
            uint16_t bitsPerSample;
        };

        struct Tune 
        {
            const char* name;
            std::vector<NoteInfo> notes;
            std::vector<uint8_t> ledAnimation;
            TimeSignature timeSignature;
        };

        enum class TunesTypes 
        {
            ROVERBYTE_JINGLE,
            JINGLE_BELLS,
            AULD_LANG_SYNE,
            LOVE_SONG,
            HAPPY_BIRTHDAY,
            EASTER_SONG,
            MOTHERS_SONG,
            FATHERS_SONG,
            CANADA_SONG,
            USA_SONG,
            CIVIC_SONG,
            WAKE_ME_UP_WHEN_SEPTEMBER_ENDS,
            HALLOWEEN_SONG,
            THANKSGIVING_SONG,
            CHRISTMAS_SONG
        };

        enum class Tone
        {
            NONE,
            SUCCESS,
            ERROR,
            WARNING,
            NOTIFICATION,
            TIMER_DROP,  // Add this for timer mode
            LEVEL_UP,
            GAME_OVER,
            MENU_SELECT,
            MENU_CHANGE
        };

        /**
         * @brief Defines cognitive error response types for audio feedback
         * 
         * Maps different system errors to distinct auditory patterns:
         * - RECORDING: Microphone/input processing errors
         * - STORAGE: Memory/persistence errors
         * - PLAYBACK: Audio output processing errors
         */
        enum class ErrorSoundType 
        {
            RECORDING = 1,  // Microphone/recording system errors
            STORAGE = 2,    // SD card/storage system errors  
            PLAYBACK = 3    // Audio playback system errors
        };
    }

    // Visual Types
    namespace VisualTypes 
    {
        enum class VisualMode 
        {
            OFF_MODE,
            ENCODING_MODE,      // Main mode for encoding
            FESTIVE_MODE,
            ROVER_EMOTION_MODE
        };

        enum class EncodingModes 
        {
            FULL_MODE,
            WEEK_MODE,
            TIMER_MODE,
            CUSTOM_MODE,
            MENU_MODE
        };

        enum class VisualPattern 
        {
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

        enum class ViewType 
        { 
            TODO_LIST,
            CHAKRAS,
            VIRTUES,
            QUOTES,
            WEATHER,
            STATS,
            NEXTMEAL,
            NUM_VIEWS
        };

        enum class FestiveTheme
        {
            NONE,
            NEW_YEAR,
            VALENTINES,
            ST_PATRICK,
            EASTER,
            CANADA_DAY,
            HALLOWEEN,
            CHRISTMAS,
            THANKSGIVING,
            INDEPENDENCE_DAY,
            FLAG_DAY,
            MEMORIAL_DAY,
            LABOR_DAY,
            DIWALI,
            MARDI_GRAS,
            RAMADAN,
            CHINESE_NEW_YEAR
        };

        enum class VisualMessage
        {
            NONE,
            SUCCESS,
            ERROR,
            WARNING,
            INFO,
            SLOTS_WIN,      // Used in handleMessage for slots game
            IR_SUCCESS,     // Used for IR blast success
            NFC_DETECTED,   // Used when NFC card is detected
            NFC_ERROR,     // Used when NFC operation fails
            LEVEL_UP,      // Used for level up animations
            BOOT_COMPLETE,  // Used when boot sequence completes
            CARD_SCANNED,   // Used when a card is successfully scanned
            POWER_LOW,      // Used for low power warnings
            NETWORK_STATUS, // Used for network connectivity updates
            SYSTEM_ERROR   // Used for system-level errors
        };

        struct NoteState 
        {
            CRGB color1;
            CRGB color2;  // For sharps/flats
            bool isSharp;
            uint8_t position;
            
            NoteState() : color1(CRGB::Black), color2(CRGB::Black), isSharp(false), position(0) {}
        };

        // Color Arrays
        static const CRGB BASE_8_COLORS[8];
        static const CRGB MONTH_COLORS[12][2];
        static const CRGB DAY_COLORS[7];
        static const CRGB CHROMATIC_COLORS[12][2];

        // Boot stage colors
        static const CRGB HARDWARE_INIT_COLOR;
        static const CRGB SYSTEM_START_COLOR;
        static const CRGB NETWORK_PREP_COLOR;
        static const CRGB FINAL_PREP_COLOR;

        /**
         * @brief Visual Animation Parameters
         */
        struct AnimationPerception 
        {
            static constexpr uint16_t LOADING_DELAY = 100;
            static constexpr uint16_t FLASH_DURATION = 100;
            static constexpr uint8_t FADE_STEP = 5;
            static constexpr uint8_t MIN_FADE = 50;
            static constexpr uint8_t MAX_FADE = 250;
            static constexpr uint8_t ERROR_MIN_FADE = 64;
        };

        /**
         * @brief Visual Pattern Parameters
         */
        struct PatternPerception 
        {
            static constexpr uint8_t LEDS_PER_STEP = 3;
            static constexpr uint8_t ERROR_LED_INDEX = 0;
            static constexpr uint8_t ERROR_LED_COUNT = 8;
            static constexpr uint8_t FADE_INCREMENT = 5;
            static constexpr uint16_t ANIMATION_DELAY = 50;
        };

        /**
         * @brief Boot Sequence Parameters
         */
        struct BootPerception 
        {
            static constexpr uint16_t STEP_DELAY = 100;
            static constexpr uint16_t SUCCESS_FLASH_DURATION = 100;
            static constexpr uint16_t ERROR_FLASH_DURATION = 200;
        };
    }

    namespace NFCTypes 
    {
        enum class InitState 
        {
            NOT_STARTED,
            HARDWARE_INIT,
            FIRMWARE_CHECK,
            SAM_CONFIG,
            COMPLETE,
            ERROR
        };
    }

    // Network Types
    namespace NetworkTypes 
    {
        struct NetworkCredentials 
        {
            const char* ssid;
            const char* password;
        };
    }

    // Menu Types
    namespace MenuTypes 
    {
        struct MenuItem 
        {
            std::string name;
            std::function<void()> action;
            std::vector<MenuItem> subItems;

            MenuItem(const std::string& n, const std::vector<MenuItem>& items) 
                : name(n), 
                  action(nullptr),
                  subItems(items)
            {
            }

            MenuItem(const std::string& n, std::function<void()> act) 
                : name(n), 
                  action(act),
                  subItems()
            {
            }

            bool operator==(const MenuItem& other) const 
            {
                return (name == other.name && 
                        subItems == other.subItems);
            }
        };
    }

    // Game Types
    namespace GameTypes 
    {
        enum class GameState 
        {
            IDLE,
            SPINNING,
            WIN,
            LOSE,
            BONUS_ROUND
        };
        
        enum class AppState {
            IDLE,       // No app is active
            SHOW_INFO,  // Displaying a quick info screen
            RUNNING     // The app is currently running
        };

        struct SlotSymbol 
        {
            uint8_t id;
            String symbol;
            uint16_t color;
            uint8_t probability;
        };

        struct GameScore 
        {
            uint32_t points;
            uint16_t level;
            uint8_t multiplier;
        };

        struct AppInfo 
        {
            std::string name;
            std::string description;
            void (*onRun)();
            void (*onUpdate)();
            void (*onExit)();
            bool isEnabled = true;
        };
    }

    // Communication Types
    namespace CommTypes 
    {
        enum class NFCState 
        {
            READY,
            SCANNING,
            TAG_FOUND,
            ERROR,
            WRITING
        };

        enum class IRCommand 
        {
            NONE,
            POWER,
            VOLUME_UP,
            VOLUME_DOWN,
            CHANNEL_UP,
            CHANNEL_DOWN,
            MENU,
            SELECT
        };

        struct NFCTag 
        {
            uint32_t id;
            String data;
            uint8_t type;
            bool isWriteable;
        };

        enum class InitState 
        {
            NOT_STARTED,
            HARDWARE_INIT,
            FIRMWARE_CHECK,
            SAM_CONFIG,
            COMPLETE,
            ERROR
        };
    }

    // Auditory Types
    namespace AuditoryTypes 
    {

        enum class SoundEffect 
        {
            BOOT_UP,
            ERROR,
            SUCCESS,
            WARNING,
            NOTIFICATION
        };
    }

    // Psychic Types
    namespace PsychicTypes 
    {
        struct NFCData 
        {
            uint8_t uid[7];
            uint8_t uidLength;
            String data;
        };

        struct IRCommand 
        {
            uint32_t code;
            const char* name;
            void (*handler)();
        };

        struct WiFiCredentials 
        {
            String ssid;
            String password;
            bool isEnterprise;
        };
    }

    // App Types
    namespace AppTypes 
    {
        enum class AppState 
        {
            DORMANT,    // No app is active
            ORIENTING,  // Displaying initial info screen
            ENGAGED     // App is actively running
        };

        struct AppNeuralNetwork 
        {
            std::string name;
            std::string description;
            std::function<void()> onActivate;    // Called when app starts
            std::function<void()> onProcess;     // Called during runtime
            std::function<void()> onDeactivate;  // Called during shutdown
        };
    }

    namespace ColorPerceptionTypes 
    {
        /**
         * @brief Core psychological color mappings
         * Each color represents a different cognitive/emotional state:
         * 0 Black   - Unconscious/Rest state
         * 1 Red     - Primal/Survival instincts
         * 2 Orange  - Creative/Expressive energy
         * 3 Yellow  - Intellectual/Mental clarity
         * 4 Green   - Emotional/Heart-centered
         * 5 Blue    - Intuitive/Communication
         * 6 Indigo  - Spiritual/Inner wisdom
         * 7 Violet  - Transcendent/Higher consciousness
         */
        extern const CRGB BASE_8_COLORS[8];
        
        /**
         * @brief Temporal-emotional color associations
         * Maps months to color pairs representing:
         * - Primary: Dominant emotional theme
         * - Secondary: Supporting emotional quality
         */
        extern const CRGB MONTH_COLORS[12][2];
        
        /**
         * @brief Circadian rhythm color mappings
         * Associates days with colors based on:
         * - Traditional planetary associations
         * - Psychological energy patterns
         */
        extern const CRGB DAY_COLORS[7];
        
        /**
         * @brief Musical-emotional correlations
         * Maps musical notes to color pairs representing:
         * - Primary: Core emotional resonance
         * - Secondary: Harmonic emotional quality
         */
        extern const CRGB CHROMATIC_COLORS[12][2];

        // Add color perception context
        struct ChromaticContext 
        {
            CRGB primary;
            CRGB secondary;
            uint8_t intensity;
        };

        // Add emotional color mapping
        struct EmotionalColor 
        {
            RoverTypes::Expression emotion;
            CRGB primaryColor;
            CRGB accentColor;
        };

        // Add strong typing for color intensity
        struct ColorIntensity 
        {
            explicit ColorIntensity(uint8_t value) : value(value) {}
            uint8_t value;
        };

        // Add strong typing for color index
        struct ColorIndex 
        {
            explicit ColorIndex(uint8_t value) : value(value % 8) {}
            uint8_t value;
        };
    }

    namespace SynestheticTypes 
    {
        // For direct sensory mappings
        struct AudioVisualMapping 
        {
            uint16_t frequency;
            ColorPerceptionTypes::ChromaticContext colors;
            uint8_t intensity;
        };

        // For temporal aspects of synesthesia
        struct SynestheticTiming 
        {
            uint32_t onset;          // When the perception starts
            uint32_t duration;       // How long it lasts
            uint8_t fadeRate;        // How quickly it fades
        };
    }
}

#endif // PROTO_PERCEPTIONS_H 