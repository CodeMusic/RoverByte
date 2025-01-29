
/*WIP
#ifndef PROTO_PERCEPTIONS_H
#define PROTO_PERCEPTIONS_H

#include <Arduino.h>
#include <FastLED.h>
#include <vector>
#include <string>

namespace PrefrontalCortex 
{
    // Basic type definitions
    #ifdef byte
    #undef byte
    #endif
    typedef uint8_t byte;

    // Memory and Storage Types
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

    // Sensor Types
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

    // Configuration Types
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

    // System State Types
    namespace SystemTypes 
    {
        enum class PowerState 
        {
            FULL_POWER,
            LOW_POWER,
            CRITICAL,
            CHARGING,
            SLEEP
        };

        enum class BehaviorState 
        {
            STARTUP,
            IDLE,
            ACTIVE,
            ERROR,
            MAINTENANCE,
            SHUTDOWN
        };

        struct SystemStatus 
        {
            PowerState powerState;
            float batteryLevel;
            float temperature;
            uint32_t uptime;
            bool isCharging;
        };

        struct ErrorLog 
        {
            uint32_t timestamp;
            String message;
            uint8_t severity;
            String component;
        };
    }

    // Rover Behavior Types
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

    // Menu and UI Types
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
            const char* name;
            const char* description;
            void (*launch)();
            bool isEnabled;
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

    // Auditory Perception Types
    namespace AudioTypes 
    {
        enum class TimeSignature 
        {
            TIME_4_4,
            TIME_3_4,
            TIME_6_8
        };

        enum class NoteIndex 
        {
            REST,
            NOTE_C,
            NOTE_D,
            NOTE_E,
            NOTE_F,
            NOTE_G,
            NOTE_A,
            NOTE_B
        };

        enum class NoteType 
        {
            WHOLE,
            HALF,
            QUARTER,
            EIGHTH
        };

        struct NoteInfo 
        {
            NoteIndex note;
            uint8_t octave;
            NoteType type;
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
    }

    // Core Types
    enum class ErrorType 
    {
        WARNING,
        FATAL,
        SILENT  // Serial-only output
    };

    // Visual Types
    namespace VisualTypes 
    {
        enum class Mode 
        {
            OFF_MODE,
            ENCODING_MODE,
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

        struct NoteState 
        {
            CRGB color1;
            CRGB color2;
            bool isSharp;
            uint8_t position;
        };

        enum class FestiveTheme 
        {
            NEW_YEAR,        // January 1
            VALENTINES,      // February 14
            ST_PATRICK,      // March 17
            EASTER,          // March/April (variable)
            CANADA_DAY,      // July 1
            HALLOWEEN,       // October 31
            CHRISTMAS,       // December 25
            THANKSGIVING,    // Fourth Thursday in November (USA)
            INDEPENDENCE_DAY,// July 4 (USA)
            DIWALI,         // Date varies (Hindu festival of lights)
            RAMADAN,        // Date varies (Islamic holy month)
            CHINESE_NEW_YEAR,// Date varies (Lunar New Year)
            MARDI_GRAS,     // Date varies (Fat Tuesday)
            LABOR_DAY,      // First Monday in September (USA)
            MEMORIAL_DAY,   // Last Monday in May (USA)
            FLAG_DAY        // June 14 (USA)
        };

        enum class VisualMessage 
        {
            NONE,
            SLOTS_WIN,
            IR_SUCCESS,
            NFC_DETECTED,
            NFC_ERROR
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

    // Rover Types
    namespace RoverTypes 
    {
        enum class Expression 
        {
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
            LOADING,
            HOME,
            MENU,
            APP,
            WARNING,
            ERROR,
            FATAL_ERROR,
            IDLE
        };

        enum class LoadingPhase 
        {
            BOOTING,
            CONNECTING_WIFI,
            SYNCING_TIME
        };

        enum class StartupErrorCode : uint32_t 
        {
            CORE_INIT_FAILED = 0x01,
            POWER_INIT_FAILED = 0x02,
            LED_INIT_FAILED = 0x03,
            DISPLAY_INIT_FAILED = 0x04,
            UI_INIT_FAILED = 0x05,
            MENU_INIT_FAILED = 0x06,
            AUDIO_INIT_FAILED = 0x07,
            STORAGE_INIT_FAILED = 0x08,
            WIFI_INIT_FAILED = 0x09,
            TIME_SYNC_FAILED = 0x0A,
            APP_INIT_FAILED = 0x0B
        };

        struct ErrorInfo 
        {
            uint32_t code;
            const char* message;
            ErrorType type;
        };
    }

    // Add ChakraTypes namespace after RoverTypes (around line 537)
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

    // Add VirtueTypes namespace
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

    // Somatosensory Types
    namespace SomatosensoryTypes 
    {
        enum class TouchState 
        {
            NONE,
            PRESSED,
            HELD,
            RELEASED
        };

        struct MenuOption 
        {
            const char* label;
            void (*callback)();
            bool isEnabled;
        };
    }

    // Auditory Types
    namespace AuditoryTypes 
    {
        struct NoteInfo 
        {
            uint16_t frequency;
            uint16_t duration;
            uint8_t volume;
        };

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
}

#endif // PROTO_PERCEPTIONS_H 
*/
