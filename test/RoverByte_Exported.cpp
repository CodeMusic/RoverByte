// FastLED configuration must come first
#include "src/VisualCortex/FastLEDConfig.h"

// Core system includes
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <FastLED.h>
#include "TFT_eSPI.h"
#include <RotaryEncoder.h>
#include <time.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include "Audio.h"
#include <XPowersLib.h>
#include "driver/i2s.h"
#include <vector>

// Include core configurations first
#include "src/CorpusCallosum/SynapticPathways.h"
#include "src/MotorCortex/PinDefinitions.h"

// Use base namespace
using namespace CorpusCallosum;

// Include all managers after namespace declaration
#include "src/PrefrontalCortex/Utilities.h"
#include "src/PrefrontalCortex/SPIManager.h"
#include "src/PrefrontalCortex/RoverBehaviorManager.h"
#include "src/VisualCortex/DisplayConfig.h"
#include "src/VisualCortex/RoverViewManager.h"
#include "src/VisualCortex/LEDManager.h"
#include "src/VisualCortex/RoverManager.h"
#include "src/SomatosensoryCortex/UIManager.h"
#include "src/SomatosensoryCortex/MenuManager.h"
#include "src/PrefrontalCortex/PowerManager.h"
#include "src/PrefrontalCortex/SDManager.h"
#include "src/VisualCortex/VisualSynesthesia.h"
#include "src/AuditoryCortex/SoundFxManager.h"
#include "src/PsychicCortex/WiFiManager.h"
#include "src/PsychicCortex/IRManager.h"
#include "src/PsychicCortex/NFCManager.h"
#include "src/GameCortex/SlotsManager.h"

// Use specific namespaces after includes
using PC::Utilities;
using PC::SPIManager;
using PC::RoverBehaviorManager;
using PC::PowerManager;
using PC::SDManager;
using VC::RoverViewManager;
using VC::LEDManager;
using VC::RoverManager;
using VC::VisualSynesthesia;
using SC::UIManager;
using SC::MenuManager;
using AC::SoundFxManager;
using PSY::WiFiManager;
using PSY::IRManager;
using PSY::NFCManager;
using GC::SlotsManager;

#include <esp_task_wdt.h>

// Create encoder using pin definitions directly
RotaryEncoder encoder(ENCODER_INA, ENCODER_INB, RotaryEncoder::LatchMode::TWO03);

void setup() {
    Serial.begin(115200);
    LOG_PROD("Starting setup...");

    // Check free heap memory before initialization
    //LOG_DEBUG("Free heap before initialization: %d", ESP.getFreeHeap());

    // Initialize SPI bus and chip selects first
    SPIManager::init();
    LOG_DEBUG("SPI Manager initialized.");

    try 
    {
        RoverViewManager::init();
        LOG_DEBUG("Display initialized successfully.");
    } 
    catch (const std::exception& e) 
    {
        LOG_ERROR("Display initialization failed: %s", e.what());
        return;
    }

    // After successful initialization
    RoverViewManager::drawErrorScreen(
        RoverViewManager::errorCode,
        RoverViewManager::errorMessage,
        RoverViewManager::isFatalError
    );
    RoverViewManager::drawLoadingScreen(RoverBehaviorManager::getStatusMessage());
    delay(100);
    if (!RoverBehaviorManager::IsInitialized()) 
    {
        try 
        {
            LOG_DEBUG("Starting RoverBehaviorManager...");
            RoverBehaviorManager::init();
            LOG_DEBUG("RoverBehaviorManager started successfully.");
        } 
        catch (const std::exception& e) 
        {
            LOG_ERROR("Initialization error: %s", e.what());
            RoverBehaviorManager::triggerFatalError(
                static_cast<uint32_t>(RoverBehaviorManager::StartupErrorCode::CORE_INIT_FAILED),
                e.what()
            );
            return;
        }
    }

    // Check free heap memory after initialization
    //LOG_DEBUG("Free heap after initialization: %d", ESP.getFreeHeap());


}

void loop() {
    static unsigned long lastDraw = 0;
    const unsigned long DRAW_INTERVAL = 50;  // 20fps
    static bool soundStarted = false;

    // Handle critical updates first with error checking
    try {
        RoverBehaviorManager::update();
        delay(1);

        // Only update UI and LED if we're not in LOADING state
        if (RoverBehaviorManager::getCurrentState() != RoverBehaviorManager::BehaviorState::LOADING) {
            UIManager::update();
            delay(1);
            
            LEDManager::update();
            delay(1);
            
            // Handle sound initialization
            if (!soundStarted && SoundFxManager::isInitialized() && 
                !SoundFxManager::isPlaying()) {
                SoundFxManager::playStartupSound();
                soundStarted = true;
                delay(1);
            }
        }
        
        // Handle display updates at fixed interval
        unsigned long currentMillis = millis();
        if (currentMillis - lastDraw >= DRAW_INTERVAL) 
        {
            lastDraw = currentMillis;
            
            // Clear sprite first
            RoverViewManager::clearSprite();
            
            // Handle different cognitive states
            if (RoverViewManager::isError || 
                RoverViewManager::isFatalError) 
            {
                // Only draw error screen in error state
                RoverViewManager::drawErrorScreen(
                    RoverViewManager::errorCode,
                    RoverViewManager::errorMessage,
                    RoverViewManager::isFatalError
                );
            }
            else if (RoverBehaviorManager::getCurrentState() == 
                     RoverBehaviorManager::BehaviorState::LOADING) 
            {
                // Draw loading screen during initialization
                RoverViewManager::drawLoadingScreen(
                    RoverBehaviorManager::getStatusMessage()
                );
            }
            else 
            {
                // Normal operation - draw current view and rover
                RoverViewManager::drawCurrentView();
                
                if (!RoverViewManager::isError && 
                    !RoverViewManager::isFatalError) 
                {
                    RoverManager::drawRover(
                        RoverManager::getCurrentMood(),
                        RoverManager::earsPerked,
                        !RoverManager::showTime,
                        10,
                        RoverManager::showTime ? 50 : 80
                    );
                }
            }
            
            // Push sprite to display with minimal delay
            RoverViewManager::pushSprite();
            delay(1); // Reduced delay for better performance
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Loop error: %s", e.what());
        delay(100); // Give system time to recover
    }
    
    // Final yield
    delay(2);
}


#ifndef PITCHPERCEPTION_H
#define PITCHPERCEPTION_H

#include "../CorpusCallosum/SynapticPathways.h"
#include "../PrefrontalCortex/ProtoPerceptions.h"
#include <Arduino.h>
#include <cstdint> // For fixed-width integer types

namespace AuditoryCortex
{
    using namespace CorpusCallosum;
    using PC::AudioTypes::NoteInfo;
    using PC::AudioTypes::NoteIndex;
    using PC::AudioTypes::NoteType;
    using PC::AudioTypes::TimeSignature;

    // Constants for standard note durations in milliseconds (base 4/4)
    static const uint16_t WHOLE_NOTE_MS = 1000;
    static const uint16_t HALF_NOTE_MS = 500;
    static const uint16_t QUARTER_NOTE_MS = 250;
    static const uint16_t EIGHTH_NOTE_MS = 125;
    static const uint16_t SIXTEENTH_NOTE_MS = 62;
    static const uint16_t THIRTY_SECOND_NOTE_MS = 31;
    static const uint16_t SIXTY_FOURTH_NOTE_MS = 15;
    static const uint16_t HUNDRED_TWENTY_EIGHTH_NOTE_MS = 7;

    class PitchPerception {
    public:
        // Musical note frequencies in Hz
        static const uint16_t NOTE_B0 = 31;
        static const uint16_t NOTE_C1 = 33;
        static const uint16_t NOTE_CS1 = 35;
        static const uint16_t NOTE_D1 = 37;
        static const uint16_t NOTE_DS1 = 39;
        static const uint16_t NOTE_E1 = 41;
        static const uint16_t NOTE_F1 = 44;
        static const uint16_t NOTE_FS1 = 46;
        static const uint16_t NOTE_G1 = 49;
        static const uint16_t NOTE_GS1 = 52;
        static const uint16_t NOTE_A1 = 55;
        static const uint16_t NOTE_AS1 = 58;
        static const uint16_t NOTE_B1 = 62;
        static const uint16_t NOTE_C2 = 65;
        static const uint16_t NOTE_CS2 = 69;
        static const uint16_t NOTE_D2 = 73;
        static const uint16_t NOTE_DS2 = 78;
        static const uint16_t NOTE_E2 = 82;
        static const uint16_t NOTE_F2 = 87;
        static const uint16_t NOTE_FS2 = 93;
        static const uint16_t NOTE_G2 = 98;
        static const uint16_t NOTE_GS2 = 104;
        static const uint16_t NOTE_A2 = 110;
        static const uint16_t NOTE_AS2 = 117;
        static const uint16_t NOTE_B2 = 123;
        static const uint16_t NOTE_C3 = 131;
        static const uint16_t NOTE_CS3 = 139;
        static const uint16_t NOTE_D3 = 147;
        static const uint16_t NOTE_DS3 = 156;
        static const uint16_t NOTE_E3 = 165;
        static const uint16_t NOTE_F3 = 175;
        static const uint16_t NOTE_FS3 = 185;
        static const uint16_t NOTE_G3 = 196;
        static const uint16_t NOTE_GS3 = 208;
        static const uint16_t NOTE_A3 = 220;
        static const uint16_t NOTE_AS3 = 233;
        static const uint16_t NOTE_B3 = 247;
        static const uint16_t NOTE_C4 = 262;
        static const uint16_t NOTE_CS4 = 277;
        static const uint16_t NOTE_D4 = 294;
        static const uint16_t NOTE_DS4 = 311;
        static const uint16_t NOTE_E4 = 330;
        static const uint16_t NOTE_F4 = 349;
        static const uint16_t NOTE_FS4 = 370;
        static const uint16_t NOTE_G4 = 392;
        static const uint16_t NOTE_GS4 = 415;
        static const uint16_t NOTE_A4 = 440;
        static const uint16_t NOTE_AS4 = 466;
        static const uint16_t NOTE_B4 = 494;
        static const uint16_t NOTE_C5 = 523;
        static const uint16_t NOTE_CS5 = 554;
        static const uint16_t NOTE_D5 = 587;
        static const uint16_t NOTE_DS5 = 622;
        static const uint16_t NOTE_E5 = 659;
        static const uint16_t NOTE_F5 = 698;
        static const uint16_t NOTE_FS5 = 740;
        static const uint16_t NOTE_G5 = 784;
        static const uint16_t NOTE_GS5 = 831;
        static const uint16_t NOTE_A5 = 880;
        static const uint16_t NOTE_AS5 = 932;
        static const uint16_t NOTE_B5 = 988;
        static const uint16_t NOTE_C6 = 1047;
        static const uint16_t NOTE_CS6 = 1109;
        static const uint16_t NOTE_D6 = 1175;
        static const uint16_t NOTE_DS6 = 1245;
        static const uint16_t NOTE_E6 = 1319;
        static const uint16_t NOTE_F6 = 1397;
        static const uint16_t NOTE_FS6 = 1480;
        static const uint16_t NOTE_G6 = 1568;
        static const uint16_t NOTE_GS6 = 1661;
        static const uint16_t NOTE_A6 = 1760;
        static const uint16_t NOTE_AS6 = 1865;
        static const uint16_t NOTE_B6 = 1976;
        static const uint16_t NOTE_C7 = 2093;
        static const uint16_t NOTE_CS7 = 2217;
        static const uint16_t NOTE_D7 = 2349;
        static const uint16_t NOTE_DS7 = 2489;
        static const uint16_t NOTE_E7 = 2637;
        static const uint16_t NOTE_F7 = 2794;
        static const uint16_t NOTE_FS7 = 2960;
        static const uint16_t NOTE_G7 = 3136;
        static const uint16_t NOTE_GS7 = 3322;
        static const uint16_t NOTE_A7 = 3520;
        static const uint16_t NOTE_AS7 = 3729;
        static const uint16_t NOTE_B7 = 3951;
        static const uint16_t NOTE_C8 = 4186;
        static const uint16_t NOTE_CS8 = 4435;
        static const uint16_t NOTE_D8 = 4699;
        static const uint16_t NOTE_DS8 = 4978;

        static const uint16_t NOTE_FREQUENCIES[];
        static const char* NOTE_NAMES[];

        static PC::AudioTypes::NoteInfo getNoteInfo(uint16_t frequency);
        static uint16_t getStandardFrequency(uint16_t frequency);
        static uint16_t getNoteFrequency(const PC::AudioTypes::NoteInfo& info);
        static const char* getNoteName(const PC::AudioTypes::NoteInfo& info);
        static uint16_t getDayBaseNote4();
        static uint16_t getDayBaseNote5();
        static uint16_t getDayBaseNote(bool is4thOctave);
        static bool isSharp(uint16_t frequency);
        static uint16_t getNoteDuration(NoteType note, TimeSignature timeSignature);

        // Helper for SoundFxManager
        static uint16_t getOctaveUp(uint16_t baseNote) { return baseNote * 2; }
        static uint16_t getOctaveAndFifthUp(uint16_t baseNote) { return baseNote * 3; }
        static uint16_t getToneDown(uint16_t baseNote) { return baseNote * 8 / 10; }
        static uint16_t getNoteMinus2(uint16_t baseNote) { return baseNote * 8 / 10; }

        static uint16_t getNoteDuration(NoteType note, TimeSignature timeSignature);

        static const uint16_t* getNoteFrequencies() {
            return NOTE_FREQUENCIES;
        }

        static bool isFlat(uint16_t frequency);
        static bool isInTune(uint16_t frequency);
        
        // Frequency analysis methods
        static uint16_t detectPitch();
        static uint16_t getFrequencyFromNote(uint8_t note);
        static uint8_t getNoteFromFrequency(uint16_t frequency);

    private:
        static const uint16_t FREQUENCY_TOLERANCE = 1; // Hz tolerance for pitch detection
    }; 

}
#endif // PITCHPERCEPTION_H
#include "../CorpusCallosum/SynapticPathways.h"
#include "../PrefrontalCortex/ProtoPerceptions.h"
#include "PitchPerception.h"
#include <time.h>

namespace AuditoryCortex
{
    // First declare the types we're using from other namespaces
    using PC::AudioTypes::NoteInfo;
    using PC::AudioTypes::NoteIndex;
    using PC::AudioTypes::NoteType;
    using PC::AudioTypes::TimeSignature;
    using namespace CorpusCallosum;

    const uint16_t PitchPerception::NOTE_FREQUENCIES[] = {
        NOTE_B0, NOTE_C1, NOTE_CS1, NOTE_D1, NOTE_DS1, NOTE_E1, NOTE_F1, NOTE_FS1, NOTE_G1, NOTE_GS1, NOTE_A1, NOTE_AS1, NOTE_B1,
        NOTE_C2, NOTE_CS2, NOTE_D2, NOTE_DS2, NOTE_E2, NOTE_F2, NOTE_FS2, NOTE_G2, NOTE_GS2, NOTE_A2, NOTE_AS2, NOTE_B2,
        NOTE_C3, NOTE_CS3, NOTE_D3, NOTE_DS3, NOTE_E3, NOTE_F3, NOTE_FS3, NOTE_G3, NOTE_GS3, NOTE_A3, NOTE_AS3, NOTE_B3,
        NOTE_C4, NOTE_CS4, NOTE_D4, NOTE_DS4, NOTE_E4, NOTE_F4, NOTE_FS4, NOTE_G4, NOTE_GS4, NOTE_A4, NOTE_AS4, NOTE_B4,
        NOTE_C5, NOTE_CS5, NOTE_D5, NOTE_DS5, NOTE_E5, NOTE_F5, NOTE_FS5, NOTE_G5, NOTE_GS5, NOTE_A5, NOTE_AS5, NOTE_B5,
        NOTE_C6, NOTE_CS6, NOTE_D6, NOTE_DS6, NOTE_E6, NOTE_F6, NOTE_FS6, NOTE_G6, NOTE_GS6, NOTE_A6, NOTE_AS6, NOTE_B6,
        NOTE_C7, NOTE_CS7, NOTE_D7, NOTE_DS7, NOTE_E7, NOTE_F7, NOTE_FS7, NOTE_G7, NOTE_GS7, NOTE_A7, NOTE_AS7, NOTE_B7,
        NOTE_C8, NOTE_CS8, NOTE_D8, NOTE_DS8
    };

    const char* PitchPerception::NOTE_NAMES[] = {
        "B0", "C1", "C#", "D1", "D#", "E1", "F1", "F#", "G1", "G#", "A1", "A#", "B1",
        "C2", "C#", "D2", "D#", "E2", "F2", "F#", "G2", "G#", "A2", "A#", "B2",
        "C3", "C#", "D3", "D#", "E3", "F3", "F#", "G3", "G#", "A3", "A#", "B3",
        "C4", "C#", "D4", "D#", "E4", "F4", "F#", "G4", "G#", "A4", "A#", "B4",
        "C5", "C#", "D5", "D#", "E5", "F5", "F#", "G5", "G#", "A5", "A#", "B5",
        "C6", "C#", "D6", "D#", "E6", "F6", "F#", "G6", "G#", "A6", "A#", "B6",
        "C7", "C#", "D7", "D#", "E7", "F7", "F#", "G7", "G#", "A7", "A#", "B7",
        "C8", "C#", "D8", "D#"
    };

    PC::AudioTypes::NoteInfo PitchPerception::getNoteInfo(uint16_t frequency) 
    {
        PC::AudioTypes::NoteInfo info(PC::AudioTypes::NoteIndex::REST, 4, PC::AudioTypes::NoteType::QUARTER);
        uint16_t minDiff = UINT16_MAX;
        
        for(int i = 0; i < sizeof(NOTE_FREQUENCIES)/sizeof(NOTE_FREQUENCIES[0]); i++) 
        {
            uint16_t diff = abs(frequency - NOTE_FREQUENCIES[i]);
            if(diff < minDiff) 
            {
                minDiff = diff;
                info = PC::AudioTypes::NoteInfo(
                    static_cast<PC::AudioTypes::NoteIndex>(i % 12),
                    (i / 12) + 1,
                    PC::AudioTypes::NoteType::QUARTER
                );
                info.isSharp = (static_cast<int>(info.note) == 1) || 
                              (static_cast<int>(info.note) == 3) || 
                              (static_cast<int>(info.note) == 6) || 
                              (static_cast<int>(info.note) == 8) || 
                              (static_cast<int>(info.note) == 10);
            }
        }
        return info;
    }

    uint16_t PitchPerception::getStandardFrequency(uint16_t frequency) {
        PC::AudioTypes::NoteInfo info = getNoteInfo(frequency);
        return NOTE_FREQUENCIES[(info.octave - 1) * 12 + static_cast<int>(info.note)];
    }

    uint16_t PitchPerception::getNoteFrequency(const PC::AudioTypes::NoteInfo& info) {
        return NOTE_FREQUENCIES[(info.octave - 1) * 12 + static_cast<int>(info.note)];
    }

    const char* PitchPerception::getNoteName(const PC::AudioTypes::NoteInfo& info) {
        return NOTE_NAMES[static_cast<int>(info.note)];
    }

    static inline int getNoteMinus2(int baseNote) {
        return baseNote * 8 / 10;
    }


    uint16_t PitchPerception::getDayBaseNote4() {
        return getDayBaseNote(true);
    }

    uint16_t PitchPerception::getDayBaseNote5() {
        return getDayBaseNote(false);
    }

    uint16_t PitchPerception::getDayBaseNote(bool is4thOctave) {
        // Declare the array to hold the day notes
        const uint16_t* dayNotes;

        // Array mapping days of the week to base notes
        if (is4thOctave) {
            static const uint16_t tempDayNotes[] = {
                NOTE_C4,  // Sunday
                NOTE_D4,  // Monday
                NOTE_E4,  // Tuesday
                NOTE_F4,  // Wednesday
                NOTE_G4,  // Thursday
                NOTE_A4,  // Friday
                NOTE_B4   // Saturday
            };
            dayNotes = tempDayNotes; // Point to the 4th octave notes
        } else {
            static const uint16_t tempDayNotes[] = {
                NOTE_C5,  // Sunday
                NOTE_D5,  // Monday
                NOTE_E5,  // Tuesday
                NOTE_F5,  // Wednesday
                NOTE_G5,  // Thursday
                NOTE_A5,  // Friday
                NOTE_B5   // Saturday
            };
            dayNotes = tempDayNotes; // Point to the 5th octave notes
        }

        // Get the current day of the week (0 = Sunday, 6 = Saturday)
        time_t now = time(nullptr);
        struct tm* timeInfo = localtime(&now);
        int dayOfWeek = timeInfo->tm_wday;

        // Return the corresponding base note for the day
        return dayNotes[dayOfWeek];
    }

    bool PitchPerception::isSharp(uint16_t frequency) {
        PC::AudioTypes::NoteInfo info = getNoteInfo(frequency);
        //noteIndex is the index of the note in the NOTE_NAMES array
        //                              0  1   2   3  4  5  6   7  8   9  10  11
        //the notes are in the order of C, C#, D, D#, E, F, F#, G, G#, A, A#, B
        //the sharp notes are              C#,    D#,       F#,    G#,    A#
        //                                  1     3         6      8      10
        //so if the noteIndex is 1, 3, 6, 8, or 10, then the note is sharp
        return static_cast<int>(info.note) == 1 || static_cast<int>(info.note) == 3 || static_cast<int>(info.note) == 6 || static_cast<int>(info.note) == 8 || static_cast<int>(info.note) == 10;
    }


    // Function to calculate note duration in milliseconds based on note type and time signature
    uint16_t PitchPerception::getNoteDuration(NoteType note, TimeSignature timeSignature) 
    {
        // Base duration for a quarter note in 4/4 time (default reference point)
        uint16_t baseDuration = WHOLE_NOTE_MS / 4;

        // Adjust base duration based on the time signature denominator
        switch (timeSignature) 
        {
            case TimeSignature::TIME_2_2: 
                baseDuration = WHOLE_NOTE_MS / 2;  // Half note gets the beat
                break;

            case TimeSignature::TIME_4_4: 
                baseDuration = WHOLE_NOTE_MS / 4;  // Quarter note gets the beat
                break;

            case TimeSignature::TIME_6_8: 
                baseDuration = WHOLE_NOTE_MS / 8;  // Eighth note gets the beat
                break;

            case TimeSignature::TIME_12_16: 
                baseDuration = WHOLE_NOTE_MS / 16;  // Sixteenth note gets the beat
                break;

            default: 
                baseDuration = WHOLE_NOTE_MS / 4;  // Default to quarter note
                break;
        }

        // Scale the note duration based on the note type
        switch (note) 
        {
            case NoteType::WHOLE: return baseDuration * 4;        // Whole note
            case NoteType::HALF: return baseDuration * 2;         // Half note
            case NoteType::QUARTER: return baseDuration;          // Quarter note
            case NoteType::EIGHTH: return baseDuration / 2;       // Eighth note
            case NoteType::SIXTEENTH: return baseDuration / 4;    // Sixteenth note
            case NoteType::THIRTY_SECOND: return baseDuration / 8; // Thirty-second note
            case NoteType::SIXTY_FOURTH: return baseDuration / 16; // Sixty-fourth note
            case NoteType::HUNDRED_TWENTY_EIGHTH: return baseDuration / 32; // Hundred-twenty-eighth note
            default: return baseDuration;               // Default to quarter note
        }
    }
}#ifndef SOUND_FX_MANAGER_H
#define SOUND_FX_MANAGER_H

#include "../CorpusCallosum/SynapticPathways.h"
#include "../AuditoryCortex/PitchPerception.h"
#include "../PrefrontalCortex/utilities.h"
#include "../PrefrontalCortex/ProtoPerceptions.h"
#include "../VisualCortex/RoverManager.h"
#include "../MotorCortex/PinDefinitions.h"
#include "Tunes.h"

#include <time.h>
#include <SPIFFS.h>
#include <FS.h>
#include "Audio.h"
#include <SD.h>

namespace AuditoryCortex
{
    using namespace CorpusCallosum;
    using VC::RoverManager;
    using VC::LEDManager;
    using PC::Utilities;
    using MC::PinDefinitions;
    using PC::AudioTypes::ErrorTone;
    using PC::AudioTypes::WavHeaderInfo;
    using PC::AudioTypes::TunesTypes;
    using PC::AudioTypes::Tune;
    using PC::AudioTypes::NoteInfo;
    using PC::AudioTypes::TimeSignature;

    // I2S Configuration Constants
    static const int EXAMPLE_I2S_CH = 0;  // I2S channel number
    static const int EXAMPLE_SAMPLE_RATE = 44100;  // Sample rate in Hz
    static const int WAVE_HEADER_SIZE = 44;  // WAV header size in bytes
    static const int BYTE_RATE = (EXAMPLE_SAMPLE_RATE * 2);  // 16-bit mono = 2 bytes per sample

    class SoundFxManager {
    private:
        /* ========================== Private Members ========================== */
        
        // Jingle-related state
        static const int JINGLE_LENGTH;
        static bool m_isTunePlaying;
        static int currentNote;
        static unsigned long lastNoteTime;
        static bool jinglePlaying;

        // Recording state
        static bool isRecording;
        static File recordFile;
        static Audio audio;
        static bool isPlayingSound;

        // Jingle metadata
        static const char* RECORD_FILENAME;
        static bool isJingleActive;
        static unsigned long jingleStartTime;
        static int currentJingleNote;

        // Initialization and settings
        static bool _isInitialized;
        static int volume;

        // Selected song state
        static TunesTypes selectedSong;
        static Tune activeTune;
        static int activeTuneLength;

        /* ========================== Private Methods ========================== */
        static void init_microphone();
        static void initializeAudio();
        static void generate_wav_header(char* wav_header, uint32_t wav_size, uint32_t sample_rate);

    public:
        /* ========================== Core Functionality ========================== */
        static void init();
        static bool isInitialized() { return _isInitialized; }

        /* ========================== Playback Functions ========================== */
        static void playErrorSound(int type);
        static void playTune(PC::AudioTypes::TunesTypes type);
        static void playTone(int frequency, int duration, int position = 0); // Custom tone playback
        static void playRotaryPressSound(int mode = 0);
        static void playRotaryTurnSound(bool clockwise);
        static void playSideButtonSound(bool start = false);
        static void playStartupSound(){ playTune(TunesTypes::ROVERBYTE_JINGLE); }
        static void playSuccessSound();
        static void playTimerDropSound(CRGB color);
        static void playMenuCloseSound();
        static void playMenuOpenSound();
        static void playMenuSelectSound();
        static void playVoiceLine(const char* line, uint32_t cardId = 0);
        static void playCardMelody(uint32_t cardId);

        /* ========================== Jingle Control ========================== */
        static void startTune();
        static void updateTune();
        static bool isTunePlaying() { return m_isTunePlaying; }
        static void stopTune() { m_isTunePlaying = false; }

        /* ========================== Volume Control ========================== */
        static void adjustVolume(int amount);

        /* ========================== Recording Functionality ========================== */
        static void startRecording();
        static void stopRecording();
        static void playRecording();
        static bool isCurrentlyRecording() { return isRecording; }
        static bool isCurrentlyPlaying() { return isPlayingSound; }
        static bool isPlaying() { return isPlayingSound; }

        /* ========================== Error Playback ========================== */
        static void playErrorCode(uint32_t errorCode, bool isFatal);

        /* ========================== Update Function ========================== */
        static void update() {
            if (m_isTunePlaying) {
                updateTune();
            }
            if (isPlayingSound) {
                audio_eof_mp3("update");
            }
        }

        /* ========================== Audio Event Callbacks ========================== */
        static void audio_eof_mp3(const char* info);
    };
}

#endif // SOUND_FX_MANAGER_H#include "../CorpusCallosum/SynapticPathways.h"
#include "PitchPerception.h"
#include "SoundFxManager.h"
#include "Arduino.h"
#include <time.h>
#include <SPIFFS.h>
#include "../PrefrontalCortex/SDManager.h"
#include "../VisualCortex/RoverViewManager.h"
#include "../VisualCortex/RoverManager.h"
#include "../VisualCortex/LEDManager.h"
#include "Tunes.h"

namespace AuditoryCortex
{
    using namespace CorpusCallosum;
    using PC::AudioTypes::NoteInfo;
    using PC::AudioTypes::TimeSignature;
    using PC::AudioTypes::NoteIndex;
    using PC::AudioTypes::NoteType;
    using PC::AudioTypes::Tune;
    using PC::AudioTypes::TunesTypes;
    using PC::Utilities;
    using VC::RoverManager;
    using VC::LEDManager;

    // RoverByte's Anthem: Quantum Tails
    const Tune Tunes::ROVERBYTE_JINGLE = {
        "Quantum Tails",
        {
            {NoteIndex::NOTE_G, 4, NoteType::QUARTER},
            {NoteIndex::NOTE_A, 4, NoteType::EIGHTH},
            {NoteIndex::NOTE_B, 4, NoteType::EIGHTH},
            {NoteIndex::REST, 0, NoteType::EIGHTH},
            {NoteIndex::NOTE_C, 5, NoteType::QUARTER},
            {NoteIndex::NOTE_E, 5, NoteType::QUARTER},
            {NoteIndex::NOTE_D, 5, NoteType::EIGHTH},
            {NoteIndex::REST, 0, NoteType::QUARTER},
            {NoteIndex::NOTE_F, 5, NoteType::HALF},
            {NoteIndex::NOTE_A, 4, NoteType::EIGHTH},
            {NoteIndex::NOTE_C, 5, NoteType::QUARTER},
            {NoteIndex::NOTE_B, 4, NoteType::EIGHTH},
            {NoteIndex::NOTE_G, 4, NoteType::QUARTER},
            {NoteIndex::REST, 0, NoteType::QUARTER},
            {NoteIndex::NOTE_D, 5, NoteType::HALF},
            {NoteIndex::NOTE_G, 4, NoteType::WHOLE}
        },
        {
            0b00000001, 0b00000010, 0b00000100, 0b00001000,
            0b00010000, 0b00100000, 0b01000000, 0b10000000,
            0b11000000, 0b01100000, 0b00110000, 0b00011000,
            0b00001100, 0b00000110, 0b00000011, 0b00000001
        },
        PC::AudioTypes::TimeSignature::TIME_6_8
    };

    // Cosmic Reflections: Painted Skies
    const Tune Tunes::CHRISTMAS_SONG = {
        "Painted Skies",
        {
            {NoteIndex::NOTE_C, 4, NoteType::QUARTER},
            {NoteIndex::NOTE_G, 4, NoteType::EIGHTH},
            {NoteIndex::NOTE_A, 4, NoteType::EIGHTH},
            {NoteIndex::REST, 0, NoteType::QUARTER},
            {NoteIndex::NOTE_B, 4, NoteType::HALF},
            {NoteIndex::NOTE_D, 5, NoteType::QUARTER},
            {NoteIndex::NOTE_E, 5, NoteType::EIGHTH},
            {NoteIndex::NOTE_C, 5, NoteType::EIGHTH},
            {NoteIndex::REST, 0, NoteType::QUARTER},
            {NoteIndex::NOTE_G, 4, NoteType::QUARTER},
            {NoteIndex::NOTE_F, 4, NoteType::QUARTER},
            {NoteIndex::NOTE_E, 4, NoteType::QUARTER},
            {NoteIndex::NOTE_C, 4, NoteType::QUARTER},
            {NoteIndex::NOTE_A, 4, NoteType::HALF}
        },
        {
            0b00000001, 0b00000011, 0b00000111, 0b00001110,
            0b00011100, 0b00111000, 0b01110000, 0b11100000,
            0b11000000, 0b10000001, 0b00000001, 0b00000111,
            0b11111111, 0b00000000, 0b00000001
        },
        PC::AudioTypes::TimeSignature::TIME_4_4
    };

    // Reflections of Unity: Symphonic Threads
    const Tune Tunes::AULD_LANG_SYNE = {
        "Symphonic Threads",
        {
            {NoteIndex::NOTE_E, 4, NoteType::EIGHTH},
            {NoteIndex::NOTE_F, 4, NoteType::EIGHTH},
            {NoteIndex::NOTE_G, 4, NoteType::QUARTER},
            {NoteIndex::REST, 0, NoteType::EIGHTH},
            {NoteIndex::NOTE_A, 4, NoteType::QUARTER},
            {NoteIndex::NOTE_B, 4, NoteType::QUARTER},
            {NoteIndex::NOTE_C, 5, NoteType::HALF},
            {NoteIndex::NOTE_D, 5, NoteType::QUARTER},
            {NoteIndex::NOTE_C, 5, NoteType::QUARTER},
            {NoteIndex::NOTE_A, 4, NoteType::HALF},
            {NoteIndex::NOTE_B, 4, NoteType::QUARTER},
            {NoteIndex::REST, 0, NoteType::QUARTER},
            {NoteIndex::NOTE_C, 5, NoteType::HALF},
            {NoteIndex::NOTE_E, 5, NoteType::QUARTER}
        },
        {
            0b00000001, 0b10000001, 0b11000011, 0b11100111,
            0b11111111, 0b11100111, 0b11000011, 0b10000001,
            0b00000001, 0b10001000, 0b11111111, 0b00000000,
            0b00001100, 0b11110000
        },
        PC::AudioTypes::TimeSignature::TIME_3_4
    };

    const Tune Tunes::JINGLE_BELLS = {
        "Winter Dance",
        {
            {NoteIndex::NOTE_E, 4, NoteType::QUARTER}, // 1
            {NoteIndex::NOTE_E, 4, NoteType::QUARTER}, // 2
            {NoteIndex::NOTE_E, 4, NoteType::HALF},    // 3
            {NoteIndex::NOTE_E, 4, NoteType::QUARTER}, // 4
            {NoteIndex::NOTE_E, 4, NoteType::QUARTER}, // 5
            {NoteIndex::NOTE_E, 4, NoteType::HALF},    // 6
            {NoteIndex::NOTE_E, 4, NoteType::QUARTER}, // 7
            {NoteIndex::NOTE_G, 4, NoteType::QUARTER}, // 8
            {NoteIndex::NOTE_C, 5, NoteType::QUARTER}, // 9
            {NoteIndex::NOTE_D, 5, NoteType::QUARTER}, // 10
            {NoteIndex::NOTE_E, 5, NoteType::WHOLE},   // 11
            {NoteIndex::NOTE_F, 5, NoteType::QUARTER}, // 12
            {NoteIndex::NOTE_F, 5, NoteType::EIGHTH},  // 13
            {NoteIndex::NOTE_F, 5, NoteType::EIGHTH},  // 14
            {NoteIndex::NOTE_F, 5, NoteType::HALF},    // 15
            {NoteIndex::NOTE_E, 5, NoteType::QUARTER}, // 16
            {NoteIndex::NOTE_D, 5, NoteType::QUARTER}, // 17
            {NoteIndex::NOTE_C, 5, NoteType::HALF},    // 18
            {NoteIndex::NOTE_G, 4, NoteType::QUARTER}, // 19
            {NoteIndex::NOTE_E, 4, NoteType::QUARTER}, // 20
            {NoteIndex::REST, 0, NoteType::HALF}       // 21
        },
        {
            0b00000001, // Swirl starts with LED 0
            0b00010001, // Pair 0/4 lights up
            0b00100010, // Pair 1/5 lights up
            0b01000100, // Pair 2/6 lights up
            0b10001000, // Pair 3/7 lights up
            0b11111111, // All LEDs on
            0b00000001, // Reset to single light
            0b00110000, // LED 4 fades in, and so on
            0b00001110, // LEDs swirl inward
            0b11100111, // Symmetric ripple outward
            0b11111111, // Bright pulse for "E5 WHOLE"
            0b00010001, // Subtle shimmer (Pair 0/4)
            0b00100010, // (Pair 1/5)
            0b01000100, // (Pair 2/6)
            0b10001000, // (Pair 3/7)
            0b11111111, // Intense flash
            0b00100010, // Pair fade
            0b00010001, // Back to Pair 0/4
            0b00000001, // Single LED on LED 0
            0b00000000  // Rest (all LEDs off)
        },
        PC::AudioTypes::TimeSignature::TIME_4_4
    };

    const Tune Tunes::LOVE_SONG = {
        "Entangled Hearts",
        {
            {NoteIndex::NOTE_A, 4, NoteType::EIGHTH},  // 1
            {NoteIndex::NOTE_C, 5, NoteType::QUARTER}, // 2
            {NoteIndex::NOTE_E, 5, NoteType::QUARTER}, // 3
            {NoteIndex::NOTE_G, 5, NoteType::EIGHTH},  // 4
            {NoteIndex::NOTE_F, 5, NoteType::EIGHTH},  // 5
            {NoteIndex::NOTE_E, 5, NoteType::QUARTER}, // 6
            {NoteIndex::NOTE_D, 5, NoteType::HALF},    // 7
            {NoteIndex::REST, 0, NoteType::EIGHTH},    // 8
            {NoteIndex::NOTE_C, 5, NoteType::QUARTER}, // 9
            {NoteIndex::NOTE_A, 4, NoteType::QUARTER}, // 10
            {NoteIndex::NOTE_B, 4, NoteType::HALF},    // 11
            {NoteIndex::NOTE_C, 5, NoteType::QUARTER}, // 12
            {NoteIndex::NOTE_E, 5, NoteType::QUARTER}, // 13
            {NoteIndex::NOTE_G, 5, NoteType::EIGHTH},  // 14
            {NoteIndex::NOTE_A, 5, NoteType::HALF},    // 15
            {NoteIndex::REST, 0, NoteType::EIGHTH},    // 16
            {NoteIndex::NOTE_F, 5, NoteType::QUARTER}, // 17
            {NoteIndex::NOTE_E, 5, NoteType::EIGHTH},  // 18
            {NoteIndex::NOTE_C, 5, NoteType::EIGHTH},  // 19
            {NoteIndex::NOTE_A, 4, NoteType::QUARTER}, // 20
            {NoteIndex::NOTE_F, 4, NoteType::HALF},    // 21
            {NoteIndex::NOTE_G, 4, NoteType::QUARTER}, // 22
            {NoteIndex::NOTE_A, 4, NoteType::HALF},    // 23
            {NoteIndex::NOTE_C, 5, NoteType::WHOLE}    // 24
        },
        {
            0b00011000, // Center glow starts
            0b00111100, // Expanding heart
            0b01111110, // Full brightness
            0b11111111, // Intense pulse
            0b01111110, // Contracting heart
            0b00111100, // Shrinking
            0b00011000, // Back to subtle glow
            0b00000000, // Rest: All off
            0b00011000, // Gentle pulse restart
            0b00111100, // Building intensity
            0b01111110, // Heartbeat synchronization
            0b11111111, // Full pulse
            0b01111110, // Retreat to calm
            0b00111100, // Gentle glow
            0b00011000, // Slow pulse
            0b00000000, // Rest: Dark pause
            0b00011000, // Restart light heartbeat
            0b00111100, // Grow again
            0b01111110, // Peak pulse
            0b11111111, // Full brightness
            0b01111110, // Dim down
            0b00111100, // Fade
            0b00011000, // Subtle light
            0b00000000  // Final rest
        },
        PC::AudioTypes::TimeSignature::TIME_6_8
    };

    const Tune Tunes::HAPPY_BIRTHDAY = {
        "Celebration Sparks",
        {
            {NoteIndex::NOTE_C, 4, NoteType::QUARTER},  // 1
            {NoteIndex::NOTE_C, 4, NoteType::EIGHTH},  // 2
            {NoteIndex::NOTE_D, 4, NoteType::QUARTER}, // 3
            {NoteIndex::NOTE_C, 4, NoteType::EIGHTH},  // 4
            {NoteIndex::NOTE_F, 4, NoteType::HALF},    // 5
            {NoteIndex::NOTE_E, 4, NoteType::QUARTER}, // 6
            {NoteIndex::NOTE_C, 4, NoteType::QUARTER}, // 7
            {NoteIndex::NOTE_C, 4, NoteType::QUARTER}, // 8
            {NoteIndex::NOTE_D, 4, NoteType::QUARTER}, // 9
            {NoteIndex::NOTE_C, 4, NoteType::EIGHTH},  // 10
            {NoteIndex::NOTE_G, 4, NoteType::HALF},    // 11
            {NoteIndex::NOTE_F, 4, NoteType::HALF},    // 12
            {NoteIndex::NOTE_C, 4, NoteType::QUARTER}, // 13
            {NoteIndex::NOTE_C, 4, NoteType::EIGHTH},  // 14
            {NoteIndex::NOTE_D, 4, NoteType::QUARTER}, // 15
            {NoteIndex::NOTE_C, 4, NoteType::EIGHTH},  // 16
            {NoteIndex::NOTE_C, 4, NoteType::HALF},    // 17
            {NoteIndex::NOTE_C, 4, NoteType::QUARTER}, // 18
            {NoteIndex::REST, 0, NoteType::HALF}       // 19
        },
        {
            0b00011000, // Heartbeat glow
            0b00111100, // Expand
            0b01111110, // Full burst
            0b11111111, // Firework explosion
            0b01111110, // Retract
            0b00111100, // Calm fade
            0b00011000, // Gentle light
            0b00111100, // Glow grows
            0b11111111, // Bright explosion
            0b01111110, // Fade again
            0b11111111, // Climax burst
            0b00011000, // Calm glow
            0b00001100, // Shrink light inward
            0b00111100, // Bright again
            0b01111110, // Another celebration burst
            0b11111111, // Flash out
            0b00111100, // Fade to dim
            0b00000000  // Rest
        },
        PC::AudioTypes::TimeSignature::TIME_3_4
    };

    const Tune Tunes::EASTER_SONG = {
        "Spring Awakening",
        {
            {NoteIndex::NOTE_E, 4, NoteType::QUARTER},  // 1
            {NoteIndex::NOTE_G, 4, NoteType::QUARTER},  // 2
            {NoteIndex::NOTE_A, 4, NoteType::EIGHTH},   // 3
            {NoteIndex::NOTE_B, 4, NoteType::EIGHTH},   // 4
            {NoteIndex::NOTE_C, 5, NoteType::HALF},     // 5
            {NoteIndex::NOTE_B, 4, NoteType::QUARTER},  // 6
            {NoteIndex::NOTE_A, 4, NoteType::QUARTER},  // 7
            {NoteIndex::REST, 0, NoteType::EIGHTH},     // 8
            {NoteIndex::NOTE_G, 4, NoteType::QUARTER},  // 9
            {NoteIndex::NOTE_E, 4, NoteType::QUARTER},  // 10
            {NoteIndex::NOTE_F, 4, NoteType::QUARTER},  // 11
            {NoteIndex::NOTE_G, 4, NoteType::HALF},     // 12
            {NoteIndex::NOTE_A, 4, NoteType::QUARTER},  // 13
            {NoteIndex::NOTE_B, 4, NoteType::QUARTER},  // 14
            {NoteIndex::NOTE_C, 5, NoteType::HALF},     // 15
            {NoteIndex::NOTE_D, 5, NoteType::QUARTER},  // 16
            {NoteIndex::NOTE_E, 5, NoteType::WHOLE}     // 17
        },
        {
            0b00000001, // Small bud (LED 0)
            0b00000011, // Bud grows outward (0/1)
            0b00000111, // Expanding (0/1/2)
            0b00001110, // More bloom (1/2/3)
            0b00011111, // Full bloom
            0b00111111, // Brighter
            0b01111110, // Calm retreat
            0b11111111, // Full light
            0b00111110, // Dim down
            0b00011100, // Almost gone
            0b00001100, // Fading bloom
            0b00000100, // Just a petal remains
            0b00000000, // Rest
            0b00000001, // Restart small bud
            0b00001111, // Faster bloom
            0b11111111, // Full spring
            0b00000000  // Rest
        },
        PC::AudioTypes::TimeSignature::TIME_6_8
    };

    const Tune Tunes::MOTHERS_SONG = {
        "Heart of the Home",
        {
            {NoteIndex::NOTE_C, 4, NoteType::HALF},     // 1
            {NoteIndex::NOTE_E, 4, NoteType::QUARTER},  // 2
            {NoteIndex::NOTE_G, 4, NoteType::QUARTER},  // 3
            {NoteIndex::NOTE_A, 4, NoteType::HALF},     // 4
            {NoteIndex::NOTE_F, 4, NoteType::QUARTER},  // 5
            {NoteIndex::NOTE_D, 4, NoteType::HALF},     // 6
            {NoteIndex::NOTE_E, 4, NoteType::QUARTER},  // 7
            {NoteIndex::NOTE_C, 4, NoteType::HALF},     // 8
            {NoteIndex::NOTE_G, 4, NoteType::QUARTER},  // 9
            {NoteIndex::REST, 0, NoteType::HALF}        // 10
        },
        {
            0b00110000, // Gentle pulse starts
            0b01111000, // Glow spreads
            0b11111100, // Full warmth
            0b11111111, // Peak heartbeat
            0b01111000, // Retract
            0b00110000, // Heartbeat fades
            0b00011000, // Smaller pulse
            0b00001100, // Retreat further
            0b00000100, // Dim light
            0b00000000  // Rest
        },
        PC::AudioTypes::TimeSignature::TIME_4_4
    };

    const Tune Tunes::FATHERS_SONG = {
        "Pillars of Strength",
        {
            {NoteIndex::NOTE_C, 4, NoteType::HALF},     // 1
            {NoteIndex::NOTE_G, 4, NoteType::QUARTER},  // 2
            {NoteIndex::NOTE_A, 4, NoteType::QUARTER},  // 3
            {NoteIndex::NOTE_F, 4, NoteType::HALF},     // 4
            {NoteIndex::NOTE_D, 4, NoteType::QUARTER},  // 5
            {NoteIndex::NOTE_E, 4, NoteType::QUARTER},  // 6
            {NoteIndex::NOTE_C, 4, NoteType::HALF},     // 7
            {NoteIndex::NOTE_G, 4, NoteType::QUARTER},  // 8
            {NoteIndex::NOTE_A, 4, NoteType::QUARTER},  // 9
            {NoteIndex::NOTE_C, 4, NoteType::WHOLE}     // 10
        },
        {
            0b00010001, // Solid base (0/4)
            0b00100010, // Pairing (1/5)
            0b01000100, // Strength builds (2/6)
            0b10001000, // Broad foundation (3/7)
            0b11111111, // Stability across all LEDs
            0b01000100, // Retreat to pillars (2/6)
            0b00100010, // Refocus (1/5)
            0b00010001, // Narrowed strength (0/4)
            0b11111111, // Full stability pulse
            0b00000000  // Rest
        },
        PC::AudioTypes::TimeSignature::TIME_4_4
    };

    const Tune Tunes::CANADA_SONG = {
        "Northern Lights",
        {
            {NoteIndex::NOTE_C, 5, NoteType::QUARTER},  // 1
            {NoteIndex::NOTE_E, 5, NoteType::QUARTER},  // 2
            {NoteIndex::NOTE_G, 5, NoteType::QUARTER},  // 3
            {NoteIndex::NOTE_F, 5, NoteType::QUARTER},  // 4
            {NoteIndex::NOTE_A, 4, NoteType::HALF},     // 5
            {NoteIndex::NOTE_B, 4, NoteType::QUARTER},  // 6
            {NoteIndex::NOTE_C, 5, NoteType::QUARTER},  // 7
            {NoteIndex::NOTE_D, 5, NoteType::HALF},     // 8
            {NoteIndex::NOTE_E, 5, NoteType::WHOLE}     // 9
        },
        {
            0b00000001, // Spark at LED 0
            0b00000011, // Wave grows (0/1)
            0b00000111, // Expands further (0/1/2)
            0b00001111, // Adding (0/1/2/3)
            0b11111111, // Aurora peak across LEDs
            0b00011111, // Retreat begins (3/2/1)
            0b00001111, // Light condenses (2/1/0)
            0b00000011, // Narrow glow
            0b00000001  // Fade to rest
        },
        PC::AudioTypes::TimeSignature::TIME_6_8
    };

    const Tune Tunes::USA_SONG = {
        "Stars and Stripes",
        {
            {NoteIndex::NOTE_G, 4, NoteType::QUARTER},  // 1
            {NoteIndex::NOTE_C, 5, NoteType::QUARTER},  // 2
            {NoteIndex::NOTE_D, 5, NoteType::QUARTER},  // 3
            {NoteIndex::NOTE_E, 5, NoteType::HALF},     // 4
            {NoteIndex::NOTE_F, 5, NoteType::QUARTER},  // 5
            {NoteIndex::NOTE_E, 5, NoteType::HALF},     // 6
            {NoteIndex::NOTE_D, 5, NoteType::QUARTER},  // 7
            {NoteIndex::NOTE_C, 5, NoteType::WHOLE}     // 8
        },
        {
            0b10101010, // Stars (pairs 0/4, 1/5, 2/6, 3/7 alternating)
            0b01010101, // Stripes flip
            0b10101010, // Alternating again
            0b11111111, // Bold full brightness
            0b10000001, // Ends with outer LEDs lit
            0b01000010, // Central symmetry grows
            0b00111100, // Flag waves inward
            0b11111111  // Bright pulse of unity
        },
        PC::AudioTypes::TimeSignature::TIME_4_4
    };

    const Tune Tunes::CIVIC_SONG = {
        "Unity in Motion",
        {
            {NoteIndex::NOTE_D, 4, NoteType::QUARTER},  // 1
            {NoteIndex::NOTE_E, 4, NoteType::QUARTER},  // 2
            {NoteIndex::NOTE_F, 4, NoteType::HALF},     // 3
            {NoteIndex::NOTE_G, 4, NoteType::QUARTER},  // 4
            {NoteIndex::NOTE_A, 4, NoteType::HALF},     // 5
            {NoteIndex::NOTE_B, 4, NoteType::QUARTER},  // 6
            {NoteIndex::NOTE_G, 4, NoteType::WHOLE}     // 7
        },
        {
            0b00000001, // LED 0 starts
            0b00000011, // Pair 0/1 grow
            0b00000111, // Circle spreads outward
            0b00001111, // Halfway expansion
            0b11111111, // Full brightness circle
            0b11110000, // Retreat backward
            0b00000000  // Rest
        },
        PC::AudioTypes::TimeSignature::TIME_4_4
    };

    const Tune Tunes::WAKE_ME_UP_WHEN_SEPTEMBER_ENDS = {
        "Autumn's Farewell",
        {
            {NoteIndex::NOTE_E, 4, NoteType::QUARTER},
            {NoteIndex::NOTE_G, 4, NoteType::QUARTER},
            {NoteIndex::NOTE_F, 4, NoteType::HALF},
            {NoteIndex::NOTE_E, 4, NoteType::QUARTER},
            {NoteIndex::REST, 0, NoteType::QUARTER},
            {NoteIndex::NOTE_D, 4, NoteType::QUARTER},
            {NoteIndex::NOTE_C, 4, NoteType::HALF},
            {NoteIndex::NOTE_E, 4, NoteType::QUARTER},
            {NoteIndex::NOTE_F, 4, NoteType::QUARTER},
            {NoteIndex::NOTE_G, 4, NoteType::HALF},
            {NoteIndex::REST, 0, NoteType::EIGHTH},
            {NoteIndex::NOTE_A, 4, NoteType::QUARTER},
            {NoteIndex::NOTE_G, 4, NoteType::QUARTER},
            {NoteIndex::NOTE_F, 4, NoteType::HALF},
            {NoteIndex::NOTE_C, 4, NoteType::QUARTER},
            {NoteIndex::NOTE_E, 4, NoteType::HALF},
            {NoteIndex::NOTE_A, 4, NoteType::QUARTER},
            {NoteIndex::NOTE_F, 4, NoteType::QUARTER},
            {NoteIndex::NOTE_G, 4, NoteType::HALF},
            {NoteIndex::REST, 0, NoteType::QUARTER},
            {NoteIndex::NOTE_D, 4, NoteType::QUARTER},
            {NoteIndex::NOTE_C, 4, NoteType::WHOLE},
            {NoteIndex::NOTE_A, 4, NoteType::HALF},
            {NoteIndex::NOTE_F, 4, NoteType::QUARTER},
            {NoteIndex::NOTE_G, 4, NoteType::QUARTER},
            {NoteIndex::NOTE_E, 4, NoteType::HALF},
            {NoteIndex::NOTE_D, 4, NoteType::QUARTER},
            {NoteIndex::NOTE_C, 4, NoteType::WHOLE},
            {NoteIndex::REST, 0, NoteType::HALF},
            {NoteIndex::NOTE_A, 4, NoteType::WHOLE}
        },
        {
            0b11111111,
            0b01111110,
            0b00111100,
            0b00011000,
            0b00000000,
            0b00010000,
            0b00111000,
            0b00011100,
            0b00001100,
            0b00000011,
            0b00000000,
            0b11110000,
            0b01111000,
            0b00011100,
            0b00001110,
            0b00000110,
            0b00000011,
            0b00000000,
            0b11111111,
            0b00111000,
            0b01111110,
            0b11111111,
            0b01111110,
            0b00011000,
            0b00000001,
            0b00000000,
            0b11111111,
            0b00000000
        },
        PC::AudioTypes::TimeSignature::TIME_4_4
    };

    const Tune Tunes::HALLOWEEN_SONG = {
        "Phantom Waltz",
        {
            {NoteIndex::NOTE_E, 5, NoteType::QUARTER},  // 1
            {NoteIndex::NOTE_D, 5, NoteType::QUARTER},  // 2
            {NoteIndex::NOTE_C, 5, NoteType::HALF},     // 3
            {NoteIndex::REST, 0, NoteType::QUARTER},    // 4
            {NoteIndex::NOTE_B, 4, NoteType::QUARTER},  // 5
            {NoteIndex::NOTE_G, 4, NoteType::EIGHTH},   // 6
            {NoteIndex::NOTE_A, 4, NoteType::EIGHTH},   // 7
            {NoteIndex::NOTE_C, 5, NoteType::QUARTER},  // 8
            {NoteIndex::NOTE_E, 5, NoteType::HALF},     // 9
            {NoteIndex::REST, 0, NoteType::HALF},       // 10
            {NoteIndex::NOTE_D, 5, NoteType::QUARTER},  // 11
            {NoteIndex::NOTE_C, 5, NoteType::QUARTER},  // 12
            {NoteIndex::NOTE_B, 4, NoteType::HALF},     // 13
            {NoteIndex::NOTE_G, 4, NoteType::QUARTER},  // 14
            {NoteIndex::NOTE_A, 4, NoteType::HALF},     // 15
            {NoteIndex::NOTE_C, 5, NoteType::WHOLE}     // 16
        },
        {
            0b00000001, // Ghostly flicker (LED 0)
            0b00000100, // LED 2 lights up
            0b00001000, // LED 3 flickers on
            0b10000000, // LED 7 appears suddenly
            0b11000000, // LEDs 6 and 7 shimmer together
            0b01100000, // LEDs 5 and 6 flicker
            0b00011000, // Focus on LEDs 3 and 4
            0b00100100, // LEDs 2 and 5 light up
            0b00010001, // Symmetry between LEDs 0 and 4
            0b11111111, // All LEDs brighten
            0b01111110, // Gradual retreat inward
            0b00011000, // Back to subtle glow
            0b00000001, // Single flicker at LED 0
            0b00000000, // Rest
            0b00111100, // Intense middle flash
            0b11111111  // Final full brightness
        },
        PC::AudioTypes::TimeSignature::TIME_3_4
    };

    const Tune Tunes::THANKSGIVING_SONG = {
        "Harvest Hymn",
        {
            {NoteIndex::NOTE_C, 4, NoteType::EIGHTH},   // 1
            {NoteIndex::NOTE_E, 4, NoteType::QUARTER},  // 2
            {NoteIndex::NOTE_G, 4, NoteType::HALF},     // 3
            {NoteIndex::NOTE_A, 4, NoteType::QUARTER},  // 4
            {NoteIndex::NOTE_F, 4, NoteType::QUARTER},  // 5
            {NoteIndex::NOTE_D, 4, NoteType::EIGHTH},   // 6
            {NoteIndex::NOTE_B, 4, NoteType::EIGHTH},   // 7
            {NoteIndex::NOTE_C, 5, NoteType::HALF},     // 8
            {NoteIndex::NOTE_D, 5, NoteType::QUARTER},  // 9
            {NoteIndex::NOTE_E, 5, NoteType::QUARTER},  // 10
            {NoteIndex::NOTE_F, 5, NoteType::HALF},     // 11
            {NoteIndex::NOTE_E, 5, NoteType::QUARTER},  // 12
            {NoteIndex::NOTE_C, 5, NoteType::QUARTER},  // 13
            {NoteIndex::NOTE_A, 4, NoteType::HALF},     // 14
            {NoteIndex::NOTE_G, 4, NoteType::WHOLE}     // 15
        },
        {
            0b00000001, // Seed planted (LED 0)
            0b00000011, // Growth outward (LEDs 0/1)
            0b00000111, // Small sprout (LEDs 0/1/2)
            0b00001110, // Bloom begins (LEDs 1/2/3)
            0b00011100, // Expands toward center
            0b00111100, // LEDs 2/3/4 shine brightly
            0b01111110, // Almost full harvest
            0b11111111, // Abundance achieved
            0b01111110, // Gradual fade
            0b00111100, // LED 3 and surroundings dim
            0b00011100, // LED 3 glows faintly
            0b00001110, // Light shifts inward
            0b00000110, // LED 2 dims gently
            0b00000011, // Light narrows
            0b00000001  // Final flicker
        },
        PC::AudioTypes::TimeSignature::TIME_6_8
    };

    int Tunes::getTuneLength(TunesTypes type) 
    {
        switch (type) 
        {
            case TunesTypes::ROVERBYTE_JINGLE: 
                return ROVERBYTE_JINGLE.notes.size();
            case TunesTypes::CHRISTMAS_SONG: 
                return CHRISTMAS_SONG.notes.size();
            case TunesTypes::AULD_LANG_SYNE: 
                return AULD_LANG_SYNE.notes.size();
            case TunesTypes::JINGLE_BELLS: 
                return JINGLE_BELLS.notes.size();
            case TunesTypes::LOVE_SONG: 
                return LOVE_SONG.notes.size();
            case TunesTypes::FATHERS_SONG: 
                return FATHERS_SONG.notes.size();
            case TunesTypes::CANADA_SONG: 
                return CANADA_SONG.notes.size();
            case TunesTypes::USA_SONG: 
                return USA_SONG.notes.size();
            case TunesTypes::CIVIC_SONG: 
                return CIVIC_SONG.notes.size();
            case TunesTypes::WAKE_ME_UP_WHEN_SEPTEMBER_ENDS: 
                return WAKE_ME_UP_WHEN_SEPTEMBER_ENDS.notes.size();
            case TunesTypes::HALLOWEEN_SONG: 
                return HALLOWEEN_SONG.notes.size();
            case TunesTypes::THANKSGIVING_SONG: 
                return THANKSGIVING_SONG.notes.size();
            default: 
                return ROVERBYTE_JINGLE.notes.size(); // Fallback
        }
    }

    // Get tune based on the type
    Tune Tunes::getTune(TunesTypes type) 
    {
        switch (type) 
        {
            case TunesTypes::CHRISTMAS_SONG: 
                return CHRISTMAS_SONG;
            case TunesTypes::AULD_LANG_SYNE: 
                return AULD_LANG_SYNE;
            case TunesTypes::JINGLE_BELLS: 
                return JINGLE_BELLS;
            case TunesTypes::LOVE_SONG: 
                return LOVE_SONG;
            case TunesTypes::FATHERS_SONG: 
                return FATHERS_SONG;
            case TunesTypes::CANADA_SONG: 
                return CANADA_SONG;
            case TunesTypes::USA_SONG: 
                return USA_SONG;
            case TunesTypes::CIVIC_SONG: 
                return CIVIC_SONG;
            case TunesTypes::WAKE_ME_UP_WHEN_SEPTEMBER_ENDS: 
                return WAKE_ME_UP_WHEN_SEPTEMBER_ENDS;
            case TunesTypes::HALLOWEEN_SONG: 
                return HALLOWEEN_SONG;
            case TunesTypes::THANKSGIVING_SONG: 
                return THANKSGIVING_SONG;
            case TunesTypes::ROVERBYTE_JINGLE:
                return ROVERBYTE_JINGLE;
            default: 
                return ROVERBYTE_JINGLE; // Fallback
        }
    }

    int Tunes::timeSignatureToDelay(TimeSignature timeSignature) 
    {
        const int DEFAULT_TEMPO_BPM = 120; // Default tempo (beats per minute)
        const int MILLISECONDS_PER_MINUTE = 60000;

        // Calculate the duration of one beat in milliseconds
        int beatDurationMs = MILLISECONDS_PER_MINUTE / DEFAULT_TEMPO_BPM;

        switch (timeSignature) 
        {
            case TimeSignature::TIME_2_2:
                // Half note gets the beat, so the delay is 2x beat duration
                return beatDurationMs * 2;

            case TimeSignature::TIME_4_4:
                // Quarter note gets the beat, delay matches beat duration
                return beatDurationMs;

            case TimeSignature::TIME_6_8:
                // Eighth note gets the beat, so divide beat duration by 2
                return beatDurationMs / 2;

            case TimeSignature::TIME_12_16:
                // Sixteenth note gets the beat, so divide beat duration by 4
                return beatDurationMs / 4;

            default:
                // Default to quarter note delay for unrecognized time signatures
                return beatDurationMs;
        }
    }
}#include "../PrefrontalCortex/ProtoPerceptions.h"
#include "../CorpusCallosum/SynapticPathways.h"
#include "SoundFxManager.h"
#include "Arduino.h"
#include <time.h>
#include <SPIFFS.h>
#include "../PrefrontalCortex/SDManager.h"
#include "../VisualCortex/RoverViewManager.h"
#include "../VisualCortex/RoverManager.h"
#include "../VisualCortex/LEDManager.h"
#include <vector>
#include "../VisualCortex/VisualSynesthesia.h"
#include "../PrefrontalCortex/Utilities.h"
#include "../MotorCortex/PinDefinitions.h"

namespace AuditoryCortex
{
    using namespace CorpusCallosum;
    using VC::RoverManager;
    using VC::LEDManager;
    using VC::VisualSynesthesia;
    using PC::Utilities;
    using MC::PinDefinitions;
    using PC::AudioTypes::TunesTypes;
    using PC::AudioTypes::Tune;
    using PC::AudioTypes::NoteInfo;
    using PC::AudioTypes::TimeSignature;

    // Initialize static members
    int SoundFxManager::currentNote = 0;
    unsigned long SoundFxManager::lastNoteTime = 0;
    bool SoundFxManager::m_isTunePlaying = false;
    Audio SoundFxManager::audio;
    bool SoundFxManager::isPlayingSound = false;
    const char* SoundFxManager::RECORD_FILENAME = "/sdcard/temp_record.wav";
    bool SoundFxManager::isRecording = false;
    File SoundFxManager::recordFile;
    bool SoundFxManager::_isInitialized = false;
    int SoundFxManager::volume = 42;

    PC::AudioTypes::TunesTypes SoundFxManager::selectedSong = PC::AudioTypes::TunesTypes::ROVERBYTE_JINGLE;
    PC::AudioTypes::Tune SoundFxManager::activeTune;

    void SoundFxManager::playTone(int frequency, int duration, int volume) 
    {
        if (frequency <= 0) return;

        const int TONE_PWM_CHANNEL = 0;  // Define PWM channel
        ledcSetup(TONE_PWM_CHANNEL, frequency, 8);  // 8-bit resolution
        ledcWrite(TONE_PWM_CHANNEL, volume);
        
        if (duration > 0) 
        {
            delay(duration);
            ledcWrite(TONE_PWM_CHANNEL, 0);
        }
    }

    void SoundFxManager::playTune(PC::AudioTypes::TunesTypes type) 
    {
        try 
        {
            selectedSong = type;
            startTune();
        }
        catch (const std::exception& e) 
        {
            Utilities::LOG_ERROR("Failed to play tune: %s", e.what());
        }
    }

    void SoundFxManager::startTune() 
    {
        currentNote = 0;
        m_isTunePlaying = true;
        lastNoteTime = 0;
        activeTune = Tunes::getTune(selectedSong);
    }

    void SoundFxManager::updateTune() 
    {
        if (!m_isTunePlaying || currentNote >= activeTune.notes.size()) 
        {
            m_isTunePlaying = false;
            return;
        }
        
        unsigned long currentTime = millis();
        const PC::AudioTypes::NoteInfo& note = activeTune.notes[currentNote];
        int noteDuration = PitchPerception::getNoteDuration(note.type, activeTune.timeSignature);
        
        if (currentTime - lastNoteTime >= noteDuration) 
        {
            if (currentNote < activeTune.notes.size()) 
            {
                uint16_t frequency = PitchPerception::getNoteFrequency(note);
                playTone(frequency, noteDuration, volume);
                
                // Update LED visualization
                for (int j = 0; j < PinDefinitions::WS2812_NUM_LEDS; j++) 
                {
                    if (bitRead(activeTune.ledAnimation[currentNote], j)) 
                    {
                        CRGB color = VisualSynesthesia::getNoteColorBlended(note);
                        LEDManager::setLED(j, color);
                    }
                }
                LEDManager::showLEDs();
                
                lastNoteTime = currentTime;
                currentNote++;
            } 
            else 
            {
                m_isTunePlaying = false;
            }
        }
    }

    bool SoundFxManager::isTunePlaying() {
        return m_isTunePlaying;
    }

    void SoundFxManager::playSuccessSound() {
        playTone(PitchPerception::NOTE_C5, 100);
        delay(50);
        playTone(PitchPerception::NOTE_E5, 100);
        delay(50);
        playTone(PitchPerception::NOTE_G5, 200);
    }

    void SoundFxManager::playRotaryPressSound(int mode)  // 0=Full, 1=Week, 2=Timer
    {
        time_t now = time(nullptr);
        struct tm* timeInfo = localtime(&now);
        int dayOfWeek = timeInfo->tm_wday;  // 0-6 (Sunday-Saturday)
        uint16_t baseNote = PitchPerception::getDayBaseNote(mode == 1);

        switch(mode) 
        {
            case 0: 
                playTone(baseNote, 100);
                break;
            case 1:  
                playTone(baseNote, 100);  
                break;
            case 2:  // Timer mode - octave up + fifth
                playTone(baseNote * 2, 100);
                break;
        }
    }

    void SoundFxManager::playRotaryTurnSound(bool clockwise) {
        if (clockwise) {
            playTone(PitchPerception::getDayBaseNote4(), 50);
            playTone(PitchPerception::getDayBaseNote5(), 50);
        } else {
            playTone(PitchPerception::getDayBaseNote5(), 50);
            playTone(PitchPerception::getDayBaseNote4(), 50);
        }
    }

    void SoundFxManager::playSideButtonSound(bool start) {
        if (start) {
            playTone(PitchPerception::getDayBaseNote4(), 50);
            playTone(PitchPerception::getDayBaseNote4(), 50);
        } else {
            playTone(PitchPerception::getDayBaseNote5(), 100);
            int baseNote = PitchPerception::getDayBaseNote5();
            playTone(PitchPerception::getNoteMinus2(baseNote), 100);
        }
    }

    void SoundFxManager::playErrorSound(int type) {
        switch(type) {
            case 1: // Recording error
                SoundFxManager::playTone(PitchPerception::NOTE_B5, 200);
                delay(100);
                SoundFxManager::playTone(PitchPerception::NOTE_G5, 200);
                delay(100);
                SoundFxManager::playTone(PitchPerception::NOTE_D5, 400);
                break;
                
            case 2: // SD card error
                SoundFxManager::playTone(PitchPerception::NOTE_G5, 200);
                delay(100);
                SoundFxManager::playTone(PitchPerception::NOTE_G5, 200);
                delay(100);
                SoundFxManager::playTone(PitchPerception::NOTE_G4, 400);
                break;
                
            case 3: // Playback error
                SoundFxManager::playTone(PitchPerception::NOTE_D5, 200);
                delay(100);
                SoundFxManager::playTone(PitchPerception::NOTE_D5, 200);
                delay(100);
                SoundFxManager::playTone(PitchPerception::NOTE_D4, 400);
                break;
        }
    }

    // Add audio callback
    void SoundFxManager::audio_eof_mp3(const char *info) {
        Serial.printf("Audio playback finished: %s\n", info);
        // Delete temporary recording after playback
        if (!SD.remove(RECORD_FILENAME)) {
            Serial.println("Failed to delete temporary recording file");
            playErrorSound(2);
        }
        isPlayingSound = false;
    }


    void SoundFxManager::startRecording() {
        if (!PC::SDManager::isInitialized()) return;
        if (isRecording) return;
        
        Serial.println("=== Starting Recording ===");
        
        // Initialize microphone
        i2s_config_t i2s_config = {
            .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM),
            .sample_rate = EXAMPLE_SAMPLE_RATE,
            .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
            .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
            .communication_format = I2S_COMM_FORMAT_STAND_I2S,
            .intr_alloc_flags = ESP_INTR_FLAG_LEVEL2,
            .dma_buf_count = 4,
            .dma_buf_len = 64,
            .use_apll = false,
        };

        i2s_pin_config_t pin_config = {
            .mck_io_num = I2S_PIN_NO_CHANGE,
            .bck_io_num = I2S_PIN_NO_CHANGE,
            .ws_io_num = BOARD_MIC_CLK,
            .data_out_num = I2S_PIN_NO_CHANGE,
            .data_in_num = BOARD_MIC_DATA,
        };

        if (i2s_driver_install((i2s_port_t)EXAMPLE_I2S_CH, &i2s_config, 0, NULL) != ESP_OK) {
            Serial.println("ERROR: Failed to install I2S driver");
            SoundFxManager::playErrorSound(1);
            RoverManager::setEarsPerked(false);
            return;
        }
        
        if (i2s_set_pin((i2s_port_t)EXAMPLE_I2S_CH, &pin_config) != ESP_OK) {
            Serial.println("ERROR: Failed to set I2S pins");
            i2s_driver_uninstall((i2s_port_t)EXAMPLE_I2S_CH);
            SoundFxManager::playErrorSound(1);
            RoverManager::setEarsPerked(false);
            return;
        }

        // Create new WAV file
        recordFile = SD.open(RECORD_FILENAME, FILE_WRITE);
        if (!recordFile) {
            Serial.println("ERROR: Failed to open file for recording");
            i2s_driver_uninstall((i2s_port_t)EXAMPLE_I2S_CH);
            SoundFxManager::playErrorSound(2);
            RoverManager::setEarsPerked(false);
            return;
        }

        // Reserve space for the header
        for (int i = 0; i < WAVE_HEADER_SIZE; i++) {
            recordFile.write(0);
        }

        isRecording = true;
        Serial.println("Recording started!");
    }

    void SoundFxManager::stopRecording() 
    {
        if (!isRecording) return;

        Serial.println("=== Stopping Recording ===");
        isRecording = false;
        
        // Memory-safe header generation
        uint32_t fileSize = recordFile.size() - WAVE_HEADER_SIZE;
        char wavHeader[WAVE_HEADER_SIZE];
        generate_wav_header(wavHeader, fileSize, EXAMPLE_SAMPLE_RATE);
        
        // Error handling with cognitive state tracking
        bool headerWriteSuccess = true;
        
        if (!recordFile.seek(0)) 
        {
            Serial.println("ERROR: Failed to seek in file");
            headerWriteSuccess = false;
        }
        else if (recordFile.write((uint8_t *)wavHeader, WAVE_HEADER_SIZE) != WAVE_HEADER_SIZE) 
        {
            Serial.println("ERROR: Failed to write WAV header");
            headerWriteSuccess = false;
        }
        
        // Cleanup resources
        recordFile.close();
        i2s_driver_uninstall((i2s_port_t)EXAMPLE_I2S_CH);
        
        if (!headerWriteSuccess) 
        {
            SoundFxManager::playErrorSound(2);
            RoverManager::setEarsPerked(false);
            return;
        }

        // Attempt playback
        audio.setVolume(volume);
        if (!SD.exists(RECORD_FILENAME) || !audio.connecttoFS(SD, RECORD_FILENAME)) 
        {
            Serial.println("ERROR: Playback failed");
            SoundFxManager::playErrorSound(3);
            RoverManager::setEarsPerked(false);
            return;
        }

        RoverManager::setEarsPerked(false);
    }


    void SoundFxManager::init_microphone() {
        i2s_config_t i2s_config = {
            .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM),
            .sample_rate = EXAMPLE_SAMPLE_RATE,
            .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
            .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
            .communication_format = I2S_COMM_FORMAT_STAND_I2S,
            .intr_alloc_flags = ESP_INTR_FLAG_LEVEL2,
            .dma_buf_count = 8,
            .dma_buf_len = 200,
            .use_apll = 0
        };

        i2s_pin_config_t pin_config = {
            .mck_io_num = I2S_PIN_NO_CHANGE,
            .bck_io_num = I2S_PIN_NO_CHANGE,
            .ws_io_num = BOARD_MIC_CLK,
            .data_out_num = I2S_PIN_NO_CHANGE,
            .data_in_num = BOARD_MIC_DATA
        };

        ESP_ERROR_CHECK(i2s_driver_install((i2s_port_t)EXAMPLE_I2S_CH, &i2s_config, 0, NULL));
        ESP_ERROR_CHECK(i2s_set_pin((i2s_port_t)EXAMPLE_I2S_CH, &pin_config));
        ESP_ERROR_CHECK(i2s_set_clk((i2s_port_t)EXAMPLE_I2S_CH, EXAMPLE_SAMPLE_RATE, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO));
    }


    void SoundFxManager::generate_wav_header(char* wav_header, uint32_t wav_size, uint32_t sample_rate)
    {
        // Cognitive mapping of WAV format structure
        const WavHeaderInfo headerInfo = 
        {
            .fileSize = wav_size + WAVE_HEADER_SIZE - 8,
            .byteRate = BYTE_RATE,
            .sampleRate = sample_rate,
            .numChannels = 1,  // Mono recording
            .bitsPerSample = 16
        };

        // Memory-safe header construction using array bounds
        const size_t headerSize = WAVE_HEADER_SIZE;
        
        // Validate output buffer
        if (!wav_header) return;

        // Clear header memory first
        memset(wav_header, 0, headerSize);

        // Construct header in a memory-safe way
        const char header[WAVE_HEADER_SIZE] = 
        {
            // RIFF chunk descriptor
            'R', 'I', 'F', 'F',
            static_cast<char>(headerInfo.fileSize & 0xFF),
            static_cast<char>((headerInfo.fileSize >> 8) & 0xFF),
            static_cast<char>((headerInfo.fileSize >> 16) & 0xFF), 
            static_cast<char>((headerInfo.fileSize >> 24) & 0xFF),
            
            // WAVE chunk
            'W', 'A', 'V', 'E',
            
            // Format subchunk
            'f', 'm', 't', ' ',
            0x10, 0x00, 0x00, 0x00,  // Subchunk1Size (16 for PCM)
            0x01, 0x00,              // AudioFormat (1 for PCM)
            static_cast<char>(headerInfo.numChannels & 0xFF),
            0x00,
            
            // Sample rate
            static_cast<char>(headerInfo.sampleRate & 0xFF),
            static_cast<char>((headerInfo.sampleRate >> 8) & 0xFF),
            static_cast<char>((headerInfo.sampleRate >> 16) & 0xFF),
            static_cast<char>((headerInfo.sampleRate >> 24) & 0xFF),
            
            // Byte rate
            static_cast<char>(headerInfo.byteRate & 0xFF),
            static_cast<char>((headerInfo.byteRate >> 8) & 0xFF),
            static_cast<char>((headerInfo.byteRate >> 16) & 0xFF),
            static_cast<char>((headerInfo.byteRate >> 24) & 0xFF)
        };

        // Copy constructed header to output buffer
        memcpy(wav_header, header, headerSize);
    }

    void SoundFxManager::init() {
        if (_isInitialized) return;
        // Initialize audio hardware
        audio.setPinout(BOARD_VOICE_BCLK, BOARD_VOICE_LRCLK, BOARD_VOICE_DIN);
        audio.setVolume(volume);  // Set a reasonable volume level
        
        // Initialize SPIFFS for sound files
        if (!SPIFFS.begin(true)) {
            Utilities::LOG_ERROR("Failed to initialize SPIFFS");
            return;
        }
        
        // Initialize I2S
        i2s_config_t i2s_config = 
        {
            .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
            .sample_rate = 44100,
            .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
            .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
            .communication_format = I2S_COMM_FORMAT_STAND_I2S,
            .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
            .dma_buf_count = 8,  // Increased from 4 for better buffering
            .dma_buf_len = 64,   // Increased from 32 for better performance
            .use_apll = true     // Changed to true for better audio quality
        };
        
        if (i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL) != ESP_OK) 
        {
            Utilities::LOG_ERROR("I2S driver installation failed");
            return;
        }

        Utilities::LOG_PROD("I2S driver installed successfully");
        playStartupSound();
        _isInitialized = true;
    }

    void SoundFxManager::adjustVolume(int amount) {
        volume += amount;
        
        // Cognitive boundary checks for volume limits
        if (volume < 0) 
        {
            volume = 91;  // Wrap around to max volume
        }
        if (volume > 100) 
        {
            volume = 0;   // Wrap around to mute
        }
        
        audio.setVolume(volume);
    }

  

    void SoundFxManager::playVoiceLine(const char* line, uint32_t cardId) {
        if (strcmp(line, "card_detected") == 0 && cardId != 0) {
            playCardMelody(cardId);
        }
        else if (strcmp(line, "waiting_for_card") == 0) {
            // Inquisitive searching tune
            SoundFxManager::playTone(PitchPerception::NOTE_E5, 100, 0);
            delay(50);
            SoundFxManager::playTone(PitchPerception::NOTE_G5, 100, 1);
            delay(50);
            SoundFxManager::playTone(PitchPerception::NOTE_A5, 150, 2);
        }
        else if (strcmp(line, "scan_complete") == 0) {
            // Success tune
            SoundFxManager::playTone(PitchPerception::NOTE_C5, 100, 0);
            delay(30);
            SoundFxManager::playTone(PitchPerception::NOTE_E5, 100, 1);
            delay(30);
            SoundFxManager::playTone(PitchPerception::NOTE_G5, 100, 2);
            delay(30);
            SoundFxManager::playTone(PitchPerception::NOTE_C6, 200);
        }
        else if (strcmp(line, "scan_error") == 0) {
            // Error tune
            SoundFxManager::playTone(PitchPerception::NOTE_G4, 200, 0);
            delay(50);
            SoundFxManager::playTone(PitchPerception::NOTE_E4, 200, 1);
            delay(50);
            SoundFxManager::playTone(PitchPerception::NOTE_C4, 300, 2);
        }
        else if (strcmp(line, "level_up") == 0) {
            // Mario-style level up fanfare
            SoundFxManager::playTone(PitchPerception::NOTE_G4, 100, 0);
            delay(50);
            SoundFxManager::playTone(PitchPerception::NOTE_C5, 100, 1);
            delay(50);
            SoundFxManager::playTone(PitchPerception::NOTE_E5, 100, 2);
            delay(50);
            SoundFxManager::playTone(PitchPerception::NOTE_G5, 100, 3);
            delay(50);
            SoundFxManager::playTone(PitchPerception::NOTE_C6, 150, 4);
            delay(100);
            SoundFxManager::playTone(PitchPerception::NOTE_E6, 400);
        }
        else if (strcmp(line, "volume_up") == 0) {
            // Volume up tune
            SoundFxManager::playTone(PitchPerception::NOTE_C5, 100, 0);
            delay(50);
            SoundFxManager::playTone(PitchPerception::NOTE_E5, 100, 1);
            delay(50);
            SoundFxManager::playTone(PitchPerception::NOTE_G5, 100, 2);
        }
        else if (strcmp(line, "volume_down") == 0) {
            // Volume down tune
            SoundFxManager::playTone(PitchPerception::NOTE_G4, 100, 0);
            delay(50);
            SoundFxManager::playTone(PitchPerception::NOTE_E4, 100, 1);
            delay(50);
            SoundFxManager::playTone(PitchPerception::NOTE_C4, 100, 2);
        }
        
    }

    void SoundFxManager::playMenuCloseSound() {
        // Play a descending tone sequence for closing
        playTone(PitchPerception::NOTE_C5, 100); // C5
        delay(50);
        playTone(PitchPerception::NOTE_B4, 100); // B4
        delay(50);
        playTone(PitchPerception::NOTE_A4, 100); // A4
        delay(50);
        playTone(PitchPerception::NOTE_G4, 100); // G4
    }

    void SoundFxManager::playMenuOpenSound() {
        // Play an ascending tone sequence for opening
        playTone(PitchPerception::NOTE_G4, 100); // G4
        delay(50);
        playTone(PitchPerception::NOTE_A4, 100); // A4
        delay(50);
        playTone(PitchPerception::NOTE_B4, 100); // B4
        delay(50);
        playTone(PitchPerception::NOTE_C5, 100); // C5
    }

    void SoundFxManager::playMenuSelectSound() {
        // Play a short, sharp tone with a slight variation
        playTone(PitchPerception::NOTE_E5, 100); // E5 for selection
        delay(50);
        playTone(PitchPerception::NOTE_C5, 100); // C5
        delay(50);
        playTone(PitchPerception::NOTE_E5, 100); // E5 again
        delay(50);
        playTone(PitchPerception::NOTE_G5, 100); // G5 for a higher note
    }

    void SoundFxManager::playCardMelody(uint32_t cardId) 
    {
        // Generate melody from card ID with harmonic relationships
        const uint8_t MELODY_LENGTH = 4;
        uint8_t notes[MELODY_LENGTH];
        
        // Extract meaningful patterns from card ID
        notes[0] = ((cardId >> 24) & 0xFF) % 12;  // Root note (first octave)
        notes[1] = ((cardId >> 16) & 0xFF) % 12 + 12;  // Harmony note (second octave)
        notes[2] = ((cardId >> 8) & 0xFF) % 12;   // Return to first octave
        notes[3] = (cardId & 0xFF) % 24;          // Wide range for final note
        
        // Base frequencies optimized for cognitive recognition
        const uint16_t baseNotes[] = {
            PitchPerception::NOTE_C4, PitchPerception::NOTE_CS4, PitchPerception::NOTE_D4, 
            PitchPerception::NOTE_DS4, PitchPerception::NOTE_E4, PitchPerception::NOTE_F4,
            PitchPerception::NOTE_FS4, PitchPerception::NOTE_G4, PitchPerception::NOTE_GS4, 
            PitchPerception::NOTE_A4, PitchPerception::NOTE_AS4, PitchPerception::NOTE_B4,
            PitchPerception::NOTE_C5, PitchPerception::NOTE_CS5, PitchPerception::NOTE_D5, 
            PitchPerception::NOTE_DS5, PitchPerception::NOTE_E5, PitchPerception::NOTE_F5,
            PitchPerception::NOTE_FS5, PitchPerception::NOTE_G5, PitchPerception::NOTE_GS5, 
            PitchPerception::NOTE_A5, PitchPerception::NOTE_AS5, PitchPerception::NOTE_B5
        };
        
        // Set excited expression
        VisualCortex::RoverManager::setTemporaryExpression(
            VisualCortex::RoverManager::EXCITED, 
            2000
        );
        
        // Play melody with dynamic timing
        for (int i = 0; i < MELODY_LENGTH; i++) 
        {
            uint8_t duration = 50 + (notes[i] % 100);  // Variable note length
            SoundFxManager::playTone(baseNotes[notes[i]], duration, i);
            delay(duration * 0.6);  // Overlap notes slightly for smoother transition
        }
    }

    void SoundFxManager::playTimerDropSound(CRGB color) 
    {
        // Map colors to musical scale degrees for cognitive association
        const int colorToNoteMap[] = {
            PitchPerception::NOTE_C4,  // Red - grounding
            PitchPerception::NOTE_D4,  // Orange - warmth
            PitchPerception::NOTE_E4,  // Yellow - brightness
            PitchPerception::NOTE_F4,  // Green - nature
            PitchPerception::NOTE_G4,  // Blue - depth
            PitchPerception::NOTE_A4,  // Indigo - mystery
            PitchPerception::NOTE_B4,  // Purple - complexity
            PitchPerception::NOTE_C5   // White - completion
        };
        
        int baseNote = PitchPerception::NOTE_C4;  // Default
        
        // Color to note mapping with improved psychological associations
        if (color == CRGB::Red) baseNote = colorToNoteMap[0];
        else if (color == CRGB::Orange) baseNote = colorToNoteMap[1];
        else if (color == CRGB::Yellow) baseNote = colorToNoteMap[2];
        else if (color == CRGB::Green) baseNote = colorToNoteMap[3];
        else if (color == CRGB::Blue) baseNote = colorToNoteMap[4];
        else if (color == CRGB::Indigo) baseNote = colorToNoteMap[5];
        else if (color == CRGB::Purple) baseNote = colorToNoteMap[6];
        else if (color == CRGB::White) baseNote = colorToNoteMap[7];
        
        // Initial clear note for attention
        SoundFxManager::playTone(baseNote, 50, 0);
        delay(10);
        
        // Enhanced water drop effect with harmonic series
        const int STEPS = 4;
        const int DURATION = 20;
        const int DROP_RANGE = 100;
        
        for (int i = 0; i < STEPS; i++) 
        {
            int pitch = baseNote - (DROP_RANGE >> i);  // Exponential pitch drop
            SoundFxManager::playTone(pitch, DURATION, 0);
            delay(DURATION - (i * 2));  // Accelerating tempo
        }
    }

    void SoundFxManager::playErrorCode(uint32_t errorCode, bool isFatal) {
        // Base frequencies for fatal vs warning
        uint16_t baseFreq = isFatal ? 440 : 880; // A4 for fatal, A5 for warning
        
        // Play binary representation of error code
        for (int i = 7; i >= 0; i--) {
            if (errorCode & (1 << i)) {
                playTone(baseFreq, 100);
            } else {
                playTone(baseFreq/2, 100);
            }
            delay(50);
        }
        
        // Final tone indicates fatal/warning
        if (isFatal) {
            playTone(220, 500); // Low A3 for fatal
        } else {
            playTone(1760, 200); // High A6 for warning
        }
    }
}#ifndef TUNES_H
#define TUNES_H

#include "../CorpusCallosum/SynapticPathways.h"
#include <Arduino.h>
#include <vector>
#include "PitchPerception.h"
#include "../PrefrontalCortex/utilities.h"

namespace AuditoryCortex
{
    using namespace CorpusCallosum;
    using PC::AudioTypes::NoteInfo;
    using PC::AudioTypes::TimeSignature;
    using PC::AudioTypes::NoteIndex;
    using PC::AudioTypes::NoteType;
    using PC::AudioTypes::Tune;
    using PC::AudioTypes::TunesTypes;
    using PC::Utilities;

    class Tunes 
    {
    public:
        // Tune retrieval and manipulation methods
        
        /**
         * Retrieves a specific tune based on the provided type
         * @param type The type of tune to retrieve
         * @return The requested Tune structure
         */
        static Tune getTune(TunesTypes type);

        /**
         * Gets the length (number of notes) in a specific tune
         * @param type The type of tune to measure
         * @return The number of notes in the tune
         */
        static int getTuneLength(TunesTypes type);

        /**
         * Calculates the delay in milliseconds for a given time signature
         * @param timeSignature The musical time signature to calculate for
         * @return The delay in milliseconds
         */
        static int timeSignatureToDelay(TimeSignature timeSignature);

    private:
        // Predefined musical compositions
        static const Tune ROVERBYTE_JINGLE;
        static const Tune JINGLE_BELLS;
        static const Tune AULD_LANG_SYNE;
        static const Tune LOVE_SONG;
        static const Tune HAPPY_BIRTHDAY;
        static const Tune EASTER_SONG;
        static const Tune MOTHERS_SONG;
        static const Tune FATHERS_SONG;
        static const Tune CANADA_SONG;
        static const Tune USA_SONG;
        static const Tune CIVIC_SONG;
        static const Tune WAKE_ME_UP_WHEN_SEPTEMBER_ENDS;
        static const Tune HALLOWEEN_SONG;
        static const Tune THANKSGIVING_SONG;
        static const Tune CHRISTMAS_SONG;
    };
}

#endif // TUNES_H#ifndef MENU_MANAGER_H
#define MENU_MANAGER_H

#include <vector>
#include <functional>
#include <string>

namespace SomatosensoryCortex 
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

    class MenuManager 
    {
    public:
        static void init();
        static void show();
        static void hide();
        static bool isVisible() { return isMenuVisible; }
        static void handleRotaryTurn(int direction);
        static void handleMenuSelect();
        static void drawMenu();
        static void enterSubmenu(const std::vector<MenuItem>& submenu);
        static void handleIRBlastMenu();
        static void selectMenuItem();
        static void goBack();
        static std::vector<MenuItem> appSettingsMenu;
        static std::vector<MenuItem> ledModesMenu;
        static std::vector<MenuItem> encodingModesMenu;
        static std::vector<MenuItem> festiveModesMenu;
        static int getSelectedIndex();

    private:
        static std::vector<MenuItem> currentMenu;
        static std::vector<MenuItem> mainMenu;
        static std::vector<std::vector<MenuItem>*> menuStack;
        static int selectedIndex;
        static bool isMenuVisible;
        static const unsigned long DEBOUNCE_DELAY = 50;
    }; 
}

#endif // MENU_MANAGER_H#include "MenuManager.h"
#include "../VisualCortex/RoverViewManager.h"
#include "../VisualCortex/LEDManager.h"
#include "../GameCortex/SlotsManager.h"
#include "../PsychicCortex/NFCManager.h"
#include "../PsychicCortex/IRManager.h"
#include "../GameCortex/AppManager.h"
#include "../AuditoryCortex/SoundFxManager.h"
#include "../VisualCortex/RoverManager.h"
#include "../PrefrontalCortex/utilities.h"
#include "../GameCortex/AppManager.h"

using namespace PrefrontalCortex;  // For Utilities
using namespace GameCortex;  // For AppManager

namespace SomatosensoryCortex 
    {
    // Example: we still keep IR/LED references here as needed
    // but major interactions launch their respective apps.

    bool MenuManager::isMenuVisible = false;
    std::vector<MenuItem> MenuManager::currentMenu;
    std::vector<MenuItem> MenuManager::mainMenu;
    std::vector<std::vector<MenuItem>*> MenuManager::menuStack;
    int MenuManager::selectedIndex = 0;

    // Used elsewhere, so keep it
    bool isRoverRadio = true;

    int MenuManager::getSelectedIndex() {
        return selectedIndex;
    }

    // Define LED Modes submenu
    std::vector<MenuItem> MenuManager::ledModesMenu = {
        MenuItem("Off", []() 
        {
            VisualCortex::LEDManager::setMode(VisualCortex::Mode::OFF_MODE);
            PrefrontalCortex::Utilities::LOG_DEBUG("LED Mode set to Off");
        }),
        MenuItem("Encodings", []() 
        {
            PrefrontalCortex::Utilities::LOG_DEBUG("Entering Encodings menu");
        }),
        MenuItem("Festive", []() 
        {
            PrefrontalCortex::Utilities::LOG_DEBUG("Entering Festive Modes menu");
        }),
        MenuItem("Rover Emotions", []() 
        {
            VisualCortex::LEDManager::setMode(VisualCortex::Mode::ROVER_EMOTION_MODE);
            PrefrontalCortex::Utilities::LOG_DEBUG("LED Mode set to Rover Emotions");
        }),
        MenuItem("Back", []() {
            MenuManager::goBack();
        })
    };

    // Define Encoding Modes submenu
    std::vector<MenuItem> MenuManager::encodingModesMenu = {
        MenuItem("Full Mode", []() 
        {
            VisualCortex::LEDManager::setEncodingMode(VisualCortex::EncodingModes::FULL_MODE);
            PrefrontalCortex::Utilities::LOG_DEBUG("Encoding Mode set to Full Mode");
        }),
        MenuItem("Week Mode", []() 
        {
            VisualCortex::LEDManager::setEncodingMode(VisualCortex::EncodingModes::WEEK_MODE);
            PrefrontalCortex::Utilities::LOG_DEBUG("Encoding Mode set to Week Mode");
        }),
        MenuItem("Timer Mode", []() 
        {
            VisualCortex::LEDManager::setEncodingMode(VisualCortex::EncodingModes::TIMER_MODE);
            PrefrontalCortex::Utilities::LOG_DEBUG("Encoding Mode set to Timer Mode");
        }),
        MenuItem("Custom Mode", []() 
        {
            VisualCortex::LEDManager::setMode(VisualCortex::Mode::ENCODING_MODE);
            VisualCortex::LEDManager::setEncodingMode(VisualCortex::EncodingModes::CUSTOM_MODE);
            PrefrontalCortex::Utilities::LOG_DEBUG("Encoding Mode set to Custom Mode");
        }),
        MenuItem("Back", []() {
            MenuManager::goBack();
        })
    };

    // Define Festive Modes submenu with all options
    std::vector<MenuItem> MenuManager::festiveModesMenu = 
    {
        MenuItem("New Year", []() 
        {
            VisualCortex::LEDManager::setFestiveTheme(VisualCortex::FestiveTheme::NEW_YEAR);
            PrefrontalCortex::Utilities::LOG_DEBUG("Festive Theme set to New Year");
        }),
        MenuItem("Valentines", []() 
        {
            VisualCortex::LEDManager::setFestiveTheme(VisualCortex::FestiveTheme::VALENTINES);
            PrefrontalCortex::Utilities::LOG_DEBUG("Festive Theme set to Valentines");
        }),
        MenuItem("St. Patrick", []() 
        {
            VisualCortex::LEDManager::setFestiveTheme(VisualCortex::FestiveTheme::ST_PATRICK);
            PrefrontalCortex::Utilities::LOG_DEBUG("Festive Theme set to St. Patrick");
        }),
        MenuItem("Easter", []() 
        {
            VisualCortex::LEDManager::setFestiveTheme(VisualCortex::FestiveTheme::EASTER);
            PrefrontalCortex::Utilities::LOG_DEBUG("Festive Theme set to Easter");
        }),
        MenuItem("Canada Day", []() 
        {
            VisualCortex::LEDManager::setFestiveTheme(VisualCortex::FestiveTheme::CANADA_DAY);
            PrefrontalCortex::Utilities::LOG_DEBUG("Festive Theme set to Canada Day");
        }),
        MenuItem("Halloween", []() 
        {
            VisualCortex::LEDManager::setFestiveTheme(VisualCortex::FestiveTheme::HALLOWEEN);
            PrefrontalCortex::Utilities::LOG_DEBUG("Festive Theme set to Halloween");
        }),
        MenuItem("Christmas", []() 
        {
            VisualCortex::LEDManager::setFestiveTheme(VisualCortex::FestiveTheme::CHRISTMAS);
            PrefrontalCortex::Utilities::LOG_DEBUG("Festive Theme set to Christmas");
        }),
        MenuItem("Thanksgiving", []() 
        {
            VisualCortex::LEDManager::setFestiveTheme(VisualCortex::FestiveTheme::THANKSGIVING);
            PrefrontalCortex::Utilities::LOG_DEBUG("Festive Theme set to Thanksgiving");
        }),
        MenuItem("Independence Day", []() 
        {
            VisualCortex::LEDManager::setFestiveTheme(VisualCortex::FestiveTheme::INDEPENDENCE_DAY);
            PrefrontalCortex::Utilities::LOG_DEBUG("Festive Theme set to Independence Day");
        }),
        MenuItem("Diwali", []() 
        {
            VisualCortex::LEDManager::setFestiveTheme(VisualCortex::FestiveTheme::DIWALI);
            PrefrontalCortex::Utilities::LOG_DEBUG("Festive Theme set to Diwali");
        }),
        MenuItem("Ramadan", []() 
        {
            VisualCortex::LEDManager::setFestiveTheme(VisualCortex::FestiveTheme::RAMADAN);
            PrefrontalCortex::Utilities::LOG_DEBUG("Festive Theme set to Ramadan");
        }),
        MenuItem("Chinese New Year", []() 
        {
            VisualCortex::LEDManager::setFestiveTheme(VisualCortex::FestiveTheme::CHINESE_NEW_YEAR);
            PrefrontalCortex::Utilities::LOG_DEBUG("Festive Theme set to Chinese New Year");
        }),
        MenuItem("Mardi Gras", []() 
        {
            VisualCortex::LEDManager::setFestiveTheme(VisualCortex::FestiveTheme::MARDI_GRAS);
            PrefrontalCortex::Utilities::LOG_DEBUG("Festive Theme set to Mardi Gras");
        }),
        MenuItem("Labor Day", []() 
        {
            VisualCortex::LEDManager::setFestiveTheme(VisualCortex::FestiveTheme::LABOR_DAY);
            PrefrontalCortex::Utilities::LOG_DEBUG("Festive Theme set to Labor Day");
        }),
        MenuItem("Memorial Day", []() 
        {
            VisualCortex::LEDManager::setFestiveTheme(VisualCortex::FestiveTheme::MEMORIAL_DAY);
            PrefrontalCortex::Utilities::LOG_DEBUG("Festive Theme set to Memorial Day");
        }),
        MenuItem("Flag Day", []() 
        {
            VisualCortex::LEDManager::setFestiveTheme(VisualCortex::FestiveTheme::FLAG_DAY);
            PrefrontalCortex::Utilities::LOG_DEBUG("Festive Theme set to Flag Day");
        }),
        MenuItem("Back", []() {
            MenuManager::goBack();
        })
    };

    // Define App Settings submenu
    std::vector<MenuItem> MenuManager::appSettingsMenu = {
        {"Change LED Mode", [&]() {
            MenuManager::enterSubmenu(MenuManager::ledModesMenu);
        }},
        {"Adjust Volume", []() {
            // Logic to adjust volume
            AuditoryCortex::SoundFxManager::adjustVolume(20);
            AuditoryCortex::SoundFxManager::playVoiceLine("volume_up", 0);
        }},
        
        {"Set Time", []() {
            // Logic to set time
            //AppManager::startApp("SetTimeApp");
            VisualCortex::RoverManager::setTemporaryExpression(
                VisualCortex::RoverManager::LOOKING_DOWN, 
                1000
            );
        }},
        {"Back", []() {
            MenuManager::goBack();
        }}
    };

    void MenuManager::init() {
        // Main menu items with explicit constructor calls
        mainMenu.push_back(MenuItem(
            "Slots App", []() {
                GameCortex::AppManager::startApp("SlotsApp");
                hide();
            }
        ));

        mainMenu.push_back(MenuItem(
            "IR Blast App", []() {
                AppManager::startApp("IrBlastApp");
                hide();
            }
        ));

        mainMenu.push_back(MenuItem(
            "NFC App", []() {
                AppManager::startApp("NfcApp");
                hide();
            }
        ));

        mainMenu.push_back(MenuItem(
            "App Settings", [&]() {
                MenuManager::enterSubmenu(MenuManager::ledModesMenu);
            }
        ));

        // Assign the main menu as our initial menu
        currentMenu = mainMenu;
    }

    void MenuManager::show() {
        isMenuVisible = true;
        VisualCortex::RoverViewManager::drawMenuBackground();
        drawMenu();
    }

    void MenuManager::hide() {
        isMenuVisible = false;
        selectedIndex = 0;
        currentMenu = mainMenu;
        menuStack.clear();
        VisualCortex::RoverViewManager::drawCurrentView();
    }

    void MenuManager::drawMenu() {
        const char* title = "Menu";
        VisualCortex::RoverViewManager::drawFullScreenMenu(title, currentMenu, selectedIndex);
    }

    void MenuManager::handleRotaryTurn(int direction) {
        if (!isMenuVisible || currentMenu.empty()) {
            // If menu is not visible, show it (optional behavior)
            show();
            return;
        }

        if (direction > 0) {
            selectedIndex = (selectedIndex - 1 + currentMenu.size()) % currentMenu.size();
        } else {
            selectedIndex = (selectedIndex + 1) % currentMenu.size();
        }
        drawMenu();
    }

    void MenuManager::handleMenuSelect() {
        if (!isMenuVisible || currentMenu.empty()) {
            show();
            return;
        }

        if (selectedIndex >= (int)currentMenu.size()) {
            selectedIndex = 0;
            return;
        }

        MenuItem& selected = currentMenu[selectedIndex];

        // If there are subitems, descend into them
        if (!selected.subItems.empty()) {
            menuStack.push_back(&currentMenu);
            currentMenu = selected.subItems;
            selectedIndex = 0;
            drawMenu();
            return;
        }

        // Otherwise, call the action (which may start an app)
        if (selected.action) {
            selected.action();
            drawMenu();
        }
    }

    // If you still want a separate IR submenu for advanced IR control, you can place it here
    void MenuManager::handleIRBlastMenu() {
        Utilities::LOG_DEBUG("handleIRBlastMenu......");
        // Example placeholder. Could call AppManager::startApp("IrBlastApp") 
        // or handle advanced IR menus here.
    }



    // Helper for going back in the submenu stack
    void MenuManager::goBack() {
        if (!menuStack.empty()) {
            currentMenu = *menuStack.back();
            menuStack.pop_back();
            selectedIndex = 0;
        }
        if (!currentMenu.empty()) {
            drawMenu();
        } else {
            isMenuVisible = false;
        }
    }

    void MenuManager::enterSubmenu(const std::vector<MenuItem>& submenu) {
        menuStack.push_back(&currentMenu);
        currentMenu = submenu;
        selectedIndex = 0;
        drawMenu();
    }

    void MenuManager::selectMenuItem() {
        if (currentMenu.empty()) return; // No items to select

        // Execute the action associated with the selected menu item
        currentMenu[selectedIndex].action();
    }
}#include "UIManager.h"
#include "../VisualCortex/RoverViewManager.h"
#include "../VisualCortex/RoverManager.h"
#include "../PrefrontalCortex/PowerManager.h"
#include "../VisualCortex/LEDManager.h"
#include "../AuditoryCortex/SoundFxManager.h"
#include "../PrefrontalCortex/RoverBehaviorManager.h"
#include "../SomatosensoryCortex/MenuManager.h"
#include "../MotorCortex/PinDefinitions.h"


namespace SomatosensoryCortex 
{

    using BehaviorState = RoverBehaviorManager::BehaviorState;

    // Static member initialization
    RotaryEncoder* UIManager::encoder = nullptr;
    int UIManager::lastEncoderPosition = 0;
    bool UIManager::rotaryPressed = false;
    bool UIManager::sideButtonPressed = false;
    unsigned long UIManager::lastDebounceTime = 0;
    bool UIManager::_isInitialized = false;
    void UIManager::init() {
        if (encoder != nullptr) {
            delete encoder;  // Clean up if already initialized
        }
        
        encoder = new RotaryEncoder(ENCODER_INA, ENCODER_INB, RotaryEncoder::LatchMode::TWO03);
        
        // Configure input pins with internal pullups
        pinMode(BOARD_USER_KEY, INPUT_PULLUP);
        pinMode(ENCODER_KEY, INPUT_PULLUP);
        
        // Add interrupt handlers for encoder
        attachInterrupt(digitalPinToInterrupt(ENCODER_INA), []() { 
            if (encoder) encoder->tick(); 
        }, CHANGE);
        attachInterrupt(digitalPinToInterrupt(ENCODER_INB), []() { 
            if (encoder) encoder->tick(); 
        }, CHANGE);
        
        lastEncoderPosition = encoder->getPosition();
        Utilities::LOG_DEBUG("UIManager initialized with interrupts");
        _isInitialized = true;
    }

    void UIManager::update() {
        if (!_isInitialized) {
            UIManager::init();
        }
        updateEncoder();
        updateSideButton();
    }

    void UIManager::updateEncoder() {
        Utilities::LOG_DEBUG("Updating encoder");
        if (!encoder) return;  // Guard against null encoder
        
        int newPos = encoder->getPosition();
        if (newPos != lastEncoderPosition) {
            Utilities::LOG_DEBUG("Encoder position changed: %d", newPos);
            lastEncoderPosition = newPos;
            PowerManager::updateLastActivityTime();
            
            // Handle encoder movement based on context
            if (MenuManager::isVisible()) {
                // In menu - navigate menu items (inverted the direction)
                MenuManager::handleRotaryTurn(newPos < lastEncoderPosition ? 1 : -1);
            } else {
                // On home screen - change views
                if (newPos > lastEncoderPosition) {
                    RoverViewManager::nextView();
                } else {
                    RoverViewManager::previousView();
                }
            }
        }
        
        // Check encoder button
        static bool lastButtonState = HIGH;
        bool currentButtonState = digitalRead(ENCODER_KEY);
        
        if (currentButtonState != lastButtonState) {
            delay(50);  // Debounce
            currentButtonState = digitalRead(ENCODER_KEY);
            if (currentButtonState != lastButtonState) {
                lastButtonState = currentButtonState;
                if (currentButtonState == LOW) {  // Button pressed
                    if (!MenuManager::isVisible()) {
                        MenuManager::show();
                        SoundFxManager::playMenuOpenSound();
                    } else {
                        MenuManager::handleMenuSelect();
                        SoundFxManager::playMenuSelectSound();
                    }
                }
            }
        }
    }

    void UIManager::handleRotaryTurn(int direction) {
        if (MenuManager::isVisible()) {
            // Invert the direction for menu navigation
            MenuManager::handleRotaryTurn(-direction);
        } else {
            if (direction > 0) {
                RoverViewManager::nextView();
            } else {
                RoverViewManager::previousView();
            }
        }
    }

    void UIManager::handleRotaryPress() {
        Utilities::LOG_DEBUG("Rotary press");
        static bool lastButtonState = HIGH;
        bool currentButtonState = digitalRead(ENCODER_KEY);
        
        if (currentButtonState == LOW && lastButtonState == HIGH) 
        {
            if (RoverViewManager::hasActiveNotification()) 
            {
                RoverViewManager::clearNotification();
            }
            else if (RoverManager::showTime) { // Check if time is currently displayed
                MenuManager::handleMenuSelect(); // Open the menu if time is visible
            } 
            else 
            {
                RoverManager::setShowTime(true);
            }
            PowerManager::updateLastActivityTime();
        }
        
        lastButtonState = currentButtonState;
    }

    void UIManager::updateSideButton() {
        Utilities::LOG_DEBUG("Updating side button");
        static bool lastButtonState = HIGH;
        bool currentButtonState = digitalRead(BOARD_USER_KEY);
        
        if (currentButtonState != lastButtonState) {
            delay(50);  // Simple debounce
            currentButtonState = digitalRead(BOARD_USER_KEY);
            if (currentButtonState != lastButtonState) {
                lastButtonState = currentButtonState;
                if (currentButtonState == LOW) {  // Button pressed
                    Utilities::LOG_DEBUG("Side button pressed");
                    PowerManager::updateLastActivityTime();
                    
                    // If menu is visible, hide it and return to home
                    if (MenuManager::isVisible()) {
                        MenuManager::hide();
                        SoundFxManager::playMenuCloseSound();
                    }
                    RoverManager::setEarsPerked(true);  // Perk ears on side button press
                } else {
                    RoverManager::setEarsPerked(false);  // Un-perk ears when released
                }
            }
        }
    }
}#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <RotaryEncoder.h>
#include <Arduino.h>

namespace SomatosensoryCortex 
{

    class UIManager {
    public:
        static void init();
        static void update();
        
        // Input states
        static bool isRotaryPressed() { return rotaryPressed; }
        static bool isSideButtonPressed() { return sideButtonPressed; }
        static int getEncoderPosition() { return lastEncoderPosition; }
        static bool isInitialized() { return _isInitialized; }
    private:
        static RotaryEncoder* encoder;
        static int lastEncoderPosition;
        static bool rotaryPressed;
        static bool sideButtonPressed;
        static unsigned long lastDebounceTime;
        static const unsigned long DEBOUNCE_DELAY = 50;
        
        static void handleRotaryTurn(int direction);
        static void handleRotaryPress();
        static void updateEncoder();
        static void updateSideButton();
        static bool _isInitialized;
    };

}

#endif#ifndef VISUAL_SYNESIA_H
#define VISUAL_SYNESIA_H

#include <FastLED.h>
#include "../PrefrontalCortex/ProtoPerceptions.h"

namespace AuditoryCortex 
{
    struct NoteInfo;  // Forward declaration
}

namespace VisualCortex 
{

    class VisualSynesthesia {
    public:
        // Static color arrays - moved to public and made extern
        static const CRGB BASE_8_COLORS[8];
        static const CRGB MONTH_COLORS[12][2];
        static const CRGB DAY_COLORS[7];
        static const CRGB CHROMATIC_COLORS[12][2];

        // Static methods
        static CRGB getBase8Color(uint8_t value);
        static CRGB getDayColor(uint8_t day);
        static void getMonthColors(uint8_t month, CRGB& color1, CRGB& color2);
        static void getHourColors(uint8_t hour, CRGB& color1, CRGB& color2);
        static CRGB getBatteryColor(uint8_t percentage);
        static CRGB getColorForFrequency(uint16_t freq);
        static CRGB getNoteColorBlended(const PrefrontalCortex::ProtoPerceptions::NoteInfo& info);
        static uint16_t convertToRGB565(CRGB color);
        

        // New synesthesia-specific methods
        static CRGB blendNoteColors(const PrefrontalCortex::ProtoPerceptions::NoteInfo& info);
        static void playNFCCardData(const char* cardData);
        
        static void playVisualChord(uint16_t baseFreq, CRGB& root, CRGB& third, CRGB& fifth);
    }; 
}

#endif // VISUAL_SYNESIA_H#include <time.h>
#include "VisualSynesthesia.h"
#include "../PrefrontalCortex/PowerManager.h"
#include "LEDManager.h"
#include "../AuditoryCortex/SoundFxManager.h"
#include "RoverManager.h"
#include "../PrefrontalCortex/utilities.h"
#include "DisplayConfig.h"
#include "../MotorCortex/PinDefinitions.h"

namespace VisualCortex 
{
    // Forward declarations
    extern TFT_eSprite spr;

    // Initialize static members
    bool RoverManager::earsPerked = false;
    int RoverManager::currentMood = 0;
    int RoverManager::hoverOffset = 0;
    bool RoverManager::movingDown = true;
    unsigned long RoverManager::lastHoverUpdate = 0;
    unsigned long RoverManager::expressionStartTime = 0;
    int RoverManager::expressionDuration = 0;
    uint16_t RoverManager::starColor = TFT_WHITE;
    const char* RoverManager::moods[] = {"happy", "looking_left", "looking_right", "intense"};
    bool RoverManager::showTime = false;

    extern bool isLowBrightness;

    RoverManager::Expression RoverManager::currentExpression = RoverManager::HAPPY;
    RoverManager::Expression RoverManager::previousExpression = RoverManager::HAPPY;

    void RoverManager::setShowTime(bool show) {
        showTime = show;
    }


    void RoverManager::drawRover(const char* mood, bool earsPerked, bool large, int x, int y) {
        LOG_SCOPE("Drawing rover");
        if (MenuManager::isVisible()) {
            return;
        }
        // Use currentExpression instead of mood parameter if it's set
        const char* actualMood = currentExpression != previousExpression ? 
                                expressionToMood(currentExpression) : 
                                mood;
        
        String moodStr(actualMood);
        float scale = large ? 1.5 : 1.0;
        
        // Get current time and colors
        time_t now = time(nullptr);
        struct tm timeInfo;
        localtime_r(&now, &timeInfo);  // Remove timezone adjustment since it's handled by configTime
        
        // Get eye colors from current month
        uint16_t leftEyeColor = monthColors[timeInfo.tm_mon][0];
        uint16_t rightEyeColor = monthColors[timeInfo.tm_mon][1];
        
        // Small rover always shows time, large rover never shows time
        if (showTime) {
            // Convert to 12-hour format
            int hours = timeInfo.tm_hour % 12;
            if (hours == 0) hours = 12;
            
            char timeStr[6];
            sprintf(timeStr, "%2d:%02d", hours, timeInfo.tm_min);
            spr.setTextFont(7);
            
            // Center time and rover using TFT_WIDTH
            int16_t timeWidth = spr.textWidth(timeStr);
            int centerX = TFT_WIDTH / 2;  // Screen center
            x = centerX - (100 * scale) / 2;  // Center rover (100 is rover width)
            
            spr.fillRect(centerX - (timeWidth/2) - 5, 25, timeWidth + 10, 40, TFT_BLACK);
            
            // Get day color for time display
            CRGB dayColor = VisualSynesthesia::getDayColor(timeInfo.tm_wday + 1);
            uint16_t timeColor = VisualSynesthesia::convertToRGB565(dayColor);
            spr.setTextColor(timeColor, TFT_BLACK);
            spr.drawString(timeStr, centerX, 30);
            y = 80;
        } else {
            y = 40;
        }
        
        // Draw rover starting from x position
        int roverX = x;  // Remove the offset calculation
        int currentY = y + hoverOffset;
        
        // Draw Rover's body
        spr.fillRect(roverX, currentY, 100 * scale, 70 * scale, TFT_WHITE);
        
        // Draw ears
        if (earsPerked) {
            spr.fillTriangle(roverX + 10*scale, currentY - 25*scale, 
                            roverX + 25*scale, currentY, 
                            roverX + 40*scale, currentY - 25*scale, TFT_WHITE);
            spr.fillTriangle(roverX + 60*scale, currentY - 25*scale, 
                            roverX + 75*scale, currentY, 
                            roverX + 90*scale, currentY - 25*scale, TFT_WHITE);
        } else {
            spr.fillTriangle(roverX + 10*scale, currentY - 10*scale, 
                            roverX + 25*scale, currentY + 5*scale, 
                            roverX + 40*scale, currentY - 10*scale, TFT_WHITE);
            spr.fillTriangle(roverX + 60*scale, currentY - 10*scale, 
                            roverX + 75*scale, currentY + 5*scale, 
                            roverX + 90*scale, currentY - 10*scale, TFT_WHITE);
        }
        
        // Draw eye panel
        spr.fillRect(roverX + 15*scale, currentY + 5*scale, 70*scale, 30*scale, color1);
        
        drawEyes(moodStr, roverX, currentY, leftEyeColor, rightEyeColor, scale);
        drawNoseAndMouth(moodStr, roverX, currentY, scale);
    }

    void RoverManager::drawEyes(String mood, int roverX, int currentY, uint16_t leftEyeColor, uint16_t rightEyeColor, float scale) {
        // Draw white background circles for eyes
        spr.fillCircle(roverX + 30*scale, currentY + 20*scale, 10*scale, TFT_WHITE);
        spr.fillCircle(roverX + 70*scale, currentY + 20*scale, 10*scale, TFT_WHITE);
        
        if (mood == "excited") {
            // Left star eye
            for (int i = -5; i <= 5; i++) {
                spr.drawLine(roverX + (30-5)*scale, currentY + 20*scale, 
                            roverX + (30+5)*scale, currentY + 20*scale, leftEyeColor);
                spr.drawLine(roverX + 30*scale, currentY + (20-5)*scale, 
                            roverX + 30*scale, currentY + (20+5)*scale, leftEyeColor);
                // Diagonal lines for star points
                spr.drawLine(roverX + (30-3)*scale, currentY + (20-3)*scale,
                            roverX + (30+3)*scale, currentY + (20+3)*scale, leftEyeColor);
                spr.drawLine(roverX + (30-3)*scale, currentY + (20+3)*scale,
                            roverX + (30+3)*scale, currentY + (20-3)*scale, leftEyeColor);
            }
            
            // Right star eye (same pattern, different position)
            for (int i = -5; i <= 5; i++) {
                spr.drawLine(roverX + (70-5)*scale, currentY + 20*scale, 
                            roverX + (70+5)*scale, currentY + 20*scale, rightEyeColor);
                spr.drawLine(roverX + 70*scale, currentY + (20-5)*scale, 
                            roverX + 70*scale, currentY + (20+5)*scale, rightEyeColor);
                // Diagonal lines for star points
                spr.drawLine(roverX + (70-3)*scale, currentY + (20-3)*scale,
                            roverX + (70+3)*scale, currentY + (20+3)*scale, rightEyeColor);
                spr.drawLine(roverX + (70-3)*scale, currentY + (20+3)*scale,
                            roverX + (70+3)*scale, currentY + (20-3)*scale, rightEyeColor);
            }
        } else if (mood == "looking_up") {
            spr.fillCircle(roverX + 30*scale, currentY + 15*scale, 5*scale, leftEyeColor);
            spr.fillCircle(roverX + 70*scale, currentY + 15*scale, 5*scale, rightEyeColor);
        } else if (mood == "looking_down") {
            spr.fillCircle(roverX + 30*scale, currentY + 25*scale, 5*scale, leftEyeColor);
            spr.fillCircle(roverX + 70*scale, currentY + 25*scale, 5*scale, rightEyeColor);
        } else if (mood == "big_smile") {
            // Normal eye position with bigger smile
            spr.fillCircle(roverX + 30*scale, currentY + 20*scale, 5*scale, leftEyeColor);
            spr.fillCircle(roverX + 70*scale, currentY + 20*scale, 5*scale, rightEyeColor);
        } else if (mood == "sleeping") {
            spr.drawLine(roverX + 25*scale, currentY + 20*scale, 
                        roverX + 35*scale, currentY + 20*scale, TFT_BLACK);
            spr.drawLine(roverX + 65*scale, currentY + 20*scale, 
                        roverX + 75*scale, currentY + 20*scale, TFT_BLACK);
        } else if (mood == "looking_left") {
            spr.fillCircle(roverX + 25*scale, currentY + 20*scale, 5*scale, leftEyeColor);
            spr.fillCircle(roverX + 65*scale, currentY + 20*scale, 5*scale, rightEyeColor);
        } else if (mood == "looking_right") {
            spr.fillCircle(roverX + 35*scale, currentY + 20*scale, 5*scale, leftEyeColor);
            spr.fillCircle(roverX + 75*scale, currentY + 20*scale, 5*scale, rightEyeColor);
        } else if (mood == "intense") {
            spr.fillCircle(roverX + 30*scale, currentY + 20*scale, 3*scale, leftEyeColor);
            spr.fillCircle(roverX + 70*scale, currentY + 20*scale, 3*scale, rightEyeColor);
        } else {
            spr.fillCircle(roverX + 30*scale, currentY + 20*scale, 5*scale, leftEyeColor);
            spr.fillCircle(roverX + 70*scale, currentY + 20*scale, 5*scale, rightEyeColor);
        }
    }

    void RoverManager::drawNoseAndMouth(String mood, int roverX, int currentY, float scale) {
        // Draw triangular nose
        spr.fillTriangle(roverX + 45*scale, currentY + 35*scale, 
                        roverX + 40*scale, currentY + 45*scale, 
                        roverX + 50*scale, currentY + 45*scale, TFT_BLACK);
        
        // Draw mouth based on mood
        if (mood == "happy") {
            spr.drawArc(roverX + 50*scale, currentY + 55*scale, 15*scale, 10*scale, 270, 450, TFT_BLACK, TFT_BLACK);
        } else if (mood == "sad") {
            spr.drawArc(roverX + 50*scale, currentY + 70*scale, 20*scale, 15*scale, 180, 360, TFT_BLACK, TFT_BLACK);
        } else if (mood == "intense") {
            spr.drawLine(roverX + 35*scale, currentY + 60*scale, 
                        roverX + 65*scale, currentY + 60*scale, TFT_BLACK);
        } else if (mood == "sleeping") {
            spr.drawArc(roverX + 50*scale, currentY + 60*scale, 15*scale, 10*scale, 0, 180, TFT_BLACK, TFT_BLACK);
        } else if (mood == "big_smile") {
            // Wider, bigger smile
            spr.drawArc(roverX + 50*scale, currentY + 55*scale, 20*scale, 15*scale, 270, 450, TFT_BLACK, TFT_BLACK);
        } else {
            spr.drawLine(roverX + 50*scale, currentY + 45*scale, 
                        roverX + 50*scale, currentY + 55*scale, TFT_BLACK);
        }
    }

    void RoverManager::updateHoverAnimation() {
        // Only update hover animation when device is awake
        if (PowerManager::getCurrentSleepState() != PowerManager::AWAKE) return;
        
        if (millis() - lastHoverUpdate >= 100) {  // Update every 100ms
            if (movingDown) {
                hoverOffset++;
                if (hoverOffset >= 3) {  // Reduced from 5 to 3
                    movingDown = false;
                }
            } else {
                hoverOffset--;
                if (hoverOffset <= -3) {  // Reduced from -5 to -3
                    movingDown = true;
                }
            }
            lastHoverUpdate = millis();
        }
    }

    const char* RoverManager::getCurrentMood() {
        return moods[currentMood];
    }

    void RoverManager::nextMood() {
        currentMood = (currentMood + 1) % NUM_MOODS;
    }

    void RoverManager::previousMood() {
        currentMood = (currentMood - 1 + NUM_MOODS) % NUM_MOODS;
    }

    void RoverManager::setRandomMood() {
        currentMood = random(0, NUM_MOODS);
        drawSprite();  // Now we can call it directly
    }

    // New function to handle temporary expressions
    void RoverManager::setTemporaryExpression(Expression exp, int duration, uint16_t color) {
        currentExpression = exp;
        starColor = color;  // Store the star color
        expressionStartTime = millis();
        expressionDuration = duration;
        
        // Update display with new expression
        drawExpression(currentExpression);
    }

    const char* RoverManager::expressionToMood(Expression exp) {
        switch(exp) {
            case HAPPY: return "happy";
            case LOOKING_UP: return "looking_up";
            case LOOKING_DOWN: return "looking_down";
            case LOOKING_LEFT: return "looking_left";
            case LOOKING_RIGHT: return "looking_right";
            case INTENSE: return "intense";
            case BIG_SMILE: return "big_smile";
            case EXCITED: return "excited";
            default: return "happy";
        }
    }


    void RoverManager::drawExpression(Expression exp) {
        const char* mood = expressionToMood(exp);
        drawRover(mood, earsPerked);
    }

    void RoverManager::setEarsPerked(bool up) {
        earsPerked = up;
        setTemporaryExpression(HAPPY);
        drawRover(moods[currentMood], up);
    }
}#pragma once
#include <FastLED.h>
#include "TFT_eSPI.h"

namespace VisualCortex 
{
        // Forward declare the sprite
        extern TFT_eSprite spr;

    class RoverManager 
    {

        public:

            enum Expression {
                HAPPY,
                LOOKING_LEFT,
                LOOKING_RIGHT,
                INTENSE,
                LOOKING_UP,      // New - not idle
                LOOKING_DOWN,    // New - not idle
                BIG_SMILE,      // New - not idle
                EXCITED,         // New expression with star eyes
                NUM_EXPRESSIONS
            };


            static const char* expressionToMood(Expression exp);
            static bool showTime;  // Added here as it's related to rover display state

            static bool isIdleExpression(Expression exp) {
                return exp <= INTENSE;  // Only first 4 are idle expressions
            }

            static void setTemporaryExpression(Expression exp, int duration = 1000, uint16_t starColor = TFT_WHITE);
            static void showThinking() { setTemporaryExpression(LOOKING_UP); }
            static void showError() { setTemporaryExpression(LOOKING_DOWN, 1000); }
            static void showSuccess() { setTemporaryExpression(BIG_SMILE, 1000); }
            static void setShowTime(bool show);

            static bool earsPerked;
            static const char* getCurrentMood();
            static void nextMood();
            static void previousMood();
            static void updateHoverAnimation();
            static void drawRover(const char* mood, bool earsPerked = false, bool large = false, int x = 85, int y = 120);
            static void setRandomMood();
            static void setEarsPerked(bool up);

        private:
            static void drawEyes(String mood, int roverX, int currentY, uint16_t leftEyeColor, uint16_t rightEyeColor, float scale);
            static void drawNoseAndMouth(String mood, int roverX, int currentY, float scale);
            static Expression currentExpression;
            static Expression previousExpression;
            // Static member variables
            static int currentMood;
            static int hoverOffset;
            static bool movingDown;
            static unsigned long lastHoverUpdate;
            static const char* moods[];
            static const int NUM_MOODS = 5;
            
            // Color definitions
            static const uint16_t monthColors[12][2];
            static const uint16_t color1;

            // Add these new members
            static unsigned long expressionStartTime;
            static int expressionDuration;
            static uint16_t starColor;
            static void drawExpression(Expression exp);
        };

        // Define the static const arrays outside the class
        inline const uint16_t RoverManager::monthColors[12][2] = {
            {0xF800, 0xF800},  // January   - Red/Red
            {0xF800, 0xFD20},  // February  - Red/Orange
            {0xFD20, 0xFD20},  // March     - Orange/Orange
            {0xFD20, 0xFFE0},  // April     - Orange/Yellow
            {0xFFE0, 0xFFE0},  // May       - Yellow/Yellow
            {0x07E0, 0x07E0},  // June      - Green/Green
            {0x07E0, 0x001F},  // July      - Green/Blue
            {0x001F, 0x001F},  // August    - Blue/Blue
            {0x001F, 0x4810},  // September - Blue/Indigo
            {0x4810, 0x4810},  // October   - Indigo/Indigo
            {0x4810, 0x780F},  // November  - Indigo/Violet
            {0x780F, 0x780F}   // December  - Violet/Violet
        };
        
        inline const uint16_t RoverManager::color1 = 0xC638;  // Silver color for eye plate
}#pragma once
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
}#include "VisualSynesthesia.h"
#include "../AuditoryCortex/SoundFxManager.h"
#include "LEDManager.h"
#include <FastLED.h>

namespace VisualCortex 
{
    // Define the static arrays
    const CRGB VisualSynesthesia::BASE_8_COLORS[] = {
        CRGB::Black,          // 0 = Off
        CRGB(255, 0, 0),      // 1 = Pure Red
        CRGB(255, 140, 0),    // 2 = Orange
        CRGB::Yellow,         // 3 = Yellow
        CRGB::Green,          // 4 = Green
        CRGB::Blue,           // 5 = Blue
        CRGB(75, 0, 130),     // 6 = Indigo
        CRGB(148, 0, 211)     // 7 = Violet
    };

    const CRGB VisualSynesthesia::MONTH_COLORS[][2] = {
        {CRGB(255, 0, 0), CRGB(255, 0, 0)},       // January
        {CRGB(255, 0, 0), CRGB(255, 140, 0)},     // February
        {CRGB(255, 140, 0), CRGB(255, 140, 0)},   // March
        {CRGB(255, 140, 0), CRGB::Yellow},        // April
        {CRGB::Yellow, CRGB::Yellow},             // May
        {CRGB::Green, CRGB::Green},               // June
        {CRGB::Green, CRGB::Blue},                // July
        {CRGB::Blue, CRGB::Blue},                  // August
        {CRGB::Blue, CRGB(75, 0, 130)},            // September
        {CRGB(75, 0, 130), CRGB(75, 0, 130)},     // October
        {CRGB(75, 0, 130), CRGB(148, 0, 211)},    // November
        {CRGB(148, 0, 211), CRGB(148, 0, 211)}    // December
    };

    const CRGB VisualSynesthesia::DAY_COLORS[] = {
        CRGB::Red,                  // Sunday
        CRGB(255, 140, 0),          // Monday
        CRGB::Yellow,               // Tuesday
        CRGB::Green,                // Wednesday
        CRGB::Blue,                 // Thursday
        CRGB(75, 0, 130),           // Friday
        CRGB(148, 0, 211)           // Saturday
    };
    const CRGB VisualSynesthesia::CHROMATIC_COLORS[][2] = {
        {CRGB::Red, CRGB::Red},                // C
        {CRGB::Red, CRGB(255, 140, 0)},        // C#/Db
        {CRGB(255, 140, 0), CRGB::Yellow},      // D
        {CRGB::Yellow, CRGB::Yellow},           // D#/Eb
        {CRGB::Yellow, CRGB::Green},            // E
        {CRGB::Green, CRGB::Green},             // F
        {CRGB::Green, CRGB::Blue},              // F#/Gb
        {CRGB::Blue, CRGB::Blue},               // G
        {CRGB::Blue, CRGB(75, 0, 130)},         // G#/Ab
        {CRGB(75, 0, 130), CRGB::Blue},         // A
        {CRGB(75, 0, 130), CRGB(148, 0, 211)},  // A#/Bb
        {CRGB(148, 0, 211), (148, 0, 211)}       // B
    };

    CRGB VisualSynesthesia::getBase8Color(uint8_t value) {
        return BASE_8_COLORS[value % 8];
    }

    CRGB VisualSynesthesia::getDayColor(uint8_t day) {
        return DAY_COLORS[(day - 1) % 7];
    }

    void VisualSynesthesia::getMonthColors(uint8_t month, CRGB& color1, CRGB& color2) {
        month = (month - 1) % 12;
        color1 = MONTH_COLORS[month][0];
        color2 = MONTH_COLORS[month][1];
    }

    void VisualSynesthesia::getHourColors(uint8_t hour, CRGB& color1, CRGB& color2) {
        hour = (hour - 1) % 12;
        color1 = CHROMATIC_COLORS[hour][0];
        color2 = CHROMATIC_COLORS[hour][1];
    }

    CRGB VisualSynesthesia::getBatteryColor(uint8_t percentage) {
        if (percentage > 75) {
            return CRGB::Green;
        } else if (percentage > 50) {
            return CRGB::Yellow;
        } else if (percentage > 25) {
            return CRGB(255, 140, 0);  // Orange
        } else {
            return CRGB::Red;
        }
    }

    CRGB VisualSynesthesia::getColorForFrequency(uint16_t freq) {
        if (freq >= PitchPerception::NOTE_C5) return CRGB::Red;
        else if (freq >= PitchPerception::NOTE_B4) return CRGB(148, 0, 211);  // Violet
        else if (freq >= PitchPerception::NOTE_A4) return CRGB(75, 0, 130);   // Indigo
        else if (freq >= PitchPerception::NOTE_G4) return CRGB::Blue;
        else if (freq >= PitchPerception::NOTE_F4) return CRGB::Green;
        else if (freq >= PitchPerception::NOTE_E4) return CRGB::Yellow;
        else if (freq >= PitchPerception::NOTE_D4) return CRGB(255, 140, 0);  // Orange
        else return CRGB::Red;
    }

    CRGB VisualSynesthesia::blendNoteColors(const NoteInfo& info) {
        if (info.isSharp) {
            // For sharp notes, blend the colors of the adjacent natural notes
            CRGB lowerColor = CHROMATIC_COLORS[(info.noteIndex - 1 + 12) % 12][0];
            CRGB upperColor = CHROMATIC_COLORS[(info.noteIndex + 1) % 12][0];
            return blend(lowerColor, upperColor, 128); // 50% blend
        } else {
            return CHROMATIC_COLORS[info.noteIndex][0];
        }
    }

    void VisualSynesthesia::playVisualChord(uint16_t baseFreq, CRGB& root, CRGB& third, CRGB& fifth) {
        // Get root note information
        NoteInfo rootInfo = PitchPerception::getNoteInfo(baseFreq);

        // Calculate major third (+4 semitones)
        int thirdNoteIndex = (rootInfo.noteIndex + 4) % 12;
        int thirdOctave = rootInfo.octave + ((rootInfo.noteIndex + 4) >= 12 ? 1 : 0);
        // Calculate perfect fifth (+7 semitones)
        int fifthNoteIndex = (rootInfo.noteIndex + 7) % 12;
        int fifthOctave = rootInfo.octave + ((rootInfo.noteIndex + 7) >= 12 ? 1 : 0);

        // Find frequencies for third and fifth
        uint16_t thirdFreq = PitchPerception::getNoteFrequencies()[(thirdOctave - 1) * 12 + thirdNoteIndex];
        uint16_t fifthFreq = PitchPerception::getNoteFrequencies()[(fifthOctave - 1) * 12 + fifthNoteIndex];

        // Get NoteInfo for third and fifth
        NoteInfo thirdInfo = PitchPerception::getNoteInfo(thirdFreq);
        NoteInfo fifthInfo = PitchPerception::getNoteInfo(fifthFreq);


        SoundFxManager::playTone(baseFreq, 200);
        LEDManager::displayNote(baseFreq, 0);
        LEDManager::displayNote(baseFreq, 1);
        SoundFxManager::playTone(thirdFreq, 200);
        LEDManager::displayNote(thirdFreq, 2);
        LEDManager::displayNote(thirdFreq, 3);
        SoundFxManager::playTone(fifthFreq, 200); 
        LEDManager::displayNote(fifthFreq, 3);
        LEDManager::displayNote(fifthFreq, 4);
        SoundFxManager::playTone(thirdFreq, 200);
        LEDManager::displayNote(thirdFreq, 5);
        LEDManager::displayNote(thirdFreq, 6);
        SoundFxManager::playTone(baseFreq, 200);
        LEDManager::displayNote(baseFreq, 7);

            // Get colors
        root = baseFreq; //getColorForFrequency(PitchPerception::getStandardFrequency(rootInfo));
        third = thirdFreq; //getColorForFrequency(PitchPerception::getStandardFrequency(thirdInfo));
        fifth = fifthFreq; //getColorForFrequency(PitchPerception::getStandardFrequency(fifthInfo));
        
    }

    uint16_t VisualSynesthesia::convertToRGB565(CRGB color) {
        return ((color.r & 0xF8) << 8) | ((color.g & 0xFC) << 3) | (color.b >> 3);
    }

    void VisualSynesthesia::playNFCCardData(const char* cardData) {
        for (int i = 0; i < strlen(cardData); i++) {
            // Map each character to a frequency
            uint16_t frequency = map(cardData[i], 32, 126, 200, 2000); // Map printable ASCII to a frequency range
            SoundFxManager::playTone(frequency, 200); // Play each note for 200 ms
            delay(250); // Delay between notes
        }
    }

}#ifndef FASTLED_CONFIG_H
#define FASTLED_CONFIG_H

// Core FastLED Configuration
#define FASTLED_INTERNAL
#define FASTLED_ESP32_I2S 0
#define FASTLED_ALLOW_INTERRUPTS 0
#define FASTLED_ESP32_FLASH_LOCK 1
#define FASTLED_RMT_MAX_CHANNELS 2
#define FASTLED_ESP32_RMT 1

// Visual Cortex specific LED configurations
#define VISUAL_CORTEX_MAX_BRIGHTNESS 255
#define VISUAL_CORTEX_DEFAULT_FPS 60
#define VISUAL_CORTEX_COLOR_CORRECTION TypicalLEDStrip
#define VISUAL_CORTEX_LED_TYPE WS2812B

// Animation timing constants
#define VISUAL_CORTEX_LOADING_DELAY 100
#define VISUAL_CORTEX_FLASH_DURATION 100
#define VISUAL_CORTEX_FADE_STEP 5
#define VISUAL_CORTEX_MIN_FADE 50
#define VISUAL_CORTEX_MAX_FADE 250
#define VISUAL_CORTEX_ERROR_MIN_FADE 64

// Pattern-specific configurations
#define VISUAL_CORTEX_LEDS_PER_STEP 3
#define VISUAL_CORTEX_ERROR_LED_INDEX 0
#define VISUAL_CORTEX_ERROR_LED_COUNT 8
#define VISUAL_CORTEX_FADE_INCREMENT 5
#define VISUAL_CORTEX_ANIMATION_DELAY 50

// Boot sequence timing
#define VISUAL_CORTEX_BOOT_STEP_DELAY 100
#define VISUAL_CORTEX_SUCCESS_FLASH_DURATION 100
#define VISUAL_CORTEX_ERROR_FLASH_DURATION 200

#endif #pragma once

namespace VisualCortex 
{

    // Screen dimensions and center points
    #define SCREEN_WIDTH 240
    #define SCREEN_HEIGHT 320
    #define SCREEN_CENTER_X (SCREEN_WIDTH / 2)
    #define SCREEN_CENTER_Y (SCREEN_HEIGHT / 2) 

    #define FRAME_OFFSET_X 40

}#include "../VisualCortex/FastLEDConfig.h"
#include "../CorpusCallosum/SynapticPathways.h"
#include "../PrefrontalCortex/Utilities.h"
#include "../MotorCortex/PinDefinitions.h"
#include "LEDManager.h"
#include "VisualSynesthesia.h"
#include "../AuditoryCortex/SoundFxManager.h"
#include "../GameCortex/AppManager.h"
#include "../AuditoryCortex/PitchPerception.h"
#include "../PsychicCortex/NFCManager.h"
#include "../PrefrontalCortex/RoverBehaviorManager.h"
#include "../GameCortex/SlotsManager.h"
#include "../PsychicCortex/IRManager.h"

using namespace CorpusCallosum;
using PC::Utilities;

namespace VisualCortex 
{

    // Boot stage colors
    const CRGB LEDManager::HARDWARE_INIT_COLOR = CRGB::Blue;    // Hardware initialization
    const CRGB LEDManager::SYSTEM_START_COLOR = CRGB::Green;    // System startup
    const CRGB LEDManager::NETWORK_PREP_COLOR = CRGB::Purple;   // Network preparation
    const CRGB LEDManager::FINAL_PREP_COLOR = CRGB::Orange;     // Final preparation

    // Static member initialization
    CRGB LEDManager::leds[PinDefinitions::WS2812_NUM_LEDS];
    Mode LEDManager::currentMode = Mode::ENCODING_MODE;
    Mode LEDManager::previousMode = Mode::ENCODING_MODE;
    EncodingModes LEDManager::currentEncodingMode = EncodingModes::FULL_MODE;
    uint8_t LEDManager::currentPosition = 0;
    uint8_t LEDManager::currentColorIndex = 0;
    uint8_t LEDManager::completedCycles = 0;
    uint8_t LEDManager::filledPositions = 0;
    uint8_t LEDManager::activeTrails = 0;
    unsigned long LEDManager::lastStepTime = 0;
    bool LEDManager::isLoading = false;
    FestiveTheme LEDManager::currentTheme = FestiveTheme::CHRISTMAS;
    uint8_t LEDManager::animationStep = 0;
    uint8_t LEDManager::fadeValue = 128;
    bool LEDManager::fadeDirection = true;
    bool LEDManager::tickTock = false;

    NoteState LEDManager::currentNotes[PinDefinitions::WS2812_NUM_LEDS];
    CRGB LEDManager::previousColors[PinDefinitions::WS2812_NUM_LEDS];
    Pattern LEDManager::currentPattern = Pattern::NONE;


    //resolve this clutter
    CRGB LEDManager::winningColor = CRGB::Green;
    bool LEDManager::transitioningColor = false;
    uint8_t LEDManager::currentFadeIndex = 0;
    unsigned long LEDManager::lastUpdate = 0;
    CRGB LEDManager::targetColor = CRGB::Blue;
    const uint8_t LEDManager::fadeSequence[] = {6, 5, 7, 4, 0, 3, 1, 2};
    bool LEDManager::readyForMelody = false;

    //--

    uint8_t LEDManager::loadingPosition = 0;

    struct FestiveDay {
        int month;
        int day;
        FestiveTheme theme;
    };

    static const FestiveDay festiveDays[] = {
        {1, 1, FestiveTheme::NEW_YEAR},        // January 1
        {2, 14, FestiveTheme::VALENTINES},     // February 14
        {3, 17, FestiveTheme::ST_PATRICK},     // March 17
        {4, 1, FestiveTheme::EASTER},          // Easter date varies (calculate based on the year)
        {7, 1, FestiveTheme::CANADA_DAY},      // July 1
        {10, 31, FestiveTheme::HALLOWEEN},     // October 31
        {12, 25, FestiveTheme::CHRISTMAS},     // December 25
        {11, 26, FestiveTheme::THANKSGIVING},   // Fourth Thursday in November (USA, varies)
        {7, 4, FestiveTheme::INDEPENDENCE_DAY},// July 4 (USA)
        {10, 14, FestiveTheme::FLAG_DAY},      // June 14 (USA)
        {5, 31, FestiveTheme::MEMORIAL_DAY},   // Last Monday in May (USA, varies)
        {9, 1, FestiveTheme::LABOR_DAY},       // First Monday in September (USA, varies)
        {2, 1, FestiveTheme::DIWALI},          // Date varies (Hindu festival of lights)
        {9, 1, FestiveTheme::MARDI_GRAS},      // Date varies (Fat Tuesday)
        {4, 1, FestiveTheme::RAMADAN},         // Date varies (Islamic holy month)
        {1, 29, FestiveTheme::CHINESE_NEW_YEAR} // Date varies (Lunar New Year)
    };

    void LEDManager::init() {
        Utilities::LOG_DEBUG("Starting LED Manager initialization...");
        
        try {
            // Initialize FastLED with hardware SPI configuration
            FastLED.addLeds<VISUAL_CORTEX_LED_TYPE, 
                           PinDefinitions::WS2812_DATA_PIN, 
                           PinDefinitions::RGB_ORDER>(
                leds, 
                PinDefinitions::WS2812_NUM_LEDS
            ).setCorrection(VISUAL_CORTEX_COLOR_CORRECTION);
            
            FastLED.setBrightness(VISUAL_CORTEX_MAX_BRIGHTNESS);
            FastLED.setMaxRefreshRate(VISUAL_CORTEX_DEFAULT_FPS);
            FastLED.clear(true);
            
            // Initialize boot sequence colors
            fill_solid(leds, PinDefinitions::WS2812_NUM_LEDS, HARDWARE_INIT_COLOR);
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
        fill_solid(leds, PinDefinitions::WS2812_NUM_LEDS, HARDWARE_INIT_COLOR);
        FastLED.show();
        delay(100);
        FastLED.clear(true);
    }

    void LEDManager::stopLoadingAnimation() {
        if (!isLoading) return;
        
        isLoading = false;
        currentMode = Mode::ENCODING_MODE;
        currentEncodingMode = EncodingModes::FULL_MODE;
        // Initialize FULL_MODE pattern
        for (int i = 0; i < PinDefinitions::WS2812_NUM_LEDS; i++) {
            leds[i] = CRGB::Blue;  // Start with blue
            previousColors[i] = CRGB::Black;
        }
        FastLED.show();

        
        Utilities::LOG_DEBUG("LED Manager: Transitioned to FULL_MODE with patterns");
    }

    void LEDManager::updateLEDs() {
        
        if (AppManager::isAppActive()) {
            switch (currentPattern) {
                case Pattern::SLOTS_GAME:
                    updateSlotsPattern();
                    break;
                case Pattern::IR_BLAST:
                    updateIRBlastPattern();
                    break;
                case Pattern::NFC_SCAN:
                    updateNFCScanPattern();
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
            case Mode::ENCODING_MODE:
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
            case Mode::OFF_MODE:
                FastLED.clear();
                FastLED.show();
                break;
            case Mode::FESTIVE_MODE:
                updateFestiveMode();
                break;
        }
    }

    void LEDManager::setMode(Mode newMode) {
        currentMode = newMode;
        FastLED.clear();
        updateLEDs();
    }

    void LEDManager::nextMode() {
        currentMode = static_cast<Mode>((static_cast<int>(currentMode) + 1) % LED_NUM_MODES);
        FastLED.clear();
        updateLEDs();
    }

    void LEDManager::updateFullMode() {
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
        time_t now = time(nullptr);
        struct tm* timeInfo = localtime(&now);
        
        // Create a common blink state for both LEDs
        bool shouldBlink = (timeInfo->tm_sec % 2 == 0);
        
        // LED 0: Month color with alternating pattern
        CRGB monthColor1, monthColor2;
        VisualSynesthesia::getMonthColors(timeInfo->tm_mon + 1, monthColor1, monthColor2);
        
        // Month LED should always blink
        if (shouldBlink) {
            monthColor1.nscale8(MONTH_DIM);
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
        static const CRGB timerColors[] = {
            CRGB::Red, CRGB::Orange, CRGB::Yellow, 
            CRGB::Green, CRGB::Blue, CRGB::Indigo, 
            CRGB::Purple, CRGB::White, CRGB::Black
        };
        static const int NUM_TIMER_COLORS = sizeof(timerColors) / sizeof(timerColors[0]);
        static CRGB backgroundColors[PinDefinitions::WS2812_NUM_LEDS] = {CRGB::Black};
        static bool isMoving = false;
        
        unsigned long currentTime = millis();
        if (currentTime - lastStepTime >= 125) {  // 125ms between moves
            lastStepTime = currentTime;
            
            if (!isMoving) {
                // Start new drop at position 0
                leds[0] = timerColors[currentColorIndex];
                currentPosition = 0;
                isMoving = true;
            } else {
                // Clear current position
                leds[currentPosition] = backgroundColors[currentPosition];
                
                // Move to next position if possible
                if (currentPosition < PinDefinitions::WS2812_NUM_LEDS - 1 && 
                    leds[currentPosition + 1] == backgroundColors[currentPosition + 1]) {
                    currentPosition++;
                    leds[currentPosition] = timerColors[currentColorIndex];
                } else {
                    // Drop has reached its final position
                    leds[currentPosition] = timerColors[currentColorIndex];
                    SoundFxManager::playTimerDropSound(timerColors[currentColorIndex]);
                    isMoving = false;
                    
                    // Check if we completed this color's cycle
                    bool allFilled = true;
                    for (int i = 0; i < PinDefinitions::WS2812_NUM_LEDS; i++) {
                        if (leds[i] == backgroundColors[i]) {
                            allFilled = false;
                            break;
                        }
                    }
                    
                    if (allFilled) {
                        // Update background for next color
                        for (int i = 0; i < PinDefinitions::WS2812_NUM_LEDS; i++) {
                            backgroundColors[i] = timerColors[currentColorIndex];
                        }
                        currentColorIndex = (currentColorIndex + 1) % NUM_TIMER_COLORS;
                        if (currentColorIndex == 0) {
                            // Reset background when starting over
                            for (int i = 0; i < PinDefinitions::WS2812_NUM_LEDS; i++) {
                                backgroundColors[i] = CRGB::Black;
                            }
                        }
                    }
                }
            }
            FastLED.show();
        }
    }

    CRGB LEDManager::getRainbowColor(uint8_t index) {
        static const CRGB rainbowColors[] = {
            CRGB::Red,
            CRGB::Orange,
            CRGB::Yellow,
            CRGB::Green,
            CRGB::Blue,
            CRGB(75, 0, 130),   // Indigo
            CRGB(148, 0, 211)   // Violet
        };
        return rainbowColors[index % NUM_RAINBOW_COLORS];
    }

    void LEDManager::startLoadingAnimation() {
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
        if (!isLoading) return;
        
        unsigned long currentTime = millis();
        if (currentTime - lastStepTime < VISUAL_CORTEX_LOADING_DELAY) return;
        
        lastStepTime = currentTime;
        int bootStep = RoverBehaviorManager::getCurrentBootStep();
        static int lastBootStep = -1;

        // Only initialize new LEDs when boot step changes
        if (bootStep != lastBootStep) 
        {
            lastBootStep = bootStep;
            loadingPosition = bootStep * VISUAL_CORTEX_LEDS_PER_STEP;
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
        if (loadingPosition < (bootStep + 1) * VISUAL_CORTEX_LEDS_PER_STEP) 
        {
            leds[loadingPosition] = currentColor;
            loadingPosition++;
        }
        FastLED.show();
    }

    bool LEDManager::isLoadingComplete() {
        return completedCycles >= PinDefinitions::WS2812_NUM_LEDS;
    }

    void LEDManager::setLED(int index, CRGB color) {
        if (index >= 0 && index < PinDefinitions::WS2812_NUM_LEDS) {
            // Direct color setting without conversion
            leds[index] = color;
        }
    }

    void LEDManager::showLEDs() {
        FastLED.show();
    }

    void LEDManager::scaleLED(int index, uint8_t scale) {
        if (index >= 0 && index < PinDefinitions::WS2812_NUM_LEDS) {
            leds[index].nscale8(scale);
        }
    }

    void LEDManager::flashSuccess() {
        // Save current LED state
        CRGB savedState[PinDefinitions::WS2812_NUM_LEDS];
        memcpy(savedState, leds, sizeof(CRGB) * PinDefinitions::WS2812_NUM_LEDS);
        
        // Flash green
        fill_solid(leds, PinDefinitions::WS2812_NUM_LEDS, CRGB::Green);
        FastLED.show();
        delay(VISUAL_CORTEX_SUCCESS_FLASH_DURATION);
        
        // Restore previous state
        memcpy(leds, savedState, sizeof(CRGB) * PinDefinitions::WS2812_NUM_LEDS);
        FastLED.show();
    }

    void LEDManager::flashLevelUp() {
        // First pass - clockwise light up in gold/yellow
        for (int i = 0; i < PinDefinitions::WS2812_NUM_LEDS; i++) {
            leds[i] = CRGB::Gold;
            FastLED.show();
            delay(50);
        }
        
        // Flash all bright white
        for (int i = 0; i < PinDefinitions::WS2812_NUM_LEDS; i++) {
            leds[i] = CRGB::White;
        }
        FastLED.show();
        delay(100);
        
        // Sparkle effect
        for (int j = 0; j < 3; j++) {  // Do 3 sparkle cycles
            for (int i = 0; i < PinDefinitions::WS2812_NUM_LEDS; i++) {
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
        for (int i = 0; i < PinDefinitions::WS2812_NUM_LEDS; i++) {
            leds[i] = CRGB::Black;
            FastLED.show();
            delay(30);
        }
    }

    void LEDManager::displayCardPattern(const uint8_t* uid, uint8_t length) {
        static unsigned long lastUpdate = 0;
        static uint8_t step = 0;
        
        if (millis() - lastUpdate < 50) return;
        lastUpdate = millis();
        
        // Use card UID to create unique patterns
        for (int i = 0; i < PinDefinitions::WS2812_NUM_LEDS; i++) {
            uint8_t hue = (uid[i % length] + step) % 255;
            leds[i] = CHSV(hue, 255, 255);
        }
        
        step = (step + 1) % 255;
        showLEDs();
    }

    void LEDManager::syncLEDsForDay() {
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
        // Check and set festive mode based on the current date
        LEDManager::checkAndSetFestiveMode();

        if (isLoading) {
            LEDManager::updateLoadingAnimation();
            return;
        }

        switch (currentMode) {
            case Mode::ENCODING_MODE:
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
            case Mode::OFF_MODE:
                FastLED.clear();
                FastLED.show();
                break;
            case Mode::FESTIVE_MODE:
                LEDManager::updateFestiveMode();
                break;
            case Mode::ROVER_EMOTION_MODE:
                LEDManager::updateRoverEmotionMode();
                break;
        }
    }

    void LEDManager::updateMenuMode() {
        
        int selectedIndex = MenuManager::getSelectedIndex();
        for (int i = 0; i < PinDefinitions::WS2812_NUM_LEDS; i++) {
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

    void LEDManager::updateRoverEmotionMode() {
        // TODO: Implement rover emotion mode
    }


    unsigned long tickTalkTime = 0;
    void LEDManager::updateCustomMode() {

        unsigned long currentTime = millis();

        if (currentTime - tickTalkTime >= 10000) {
            tickTalkTime = currentTime;
            tickTock = !tickTock;
        }

        if (tickTock) {
            int batteryLevel8 = PowerManager::getBatteryPercentage() % 8;
            int uptime8 = PowerManager::getUpTime() % 8;

            CRGB color = VisualSynesthesia::getBase8Color(batteryLevel8);

            for (int i = 0; i < PinDefinitions::WS2812_NUM_LEDS; i++) {
                if (i <= uptime8) {
                    setLED(i, color);
                    delay(50);
                    showLEDs();
                } else {
                    setLED(i, CRGB::Black);
                }
            }
        } else
        {
            for (int i = 0; i < PinDefinitions::WS2812_NUM_LEDS; i++) {
                leds[i] = CHSV(((i * 256 / PinDefinitions::WS2812_NUM_LEDS) + currentTime/10) % 256, 255, 255);
            }
        }

        showLEDs();

    }

    void LEDManager::updateFestiveMode() {
        unsigned long currentTime = millis();
        if (currentTime - lastStepTime >= 50) {  // 50ms animation interval
            lastStepTime = currentTime;

            switch (currentTheme) {
                case FestiveTheme::NEW_YEAR:
                    // Fireworks effect with bright colors
                    for (int i = 0; i < PinDefinitions::WS2812_NUM_LEDS; i++) {
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

                    for (int i = 0; i < PinDefinitions::WS2812_NUM_LEDS; i++) {
                        CRGB color = (i % 2 == 0) ? CRGB::Red : CRGB::Pink;
                        color.nscale8(fadeValue);
                        leds[i] = color;
                    }
                    break;

                case FestiveTheme::ST_PATRICK:
                    // Rotating shamrock effect with green shades
                    for (int i = 0; i < PinDefinitions::WS2812_NUM_LEDS; i++) {
                        int adjustedPos = (i + animationStep) % PinDefinitions::WS2812_NUM_LEDS;
                        switch (adjustedPos % 3) {
                            case 0: leds[i] = CRGB::Green; break;
                            case 1: leds[i] = CRGB(0, 180, 0); break;
                            case 2: leds[i] = CRGB(0, 100, 0); break;
                        }
                    }
                    animationStep = (animationStep + 1) % PinDefinitions::WS2812_NUM_LEDS;
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

                    for (int i = 0; i < PinDefinitions::WS2812_NUM_LEDS; i++) {
                        leds[i] = blend(previousColors[i], targetColor, animationStep);
                        previousColors[i] = leds[i];
                    }
                    break;

                case FestiveTheme::CANADA_DAY:
                    // Waving flag effect with red and white
                    animationStep = (animationStep + 1) % 255;
                    for (int i = 0; i < PinDefinitions::WS2812_NUM_LEDS; i++) {
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

                    for (int i = 0; i < PinDefinitions::WS2812_NUM_LEDS; i++) {
                        CRGB color = (i % 2 == 0) ? CRGB::Orange : CRGB::Purple;
                        color.nscale8(fadeValue);
                        leds[i] = color;
                    }
                    break;

                case FestiveTheme::THANKSGIVING:
                    // Autumn colors fading effect
                    for (int i = 0; i < PinDefinitions::WS2812_NUM_LEDS; i++) {
                        leds[i] = (i % 2 == 0) ? CRGB::Orange : CRGB::Brown; // Alternating colors
                    }
                    break;

                case FestiveTheme::INDEPENDENCE_DAY:
                    // Red, white, and blue flashing
                    for (int i = 0; i < PinDefinitions::WS2812_NUM_LEDS; i++) {
                        leds[i] = (i % 3 == 0) ? CRGB::Red : (i % 3 == 1) ? CRGB::White : CRGB::Blue;
                    }
                    break;

                case FestiveTheme::DIWALI:
                    // Colorful lights effect
                    for (int i = 0; i < PinDefinitions::WS2812_NUM_LEDS; i++) {
                        leds[i] = CRGB(random8(255), random8(255), random8(255)); // Random colors
                    }
                    break;

                case FestiveTheme::RAMADAN:
                    // Soft white and gold glow
                    for (int i = 0; i < PinDefinitions::WS2812_NUM_LEDS; i++) {
                        leds[i] = (i % 2 == 0) ? CRGB::White : CRGB::Gold; // Alternating colors
                    }
                    break;

                case FestiveTheme::CHINESE_NEW_YEAR:
                    // Red and gold flashing
                    for (int i = 0; i < PinDefinitions::WS2812_NUM_LEDS; i++) {
                        leds[i] = (i % 2 == 0) ? CRGB::Red : CRGB::Gold; // Alternating colors
                    }
                    break;

                case FestiveTheme::MARDI_GRAS:
                    // Purple, green, and gold flashing
                    for (int i = 0; i < PinDefinitions::WS2812_NUM_LEDS; i++) {
                        leds[i] = (i % 3 == 0) ? CRGB::Purple : (i % 3 == 1) ? CRGB::Green : CRGB::Gold; // Alternating colors
                    }
                    break;

                case FestiveTheme::LABOR_DAY:
                    // Red and white stripes
                    for (int i = 0; i < PinDefinitions::WS2812_NUM_LEDS; i++) {
                        leds[i] = (i % 2 == 0) ? CRGB::Red : CRGB::White; // Alternating colors
                    }
                    break;

                case FestiveTheme::MEMORIAL_DAY:
                    // Red, white, and blue stripes
                    for (int i = 0; i < PinDefinitions::WS2812_NUM_LEDS; i++) {
                        leds[i] = (i % 3 == 0) ? CRGB::Red : (i % 3 == 1) ? CRGB::White : CRGB::Blue; // Alternating colors
                    }
                    break;

                case FestiveTheme::FLAG_DAY:
                    // Red and white stripes with blue at the ends
                    for (int i = 0; i < PinDefinitions::WS2812_NUM_LEDS; i++) {
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

    void LEDManager::setFestiveTheme(FestiveTheme theme) {
        currentTheme = theme;
        currentMode = Mode::FESTIVE_MODE;
        FastLED.clear();
        LEDManager::updateLEDs();
    }

    void LEDManager::updateIRBlastPattern() {
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
        // Move slots LED code from SlotsManager
        // But keep using LEDManager's methods
    }

    void LEDManager::updateNFCScanPattern() {
        if (millis() - lastUpdate < VISUAL_CORTEX_ANIMATION_DELAY) return;
        lastUpdate = millis();

        if (transitioningColor) {
            if (currentFadeIndex < sizeof(fadeSequence)) {
                // Fade sequence from blue to green
                uint8_t ledIndex = fadeSequence[currentFadeIndex];
                leds[ledIndex] = blend(CRGB::Blue, CRGB::Green, fadeValue);
                fadeValue += VISUAL_CORTEX_FADE_INCREMENT;
                
                if (fadeValue >= VISUAL_CORTEX_MAX_FADE) {
                    fadeValue = 0;
                    currentFadeIndex++;
                }
            } else if (!readyForMelody) {
                readyForMelody = true;
                SoundFxManager::playCardMelody(NFCManager::getLastCardId());
            }
        } else {
            // Normal blue pulse
            fadeValue += (fadeDirection ? VISUAL_CORTEX_FADE_INCREMENT : -VISUAL_CORTEX_FADE_INCREMENT);
            
            if (fadeValue <= VISUAL_CORTEX_MIN_FADE) fadeDirection = true;
            if (fadeValue >= VISUAL_CORTEX_MAX_FADE) fadeDirection = false;
            
            fill_solid(leds, PinDefinitions::WS2812_NUM_LEDS, CRGB::Blue);
            fadeToBlackBy(leds, PinDefinitions::WS2812_NUM_LEDS, 255 - fadeValue);
        }
        FastLED.show();
    }

    void LEDManager::setPattern(Pattern pattern) {
        currentPattern = pattern;
    }

    void LEDManager::handleMessage(LEDMessage message) {
        switch(message) {
            case LEDMessage::SLOTS_WIN:
                // Store winning color and start victory flash
                LEDManager::currentPattern = Pattern::SLOTS_GAME;
                // Store color for use in updateSlotsPattern
                winningColor = CRGB::Green;
                break;
                
            case LEDMessage::IR_SUCCESS:
                flashSuccess();
                break;
                
            case LEDMessage::NFC_DETECTED:
                LEDManager::currentPattern = Pattern::NFC_SCAN;
                LEDManager::transitioningColor = true;
                LEDManager::fadeValue = 0;
                LEDManager::currentFadeIndex = 0;
                LEDManager::readyForMelody = false;
                break;
                
            case LEDMessage::NFC_ERROR:
                LEDManager::currentPattern = Pattern::NFC_SCAN;
                LEDManager::targetColor = CRGB::Red;
                LEDManager::fadeValue = 0;
                LEDManager::transitioningColor = true;
                break;
                
            default:
                break;
        }
    }



    CRGB LEDManager::getNoteColor(uint16_t frequency) {
        return VisualSynesthesia::getColorForFrequency(frequency);
    }

    void LEDManager::displayNote(uint16_t frequency, uint8_t position) {
        position = position % PinDefinitions::WS2812_NUM_LEDS;
        
        NoteInfo info = PitchPerception::getNoteInfo(frequency);
        LEDManager::currentNotes[position].isSharp = info.isSharp;
        LEDManager::currentNotes[position].position = position;
        
        if (info.isSharp) {
            // For sharp/flat notes, get colors of adjacent natural notes
            uint16_t lowerFreq = PitchPerception::getStandardFrequency(frequency - 10);
            uint16_t upperFreq = PitchPerception::getStandardFrequency(frequency + 10);
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

    void LEDManager::setErrorLED(bool state) {
        if (state) {
            leds[ERROR_LED_INDEX] = CRGB::Red;
        } else {
            leds[ERROR_LED_INDEX] = CRGB::Black;
        }
        FastLED.show();
    }

    void LEDManager::setErrorPattern(uint32_t errorCode, bool isFatal) {
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
        // Clear error LEDs
        fill_solid(leds + ERROR_LED_INDEX, ERROR_LED_COUNT * 2, CRGB::Black);
        FastLED.show();
    }

    void LEDManager::updateErrorPattern() {
        // Only update for fatal errors (pulsing effect)
        if (RoverViewManager::isFatalError) {
            // Update fade value
            if (fadeDirection) {
                fadeValue = min(255, fadeValue + VISUAL_CORTEX_FADE_INCREMENT);
                if (fadeValue >= VISUAL_CORTEX_MAX_FADE) fadeDirection = false;
            } else {
                fadeValue = max(VISUAL_CORTEX_ERROR_MIN_FADE, fadeValue - VISUAL_CORTEX_FADE_INCREMENT);
                if (fadeValue <= VISUAL_CORTEX_ERROR_MIN_FADE) fadeDirection = true;
            }
            
            // Apply fade to error LEDs
            for (uint8_t i = 0; i < VISUAL_CORTEX_ERROR_LED_COUNT; i++) {
                if (leds[VISUAL_CORTEX_ERROR_LED_INDEX + i].r > 0) { // Only fade red LEDs (fatal errors)
                    leds[VISUAL_CORTEX_ERROR_LED_INDEX + i].fadeToBlackBy(255 - fadeValue);
                }
            }
            
            FastLED.show();
        }
    }

    void LEDManager::checkAndSetFestiveMode() {
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
        setMode(Mode::ENCODING_MODE);
    }

    void LEDManager::setEncodingMode(EncodingModes mode) {
        currentMode = Mode::ENCODING_MODE; // Set the main mode to ENCODING_MODE
        currentEncodingMode = mode; // Set the specific encoding mode
        // Additional logic to handle the encoding mode can be added here
    }
}#ifndef ROVER_VIEW_MANAGER_H
#define ROVER_VIEW_MANAGER_H

#include "TFT_eSPI.h"
#include "../PrefrontalCortex/utilities.h"
#include "VisualSynesthesia.h"
#include "../PrefrontalCortex/PowerManager.h"
#include "../AuditoryCortex/SoundFxManager.h"
#include <vector>
#include "../SomatosensoryCortex/MenuManager.h"
#include "../MotorCortex/PinDefinitions.h"

namespace VisualCortex 
{
    extern TFT_eSPI tft;
    extern TFT_eSprite spr;

    class RoverViewManager {
        
    public:
        enum ViewType { 
            TODO_LIST,
            CHAKRAS,
            VIRTUES,
            QUOTES,
            WEATHER,
            STATS,
            NEXTMEAL,
            NUM_VIEWS
        };

        static ViewType currentView;
        
        static void init();
        static void setCurrentView(ViewType view);
        static void nextView();
        static void previousView();
        static void drawStatusBar();
        static void drawCurrentView();
        static ViewType getCurrentView() { return currentView; }
        static void drawLoadingScreen(const char* statusText);  
        static void incrementExperience(uint16_t amount);
        static void drawAppSplash(const char* title, const char* description);

        static void setTextColor(uint16_t color);
        static void drawString(const char* str, int x, int y);

        struct Notification {
            const char* header;
            const char* content;
            const char* symbol;
            unsigned long startTime;
            int duration;
        };

        static void showNotification(const char* header, const char* content, const char* symbol, int duration = 3000);
        static void drawNotification();
        static void clearNotification();
        static bool hasActiveNotification();

        enum class InputType {
            INPUT_LEFT,
            INPUT_RIGHT
        };
        static void handleInput(InputType input);

        static void drawFullScreenMenu(const char* title, const std::vector<SomatosensoryCortex::MenuItem>& items, int selectedIndex);

        static void drawMenuBackground();
        static String formatUptime(unsigned long uptimeMillis);
        static void drawErrorScreen(uint32_t errorCode, const char* errorMessage, bool isFatal);
        static uint32_t errorCode;
        static const char* errorMessage;
        static bool isError;
        static bool isFatalError;

        static String wordWrap(String text, int maxWidth);
        static void drawWordWrappedText(const char* text, int x, int y, int maxWidth);

        static void clearSprite();
        static void pushSprite();

    private:
        static int currentFrameX;
        static int currentFrameY;
        
        struct ChakraInfo {
            const char* name;
            const char* attributes;
            uint16_t color;
            void (*drawSymbol)(int x, int y, int size);
        };

        static const int STATUS_BAR_Y = 170;
        static const int STATUS_BAR_HEIGHT = 30;
        static unsigned long lastStatusUpdate;
        static const unsigned long STATUS_CHANGE_INTERVAL = 3000;
        static int statusRotation;

        struct VirtueInfo {
            const char* virtue;
            const char* description;
            uint16_t color;
            void (*drawSymbol)(int x, int y, int size);
        };

        static const ChakraInfo CHAKRA_DATA[];
        static const VirtueInfo VIRTUE_DATA[];


        static const int FRAME_X = 2;
        static const int FRAME_Y = 190;
        static const int FRAME_WIDTH = 280;
        static const int FRAME_HEIGHT = 120;
        static const int TITLE_Y_OFFSET = 15;
        static const int CONTENT_LEFT_OFFSET = 35;
        static const uint16_t FRAME_COLOR = 0xC618;
        static const uint16_t FRAME_BORDER_COLOR = TFT_DARKGREY;


        
        // Drawing methods for different views
        static void drawFrame();
        static void drawTodoList();
        static void drawQuotes();
        static void drawWeather();
        static void drawStats();
        static void drawNextMeal();
        static void drawChakras();
        static void drawVirtues();
        static void drawRootChakra(int x, int y, int size);
        static void drawSacralChakra(int x, int y, int size);
        static void drawSolarChakra(int x, int y, int size);
        static void drawHeartChakra(int x, int y, int size);
        static void drawThroatChakra(int x, int y, int size);
        static void drawThirdEyeChakra(int x, int y, int size);
        static void drawCrownChakra(int x, int y, int size);
        static void drawChastitySymbol(int x, int y, int size);
        static void drawTemperanceSymbol(int x, int y, int size);
        static void drawCharitySymbol(int x, int y, int size);
        static void drawDiligenceSymbol(int x, int y, int size);
        static void drawForgivenessSymbol(int x, int y, int size);
        static void drawKindnessSymbol(int x, int y, int size);
        static void drawHumilitySymbol(int x, int y, int size);
        static void drawBatteryCharging(int x, int y, int size);
        static void drawBattery(int x, int y, int size);
        

        static void updateExperienceBar(const String& expStr);


        static uint32_t experience;
        static uint16_t experienceToNextLevel;
        static uint8_t level;
        static uint16_t calculateNextLevelExperience(uint8_t currentLevel);

        static void drawSymbol(const char* symbol, int x, int y, int size);
        static Notification currentNotification;
        static bool notificationActive;

        // Animation timing
        static unsigned long lastCounterUpdate;
        static const unsigned long COUNTER_SPEED = 1000;  // 1 second interval
        static unsigned long lastAnimationStep;
        static const unsigned long ANIMATION_DELAY = 250; // 250ms between steps
        static bool isAnimating;
        static int animationStep;
        static const int TOTAL_ANIMATION_STEPS = 14;

        static uint32_t roverExperience;
        static uint32_t roverExperienceToNextLevel;
        static uint8_t roverLevel;

        // Expression timing
        static unsigned long lastExpressionChange;
        static unsigned long nextExpressionInterval;
        static const unsigned long DEFAULT_EXPRESSION_INTERVAL = 60000; // 1 minute default

        static constexpr unsigned long WARNING_DURATION = 3000; // 3 seconds
        static unsigned long warningStartTime;
    };
}

#endif 
#include "RoverViewManager.h"
#include "../PrefrontalCortex/utilities.h"
#include "../PrefrontalCortex/PowerManager.h"
#include "DisplayConfig.h"
#include "RoverManager.h"
#include "../PsychicCortex/NFCManager.h"
#include "../AuditoryCortex/SoundFxManager.h"
#include "../VisualCortex/LEDManager.h"
#include "../MotorCortex/PinDefinitions.h"
#include "../PrefrontalCortex/RoverBehaviorManager.h"
#include <TFT_eSPI.h>
#include "../PrefrontalCortex/SDManager.h"
#include "../PsychicCortex/WiFiManager.h"
#include "../CorpusCallosum/SynapticPathways.h"

using namespace CorpusCallosum;

namespace VisualCortex 
{
    TFT_eSPI tft = TFT_eSPI();
    TFT_eSprite spr = TFT_eSprite(&tft);

    // Initialize static members
    RoverViewManager::ViewType RoverViewManager::currentView = RoverViewManager::VIRTUES;
    unsigned long RoverViewManager::lastStatusUpdate = 0;
    int RoverViewManager::statusRotation = 0;
    int RoverViewManager::currentFrameX = 0;
    int RoverViewManager::currentFrameY = 0;
    uint32_t RoverViewManager::experience = 0;
    uint16_t RoverViewManager::experienceToNextLevel = 100;
    uint8_t RoverViewManager::level = 1;
    RoverViewManager::Notification RoverViewManager::currentNotification = {"", "", "", 0, 0};
    bool RoverViewManager::notificationActive = false;
    unsigned long RoverViewManager::lastCounterUpdate = 0;
    unsigned long RoverViewManager::lastAnimationStep = 0;
    bool RoverViewManager::isAnimating = false;
    int RoverViewManager::animationStep = 0;
    unsigned long RoverViewManager::lastExpressionChange = 0;
    unsigned long RoverViewManager::nextExpressionInterval = DEFAULT_EXPRESSION_INTERVAL;

    uint32_t RoverViewManager::roverExperience = 0;
    uint32_t RoverViewManager::roverExperienceToNextLevel = 327;
    uint8_t RoverViewManager::roverLevel = 1;

    bool RoverViewManager::isError = false;
    bool RoverViewManager::isFatalError = false;
    uint32_t RoverViewManager::errorCode = 0;
    const char* RoverViewManager::errorMessage = nullptr;

    // Forward declare all drawing functions
    void drawRootChakra(int x, int y, int size);
    void drawSacralChakra(int x, int y, int size);
    void drawSolarChakra(int x, int y, int size);
    void drawHeartChakra(int x, int y, int size);
    void drawThroatChakra(int x, int y, int size);
    void drawThirdEyeChakra(int x, int y, int size);
    void drawCrownChakra(int x, int y, int size);
    void drawChastitySymbol(int x, int y, int size);
    void drawTemperanceSymbol(int x, int y, int size);
    void drawCharitySymbol(int x, int y, int size);
    void drawDiligenceSymbol(int x, int y, int size);
    void drawForgivenessSymbol(int x, int y, int size);
    void drawKindnessSymbol(int x, int y, int size);
    void drawHumilitySymbol(int x, int y, int size);
    void drawBatteryCharging(int x, int y, int size);
    void drawBattery(int x, int y, int size);

    // Update array names to match header
    const RoverViewManager::ChakraInfo RoverViewManager::CHAKRA_DATA[] = {
        {"Root Chakra", "Survival, Grounding, \nStability, Comfort, Safety", TFT_RED, drawRootChakra},
        {"Sacral Chakra", "Sensuality, Sexuality, \nPleasure, Creativity, Emotions", 0xFDA0, drawSacralChakra},
        {"Solar Plexus Chakra", "Strength, Ego, Power, \nSelf-esteem, Digestion", 0xFFE0, drawSolarChakra},
        {"Heart Chakra", "Love, Acceptance, Compassion, \nKindness, Peace", 0x07E0, drawHeartChakra},
        {"Throat Chakra", "Communication, Expression, \nHonesty, Purification", 0x001F, drawThroatChakra},
        {"Third Eye Chakra", "Intuition, Visualization, \nImagination, Clairvoyance", 0x180E, drawThirdEyeChakra},
        {"Crown Chakra", "Knowledge, Fulfillment, \nSpirituality, Reality", 0x780F, drawCrownChakra}
    };

    const RoverViewManager::VirtueInfo RoverViewManager::VIRTUE_DATA[] = {
        {"Chastity cures Lust", "Purity \nquells \nexcessive sexual appetites", TFT_RED, drawChastitySymbol},
        {"Temperance cures Gluttony", "Self-restraint \nquells \nover-indulgence", 0xFDA0, drawTemperanceSymbol},
        {"Charity cures Greed", "Giving \nquells \navarice", 0xFFE0, drawCharitySymbol},
        {"Diligence cures Sloth", "Integrity and effort \nquells \nlaziness", 0x07E0, drawDiligenceSymbol},
        {"Forgiveness cures Wrath", "Keep composure \nto quell \nanger", 0x001F, drawForgivenessSymbol},
        {"Kindness cures Envy", "Admiration \nquells \njealousy", 0x180E, drawKindnessSymbol},
        {"Humility cures Pride", "Humbleness \nquells \nvanity", 0x780F, drawHumilitySymbol}
    };

    uint16_t getCurrentDayColor() {
        time_t now = time(nullptr);
        struct tm* timeInfo = localtime(&now);
        CRGB dayColor = VisualSynesthesia::getDayColor(timeInfo->tm_wday + 1);
        return VisualSynesthesia::convertToRGB565(dayColor);
    }

    void RoverViewManager::init() {
        tft.init();
        tft.setRotation(0);
        tft.writecommand(TFT_SLPOUT);
        delay(120);
        tft.writecommand(TFT_DISPON);
        
        if (!spr.createSprite(SCREEN_WIDTH, SCREEN_HEIGHT)) 
        {
            throw std::runtime_error("Sprite creation failed");
        }
        spr.setTextDatum(MC_DATUM);
        
        // Create sprite with screen dimensions
        spr.createSprite(SCREEN_WIDTH, SCREEN_HEIGHT);
        spr.fillSprite(TFT_BLACK);
        spr.pushSprite(0, 0);
        
        // Draw initial frame
        RoverViewManager::drawFrame();
        RoverViewManager::drawCurrentView();
    }

    void RoverViewManager::setCurrentView(ViewType view) {
        Utilities::LOG_DEBUG("Changing view to: %d", view);
        currentView = view;
    }

    void RoverViewManager::nextView() {
        Utilities::LOG_DEBUG("Changing from view %d to next view", currentView);
        currentView = static_cast<ViewType>((currentView + 1) % ViewType::NUM_VIEWS);
        Utilities::LOG_DEBUG("New view is %d", currentView);
        drawCurrentView();
    }

    void RoverViewManager::previousView() {
        Utilities::LOG_DEBUG("Changing from view %d to previous view", currentView);
        currentView = static_cast<ViewType>((currentView - 1 + ViewType::NUM_VIEWS) % ViewType::NUM_VIEWS);
        Utilities::LOG_DEBUG("New view is %d", currentView);
        drawCurrentView();
    }

    void RoverViewManager::drawCurrentView() {
        LOG_SCOPE("Drawing current view");
        if (MenuManager::isVisible()) {
            return;
        }
        static bool isRecovering = false;
        
        try {
            if (isRecovering) {
                spr.fillSprite(TFT_BLACK);
                spr.setTextFont(2);
                spr.setTextColor(TFT_WHITE, TFT_BLACK);
                spr.drawString("Display Error", SCREEN_CENTER_X, SCREEN_HEIGHT/2);
                spr.pushSprite(0, 0);
                delay(1000);
                isRecovering = false;
                return;
            }
            
            spr.fillSprite(TFT_BLACK);
            

            
            RoverViewManager::drawFrame();
            RoverViewManager::drawStatusBar();
            
            spr.setTextColor(TFT_BLACK, FRAME_COLOR);
            spr.setTextDatum(MC_DATUM);
            
            switch(currentView) {
                case TODO_LIST:
                    RoverViewManager::drawTodoList();
                    break;
                case CHAKRAS:
                    RoverViewManager::drawChakras();
                    break;
                case VIRTUES:
                    RoverViewManager::drawVirtues();
                    break;
                case QUOTES:
                    RoverViewManager::drawQuotes();
                    break;
                case WEATHER:
                    RoverViewManager::drawWeather();
                    break;
                case STATS:
                    RoverViewManager::drawStats();
                    break;
                case NEXTMEAL:
                    RoverViewManager::drawNextMeal();
                    break;
                default:
                    break;
            }
            
            spr.pushSprite(0, 0);
            
        } catch (const std::exception& e) {
            Utilities::LOG_ERROR("Error in drawCurrentView: %s", e.what());
            isRecovering = true;
            RoverViewManager::drawCurrentView();  // Try again with error recovery
        }
    }

    void RoverViewManager::drawLoadingScreen(const char* statusText) {
        static unsigned long lastBoneRotation = 0;
        static int rotationAngle = 0;
        
        // Create temporary sprite for bone
        TFT_eSprite boneSpr = TFT_eSprite(&tft);
        boneSpr.createSprite(80, 80);
        boneSpr.fillSprite(TFT_BLACK);
        
        // Draw bone centered in sprite
        int tempX = 20;  // Center of sprite
        int tempY = 20;  // Center of sprite
        int boneWidth = 40;
        int boneHeight = 15;
        int circleRadius = 8;
        
        // Draw bone components
        boneSpr.fillRect(tempX - boneWidth/2, tempY - boneHeight/2, boneWidth, boneHeight, TFT_WHITE);
        boneSpr.fillCircle(tempX - boneWidth/2, tempY - boneHeight/2, circleRadius, TFT_WHITE);
        boneSpr.fillCircle(tempX - boneWidth/2, tempY + boneHeight/2, circleRadius, TFT_WHITE);
        boneSpr.fillCircle(tempX + boneWidth/2, tempY - boneHeight/2, circleRadius, TFT_WHITE);
        boneSpr.fillCircle(tempX + boneWidth/2, tempY + boneHeight/2, circleRadius, TFT_WHITE);
        
        spr.fillSprite(TFT_BLACK);
        
        // Keep text centered
        spr.setTextFont(2);
        spr.setTextColor(TFT_WHITE);
        spr.drawCentreString("Loading...", SCREEN_CENTER_X - 25, 75, 2);
        
        if (statusText) {
            spr.setTextFont(1);
            spr.drawCentreString(statusText, SCREEN_CENTER_X - 25, 100, 1);
        }
        
        // Push rotated bone sprite to main sprite
        boneSpr.pushRotated(&spr, rotationAngle);
        
        if (millis() - lastBoneRotation > 50) {
            rotationAngle = (rotationAngle + 45) % 360;
            lastBoneRotation = millis();
        }
        
        boneSpr.deleteSprite();
        
        // Push main sprite to display
        spr.pushSprite(0, 0);
    }

    void RoverViewManager::drawFrame() {
        LOG_SCOPE("Drawing frame");
        int frameX = (TFT_WIDTH - FRAME_WIDTH) / 2;
        int frameY = FRAME_Y;
        
        // Draw the frame
        spr.fillRect(frameX, frameY, FRAME_WIDTH, FRAME_HEIGHT + 10, FRAME_COLOR);
        spr.drawRect(frameX - 1, frameY - 1, FRAME_WIDTH + 2, FRAME_HEIGHT + 10, FRAME_BORDER_COLOR);
        
        // Store frame position for content alignment
        currentFrameX = frameX;
        currentFrameY = frameY;
    }

    void RoverViewManager::drawTodoList() {
        LOG_SCOPE("Drawing TODO list view");

        spr.setTextFont(4); // Larger font for header
        spr.setTextColor(TFT_BLACK, FRAME_COLOR);
        spr.drawString("Today's Tasks:", SCREEN_CENTER_X - FRAME_OFFSET_X, FRAME_Y + TITLE_Y_OFFSET);

        spr.setTextFont(2); // Regular font for content
        String task1 = RoverViewManager::wordWrap("1. Service Canada", SCREEN_WIDTH - 50);
        String task2 = RoverViewManager::wordWrap("2. Call Doctor", SCREEN_WIDTH - 50);
        String task3 = RoverViewManager::wordWrap("3. Call Therapist", SCREEN_WIDTH - 50);

        spr.drawString(task1, SCREEN_CENTER_X - FRAME_OFFSET_X, FRAME_Y + 35);
        spr.drawString(task2, SCREEN_CENTER_X - FRAME_OFFSET_X, FRAME_Y + 55);
        spr.drawString(task3, SCREEN_CENTER_X - FRAME_OFFSET_X, FRAME_Y + 75);
    }

    void RoverViewManager::drawQuotes() {
        LOG_SCOPE("Drawing quotes view");

        spr.setTextFont(4); // Larger font for header
        spr.setTextColor(TFT_BLACK, FRAME_COLOR);
        spr.drawString("Quote of the Day", SCREEN_CENTER_X - FRAME_OFFSET_X, FRAME_Y + TITLE_Y_OFFSET);

        spr.setTextFont(2); // Regular font for content
        String quote1 = RoverViewManager::wordWrap("\"The best way to predict", SCREEN_WIDTH - 50);
        String quote2 = RoverViewManager::wordWrap("the future is to create it.\"", SCREEN_WIDTH - 50);
        String author = RoverViewManager::wordWrap("- Peter Drucker", SCREEN_WIDTH - 50);

        spr.drawString(quote1, SCREEN_CENTER_X - FRAME_OFFSET_X, FRAME_Y + 45);
        spr.drawString(quote2, SCREEN_CENTER_X - FRAME_OFFSET_X, FRAME_Y + 65);
        spr.drawString(author, SCREEN_CENTER_X - FRAME_OFFSET_X, FRAME_Y + 85);
    }

    void RoverViewManager::drawWeather() {
        LOG_SCOPE("Drawing weather view");

        spr.setTextFont(4); // Larger font for header
        spr.setTextColor(TFT_BLACK, FRAME_COLOR);
        spr.drawString("Weather", SCREEN_CENTER_X - FRAME_OFFSET_X, FRAME_Y + TITLE_Y_OFFSET + 15);

        spr.setTextFont(2); // Regular font for content
        String weather = RoverViewManager::wordWrap("Sunny", SCREEN_WIDTH - 50);
        String temperature = RoverViewManager::wordWrap("72°F / 22°C", SCREEN_WIDTH - 50);
        String humidity = RoverViewManager::wordWrap("Humidity: 45%", SCREEN_WIDTH - 50);

        spr.drawString(weather, SCREEN_CENTER_X - FRAME_OFFSET_X, FRAME_Y + 55);
        spr.drawString(temperature, SCREEN_CENTER_X - FRAME_OFFSET_X, FRAME_Y + 75);
        spr.drawString(humidity, SCREEN_CENTER_X - FRAME_OFFSET_X, FRAME_Y + 95);
    }

    void RoverViewManager::drawStats() {
        LOG_SCOPE("Drawing stats view");
        
        spr.setTextFont(3);
        spr.setTextColor(TFT_BLACK, FRAME_COLOR);
        spr.drawString("System Stats", SCREEN_CENTER_X - FRAME_OFFSET_X, FRAME_Y + TITLE_Y_OFFSET + 15);
        
        spr.setTextFont(2);
        unsigned long uptime = millis(); // Get the uptime in milliseconds
        String uptimeString = RoverViewManager::formatUptime(uptime);
        
        spr.drawString("Uptime: " + uptimeString, SCREEN_CENTER_X - FRAME_OFFSET_X, FRAME_Y + 15);
        spr.drawString("SD Card Size: " + String(SDManager::getCardSize()) + "MB", SCREEN_CENTER_X - FRAME_OFFSET_X, FRAME_Y + 75);
        spr.drawString("SD Card Used: " + String(SDManager::getUsedSpace()) + "MB", SCREEN_CENTER_X - FRAME_OFFSET_X, FRAME_Y + 95);
        spr.drawString("SD Card Total: " + String(SDManager::getTotalSpace()) + "MB", SCREEN_CENTER_X - FRAME_OFFSET_X, FRAME_Y + 115);
        spr.drawString("WiFi: " + String(WiFiManager::isConnected() ? "Connected" : "Disconnected"), SCREEN_CENTER_X - FRAME_OFFSET_X, FRAME_Y + 135);
    }

    String RoverViewManager::formatUptime(unsigned long uptimeMillis) {
        unsigned long seconds = uptimeMillis / 1000;
        unsigned long minutes = seconds / 60;
        unsigned long hours = minutes / 60;

        seconds %= 60;
        minutes %= 60;

        String formattedUptime = String(hours) + "h " + String(minutes) + "m " + String(seconds) + "s";
        return formattedUptime;
    }

    // Define symbols for each virtue
    void RoverViewManager::drawChastitySymbol(int x, int y, int size) {
        // Pure white lily
        spr.drawCircle(x, y, size/2, TFT_RED);
        spr.drawLine(x, y - size/2, x, y + size/2, TFT_RED);
        spr.drawLine(x - size/2, y, x + size/2, y, TFT_RED);
    }

    void RoverViewManager::drawTemperanceSymbol(int x, int y, int size) {
        uint16_t symbolColor = getCurrentDayColor();
        
        spr.drawLine(x - size/2, y, x + size/2, y, symbolColor);
        spr.fillTriangle(x, y + size/6, x - size/6, y + size/6, x + size/6, y + size/6, symbolColor);
        spr.drawLine(x, y + size/6, x, y + size/2, symbolColor);
        spr.drawLine(x - size/4, y + size/2, x + size/4, y + size/2, symbolColor);
        spr.fillCircle(x - size/2, y, size/8, symbolColor);
        spr.fillCircle(x + size/2, y, size/8, symbolColor);
    }

    void RoverViewManager::drawCharitySymbol(int x, int y, int size) {
        uint16_t symbolColor = getCurrentDayColor();
        spr.fillCircle(x - size/4, y - size/4, size/4, symbolColor);
        spr.fillCircle(x + size/4, y - size/4, size/4, symbolColor);
        spr.fillTriangle(x, y + size/3, x - size/2, y - size/6, x + size/2, y - size/6, symbolColor);
    }

    void RoverViewManager::drawDiligenceSymbol(int x, int y, int size) {
        uint16_t symbolColor = getCurrentDayColor();
        for(int i = 0; i < 6; i++) {
            float angle = i * PI / 3;
            int x1 = x + cos(angle) * size/2;
            int y1 = y + sin(angle) * size/2;
            spr.drawLine(x, y, x1, y1, symbolColor);
        }
    }

    void RoverViewManager::drawForgivenessSymbol(int x, int y, int size) {
        uint16_t symbolColor = getCurrentDayColor();
        spr.drawCircle(x, y, size/2, symbolColor);
        spr.drawCircle(x, y, size/3, symbolColor);
        spr.drawLine(x - size/2, y, x + size/2, y, symbolColor);
    }

    void RoverViewManager::drawKindnessSymbol(int x, int y, int size) {
        uint16_t symbolColor = getCurrentDayColor();
        spr.drawRect(x - size/2, y - size/2, size, size, symbolColor);
        spr.drawLine(x - size/2, y, x + size/2, y, symbolColor);
        spr.drawLine(x, y - size/2, x, y + size/2, symbolColor);
    }

    void RoverViewManager::drawHumilitySymbol(int x, int y, int size) {
        uint16_t symbolColor = getCurrentDayColor();
        spr.drawCircle(x, y, size/2, symbolColor);
        spr.drawLine(x, y, x, y + size/2, symbolColor);
    }

    void RoverViewManager::drawBatteryCharging(int x, int y, int size) {
        // Draw base battery first
        drawBattery(x, y, size);
        
        // Add lightning bolt overlay
        int batteryX = x - size/2;
        int batteryY = y - size/4;
        
        // Lightning bolt
        spr.fillTriangle(x - 2, batteryY + 2, x - size/4, y, x + 2, y, TFT_YELLOW);
        spr.fillTriangle(x - 2, y, x + size/4, y, x + 2, batteryY + size/2 - 2, TFT_YELLOW);
    }

    void RoverViewManager::drawBattery(int x, int y, int size) {
        int batteryWidth = size;
        int batteryHeight = size/2;
        int batteryX = x - size/2;
        int batteryY = y - size/4;
        
        // Main battery body
        spr.drawRect(batteryX, batteryY, batteryWidth, batteryHeight, TFT_WHITE);
        
        // Battery terminal
        int terminalWidth = size/8;
        int terminalHeight = batteryHeight/2;
        spr.fillRect(batteryX + batteryWidth, batteryY + (batteryHeight - terminalHeight)/2, 
                    terminalWidth, terminalHeight, TFT_WHITE);
        
        // Fill level
        int fillWidth = (batteryWidth - 4) * PowerManager::getBatteryPercentage() / 100;
        spr.fillRect(batteryX + 2, batteryY + 2, fillWidth, batteryHeight - 4, TFT_WHITE);
    }

    void RoverViewManager::drawChakras() {
        LOG_SCOPE("Drawing chakras view");

        // Set unique font for the header
        spr.setTextFont(5); // Unique font for chakras header
        spr.setTextColor(TFT_BLACK, FRAME_COLOR);
        spr.drawString("Chakras", SCREEN_CENTER_X - 40, FRAME_Y + TITLE_Y_OFFSET); // Centered header

        // Set regular font for content
        spr.setTextFont(2);
        int y = FRAME_Y + 45; // Start drawing content below the header

        for (int i = 0; i < sizeof(CHAKRA_DATA) / sizeof(CHAKRA_DATA[0]); i++) {
            const ChakraInfo& chakra = CHAKRA_DATA[i];
            
            // Draw chakra symbol
            chakra.drawSymbol(SCREEN_CENTER_X - 40, y, 20); // Adjust position for symbol
            y += 25; // Move down for the next chakra

            // Draw attributes with word wrap
            String wrappedAttributes = wordWrap(chakra.attributes, SCREEN_WIDTH - 50);
            spr.drawString(wrappedAttributes, SCREEN_CENTER_X - 40, y);
            y += 40; // Add space for the next chakra
        }
    }

    void RoverViewManager::drawVirtues() {
        LOG_SCOPE("Drawing virtues view");

        // Set unique font for the header
        spr.setTextFont(5); // Unique font for virtues header
        spr.setTextColor(TFT_BLACK, FRAME_COLOR);
        spr.drawString("Virtues", SCREEN_CENTER_X - 40, FRAME_Y + TITLE_Y_OFFSET); // Centered header

        // Set regular font for content
        spr.setTextFont(2);
        int y = FRAME_Y + 45; // Start drawing content below the header

        for (int i = 0; i < sizeof(VIRTUE_DATA) / sizeof(VIRTUE_DATA[0]); i++) {
            const VirtueInfo& virtue = VIRTUE_DATA[i];

            // Draw attributes with word wrap
            String wrappedDescription = wordWrap(virtue.description, SCREEN_WIDTH - 50);
            spr.drawString(wrappedDescription, SCREEN_CENTER_X - 40, y);
            y += 40; // Add space for the next virtue
        }
    }

    void RoverViewManager::drawNextMeal() {
        LOG_SCOPE("Drawing recipe view");
        
        spr.setTextFont(4);
        spr.setTextColor(TFT_BLACK, FRAME_COLOR);
        spr.drawString("Garlic Pasta", SCREEN_CENTER_X, FRAME_Y + TITLE_Y_OFFSET + 15);
        
        spr.setTextFont(2);
        
        const char* ingredients[] = {
            "8 oz Spaghetti",
            "4 Garlic cloves",
            "3 tbsp Olive oil",
            "Salt & Pepper",
        };
        
        int y = FRAME_Y + 55;
        for(int i = 0; i < 4; i++) {
            spr.drawString(ingredients[i], SCREEN_CENTER_X, y);
            y += 20;
        }
    }

    // Implement all the chakra drawing functions
    void RoverViewManager::drawRootChakra(int x, int y, int size) {
        // Basic square with downward triangle for root chakra
        spr.drawRect(x - size/2, y - size/2, size, size, TFT_RED);
        spr.fillTriangle(x, y + size/2, x - size/2, y - size/2, x + size/2, y - size/2, TFT_RED);
    }

    void RoverViewManager::drawSacralChakra(int x, int y, int size) {
        // Crescent moon shape for sacral chakra
        spr.drawCircle(x, y, size/2, 0xFDA0);
        spr.drawCircle(x + size/4, y, size/2, 0xFDA0);
    }

    void RoverViewManager::drawSolarChakra(int x, int y, int size) {
        // Sun-like pattern for solar plexus
        spr.drawCircle(x, y, size/2, 0xFFE0);
        for(int i = 0; i < 8; i++) {
            float angle = i * PI / 4;
            int x1 = x + cos(angle) * size/2;
            int y1 = y + sin(angle) * size/2;
            spr.drawLine(x, y, x1, y1, 0xFFE0);
        }
    }

    void RoverViewManager::drawHeartChakra(int x, int y, int size) {
        // Heart shape for heart chakra
        spr.fillCircle(x - size/4, y, size/4, 0x07E0);
        spr.fillCircle(x + size/4, y, size/4, 0x07E0);
        spr.fillTriangle(x - size/2, y, x + size/2, y, x, y + size/2, 0x07E0);
    }

    void RoverViewManager::drawThroatChakra(int x, int y, int size) {
        // Circle with wings for throat chakra
        spr.drawCircle(x, y, size/3, 0x001F);
        spr.drawLine(x - size/2, y, x + size/2, y, 0x001F);
        spr.drawLine(x - size/2, y - size/4, x + size/2, y - size/4, 0x001F);
    }

    void RoverViewManager::drawThirdEyeChakra(int x, int y, int size) {
        // Eye shape for third eye chakra
        spr.drawEllipse(x, y, size/2, size/3, 0x180E);
        spr.fillCircle(x, y, size/6, 0x180E);
    }

    void RoverViewManager::drawCrownChakra(int x, int y, int size) {
        // Crown-like pattern
        for(int i = 0; i < 7; i++) {
            int x1 = x - size/2 + (i * size/6);
            spr.drawLine(x1, y + size/2, x1, y - size/2, 0x780F);
        }
        spr.drawLine(x - size/2, y - size/2, x + size/2, y - size/2, 0x780F);
    }

    void RoverViewManager::drawStatusBar() {
        LOG_SCOPE("Drawing status bar");
        try {
            time_t now = time(nullptr);
            if (now == -1) {
                LOG_PROD("Error getting time in drawStatusBar");
                return;
            }
            
            struct tm* timeInfo = localtime(&now);
            if (!timeInfo) {
                LOG_PROD("Error converting time in drawStatusBar");
                return;
            }
            
            // Status bar positioning
            int dateWidth = 40;
            int dateHeight = 30;
            int dateX = 0;  // Changed from 2 to 0
            
            // Get month colors
            CRGB monthColor1, monthColor2;
            VisualSynesthesia::getMonthColors(timeInfo->tm_mon + 1, monthColor1, monthColor2);
            
            // Draw month color square
            uint32_t monthTftColor = spr.color565(monthColor1.r, monthColor1.g, monthColor1.b);
            if (monthColor1.r == monthColor2.r && 
                monthColor1.g == monthColor2.g && 
                monthColor1.b == monthColor2.b) {
                spr.fillRect(dateX, STATUS_BAR_Y - 5, dateWidth, dateHeight, monthTftColor);  // Added -2 to Y
            } else {
                for (int i = 0; i < dateWidth; i++) {
                    float ratio = (float)i / dateWidth;
                    uint8_t r = monthColor1.r + (monthColor2.r - monthColor1.r) * ratio;
                    uint8_t g = monthColor1.g + (monthColor2.g - monthColor1.g) * ratio;
                    uint8_t b = monthColor1.b + (monthColor2.b - monthColor1.b) * ratio;
                    uint32_t tftColor = spr.color565(r, g, b);
                    spr.drawFastVLine(dateX + i, STATUS_BAR_Y - 5, dateHeight, tftColor);  // Added -2 to Y
                }
            }
            
            // Draw day number
            char dayStr[3];
            sprintf(dayStr, "%d", timeInfo->tm_mday);
            spr.setTextFont(2);
            spr.setTextColor(TFT_WHITE, monthTftColor);
            spr.drawString(dayStr, dateX + dateWidth/2, STATUS_BAR_Y + dateHeight/2);
            
            // Status text section
            int statusX = dateX + dateWidth + 30;
            
            if (millis() - lastStatusUpdate >= STATUS_CHANGE_INTERVAL) {
                statusRotation = (statusRotation + 1) % 2;
                lastStatusUpdate = millis();
            }
            
            spr.setTextFont(2);
            spr.setTextColor(TFT_WHITE, TFT_BLACK);
            
            switch (statusRotation) {
                case 0:
                    char statsStr[20];
                    sprintf(statsStr, "Lvl:%d Exp:%d", 
                            (roverLevel / 10) + 1,  // Level increases every 10 scans
                            roverExperience);
                    spr.drawString(statsStr, statusX + 35, STATUS_BAR_Y + dateHeight/2);
                    break;
                case 1:
                    if (PowerManager::isCharging()) {
                        drawBatteryCharging(statusX, STATUS_BAR_Y + dateHeight/2, 19);
                        char batteryStr[5];
                        sprintf(batteryStr, "%d%%", PowerManager::getBatteryPercentage());
                        spr.drawString(batteryStr, statusX + 45, STATUS_BAR_Y + dateHeight/2);
                    } else {
                        drawBattery(statusX, STATUS_BAR_Y + dateHeight/2, 19);
                        char batteryStr[5];
                        sprintf(batteryStr, "%d%%", PowerManager::getBatteryPercentage());
                        spr.drawString(batteryStr, statusX + 45, STATUS_BAR_Y + dateHeight/2);
                    }
                    break;
            }
            
        } catch (const std::exception& e) {
            LOG_PROD("Error in drawStatusBar: %s", e.what());
        }
    }   

    void RoverViewManager::incrementExperience(uint16_t amount) {
        experience += amount;
        
        while (experience >= 327) {
            level++;
            experience -= 327;
            LEDManager::flashLevelUp();
            
            char levelStr[32];
            snprintf(levelStr, sizeof(levelStr), "Level %d!", level);
            showNotification("LEVEL UP", levelStr, "XP", 2000);
        }
        
        // Update experience display
        char expStr[32];
        snprintf(expStr, sizeof(expStr), "XP: %d/327", experience);
        updateExperienceBar(expStr);
    }

    uint16_t RoverViewManager::calculateNextLevelExperience(uint8_t currentLevel) {
        // Simple exponential growth formula
        return 100 * (currentLevel + 1);
    }       

    void RoverViewManager::showNotification(const char* header, const char* content, const char* symbol, int duration) {
        currentNotification = {header, content, symbol, millis(), duration};
        notificationActive = true;
        drawNotification();
    }

    void RoverViewManager::drawNotification() {
        if (!notificationActive) return;
        
        // Fill entire screen with dark background
        spr.fillSprite(TFT_BLACK);
        
        // Draw full-height notification box
        int boxWidth = 160;
        int boxHeight = SCREEN_HEIGHT - 20; // Full height minus margins
        int boxX = 10;
        int boxY = 10;
        
        spr.fillRoundRect(boxX, boxY, boxWidth, boxHeight, 8, TFT_DARKGREY);
        spr.drawRoundRect(boxX, boxY, boxWidth, boxHeight, 8, TFT_WHITE);
        
        // Draw header
        spr.setTextFont(2);
        spr.setTextColor(TFT_WHITE);
        spr.drawCentreString(currentNotification.header, boxX + boxWidth/2, boxY + 10, 2);
        
        // If this is an NFC notification, try to read card data
        if (strcmp(currentNotification.symbol, "NFC") == 0) {
            uint32_t cardId = NFCManager::getLastCardId();
            char idStr[32];
            sprintf(idStr, "Card ID: %08X", cardId);
            spr.drawCentreString(idStr, boxX + boxWidth/2, boxY + 40, 2);
            
            // Try to read card data
            if (NFCManager::isCardEncrypted()) {
                drawSymbol("PADLOCK", boxX + boxWidth/2, boxY + boxHeight/2, 40);
                spr.drawCentreString("Encrypted Card", boxX + boxWidth/2, boxY + boxHeight - 60, 2);
            } else {
                // Show card data if available
                const char* cardData = NFCManager::getCardData();
                if (cardData) {
                    RoverViewManager::drawWordWrappedText(cardData, boxX + 10, boxY + 80, boxWidth - 20);
                }
            }
        } else {
            // Regular notification display
            RoverViewManager::drawSymbol(currentNotification.symbol, boxX + boxWidth/2, boxY + boxHeight/2, 40);
            RoverViewManager::drawWordWrappedText(currentNotification.content, boxX + 100, boxY + boxHeight - 60, boxWidth - 20);
        }
    }

    void RoverViewManager::drawSymbol(const char* symbol, int x, int y, int size) {
        if (strcmp(symbol, "PADLOCK") == 0) {
            // Draw padlock body
            spr.fillRoundRect(x - size/3, y, size*2/3, size/2, size/8, TFT_WHITE);
            // Draw shackle
            spr.drawRoundRect(x - size/2, y - size/3, size, size/2, size/8, TFT_WHITE);
        } else if (strcmp(symbol, "NFC") == 0) {
            // Draw magnifying glass
            int glassSize = size * 0.8;
            // Draw circle
            spr.drawCircle(x, y, glassSize/2, TFT_WHITE);
            // Draw handle
            spr.drawLine(x + (glassSize/2 * 0.7), y + (glassSize/2 * 0.7), 
                        x + glassSize, y + glassSize, TFT_WHITE);
            // Fill circle with thinner border
            spr.fillCircle(x, y, (glassSize/2) - 2, TFT_BLACK);
            spr.drawCircle(x, y, (glassSize/2) - 2, TFT_WHITE);
            // Make handle thicker
            spr.drawLine(x + (glassSize/2 * 0.7) - 1, y + (glassSize/2 * 0.7), 
                        x + glassSize - 1, y + glassSize, TFT_WHITE);
            spr.drawLine(x + (glassSize/2 * 0.7) + 1, y + (glassSize/2 * 0.7), 
                        x + glassSize + 1, y + glassSize, TFT_WHITE);
        }
    }

    void RoverViewManager::clearNotification() {
        notificationActive = false;
        currentNotification = {"", "", "", 0, 0};
    }

    bool RoverViewManager::hasActiveNotification() {
        return notificationActive;
    }   

    void RoverViewManager::handleInput(InputType input) {
        if (hasActiveNotification()) {
            clearNotification();
            return;
        }
        
        switch (input) {
            case InputType::INPUT_LEFT:
                previousView();
                break;
                
            case InputType::INPUT_RIGHT:
                nextView();
                break;
        }
    }

    void RoverViewManager::drawWordWrappedText(const char* text, int x, int y, int maxWidth) {
        if (!text) return;
        
        const int lineHeight = 20;  // Adjust based on your font size
        char buffer[256];
        int currentLine = 0;
        int bufferIndex = 0;
        int lastSpace = -1;
        
        for (int i = 0; text[i] != '\0'; i++) {
            buffer[bufferIndex++] = text[i];
            buffer[bufferIndex] = '\0';
            
            if (text[i] == ' ') {
                lastSpace = bufferIndex - 1;
            }
            
            // Check if current line is too long
            if (spr.textWidth(buffer) > maxWidth) {
                if (lastSpace != -1) {
                    // Break at last space
                    buffer[lastSpace] = '\0';
                    spr.drawString(buffer, x, y + (currentLine * lineHeight));
                    
                    // Start new line from word after space
                    bufferIndex = 0;
                    for (int j = lastSpace + 1; j < i; j++) {
                        buffer[bufferIndex++] = text[j];
                    }
                    buffer[bufferIndex] = '\0';
                    lastSpace = -1;
                } else {
                    // Force break if no space found
                    buffer[bufferIndex-1] = '\0';
                    spr.drawString(buffer, x, y + (currentLine * lineHeight));
                    bufferIndex = 0;
                    buffer[bufferIndex++] = text[i];
                    buffer[bufferIndex] = '\0';
                }
                currentLine++;
            }
        }
        
        // Draw remaining text
        if (bufferIndex > 0) {
            spr.drawString(buffer, x, y + (currentLine * lineHeight));
        }
    }

    void RoverViewManager::drawFullScreenMenu(const char* title, const std::vector<SomatosensoryCortex::MenuItem>& items, int selectedIndex) {
        spr.fillSprite(TFT_BLACK);
        
        // Draw title - adjust position to be more centered
        spr.setTextFont(4);
        spr.setTextColor(TFT_WHITE);
        spr.drawString(title, SCREEN_CENTER_X - 30, 30);
        
        // Draw menu items
        spr.setTextFont(2);
        int y = 70;
        int menuX = SCREEN_CENTER_X - 30;  // Align with title
        
        for (size_t i = 0; i < items.size(); i++) {
            if (i == selectedIndex) {
                spr.setTextColor(TFT_BLACK, TFT_WHITE);
                spr.fillRect(15, y - 10, SCREEN_WIDTH - 30, 20, TFT_WHITE);
            } else {
                spr.setTextColor(TFT_WHITE, TFT_BLACK);
            }
            spr.drawString(items[i].name.c_str(), menuX, y);
            y += 25;
        }
        
        spr.pushSprite(0, 0);
    }

    void RoverViewManager::drawAppSplash(const char* title, const char* description) {
        tft.fillScreen(TFT_BLACK);

        // Draw title near center
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setTextSize(2); // scale up text
        tft.setTextDatum(MC_DATUM); // middle-center
        tft.drawString(title, tft.width() / 2, tft.height() / 2 - 20);

        // Draw description a bit lower
        tft.setTextSize(1);
        tft.setTextDatum(MC_DATUM);
        tft.drawString(description, tft.width() / 2, tft.height() / 2 + 10);

        // Draw instructions at the bottom
        tft.setTextSize(1);
        tft.setTextDatum(BC_DATUM); // bottom-center
        tft.drawString("Press Rotary to Start  |  Side Button = Exit", tft.width() / 2, tft.height() - 5);
    }

    void RoverViewManager::drawMenuBackground() {
        tft.fillScreen(TFT_BLACK);
    }

    void RoverViewManager::setTextColor(uint16_t color) {
        tft.setTextColor(color);
    }

    void RoverViewManager::drawString(const char* str, int x, int y) {
        tft.drawString(str, x, y);
    }

    void RoverViewManager::updateExperienceBar(const String& expStr) {
        roverExperience += expStr.toInt();
        if (roverExperience >= roverExperienceToNextLevel) {
            roverLevel++;
            roverExperience -= roverExperienceToNextLevel;
            roverExperienceToNextLevel = calculateNextLevelExperience(roverLevel);
        }
    }

    void RoverViewManager::drawErrorScreen(uint32_t errorCode, const char* errorMessage, bool isFatal) {
        spr.fillSprite(TFT_BLACK);
        
        // Draw ERRORBYTE text and code centered
        spr.setTextFont(2);
        spr.setTextColor(TFT_RED);  // ERRORBYTE always red
        spr.drawCentreString("ERRORBYTE", SCREEN_CENTER_X - 40, 20, 2);
        
        char errorCodeStr[32];
        sprintf(errorCodeStr, "0x%08X", (uint8_t)errorCode);
        spr.setTextColor(isFatal ? TFT_RED : TFT_YELLOW);
        spr.drawCentreString(errorCodeStr, SCREEN_CENTER_X - 40, 40, 2);
        
        // Define scale first
        static const float scale = 0.8f;  // Make it static const
        
        // Center all rover graphics
        const int roverY = 80;
        const int roverX = SCREEN_CENTER_X - (int)(90*scale/2) - 35;  // Cast to int
        
        // Body and ears - wider body
        spr.fillRect(roverX, roverY, 90*scale, 76*scale, TFT_WHITE);
        spr.fillTriangle(roverX + 10*scale, roverY - 10*scale,
                        roverX + 25*scale, roverY + 5*scale,
                        roverX + 40*scale, roverY - 10*scale, TFT_WHITE);
        spr.fillTriangle(roverX + 60*scale, roverY - 10*scale,
                        roverX + 75*scale, roverY + 5*scale,
                        roverX + 90*scale, roverY - 10*scale, TFT_WHITE);
        
        // Eye panel - shorter width
        spr.fillRect(roverX + 10*scale, roverY + 10*scale, 75*scale, 30*scale, 0x7BEF);
        
        // Draw X eyes with thicker lines
        uint16_t eyeColor = isFatal ? TFT_RED : TFT_YELLOW;
        // Left X - thicker
        for(int i = 0; i < 2; i++) {
            spr.drawLine(roverX + (20+i)*scale, roverY + 15*scale, 
                        roverX + (30+i)*scale, roverY + 25*scale, eyeColor);
            spr.drawLine(roverX + (30+i)*scale, roverY + 15*scale,
                        roverX + (20+i)*scale, roverY + 25*scale, eyeColor);
        }
        // Right X - thicker
        for(int i = 0; i < 2; i++) {
            spr.drawLine(roverX + (60+i)*scale, roverY + 15*scale,
                        roverX + (70+i)*scale, roverY + 25*scale, eyeColor);
            spr.drawLine(roverX + (70+i)*scale, roverY + 15*scale,
                        roverX + (60+i)*scale, roverY + 25*scale, eyeColor);
        }
        
        // Draw triangular nose
        spr.fillTriangle(roverX + 45*scale, roverY + 35*scale, 
                        roverX + 40*scale, roverY + 45*scale, 
                        roverX + 50*scale, roverY + 45*scale, TFT_BLACK);
        
        // Vertical line from nose to mouth
        spr.drawLine(roverX + 45*scale, roverY + 45*scale,
                    roverX + 45*scale, roverY + 55*scale, TFT_BLACK);
        
        // Draw full frown with ends
        spr.drawArc(roverX + 45*scale, roverY + 70*scale, 20*scale, 15*scale, 180, 360, TFT_BLACK, TFT_BLACK);
        // Left end
        spr.fillRect(roverX + 25*scale, roverY + 65*scale, 2*scale, 4*scale, TFT_BLACK);
        // Right end
        spr.fillRect(roverX + 63*scale, roverY + 65*scale, 2*scale, 4*scale, TFT_BLACK);
        
        // Draw error message
        spr.setTextFont(2);
        spr.setTextColor(isFatal ? TFT_RED : TFT_YELLOW);
        spr.drawCentreString(errorMessage, SCREEN_CENTER_X - 40, 160, 2);
        
        // For warnings, show countdown on separate line in white
        if (!isFatal && RoverBehaviorManager::isWarningCountdownActive()) {
            spr.setTextColor(TFT_WHITE);
            char countdownStr[32];
            int remainingSeconds = RoverBehaviorManager::getRemainingWarningSeconds();
            sprintf(countdownStr, "Clearing in %d...", remainingSeconds);
            spr.drawCentreString(countdownStr, SCREEN_CENTER_X - 40, 180, 2);
        }
        
        // Show reboot instruction ONLY for fatal errors
        if (isFatal) {
            spr.setTextFont(1);
            spr.setTextColor(TFT_YELLOW);
            spr.drawCentreString("Press Rotary to REBOOT", SCREEN_CENTER_X - 40, 225, 1);
        }
        
        spr.pushSprite(0, 0);
        LEDManager::setErrorPattern(errorCode, isFatal);
    }

    String RoverViewManager::wordWrap(String text, int maxWidth) {
        String wrappedText;
        int lastSpace = -1;
        int lineLength = 0;

        for (int i = 0; i < text.length(); i++) {
            char c = text.charAt(i);
            lineLength++;

            if (c == ' ') {
                lastSpace = i; // Record the last space
            }

            if (lineLength > maxWidth) {
                if (lastSpace != -1) {
                    wrappedText += text.substring(0, lastSpace) + "\n"; // Add line break
                    text = text.substring(lastSpace + 1); // Update text
                    i = -1; // Reset index for new line
                    lineLength = 0; // Reset line length
                    lastSpace = -1; // Reset last space
                }
            }
        }

        wrappedText += text; // Add any remaining text
        return wrappedText;
    }
}#ifndef IR_MANAGER_H
#define IR_MANAGER_H

#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <string>
#include "../MotorCortex/PinDefinitions.h"

// Forward declarations
namespace PrefrontalCortex { class Utilities; }
namespace VisualCortex { class RoverViewManager; }

namespace PsychicCortex 
{

    class IRManager {
    public:
        static void init();
        static void startBlast();
        static void stopBlast();
        static void update();
        static bool isBlasting() { return blasting; }
        static void handleRotaryTurn(int direction);
        static void handleRotaryPress();
        
    private:
        static unsigned long lastSendTime;
        static unsigned long lastLEDUpdate;
        static uint16_t currentCode;
        static uint8_t currentRegion;
        static uint8_t currentLEDPosition;
        static bool animationDirection;
        static const uint16_t SEND_DELAY_MS = 100;
        static const uint16_t LED_UPDATE_MS = 50;
        static IRsend irsend;
        
        static void sendCode(uint16_t code);
        static void setupIROutput();
        static void updateLEDAnimation();
        static bool blasting;
        
    };

}

#endif#include "NFCManager.h"
#include "../AuditoryCortex/SoundFxManager.h"
#include "../VisualCortex/LEDManager.h"
#include "../VisualCortex/RoverViewManager.h"
#include "../VisualCortex/VisualSynesthesia.h"
#include "../PrefrontalCortex/SDManager.h"
#include "../PrefrontalCortex/Utilities.h"
#include <Wire.h>
#include <Adafruit_PN532.h>
#include <string.h> // For strcmp

#define DEBUG_MODE 


using namespace CorpusCallosum;

namespace PsychicCortex 
{
    Adafruit_PN532 NFCManager::nfc(SDA_PIN, SCL_PIN);
    uint32_t NFCManager::lastCardId = 0;
    uint32_t NFCManager::totalScans = 0;
    bool NFCManager::cardPresent = false;
    unsigned long NFCManager::lastInitAttempt = 0;
    bool NFCManager::initInProgress = false;
    uint8_t NFCManager::initStage = 0;
    bool NFCManager::isProcessingScan = false;
    char NFCManager::cardData[256] = {0};

    // Example valid card IDs
    const char* validCardIDs[] = {
        "ROVER123",
        "BYTE456",
        "ROVERBYTE789"
    };

    void NFCManager::init() {
        PrefrontalCortex::Utilities::LOG_PROD("Starting NFC initialization...");
        Wire.begin(BOARD_I2C_SDA, BOARD_I2C_SCL);
        
        nfc.begin();
        delay(1000);
        
        uint32_t versiondata = nfc.getFirmwareVersion();
        if (!versiondata) {
            PrefrontalCortex::Utilities::LOG_ERROR("Didn't find PN532 board");
            return;
        }
        
        PrefrontalCortex::Utilities::LOG_PROD("Found chip PN5%x", (versiondata >> 24) & 0xFF);
        PrefrontalCortex::Utilities::LOG_PROD("Firmware ver. %d.%d", (versiondata >> 16) & 0xFF, (versiondata >> 8) & 0xFF);
        
        nfc.SAMConfig();
        PrefrontalCortex::Utilities::LOG_PROD("NFC initialization complete");
        
        AuditoryCortex::SoundFxManager::playStartupSound();
    }

    void NFCManager::handleRotaryTurn(int direction) {
    update();
    SoundFxManager::playVoiceLine("waiting_for_card");
    }

    void NFCManager::handleRotaryPress() {
        checkForCard();
        SoundFxManager::playVoiceLine("card_detected");
    }

    void NFCManager::startBackgroundInit() {
        initInProgress = true;
        initStage = 0;
        lastInitAttempt = millis();
    }

    void NFCManager::update() {
        if (initInProgress && !SoundFxManager::isPlaying()) {
            if (millis() - lastInitAttempt < 1000) return; // Don't try too frequently
            
            switch (initStage) {
                case 0:
                    nfc.begin();
                    initStage++;
                    break;
                    
                case 1:
                    if (nfc.getFirmwareVersion()) {
                        nfc.SAMConfig();
                        initInProgress = false;
                        Utilities::LOG_PROD("NFC initialization complete");
                    } else {
                        Utilities::LOG_DEBUG("NFC init retry...");
                    }
                    break;
            }
            lastInitAttempt = millis();
            return;
        }
        
        if (isProcessingScan || SoundFxManager::isPlaying()) return;
        
        static unsigned long lastScanTime = 0;
        const unsigned long SCAN_COOLDOWN = 5000;
        
        if (millis() - lastScanTime < SCAN_COOLDOWN) return;
        
        uint8_t success;
        uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
        uint8_t uidLength;
        
        if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength)) {
            if (!cardPresent) {
                cardPresent = true;
                LEDManager::setPattern(VisualPattern::NFC_SCAN);
                
                if (isCardValid()) {
                    LEDManager::handleMessage(VisualMessage::NFC_DETECTED);
                    
                    // Calculate experience based on card data
                    uint16_t expGain = (uid[0] + uid[1] + uid[2] + uid[3]) % 50 + 10;
                    RoverViewManager::incrementExperience(expGain);
                    
                    // Start entertainment pattern using card data
                    LEDManager::displayCardPattern(uid, uidLength);
                    
                    // Read card data and generate song
                    readCardData();
                    VisualSynesthesia::playNFCCardData(cardData);
                } else { 
                    LEDManager::handleMessage(VisualMessage::NFC_ERROR);
                }
            }
        } else {
            cardPresent = false;
            LEDManager::setPattern(VisualPattern::NONE);
        }
    }

    void NFCManager::readCardData() {
        memset(cardData, 0, sizeof(cardData));
        
        if (isCardEncrypted()) {
            strcpy(cardData, "CARD ENCRYPTED");
            return;
        }
        
        uint8_t data[4];
        int dataIndex = 0;
        const uint8_t MAX_PAGES = 32; // Protect against buffer overflow
        
        for (uint8_t page = 4; page < MAX_PAGES && dataIndex < sizeof(cardData) - 1; page++) {
            if (nfc.ntag2xx_ReadPage(page, data)) {
                for (int i = 0; i < 4 && dataIndex < sizeof(cardData) - 1; i++) {
                    if (data[i] >= 32 && data[i] <= 126)  // Printable ASCII only
                    {
                        cardData[dataIndex++] = data[i];
                    }
                }
            } else {
                break; // Stop reading if page read fails
            }
        }
        
        cardData[dataIndex] = '\0'; // Ensure null termination
    }

    void NFCManager::handleNFCScan() {
        if (isCardPresent()) {
            // Card present - start NFC scan flow
            SoundFxManager::playVoiceLine("card_detected");
            checkForCard();  // Process the card
        } else {
            Serial.println("No card present");
        }
    }

    bool NFCManager::isCardEncrypted() {
        return checkForEncryption();
    }

    bool NFCManager::checkForEncryption() {
        uint8_t data[4];
        // Check authentication block
        if (!nfc.ntag2xx_ReadPage(3, data)) {
            return true; // If we can't read the auth block, assume encrypted
        }
        // Check if any authentication bits are set
        return (data[3] & 0x0F) != 0;
    }

    void NFCManager::checkForCard() {
        uint8_t success;
        uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
        uint8_t uidLength;
        
        success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
        
        if (success) {
            lastCardId = (uid[0] << 24) | (uid[1] << 16) | (uid[2] << 8) | uid[3];
            cardPresent = true;
            LEDManager::displayCardPattern(uid, uidLength);
            readCardData();
        } else {
            cardPresent = false;
        }
    }

    void NFCManager::stop() {
        isProcessingScan = false;
        initInProgress = false;
        cardPresent = false;
    }

    bool NFCManager::isCardValid() 
    {
        if (strlen(cardData) == 0) 
        {
            Utilities::LOG_DEBUG("Card data is empty");
            return false;
        }

        for (const char* validID : validCardIDs) 
        {
            if (strcmp(cardData, validID) == 0) 
            {
                char logBuffer[64];
                snprintf(logBuffer, sizeof(logBuffer), "Valid card detected: %s", cardData);
                Utilities::LOG_PROD(logBuffer);
                return true;
            }
        }
        
        char logBuffer[64];
        snprintf(logBuffer, sizeof(logBuffer), "Invalid card detected: %s", cardData);
        Utilities::LOG_DEBUG(logBuffer);
        #ifdef DEBUG_MODE
            return true; // In debug mode, accept all cards
        #else
            return false; // In production, only accept valid cards
        #endif
    }

}#include "WiFiManager.h"
#include "../PrefrontalCortex/RoverBehaviorManager.h"
#include "../VisualCortex/RoverViewManager.h"
#include "../VisualCortex/LEDManager.h"
#include "../PrefrontalCortex/PowerManager.h"
#include "../VisualCortex/RoverManager.h"
#include "../PrefrontalCortex/utilities.h"
#include "../MotorCortex/PinDefinitions.h"
#include <WiFi.h>
#include <time.h>

namespace PsychicCortex 
{

    #define WIFI_RETRY_INTERVAL 60000  // 1 minute between retry attempts
    #define TIME_CHECK_INTERVAL 500    // Time sync check interval
    #define RESYNC_TIME_INTERVAL 86400000 // 24h in ms


    bool WiFiManager::isWiFiConnected = false;
    unsigned long WiFiManager::lastWiFiAttempt = 0;
    bool WiFiManager::timeInitialized = false;

    bool WiFiManager::init() {
        bool success = false;
        WiFi.begin(PRIMARY_SSID, PRIMARY_PASSWORD);
        
        if (WiFi.waitForConnectResult() == WL_CONNECTED) 
        {
            isWiFiConnected = true;
            success = true;
        } 
        else 
        {
            isWiFiConnected = false;
        }

        return success;
    }

    void WiFiManager::checkConnection() 
    {
        static unsigned long lastTimeCheck = 0;
        static int connectionAttempts = 0;
        static bool usingBackupNetwork = false;
        const unsigned long CHECK_INTERVAL = 500;

        if (!isWiFiConnected) 
        {
            if (millis() - lastTimeCheck >= CHECK_INTERVAL) 
            {
                lastTimeCheck = millis();
                
                if (WiFi.status() != WL_CONNECTED) 
                {
                    PrefrontalCortex::Utilities::debugLog(PrefrontalCortex::Utilities::LOG_LEVEL_DEBUG, 
                        "WiFi Status: %d", WiFi.status());
                    
                    if (connectionAttempts == 0) 
                    {
                        WiFi.disconnect(true);
                        delay(100);
                        if (!usingBackupNetwork) 
                        {
                            PrefrontalCortex::Utilities::debugLog(PrefrontalCortex::Utilities::LOG_LEVEL_DEBUG, 
                                "Attempting primary network...");
                            WiFi.begin(PRIMARY_SSID, PRIMARY_PASSWORD);
                        } 
                        else 
                        {
                            PrefrontalCortex::Utilities::debugLog(PrefrontalCortex::Utilities::LOG_LEVEL_DEBUG, 
                                "Attempting backup network...");
                            WiFi.begin(BACKUP_SSID, BACKUP_PASSWORD);
                        }
                    }
                    
                    connectionAttempts++;
                    if (connectionAttempts >= 10) 
                    {
                        connectionAttempts = 0;
                        usingBackupNetwork = !usingBackupNetwork;
                    }
                } 
                else 
                {
                    isWiFiConnected = true;
                    connectionAttempts = 0;
                    PrefrontalCortex::Utilities::debugLog(
                        PrefrontalCortex::Utilities::LOG_LEVEL_PRODUCTION, 
                        "Network connection established"
                    );
                }
            }
        } 
        else if (!timeInitialized || millis() - lastTimeCheck >= RESYNC_TIME_INTERVAL) 
        { 
            syncTime();
        }
    }

    bool WiFiManager::syncTime() 
    {
        static unsigned long lastTimeCheck = 0;
        static int timeAttempts = 0;
        
        if (!timeInitialized && millis() - lastTimeCheck >= TIME_CHECK_INTERVAL) 
        {
            configTime(GMT_OFFSET_SEC, DAY_LIGHT_OFFSET_SEC, NTP_SERVER);
            if (time(nullptr) > 1000000000) 
            {
                timeInitialized = true;
                PrefrontalCortex::Utilities::debugLog(PrefrontalCortex::Utilities::LOG_LEVEL_PRODUCTION, "Time sync complete");
                return true;
            } 
            else 
            {
                timeAttempts++;
                if (timeAttempts >= 40) 
                {
                    timeAttempts = 0;
                }
            }
            lastTimeCheck = millis();
        }
        return false;
    }

    bool WiFiManager::connectToWiFi() 
        {
        Serial.println("Starting WiFi connection process");
        WiFi.disconnect(true);  // Ensure clean connection attempt
        delay(100);
        WiFi.begin(PRIMARY_SSID, PRIMARY_PASSWORD);
        lastWiFiAttempt = millis();
        isWiFiConnected = false;  // Reset connection state
        checkConnection();
        return isWiFiConnected;
    }
    // Generic error handler
    void handleError(const char* errorMessage) {
        PrefrontalCortex::Utilities::debugLog(PrefrontalCortex::Utilities::LOG_LEVEL_PRODUCTION, "Error: %s", errorMessage);
        VisualCortex::RoverManager::setTemporaryExpression(VisualCortex::RoverManager::LOOKING_DOWN, 1000);
    }

    // Example usage in various scenarios
    void processAPIRequest(bool success = false) {  // Parameter with default value
        VisualCortex::RoverManager::setTemporaryExpression(VisualCortex::RoverManager::LOOKING_UP);  // Looking up while thinking
        
        if (success) {
            VisualCortex::RoverManager::setTemporaryExpression(VisualCortex::RoverManager::BIG_SMILE, 1000);
        } else {
            handleError("API request failed");
        }
    }

}#ifndef NFC_MANAGER_H
#define NFC_MANAGER_H

#include <Adafruit_PN532.h>
#include "../PrefrontalCortex/utilities.h"
#include "../VisualCortex/LEDManager.h"
#include "../CorpusCallosum/SynapticPathways.h"

namespace PsychicCortex 
{
    using namespace CorpusCallosum;
    using VC::Pattern;
    using VC::LEDMessage;

    class NFCManager {
    public:
        static constexpr size_t MAX_CARD_DATA = 256;
        static constexpr uint8_t MAX_INIT_RETRIES = 3;
        static constexpr unsigned long INIT_TIMEOUT_MS = 5000;
        
        enum class InitState 
        {
            NOT_STARTED,
            HARDWARE_INIT,
            FIRMWARE_CHECK,
            SAM_CONFIG,
            COMPLETE,
            ERROR
        };

        static void init();
        static void update();
        static void checkForCard();
        static bool isCardPresent() { return cardPresent; }
        static uint32_t getTotalScans() { return totalScans; }
        static uint32_t getLastCardId() { return lastCardId; }
        static void handleNFCScan();
        static bool isCardEncrypted();
        static const char* getCardData() { return cardData; }
        static void readCardData();
        static void startBackgroundInit();
        static bool isInitializing() { return initInProgress; }
        static void stop();
        static bool isCardValid();
        static void handleRotaryTurn(int direction);
        static void handleRotaryPress();
        
    private:
        static Adafruit_PN532 nfc;
        static uint32_t lastCardId;
        static uint32_t totalScans;
        static bool cardPresent;
        static unsigned long lastInitAttempt;
        static bool initInProgress;
        static uint8_t initStage;
        static bool isProcessingScan;
        static char cardData[256];
        static bool isReadingData;
        static const uint8_t PAGE_COUNT = 135;
        static bool checkForEncryption();
        static InitState initState;
        static uint8_t initRetries;
        static unsigned long initStartTime;
    };

}

#endif #include "IRManager.h"
#include "../VisualCortex/LEDManager.h"
#include "../AuditoryCortex/SoundFxManager.h"
#include "../SomatosensoryCortex/MenuManager.h"
#include "../VisualCortex/RoverViewManager.h"

using namespace VisualCortex;
using namespace PrefrontalCortex;

namespace PsychicCortex 
{

        bool IRManager::blasting = false;
        unsigned long IRManager::lastSendTime = 0;
        uint16_t IRManager::currentCode = 0;
        uint8_t IRManager::currentRegion = 0;
        IRsend IRManager::irsend(BOARD_IR_RX);  // Initialize with IR pin

    void IRManager::init() {
        pinMode(BOARD_IR_EN, OUTPUT);
        pinMode(BOARD_IR_RX, OUTPUT);
        digitalWrite(BOARD_IR_EN, LOW); // Initially disabled
        irsend.begin();
    }

    void IRManager::handleRotaryTurn(int direction) {
        if (direction == 1) {
            currentRegion = (currentRegion + 1) % 4;
        } else if (direction == -1) {
            currentRegion = (currentRegion + 3) % 4;
        }
        LEDManager::setPattern(Pattern::NONE);
    }

    void IRManager::handleRotaryPress() {
        startBlast();
    }


    void IRManager::startBlast() {
        blasting = true;
        LEDManager::setPattern(Pattern::IR_BLAST);
    }

    void IRManager::stopBlast() {
        blasting = false;
        LEDManager::setPattern(Pattern::NONE);
    }


    void IRManager::update() {
        if (!blasting) return;
        
        // Update IR codes
        if (millis() - lastSendTime >= SEND_DELAY_MS) {
            sendCode(currentCode++);
            lastSendTime = millis();
            
            if (currentCode >= 100) {
                currentCode = 0;
                currentRegion++;
                if (currentRegion >= 4) {
                    stopBlast();
                    MenuManager::show();
                    return;
                }
            }
            
            // Calculate and show progress
            int totalCodes = 100 * 4;
            int currentTotal = (currentRegion * 100) + currentCode;
            int progressPercent = (currentTotal * 100) / totalCodes;
            
            char progressStr[32];
            snprintf(progressStr, sizeof(progressStr), "                %d%% [%d:%d]", 
                    progressPercent, currentRegion, currentCode);
            
            RoverViewManager::showNotification("IR", progressStr, "BLAST", 500);
        }
    }

    void IRManager::sendCode(uint16_t code) {
        // Send different protocols based on region, following TV-B-Gone approach
        switch(currentRegion) {
            case 0:  // Sony (SIRC) - Most common for Bravia
                digitalWrite(BOARD_IR_EN, HIGH);
                // Power codes: 21/0x15, 0xA90, 0x290
                // Input codes: 25/0x19, 0xA50, 0x250
                // Vol codes: 18/0x12 (up), 19/0x13 (down), 20/0x14 (mute)
                if (code < 20) {
                    irsend.sendSony(code + 0x10, 12);  // Basic commands (0x10-0x20)
                } else if (code < 40) {
                    irsend.sendSony(code + 0xA80, 12); // Extended set A
                } else if (code < 60) {
                    irsend.sendSony(code + 0x240, 12); // Extended set B
                } else {
                    irsend.sendSony(code, 15);         // 15-bit codes
                }
                digitalWrite(BOARD_IR_EN, LOW);
                break;
                
            case 1:  // NEC - Common for many TVs
                digitalWrite(BOARD_IR_EN, HIGH);
                if (code < 50) {
                    irsend.sendNEC(0x04FB0000UL + code); // Sony TV NEC variants
                } else {
                    irsend.sendNEC(0x10000000UL + code); // Other NEC codes
                }
                digitalWrite(BOARD_IR_EN, LOW);
                break;
                
            case 2:  // RC5/RC6 - Common for European TVs
                digitalWrite(BOARD_IR_EN, HIGH);
                if (code % 2) {
                    irsend.sendRC5(code, 12);
                } else {
                    irsend.sendRC6(code, 20);
                }
                digitalWrite(BOARD_IR_EN, LOW);
                break;
                
            case 3:  // Samsung - Common for newer TVs
                digitalWrite(BOARD_IR_EN, HIGH);
                irsend.sendSAMSUNG(0xE0E0 + code);
                digitalWrite(BOARD_IR_EN, LOW);
                break;
        }
    } 
}#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include "../PrefrontalCortex/RoverBehaviorManager.h"
#include "../PrefrontalCortex/utilities.h"
#include "../MotorCortex/PinDefinitions.h"

namespace PsychicCortex 
{

    #define GMT_OFFSET_SEC (-5 * 3600)  // EST (DST)
    #define DAY_LIGHT_OFFSET_SEC 3600   // 1 hour of daylight saving
    #define NTP_SERVER "pool.ntp.org"

    class WiFiManager {
    public:
        static bool init();
        static void checkConnection();
        static bool isConnected() { return isWiFiConnected; }
        static bool connectToWiFi();
        static bool getTimeInitialized() { return timeInitialized; }
        static bool syncTime();

    private:
        static bool isRecording;
        static bool isWiFiConnected;
        static unsigned long lastWiFiAttempt;
        static bool timeInitialized;
        static unsigned long lastTimeCheck;
        // WiFi credentials    ( MOVE TO SECRETS )
        static constexpr const char* PRIMARY_SSID = "RevivalNetwork ";
        static constexpr const char* PRIMARY_PASSWORD = "xunjmq84";
        static constexpr const char* BACKUP_SSID = "CodeMusicai";
        static constexpr const char* BACKUP_PASSWORD = "cnatural";
    };

}

#endif #ifndef SYNAPTIC_PATHWAYS_H
#define SYNAPTIC_PATHWAYS_H

// Core system includes
#include <Arduino.h>
#include <FastLED.h>
#include <vector>
#include "../PrefrontalCortex/ProtoPerceptions.h"

// Forward declare all cortex namespaces
namespace PrefrontalCortex 
{
    class Utilities;
    class SPIManager;
    class RoverBehaviorManager;
    class PowerManager;
    class SDManager;
    class WiFiManager;

    // Import all type namespaces from ProtoPerceptions.h
    using namespace StorageTypes;
    using namespace SensorTypes;
    using namespace ConfigTypes;
    using namespace SystemTypes;
    using namespace BehaviorTypes;
    using namespace UITypes;
    using namespace GameTypes;
    using namespace CommTypes;
    using namespace AudioTypes;
    using namespace VisualTypes;
    using namespace RoverTypes;
    using namespace ChakraTypes;
    using namespace VirtueTypes;
    using namespace SomatosensoryTypes;
    using namespace AuditoryTypes;
    using namespace PsychicTypes;
}

namespace VisualCortex 
{
    class DisplayConfig;
    class LEDManager;
    class RoverViewManager;
    class RoverManager;
    class VisualSynesthesia;
    enum class VisualPattern;
    enum class VisualMessage;
}

namespace SomatosensoryCortex 
{
    class UIManager;
    class MenuManager;
}

namespace AuditoryCortex 
{
    class SoundFxManager;
    struct NoteInfo;
    class PitchPerception;
}

namespace PsychicCortex 
{
    class NFCManager;
    class IRManager;
    class WiFiManager;
}

namespace GameCortex 
{
    class SlotsManager;
    class AppManager;
}

namespace MotorCortex 
{
    class PinDefinitions;
}

// Create namespace aliases
namespace CorpusCallosum 
{
    namespace PC = PrefrontalCortex;
    namespace VC = VisualCortex;
    namespace SC = SomatosensoryCortex;
    namespace AC = AuditoryCortex;
    namespace PSY = PsychicCortex;
    namespace GC = GameCortex;
    namespace MC = MotorCortex;

    // Add these using declarations
    using namespace PC::StorageTypes;
    using namespace PC::SensorTypes;
    using namespace PC::ConfigTypes;
    using namespace PC::SystemTypes;
    using namespace PC::BehaviorTypes;
    using namespace PC::UITypes;
    using namespace PC::GameTypes;
    using namespace PC::CommTypes;
    using namespace PC::AudioTypes;
    using namespace PC::VisualTypes;
    using namespace PC::RoverTypes;
    using namespace PC::ChakraTypes;
    using namespace PC::VirtueTypes;
    using namespace PC::SomatosensoryTypes;
    using namespace PC::AuditoryTypes;
    using namespace PC::PsychicTypes;
}

#endif#pragma once

#include "../CorpusCallosum/SynapticPathways.h"
#include "../PrefrontalCortex/ProtoPerceptions.h"
#include <vector>
#include <functional>
#include <string>

namespace GameCortex
{
    using namespace CorpusCallosum;
    using PC::GameTypes::AppInfo;
    using PC::GameTypes::AppState;

    class AppManager {
    public:
        // Initialize the app manager
        static void init();
        static bool isInitialized() { return initialized; }

        // Register a new app
        static bool registerApp(const AppInfo& app);

        // Start an app by name (triggers onRun, switches to SHOW_INFO state initially)
        static bool startApp(const std::string& appName);

        // Called in the main loop to update the active app
        static void update();

        // Handle rotary inputs if the app is active
        static void handleRotaryTurn(int direction);
        static void handleRotaryPress();

        // Exit current app
        static void exitApp();

        // Is there an app running now?
        static bool isAppActive();

        // Returns the current state (DORMANT, ORIENTING, ENGAGED)
        static AppState getCurrentState();

        // Name/description of the currently running app
        static std::string getCurrentAppName();
        static std::string getCurrentAppDescription();

    private:
        static bool initialized;
        static std::vector<AppInfo> appRegistry;
        static AppInfo* activeApp;
        static AppState currentState;
    }; 
}#include "AppManager.h"
#include "../VisualCortex/RoverViewManager.h"
#include "../SomatosensoryCortex/MenuManager.h"
#include "../GameCortex/AppRegistration.h"
#include "../GameCortex/SlotsManager.h"
#include "../PrefrontalCortex/Utilities.h"
#include "../PsychicCortex/IRManager.h"
#include "../PsychicCortex/NFCManager.h"

namespace GameCortex
{
    using namespace CorpusCallosum;
    using PC::GameTypes::AppInfo;
    using PC::GameTypes::AppState;
    using PC::Utilities;
    using PSY::IRManager;
    using PSY::NFCManager;

    // Static member definitions
    bool AppManager::initialized = false;
    std::vector<AppInfo> AppManager::appRegistry;
    AppInfo* AppManager::activeApp = nullptr;
    AppState AppManager::currentState = AppState::IDLE;

    void AppManager::init() {
        if (initialized) {
            Utilities::LOG_DEBUG("AppManager already initialized");
            return;
        }
        
        try {
            appRegistry.clear();
            activeApp = nullptr;
            currentState = AppState::IDLE;
            initialized = true;
            Utilities::LOG_DEBUG("AppManager initialized successfully");
        } catch (const std::exception& e) {
            Utilities::LOG_ERROR("AppManager init failed: %s", e.what());
            initialized = false;
            throw;
        }
    }

    bool AppManager::registerApp(const AppInfo& app) {
        if (!initialized) {
            Utilities::LOG_ERROR("Cannot register app - AppManager not initialized");
            return false;
        }

        try {
            if (app.name.empty()) {
                Utilities::LOG_ERROR("Invalid app registration - empty name");
                return false;
            }
            appRegistry.push_back(app);
            Utilities::LOG_DEBUG("Registered app: %s", app.name.c_str());
            return true;
        } catch (const std::exception& e) {
            Utilities::LOG_ERROR("App registration failed: %s", e.what());
            return false;
        }
    }

    bool AppManager::startApp(const std::string& appName) {
        // First call onExit for any currently running app
        if (activeApp && activeApp->onExit) {
            activeApp->onExit();
        }

        for (auto& app : appRegistry) {
            if (app.name == appName) {
                activeApp = &app;
                currentState = AppState::SHOW_INFO;
                // Call the onRun function if it exists
                if (activeApp->onRun) {
                    activeApp->onRun();
                }
                return true;
            }
        }
        return false;
    }

    void AppManager::update() {
        if (!activeApp) return;

        switch (currentState) {
        case AppState::SHOW_INFO:
            currentState = AppState::RUNNING;
            break;

        case AppState::RUNNING:
            // Call the app's onUpdate function if it exists
            if (activeApp->onUpdate) {
                activeApp->onUpdate();
            }
            break;

        case AppState::IDLE:
        default:
            break;
        }
    }

    void AppManager::handleRotaryTurn(int direction) {
        if (currentState == AppState::RUNNING && activeApp) {
            Utilities::LOG_DEBUG("Rotary turn: %d", direction);
            const std::string appName = activeApp->name;
            
            if (appName == "SlotsApp") {
                SlotsManager::handleRotaryTurn(direction);
            } else if (appName == "IrBlastApp") {
                IRManager::handleRotaryTurn(direction);
            } else if (appName == "NfcApp") {
                NFCManager::handleRotaryTurn(direction);
            }
        }
    }

    void AppManager::handleRotaryPress() {
        if (currentState == AppState::RUNNING && activeApp) {
            const std::string appName = activeApp->name;
            
            if (appName == "SlotsApp") {
                SlotsManager::handleRotaryPress();
            } else if (appName == "IrBlastApp") {
                IRManager::handleRotaryPress();
            } else if (appName == "NfcApp") {
                NFCManager::handleRotaryPress();
            }
        }
    }

    void AppManager::exitApp() {
        // Call onExit before resetting state
        if (activeApp && activeApp->onExit) {
            activeApp->onExit();
        }
        activeApp = nullptr;
        currentState = AppState::IDLE;
    }

    bool AppManager::isAppActive() {
        return (currentState != AppState::IDLE && activeApp != nullptr);
    }

    AppState AppManager::getCurrentState() {
        return currentState;
    }

    std::string AppManager::getCurrentAppName() {
        if (activeApp) return activeApp->name;
        return "";
    }

    std::string AppManager::getCurrentAppDescription() {
        if (activeApp) return activeApp->description;
        return "";
    } 
}#include "SlotsManager.h"
#include "../CorpusCallosum/SynapticPathways.h"
#include "../VisualCortex/LEDManager.h"
#include "../VisualCortex/RoverViewManager.h"
#include "../AuditoryCortex/SoundFxManager.h"
#include "../SomatosensoryCortex/MenuManager.h"

namespace GameCortex
{
    using namespace CorpusCallosum;
    using PC::GameTypes::GameState;
    using PC::GameTypes::SlotSymbol;
    using PC::GameTypes::GameScore;
    using VC::LEDManager;
    using VC::RoverViewManager;
    using AC::SoundFxManager;
    using SC::MenuManager;

    // Initialize static members
    uint8_t SlotsManager::activeSlotPair = 0;
    bool SlotsManager::slotLocked[4] = {false};
    CRGB SlotsManager::slotColors[4];
    unsigned long SlotsManager::animationTimer = 0;
    bool SlotsManager::showingResult = false;
    bool SlotsManager::gameActive = false;
    unsigned long SlotsManager::lockTimer = 0;

    void SlotsManager::init() {
        gameActive = true;
        // Reset game state
        activeSlotPair = 0;
        showingResult = false;
        animationTimer = millis();
        for (int i = 0; i < 4; i++) {
            slotLocked[i] = false;
            slotColors[i] = CRGB::Black;
        }
    }

    void SlotsManager::handleRotaryTurn(int direction) {
        update();
    }

    void SlotsManager::handleRotaryPress() {
        spin();
    }

    void SlotsManager::update() {
        if (!gameActive) return;
        
        if (showingResult) {
            // Flash winning slots
            if (millis() - animationTimer > 250) {
                static bool flashState = false;
                FastLED.clear();
                
                for(int i = 0; i < 4; i++) {
                    if (slotLocked[i]) {
                        CRGB color = flashState ? slotColors[i] : CRGB::Black;
                        LEDManager::setLED(i*2, color);
                        LEDManager::setLED(i*2+1, color);
                    }
                }
                LEDManager::showLEDs();
                flashState = !flashState;
                animationTimer = millis();
            }
            
            // End game after showing result
            if (millis() - animationTimer > 3000) {
                gameActive = false;
                MenuManager::show();
            }
        } else {
            // Regular slot spinning animation
            if (millis() - animationTimer > 50) {
                FastLED.clear();
                
                for(int i = 0; i < 4; i++) {
                    if (!slotLocked[i]) {
                        slotColors[i] = getRainbowColor(random(7));
                    }
                    LEDManager::setLED(i*2, slotColors[i]);
                    LEDManager::setLED(i*2+1, slotColors[i]);
                }
                LEDManager::showLEDs();
                animationTimer = millis();
                
                // Check if it's time to lock the next reel
                if (!slotLocked[activeSlotPair] && millis() >= lockTimer) {
                    slotLocked[activeSlotPair] = true;
                    SoundFxManager::playTone(1000 + (activeSlotPair * 200), 100);
                    
                    activeSlotPair++;
                    if (activeSlotPair < 4) {
                        // Set next lock timer
                        lockTimer = millis() + random(200, 1000);
                    } else {
                        checkResult();
                    }
                }
            }
        }
    }

    void SlotsManager::spin() {
        if (!gameActive || showingResult) return;
        
        // Lock current slot pair
        slotLocked[activeSlotPair] = true;
        SoundFxManager::playTone(1000 + (activeSlotPair * 200), 100);
        
        // Move to next slot pair
        activeSlotPair++;
        
        // Check if all slots are locked
        if (activeSlotPair >= 4) {
            checkResult();
        }
    }

    void SlotsManager::checkResult() {
        // Check if any adjacent pairs match
        bool hasMatch = false;
        for(int i = 0; i < 3; i++) {
            if (slotColors[i] == slotColors[i+1]) {
                hasMatch = true;
                break;
            }
        }
        showResult(hasMatch);
    }

    void SlotsManager::showResult(bool won) {
        showingResult = true;
        animationTimer = millis();
        
        if (won) {
            SoundFxManager::playStartupSound();
            RoverViewManager::showNotification("SLOTS", "Winner!", "GAME", 3000);
        } else {
            SoundFxManager::playErrorSound(1);
            RoverViewManager::showNotification("SLOTS", "Try Again!", "GAME", 3000);
        }
    }

    CRGB SlotsManager::getRainbowColor(uint8_t index) {
        switch(index) {
            case 0: return CRGB::Red;
            case 1: return CRGB::Orange;
            case 2: return CRGB::Yellow;
            case 3: return CRGB::Green;
            case 4: return CRGB::Blue;
            case 5: return CRGB::Purple;
            default: return CRGB::White;
        }
    }

    void SlotsManager::reset() {
        gameActive = false;
        showingResult = false;
        FastLED.clear();
        FastLED.show();
    }

    void SlotsManager::startGame() {
        gameActive = true;
        showingResult = false;
        activeSlotPair = 0;
        
        // Initialize slots
        for(int i = 0; i < 4; i++) {
            slotLocked[i] = false;
            slotColors[i] = getRainbowColor(random(7));
            LEDManager::setLED(i*2, slotColors[i]);
            LEDManager::setLED(i*2+1, slotColors[i]);
        }
        LEDManager::showLEDs();
        
        animationTimer = millis();
        lockTimer = millis() + random(200, 1000); // Set first auto-lock timer
        MenuManager::hide();
        RoverViewManager::showNotification("SLOTS", "Auto-locking reels...", "GAME", 2000);
    }
}#include "AppRegistration.h"
#include "AppManager.h"
#include "../GameCortex/SlotsManager.h"
#include "../PsychicCortex/NFCManager.h"
#include "../PsychicCortex/IRManager.h"
#include "../VisualCortex/LEDManager.h"
#include "../SomatosensoryCortex/MenuManager.h"
#include "../PrefrontalCortex/Utilities.h"
#include <Arduino.h>

using namespace PsychicCortex;      // For IRManager, NFCManager
using namespace SomatosensoryCortex; // For MenuManager
using namespace PrefrontalCortex;    // For Utilities

namespace GameCortex
{

    void AppRegistration::slotsOnRun() {
        Serial.println("[SlotsApp] onRun: Starting Slots game...");
        SlotsManager::init();
    }
    void AppRegistration::slotsOnUpdate() {
        // Called every cycle while SlotsApp is RUNNING
        if (!SlotsManager::isGameActive()) {
            Serial.println("[SlotsApp] No active Slots game, exiting app...");
            AppManager::exitApp();
        } else {
            SlotsManager::update();
        }
    }
    void AppRegistration::slotsOnExit() {
        Serial.println("[SlotsApp] onExit: Cleaning up Slots game...");
        SlotsManager::reset();
    }

    // Example "IrBlastApp" callbacks
    void AppRegistration::irOnRun() {
        Serial.println("[IrBlastApp] onRun: Setting up IR blasting...");
        IRManager::init();
    }
    void AppRegistration::irOnUpdate() {
        // Could handle IR blasting logic, e.g. sending signals
        IRManager::update();
    }
    void AppRegistration::irOnExit() {
        Serial.println("[IrBlastApp] onExit: IR blasting done.");
        IRManager::stopBlast();
    }

    // Example "NfcApp" callbacks
    void AppRegistration::nfcOnRun() {
        Serial.println("[NfcApp] onRun: Initializing NFC scan...");
        NFCManager::init();
    }
    void AppRegistration::nfcOnUpdate() {
        // Ask NFCManager to handle repeated scans
        NFCManager::update();
    }
    void AppRegistration::nfcOnExit() {
        Serial.println("[NfcApp] onExit: Stopping NFC logic...");
        // NFCManager may not have a teardown, but you can add if needed
        NFCManager::stop();
    }

    // Example "AppSettings" callbacks (LED config, etc.)
    void AppRegistration::settingsOnRun() {
        Serial.println("[AppSettings] onRun: Opening LED Settings...");
        MenuManager::enterSubmenu(MenuManager::appSettingsMenu);
        // e.g., store or retrieve LED state
    }
    void AppRegistration::settingsOnUpdate() {

        // e.g., let user pick a mode
    }

    void AppRegistration::settingsOnExit() {
        Serial.println("[AppSettings] onExit: Closing LED Settings...");
    }

    // Registering the apps
    void AppRegistration::registerDefaultApps() {
        if (!AppManager::isInitialized()) {
            Utilities::LOG_ERROR("Cannot register apps - AppManager not initialized");
            return;
        }

        try {
            bool success = true;
            
            // Register SlotsApp
            if (success) {
                AppInfo slotsApp;
                slotsApp.name = "SlotsApp";
                slotsApp.description = "A fun slot-machine mini-game.";
                slotsApp.onRun = &slotsOnRun;
                slotsApp.onUpdate = &slotsOnUpdate;
                slotsApp.onExit = &slotsOnExit;
                success = AppManager::registerApp(slotsApp);
                Utilities::LOG_DEBUG(success ? "SlotsApp registered" : "SlotsApp registration failed");
            }

            // Only continue registering if previous was successful
            if (success) {
                AppInfo irApp;
                irApp.name = "IrBlastApp";
                irApp.description = "IR blasting and remote control.";
                irApp.onRun = &irOnRun;
                irApp.onUpdate = &irOnUpdate;
                irApp.onExit = &irOnExit;
                success = AppManager::registerApp(irApp);
                Utilities::LOG_DEBUG(success ? "IrBlastApp registered" : "IrBlastApp registration failed");
            }

            if (success) {
                AppInfo nfcApp;
                nfcApp.name = "NfcApp";
                nfcApp.description = "NFC scanning and reading.";
                nfcApp.onRun = &nfcOnRun;
                nfcApp.onUpdate = &nfcOnUpdate;
                nfcApp.onExit = &nfcOnExit;
                success = AppManager::registerApp(nfcApp);
            }
            
            if (success) {
                AppInfo settingsApp;
                settingsApp.name = "AppSettings";
                settingsApp.description = "Configure LED modes, etc.";
                settingsApp.onRun = &settingsOnRun;
                settingsApp.onUpdate = &settingsOnUpdate;
                settingsApp.onExit = &settingsOnExit;
                success = AppManager::registerApp(settingsApp);
            }

            if (!success) {
                Utilities::LOG_ERROR("Failed to register one or more apps");
            }

        } catch (const std::exception& e) {
            Utilities::LOG_ERROR("App registration error: %s", e.what());
            // Don't throw - allow boot to continue without apps
        }
    }
}#pragma once
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
}#ifndef APPREGISTRATION_H
#define APPREGISTRATION_H

#include "AppManager.h"
#include "../GameCortex/SlotsManager.h"
#include "../PsychicCortex/NFCManager.h"
#include "../PsychicCortex/IRManager.h"
#include "../VisualCortex/LEDManager.h"
#include "../SomatosensoryCortex/MenuManager.h"
#include <Arduino.h>

namespace GameCortex
{
    class AppRegistration {
    public:
        // SlotsApp callbacks
        static void slotsOnRun();
        static void slotsOnUpdate();
        static void slotsOnExit();

        // IrBlastApp callbacks
        static void irOnRun();
        static void irOnUpdate();
        static void irOnExit();

        // NfcApp callbacks
        static void nfcOnRun();
        static void nfcOnUpdate();
        static void nfcOnExit();

        // AppSettings callbacks
        static void settingsOnRun();
        static void settingsOnUpdate();
        static void settingsOnExit();

        // Function to register all default apps
        static void registerDefaultApps();


    };
}
#endif // APPREGISTRATION_H#ifndef PIN_DEFINITIONS_H
#define PIN_DEFINITIONS_H


#include <FastLED.h>

namespace MotorCortex
{
    class PinDefinitions
    {
        public:
            // LED Pins
            static constexpr uint8_t LED_DATA_PIN = 48;
            static constexpr uint8_t LED_CLOCK_PIN = 47;

            // buttons
            #define BOARD_USER_KEY 6
            #define BOARD_PWR_EN   15

            // WS2812 Configuration
            static constexpr uint8_t WS2812_DATA_PIN = 14;
            static constexpr uint8_t WS2812_NUM_LEDS = 8;
            static constexpr EOrder WS2812_COLOR_ORDER = GRB;

            // LED function-specific indices
            #define LED_ERROR_START 0
            #define LED_ERROR_COUNT 8
            #define LED_LOADING_STEP 3

            // IR
            #define BOARD_IR_EN 2
            #define BOARD_IR_RX 1

            // MIC
            #define BOARD_MIC_DATA 42
            #define BOARD_MIC_CLK  39

            // VOICE
            // #define BOARD_VOICE_MODE  4
            #define BOARD_VOICE_BCLK  46
            #define BOARD_VOICE_LRCLK 40
            #define BOARD_VOICE_DIN   7

            // --------- DISPLAY ---------
            // About LCD definition in the file: lib/TFT_eSPI/User_Setups/Setup214_LilyGo_T_Embed_PN532.h
            // #define ST7789_DRIVER     // Configure all registers
            // #define TFT_WIDTH  170
            // #define TFT_HEIGHT 320

            // #define TFT_BL     21   // LED back-light
            // #define TFT_MISO   10   
            // #define TFT_MOSI   9
            // #define TFT_SCLK   11
            #define TFT_CS     41 
            // #define TFT_DC     16
            // #define TFT_RST    40 // Connect reset to ensure display initialises


            // --------- ENCODER ---------
            #define ENCODER_INA 4
            #define ENCODER_INB 5
            #define ENCODER_KEY 0

            // --------- IIC ---------
            #define BOARD_I2C_SDA  8
            #define BOARD_I2C_SCL  18

            // IIC addr
            #define BOARD_I2C_ADDR_1 0x24  // PN532
            #define BOARD_I2C_ADDR_2 0x55  // PMU
            #define BOARD_I2C_ADDR_3 0x6b  // BQ25896

            // NFC
            #define BOARD_PN532_SCL     BOARD_I2C_SCL
            #define BOARD_PN532_SDA     BOARD_I2C_SDA
            #define BOARD_PN532_RF_REST 45
            #define BOARD_PN532_IRQ     17

            // --------- SPI ---------
            #define BOARD_SPI_SCK  11
            #define BOARD_SPI_MOSI 9
            #define BOARD_SPI_MISO 10

            // TF card
            #define BOARD_SD_CS   13
            #define BOARD_SD_SCK  BOARD_SPI_SCK
            #define BOARD_SD_MOSI BOARD_SPI_MOSI
            #define BOARD_SD_MISO BOARD_SPI_MISO

            // LORA
            #define BOARD_LORA_CS   12
            #define BOARD_LORA_SCK  BOARD_SPI_SCK
            #define BOARD_LORA_MOSI BOARD_SPI_MOSI
            #define BOARD_LORA_MISO BOARD_SPI_MISO
            #define BOARD_LORA_IO2  38
            #define BOARD_LORA_IO0  3
            #define BOARD_LORA_SW1  47
            #define BOARD_LORA_SW0  48

            // Core configuration
            #define SCREEN_CENTER_X 85
            #define CLOCK_PIN 45

            // Add if not already present:
            #define SD_CS    13  // SD Card CS pin
            #define SD_MOSI  10  // SD Card MOSI pin
            #define SD_MISO  9  // SD Card MISO pin
            #define SD_SCK   11  // SD Card SCK pin    

            // NFC
            #define SDA_PIN 4
            #define SCL_PIN 5

    };  // Add semicolon here
}

#endif #include "SPIManager.h"

namespace PrefrontalCortex 
{

    // Initialize the static member
    bool SPIManager::initialized = false;

    bool SPIManager::isInitialized() {
        return initialized;
    }

    void SPIManager::init() {
        if (initialized) return;
        // Initialize all CS pins as outputs and set them HIGH (disabled)
        pinMode(TFT_CS, OUTPUT);
        pinMode(BOARD_SD_CS, OUTPUT);
        pinMode(BOARD_LORA_CS, OUTPUT);
        
        // Ensure all devices are deselected
        digitalWrite(TFT_CS, HIGH);
        digitalWrite(BOARD_SD_CS, HIGH);
        digitalWrite(BOARD_LORA_CS, HIGH);
        
        // Configure SPI bus
        SPI.begin(BOARD_SPI_SCK, BOARD_SPI_MISO, BOARD_SPI_MOSI);
        SPI.setFrequency(20000000); // 20MHz - adjust if needed
        
        Utilities::LOG_DEBUG("SPI bus and chip selects initialized");
        initialized = true;
    }

    void SPIManager::selectDevice(uint8_t deviceCS) {
        // Deselect all devices first
        digitalWrite(TFT_CS, HIGH);
        digitalWrite(BOARD_SD_CS, HIGH);
        digitalWrite(BOARD_LORA_CS, HIGH);
        
        // Select the requested device
        digitalWrite(deviceCS, LOW);
    }

    void SPIManager::deselectAll() {
        digitalWrite(TFT_CS, HIGH);
        digitalWrite(BOARD_SD_CS, HIGH);
        digitalWrite(BOARD_LORA_CS, HIGH);
    }

}#define FASTLED_ESP32_SPI_BUS FSPI
#define FASTLED_ALL_PINS_HARDWARE_SPI
#define FASTLED_ESP32_SPI_CLOCK_DIVIDER 16

#include "../CorpusCallosum/SynapticPathways.h"
#include "../MotorCortex/PinDefinitions.h"
#include "RoverBehaviorManager.h"
#include "utilities.h"
#include "SPIManager.h"
#include "SDManager.h"
#include "../PsychicCortex/WiFiManager.h"
#include "../SomatosensoryCortex/UIManager.h"
#include "../VisualCortex/RoverViewManager.h"
#include "../VisualCortex/LEDManager.h"
#include "../VisualCortex/RoverManager.h"
#include "../SomatosensoryCortex/MenuManager.h"
#include "../AuditoryCortex/SoundFxManager.h"
#include "../GameCortex/AppManager.h"
#include "../GameCortex/AppRegistration.h"
#include <SPIFFS.h>

namespace PrefrontalCortex 
{
    namespace PC = PrefrontalCortex;  // Add namespace alias
    using namespace CorpusCallosum;
    using VC::RoverViewManager;
    using VC::LEDManager;
    using VC::RoverManager;
    using SC::MenuManager;
    using AC::SoundFxManager;
    using GC::AppManager;
    using GC::AppRegistration;
    using PSY::WiFiManager;

    // Forward declare external sprite
    extern TFT_eSprite spr;

    // Static member initialization with proper namespace qualification
    bool RoverBehaviorManager::initialized = false;
    RoverTypes::BehaviorState RoverBehaviorManager::currentState = RoverTypes::BehaviorState::LOADING;
    String RoverBehaviorManager::statusMessage = "Starting...";
    RoverTypes::LoadingPhase RoverBehaviorManager::loadingPhase = RoverTypes::LoadingPhase::BOOTING;
    const char* RoverBehaviorManager::currentStatusMessage = "Starting...";
    RoverTypes::BehaviorState RoverBehaviorManager::previousState = RoverTypes::BehaviorState::LOADING;
    unsigned long RoverBehaviorManager::warningStartTime = 0;
    bool RoverBehaviorManager::isCountingDown = false;
    int RoverBehaviorManager::currentBootStep = 0;
    unsigned long RoverBehaviorManager::fatalErrorStartTime = 0;
    bool RoverBehaviorManager::isFatalError = false;
    VisualTypes::VisualPattern RoverBehaviorManager::pattern = VisualTypes::VisualPattern::NONE;

    void RoverBehaviorManager::init() 
    {
        if (initialized) 
        {
            Utilities::LOG_DEBUG("RoverBehaviorManager already initialized");
            return;
        }

        try 
        {
            // Initialize SD card first
            SDManager::init(BOARD_SD_CS);
            Utilities::LOG_DEBUG("SD Manager initialized");

            // Initialize other managers
            LEDManager::init();
            RoverViewManager::init();
            MenuManager::init();
            SoundFxManager::init();
            AppManager::init();

            initialized = true;
            Utilities::LOG_DEBUG("RoverBehaviorManager initialized successfully");
        }
        catch (const std::exception& e) 
        {
            Utilities::LOG_ERROR("RoverBehaviorManager init failed: %s", e.what());
            throw;
        }
    }

    void RoverBehaviorManager::setState(RoverTypes::BehaviorState newState) 
    {
        if (currentState == newState) return;

        currentState = newState;
        
        switch (currentState) 
        {
            case RoverTypes::BehaviorState::FULL_DISPLAY:
                LEDManager::setEncodingMode(VisualTypes::EncodingModes::FULL_MODE);
                break;

            case RoverTypes::BehaviorState::MENU_MODE:
                LEDManager::setEncodingMode(VisualTypes::EncodingModes::MENU_MODE);
                break;

            default:
                // No LED encoding mode change for other states
                break;
        }
    }

    bool RoverBehaviorManager::IsInitialized() 
    {
        return initialized;
    }

    void RoverBehaviorManager::update() 
    {
        switch (currentState) 
        {
            case RoverTypes::BehaviorState::LOADING:
                handleLoading();
                break;
            case RoverTypes::BehaviorState::HOME:
                handleHome();
                break;
            case RoverTypes::BehaviorState::MENU:
                handleMenu();
                break;
            case RoverTypes::BehaviorState::APP:
                handleApp();
                break;
            case RoverTypes::BehaviorState::ERROR:
                handleError();
                break;
            case RoverTypes::BehaviorState::WARNING:
                updateWarningCountdown();
                break;
            case RoverTypes::BehaviorState::FATAL_ERROR:
                handleFatalError();
                break;
        }

        // Decide what to draw
        switch (currentState) 
        {
            case RoverTypes::BehaviorState::LOADING:
                RoverViewManager::drawLoadingScreen(currentStatusMessage);
                break;
            case RoverTypes::BehaviorState::HOME:
                if (MenuManager::isVisible()) 
                {
                    MenuManager::drawMenu();
                } 
                else if (RoverViewManager::hasActiveNotification()) 
                {
                    RoverViewManager::drawNotification();
                } 
                else 
                {
                    RoverViewManager::drawCurrentView();
                }
                break;
            case RoverTypes::BehaviorState::MENU:
                MenuManager::drawMenu();
                break;
            case RoverTypes::BehaviorState::APP:
                if (MenuManager::isVisible()) {
                    MenuManager::drawMenu();
                } else if (RoverViewManager::hasActiveNotification()) {
                    RoverViewManager::drawNotification();
                } else {
                    RoverViewManager::drawCurrentView();
                }
                break;
            case RoverTypes::BehaviorState::ERROR:
                RoverViewManager::drawLoadingScreen(currentStatusMessage);
                break;
            case RoverTypes::BehaviorState::FATAL_ERROR:
                handleFatalError();
                break;
        }

        // Push rendered sprite to display
        spr.pushSprite(0, 0);

        // Add to existing update function
        if (currentState == RoverTypes::BehaviorState::WARNING) {
            updateWarningCountdown();
        }
    }

    RoverTypes::BehaviorState RoverBehaviorManager::getCurrentState() {
        return currentState;
    }

    void RoverBehaviorManager::handleLoading() 
    {
        LEDManager::updateLoadingAnimation();

        switch (loadingPhase) 
        {
            case RoverTypes::LoadingPhase::BOOTING:
                handleBooting();
                break;
            case RoverTypes::LoadingPhase::CONNECTING_WIFI:
                handleWiFiConnection();
                break;
            case RoverTypes::LoadingPhase::SYNCING_TIME:
                handleTimeSync();
                break;
        }
    }

    void RoverBehaviorManager::handleHome() {
        // Basic updates in home state
        LEDManager::updateLoadingAnimation();
        RoverManager::updateHoverAnimation();
    }

    void RoverBehaviorManager::handleMenu() {
        Utilities::LOG_DEBUG("handleMenu...");
        // The menu system manages its own logic
    }

    void RoverBehaviorManager::handleApp() {
        if (!AppManager::isAppActive()) {
            setState(RoverTypes::BehaviorState::HOME);
        }
    }

    void RoverBehaviorManager::handleError() {
        // Remove auto-reboot completely
        static bool inError = false;
        if (!inError) {
            inError = true;
        }
        // No reboot logic here
    }

    void RoverBehaviorManager::handleFatalError() {
        if (!RoverViewManager::isFatalError) return;
        
        // Only check for manual reboot via button press, but don't actually reboot
        if (SC::UIManager::isRotaryPressed()) {
            // ESP.restart(); - Removed
        }
    }

    void RoverBehaviorManager::updateWarningCountdown() {
        if (!isCountingDown && !isFatalError) return;
        
        unsigned long elapsed;
        if (isFatalError) {
            elapsed = millis() - fatalErrorStartTime;
            // Remove reboot logic completely
            return;
        }
        
        // Rest of the warning countdown logic remains the same
        elapsed = millis() - warningStartTime;
        if (elapsed >= WARNING_DURATION) {
            isCountingDown = false;
            setState(RoverTypes::BehaviorState::IDLE);
            RoverViewManager::isError = false;
            LEDManager::clearErrorPattern();
            return;
        }
        
        // Update countdown display
        int remainingSeconds = ((WARNING_DURATION - elapsed) / 1000);
        char countdownMsg[32];
        sprintf(countdownMsg, "%s", RoverViewManager::errorMessage);
        RoverViewManager::drawErrorScreen(
            RoverViewManager::errorCode,
            countdownMsg,
            false
        );
    }

    //----- Sub-phase Handlers for LOADING -----
    void RoverBehaviorManager::handleBooting() 
    {
        static unsigned long lastMsgChange = 0;
        static int step = 0;
        const unsigned long stepDelay = 800;
        
        if (millis() - lastMsgChange > stepDelay) 
        {
            try 
            {
                switch(step) 
                {
                    case 0:
                        currentStatusMessage = "Initializing hardware...";
                        break;
                    case 1:
                        currentStatusMessage = "Loading display...";
                        RoverViewManager::init();
                        break;
                    case 2:
                        currentStatusMessage = "Initializing Sound...";
                        SoundFxManager::init();
                        break;
                    case 3:
                        currentStatusMessage = "Starting UI...";
                        SC::UIManager::init();
                        break;
                    case 4:
                        currentStatusMessage = "Preparing apps...";
                        MenuManager::init();
                        break;
                    case 5:
                        currentStatusMessage = "Registering apps...";
                        AppManager::init();
                        if (AppManager::isInitialized()) {
                            AppRegistration::registerDefaultApps();
                        }
                        break;
                }
                
                currentBootStep = step;
                lastMsgChange = millis();
                
                // Draw loading screen before incrementing step
                RoverViewManager::drawLoadingScreen(currentStatusMessage);
                
                step++;
                
                if (step >= 5) 
                {
                    Utilities::LOG_DEBUG("Boot sequence complete, moving to WiFi phase");
                    setLoadingPhase(RoverTypes::LoadingPhase::CONNECTING_WIFI);
                    step = 0;
                }
            } 
            catch (const std::exception& e) 
            {
                Utilities::LOG_ERROR("Boot step %d failed: %s", step, e.what());
                switch(step) 
                {
                    case 1:
                        triggerFatalError(
                            static_cast<uint32_t>(RoverTypes::StartupErrorCode::DISPLAY_INIT_FAILED),
                            "Display initialization failed"
                        );
                        break;
                    case 2:
                        triggerFatalError(
                            static_cast<uint32_t>(RoverTypes::StartupErrorCode::UI_INIT_FAILED),
                            "UI initialization failed"
                        );
                        break;
                    case 3:
                    case 4:
                        triggerFatalError(
                            static_cast<uint32_t>(RoverTypes::StartupErrorCode::APP_INIT_FAILED),
                            "App initialization failed"
                        );
                        break;
                }
                return;
            }
        }
    }

    void RoverBehaviorManager::handleWiFiConnection() 
    {
        static unsigned long startAttempt = millis();
        static bool timeoutWarningShown = false;
        const unsigned long WIFI_TIMEOUT = 20000;

        if (!PSY::WiFiManager::isConnected()) 
        {
            PSY::WiFiManager::init();
            delay(250);
            PSY::WiFiManager::checkConnection();
        }

        if (!timeoutWarningShown && millis() - startAttempt > WIFI_TIMEOUT) 
        {
            timeoutWarningShown = true;
            triggerError(
                static_cast<uint32_t>(RoverTypes::StartupErrorCode::WIFI_INIT_FAILED),
                "WiFi connection timeout",
                ErrorType::WARNING
            );
            setLoadingPhase(RoverTypes::LoadingPhase::SYNCING_TIME);
            return;
        }
        
        if (PSY::WiFiManager::isConnected()) 
        {
            setLoadingPhase(RoverTypes::LoadingPhase::SYNCING_TIME);
        }
    }

    void RoverBehaviorManager::handleTimeSync() 
    {
        static int retryCount = 0;
        const int MAX_RETRIES = 3;
        
        if (!PSY::WiFiManager::getTimeInitialized()) 
        {
            if (retryCount >= MAX_RETRIES) 
            {
                triggerError(
                    static_cast<uint32_t>(RoverTypes::StartupErrorCode::TIME_SYNC_FAILED),
                    "Failed to sync time",
                    ErrorType::WARNING
                );
                setState(RoverTypes::BehaviorState::HOME);
                return;
            }
            PSY::WiFiManager::syncTime();
            retryCount++;
        } 
        else 
        {
            setState(RoverTypes::BehaviorState::HOME);
            retryCount = 0;
        }
    }

    void RoverBehaviorManager::triggerFatalError(uint32_t errorCode, const char* errorMessage) {
        setState(RoverTypes::BehaviorState::FATAL_ERROR);
        RoverViewManager::errorCode = errorCode;
        RoverViewManager::errorMessage = errorMessage;
        RoverViewManager::isError = true;
        RoverViewManager::isFatalError = true;
        
        // Draw error screen
        RoverViewManager::drawErrorScreen(errorCode, errorMessage, true);
    }

    void RoverBehaviorManager::triggerError(uint32_t errorCode, const char* errorMessage, ErrorType type) 
    {
        // Always log to serial
        Serial.printf("ERROR 0x%08X: %s (Type: %s)\n", 
            errorCode, 
            errorMessage, 
            type == ErrorType::FATAL ? "FATAL" : 
            type == ErrorType::WARNING ? "WARNING" : "SILENT"
        );
        
        if (type == ErrorType::SILENT) return;
        
        if (type == ErrorType::FATAL) 
        {
            setState(RoverTypes::BehaviorState::FATAL_ERROR);
        } 
        else 
        {
            setState(RoverTypes::BehaviorState::WARNING);
            warningStartTime = millis();
            isCountingDown = true;
        }
        
        RoverViewManager::errorCode = errorCode;
        RoverViewManager::errorMessage = errorMessage;
        RoverViewManager::isError = true;
        RoverViewManager::isFatalError = (type == ErrorType::FATAL);
        
        // Play error sound and set LED pattern
        SoundFxManager::playErrorCode(errorCode, type == ErrorType::FATAL);
        LEDManager::setErrorPattern(errorCode, type == ErrorType::FATAL);
        
        // Draw error screen with countdown for warnings
        RoverViewManager::drawErrorScreen(errorCode, errorMessage, type == ErrorType::FATAL);
    }

    int RoverBehaviorManager::getCurrentBootStep() {
        return currentBootStep;
    }

    void RoverBehaviorManager::setLoadingPhase(RoverTypes::LoadingPhase phase) 
    {
        loadingPhase = phase;
        
        switch(phase) 
        {
            case RoverTypes::LoadingPhase::BOOTING:
                currentStatusMessage = "Booting...";
                break;
            case RoverTypes::LoadingPhase::CONNECTING_WIFI:
                currentStatusMessage = "Connecting to WiFi...";
                break;
            case RoverTypes::LoadingPhase::SYNCING_TIME:
                currentStatusMessage = "Syncing time...";
                break;
        }
    }

    RoverTypes::LoadingPhase RoverBehaviorManager::getLoadingPhase() 
    {
        return loadingPhase;
    }

    const char* RoverBehaviorManager::getStatusMessage() 
    {
        return currentStatusMessage;
    }

}#ifndef UTILITIES_H
#define UTILITIES_H
#include "../PrefrontalCortex/ProtoPerceptions.h"
#include "../CorpusCallosum/SynapticPathways.h"

namespace PrefrontalCortex 
{
    using namespace CorpusCallosum;
    using namespace SystemTypes;
    using namespace StorageTypes;
    
    class Utilities 
    {
    public:
        // Use SystemTypes::LogLevel from ProtoPerceptions
        static SystemTypes::LogLevel CURRENT_LOG_LEVEL;
        static const char* LOG_LEVEL_STRINGS[];

        // Logging macros for convenience
        static void LOG_PROD(const char* format, ...) { debugLog(SystemTypes::LogLevel::PRODUCTION, format); }
        static void LOG_ERROR(const char* format, ...) { debugLog(SystemTypes::LogLevel::ERROR, format); }
        static void LOG_DEBUG(const char* format, ...) { debugLog(SystemTypes::LogLevel::DEBUG, format); }
        static void LOG_INFO(const char* format, ...) { debugLog(SystemTypes::LogLevel::INFO, format); }
        static void LOG_WARNING(const char* format, ...) { debugLog(SystemTypes::LogLevel::WARNING, format); }

    private:
        static void debugLog(SystemTypes::LogLevel level, const char* format, ...);
    };
}

#endif // UTILITIES_H
#pragma once
#include "../CorpusCallosum/SynapticPathways.h"
#include "../PrefrontalCortex/ProtoPerceptions.h"
#include "../PrefrontalCortex/utilities.h"
#include <Arduino.h>
#include <XPowersLib.h>
#include <FastLED.h>
#include "TFT_eSPI.h"

namespace PrefrontalCortex 
{
    using namespace CorpusCallosum;
    using PC::PowerTypes::PowerState;
    using PC::PowerTypes::BatteryStatus;
    using PC::PowerTypes::PowerConfig;

    // Forward declarations
    extern TFT_eSPI tft;
    extern void setBacklight(uint8_t brightness);
    extern void drawSprite();

    class PowerManager 
    {
    public:
        // Timeout constants
        static const unsigned long IDLE_TIMEOUT = 60000;    // Base timeout (1 minute)
        static const unsigned long DIM_TIMEOUT = IDLE_TIMEOUT * 2;
        static const unsigned long SLEEP_TIMEOUT = IDLE_TIMEOUT * 3;
        static const uint8_t DIM_BRIGHTNESS = 128;         // 50% brightness

        // PWM constants
        static const uint8_t PWM_CHANNEL = 0;
        static const uint32_t PWM_FREQUENCY = 5000;
        static const uint8_t PWM_RESOLUTION = 8;
        static const uint8_t BACKLIGHT_PIN = 38;  // Verify this pin number
        
        // Core methods
        static void init();
        static void update();
        static void wakeFromSleep();
        static void enterDeepSleep();  // This is our primary sleep method

        static unsigned long getUpTime();
        
        // State getters/setters
        static PowerState getCurrentPowerState();
        static void updateLastActivityTime();
        
        // Display control
        static void setBacklight(uint8_t brightness);
        static void setupBacklight();
        
        // Battery management
        static BatteryStatus getBatteryStatus();
        static String getChargeStatus();
        static bool isCharging();

        // Add missing method declarations
        static void initializeBattery();
        static int calculateBatteryPercentage(int voltage);
        static void checkPowerState();
        static int getBatteryPercentage();

    private:
        static XPowersPPM PPM;
        static bool batteryInitialized;
        static unsigned long lastActivityTime;
        static PowerState currentPowerState;
        static unsigned long startUpTime;
    };
}
#ifndef PROTO_PERCEPTIONS_H
#define PROTO_PERCEPTIONS_H

#include <Arduino.h>
#include <FastLED.h>
#include <vector>
#include <string>
#include <functional>

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

        enum class BehaviorState 
        {
            STARTUP,
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
            WARNING
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

        struct NoteState 
        {
            CRGB color1;
            CRGB color2;  // For sharps/flats
            bool isSharp;
            uint8_t position;
            
            NoteState() : color1(CRGB::Black), color2(CRGB::Black), isSharp(false), position(0) {}
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
            APP_INIT_FAILED
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

    // Power Management Types
    namespace PowerTypes 
    {
        enum class PowerState 
        {
            AWAKE,          // Full power, display at max brightness
            DIM_DISPLAY,    // Display dimmed to save power
            DISPLAY_OFF,    // Display off but system still running
            DEEP_SLEEP     // System in deep sleep mode
        };

        struct BatteryStatus 
        {
            float voltageLevel;      // Current voltage
            int percentageCharge;    // 0-100%
            bool isCharging;         // Charging state
            float temperature;       // Battery temperature
            uint32_t lastUpdateTime; // Last time status was updated
        };

        struct PowerConfig 
        {
            uint32_t idleTimeout;     // Time before entering idle state
            uint32_t dimTimeout;      // Time before dimming display
            uint32_t sleepTimeout;    // Time before entering sleep
            uint8_t dimBrightness;    // Brightness level when dimmed
            bool enableDeepSleep;     // Whether deep sleep is allowed
        };
    }
}

#endif // PROTO_PERCEPTIONS_H #include "utilities.h"
#include <stdarg.h>
#include <stdio.h>
#include <Arduino.h>

namespace PrefrontalCortex 
{
    using namespace CorpusCallosum;
    using namespace SystemTypes;
    using namespace StorageTypes;
    
    // Initialize static members
    SystemTypes::LogLevel Utilities::CURRENT_LOG_LEVEL = SystemTypes::LogLevel::DEBUG;
    
    const char* Utilities::LOG_LEVEL_STRINGS[] = 
    {
        "PROD",
        "ERROR", 
        "DEBUG",
        "INFO",
        "WARNING"
    };

    void Utilities::debugLog(SystemTypes::LogLevel level, const char* format, ...) 
    {
        if (level <= CURRENT_LOG_LEVEL) 
        {
            char buffer[256];
            va_list args;
            va_start(args, format);
            vsnprintf(buffer, sizeof(buffer), format, args);
            va_end(args);
            
            Serial.printf("[%s] %s\n", LOG_LEVEL_STRINGS[static_cast<int>(level)], buffer);
        }
    }
}#include "PowerManager.h"
#include "../VisualCortex/LEDManager.h"
#include "../VisualCortex/RoverViewManager.h"
#include "../VisualCortex/RoverManager.h"

namespace PrefrontalCortex 
{
    namespace PC = PrefrontalCortex;  // Add namespace alias
    using VC::LEDManager;
    using VC::RoverViewManager;
    using VC::RoverManager;

    // Initialize static members
    XPowersPPM PowerManager::PPM;
    bool PowerManager::batteryInitialized = false;
    unsigned long PowerManager::lastActivityTime = 0;
    PowerState PowerManager::currentPowerState = PowerState::AWAKE;
    unsigned long PowerManager::startUpTime = 0;
    void PowerManager::init() {
        startUpTime = millis();
        // Ensure LED power stays enabled
        pinMode(BOARD_PWR_EN, OUTPUT);
        digitalWrite(BOARD_PWR_EN, HIGH);
        
        // Initialize LEDC for backlight first
        setupBacklight();
        
        // Initialize I2C for battery management
        Wire.begin(BOARD_I2C_SDA, BOARD_I2C_SCL);
        Wire.setClock(100000);
        
        delay(100);  // Give I2C time to stabilize
        
        // Initialize battery directly
        bool result = PPM.init(Wire, BOARD_I2C_SDA, BOARD_I2C_SCL, BQ25896_SLAVE_ADDRESS);
        if (result) {
            batteryInitialized = true;
            Utilities::LOG_PROD("Battery management initialized successfully");
            
            // Configure battery parameters
            PPM.setSysPowerDownVoltage(3300);
            PPM.setInputCurrentLimit(3250);
            PPM.disableCurrentLimitPin();
            PPM.setChargeTargetVoltage(4208);
            PPM.setPrechargeCurr(64);
            PPM.setChargerConstantCurr(832);
            PPM.enableADCMeasure();
            PPM.enableCharge();
        } else {
            Utilities::LOG_ERROR("Failed to initialize battery management");
        }
        
        lastActivityTime = millis();
        currentPowerState = PowerState::AWAKE;
    }

    unsigned long PowerManager::getUpTime() {
        return millis() - startUpTime;
    }

    void PowerManager::initializeBattery() {
        if (!batteryInitialized) {
            Utilities::LOG_DEBUG("Initializing battery management...");
            
            if (!PPM.begin(Wire, AXP2101_SLAVE_ADDRESS, BOARD_I2C_SDA, BOARD_I2C_SCL)) {
                Utilities::LOG_ERROR("Failed to initialize battery management");
                return;
            }
            
            // Configure battery parameters
            PPM.setSysPowerDownVoltage(3300);
            PPM.setInputCurrentLimit(3250);
            PPM.disableCurrentLimitPin();
            PPM.setChargeTargetVoltage(4208);
            PPM.setPrechargeCurr(64);
            PPM.setChargerConstantCurr(832);
            PPM.enableADCMeasure();
            PPM.enableCharge();
            
            batteryInitialized = true;
            
            // Detailed logging
            Utilities::LOG_DEBUG("Battery configuration complete:");
            Utilities::LOG_DEBUG("- System power down voltage: %d", PPM.getSysPowerDownVoltage());
            Utilities::LOG_DEBUG("- Input current limit: %d", PPM.getInputCurrentLimit());
            Utilities::LOG_DEBUG("- Charge target voltage: %d", PPM.getChargeTargetVoltage());
            Utilities::LOG_DEBUG("- Precharge current: %d", PPM.getPrechargeCurr());
            Utilities::LOG_DEBUG("- Constant current: %d", PPM.getChargerConstantCurr());
            
            Utilities::LOG_PROD("Battery management initialized successfully");
        }
    }

    int PowerManager::calculateBatteryPercentage(int voltage) {
        const int maxVoltage = 4200; // 4.2V fully charged
        const int minVoltage = 3300; // 3.3V empty
        
        int percentage = map(voltage, minVoltage, maxVoltage, 0, 100);
        return constrain(percentage, 0, 100);
    }

    void PowerManager::checkPowerState() {
        unsigned long idleTime = millis() - lastActivityTime;
        PowerState newState = currentPowerState;

        // State transitions
        if (idleTime < IDLE_TIMEOUT) {
            newState = PowerState::AWAKE;
        } else if (idleTime < IDLE_TIMEOUT * 2) {
            newState = PowerState::DIM_DISPLAY;
        } else if (idleTime < IDLE_TIMEOUT * 3) {
            newState = PowerState::DISPLAY_OFF;
        } else {
            newState = PowerState::DEEP_SLEEP;
        }

        // Only handle state change if needed
        if (newState != currentPowerState) {
            // Production level logging for state changes
            Utilities::LOG_PROD("Sleep state changing from %d to %d", currentPowerState, newState);
            currentPowerState = newState;
        }
    }

    void PowerManager::wakeFromSleep() {
        Utilities::LOG_PROD("Waking from sleep mode");
        lastActivityTime = millis();
        currentPowerState = PowerState::AWAKE;
        RoverManager::setShowTime(false);
        
        // Initialize display first
        tft.init();
        tft.writecommand(TFT_SLPOUT);
        delay(120);
        tft.writecommand(TFT_DISPON);
        
        // Initialize LED system
        LEDManager::init();
        
        // Restore backlight
        setBacklight(255);
        
        // Re-initialize critical components
        RoverViewManager::init();
        
        // Force display update
        drawSprite();
    }

    PowerState PowerManager::getCurrentPowerState() {
        return currentPowerState;
    }

    int PowerManager::getBatteryPercentage() {
        if (!batteryInitialized) return 0;
        
        int voltage = PPM.getBattVoltage();
        return calculateBatteryPercentage(voltage);
    }

    String PowerManager::getChargeStatus() {
        if (!batteryInitialized) return "Not Found";
        return PPM.getChargeStatusString();
    }

    bool PowerManager::isCharging() {
        if (!batteryInitialized) return false;
        return PPM.isVbusIn();
    }

    void PowerManager::updateLastActivityTime() {
        lastActivityTime = millis();
    }

    void PowerManager::enterDeepSleep() {
        LEDManager::stopLoadingAnimation();

        FastLED.show();
        tft.writecommand(TFT_DISPOFF);
        tft.writecommand(TFT_SLPIN);
        
        // Configure wake-up sources with correct GPIO pins
        gpio_pullup_en(GPIO_NUM_0);    // Enable pull-up on BOARD_USER_KEY
        gpio_pullup_en(GPIO_NUM_21);   // Enable pull-up on ENCODER_KEY
        
        esp_sleep_enable_ext0_wakeup(GPIO_NUM_0, 0);    // Wake on BOARD_USER_KEY falling edge
        esp_sleep_enable_ext1_wakeup(1ULL << GPIO_NUM_21, ESP_EXT1_WAKEUP_ANY_HIGH);  // Wake on ENCODER_KEY
        
        // Wait for buttons to be released
        while (digitalRead(BOARD_USER_KEY) == LOW || digitalRead(ENCODER_KEY) == LOW) {
            delay(10);
        }
        delay(100);
        
        esp_deep_sleep_start();
    }

    void PowerManager::setupBacklight() {
        ledcSetup(PWM_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION);
        ledcAttachPin(BACKLIGHT_PIN, PWM_CHANNEL);
        ledcWrite(PWM_CHANNEL, 255);  // Full brightness
        Serial.println("Backlight setup complete");
    }

    void PowerManager::setBacklight(uint8_t brightness) {
        ledcWrite(PWM_CHANNEL, brightness);
        drawSprite();
    }

    void PowerManager::update() {
        unsigned long currentTime = millis();
        PowerState newState = currentPowerState;  // Changed from SleepState to PowerState
        
        // Debug level logging for idle time
        unsigned long idleTime = currentTime - lastActivityTime;
        Utilities::LOG_DEBUG("Idle time: %lu ms, Current state: %d", idleTime, currentPowerState);
        
        // State transitions
        if (idleTime < IDLE_TIMEOUT) {
            newState = PowerState::AWAKE;
        } else if (idleTime < DIM_TIMEOUT) {
            newState = PowerState::DIM_DISPLAY;
        } else if (idleTime < SLEEP_TIMEOUT) {
            newState = PowerState::DISPLAY_OFF;
        } else {
            newState = PowerState::DEEP_SLEEP;
        }
        
        // Only handle state change if needed
        if (newState != currentPowerState) {
            Utilities::LOG_PROD("Power state changing from %d to %d", currentPowerState, newState);
            
            // Handle state-specific actions
            switch (newState) {
                case PowerState::DIM_DISPLAY:
                    setBacklight(DIM_BRIGHTNESS);
                    break;
                    
                case PowerState::DISPLAY_OFF:
                    setBacklight(0);
                    break;
                    
                case PowerState::DEEP_SLEEP:
                    enterDeepSleep();
                    break;
                    
                case PowerState::AWAKE:
                    setBacklight(255);
                    break;
            }
            
            currentPowerState = newState;
        }
    }

}

#ifndef SDMANAGER_H
#define SDMANAGER_H

#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "../VisualCortex/LEDManager.h"
#include "../VisualCortex/RoverManager.h"
#include "../PrefrontalCortex/RoverBehaviorManager.h"
#include "../PrefrontalCortex/utilities.h"
#include "../MotorCortex/PinDefinitions.h"

namespace PrefrontalCortex 
{

    class SDManager {
    public:
        static void init(uint8_t cs);
        static void listDir(fs::FS &fs, const char *dirname, uint8_t levels);
        static void createDir(fs::FS &fs, const char *path);
        static void removeDir(fs::FS &fs, const char *path);
        static void readFile(fs::FS &fs, const char *path);
        static void writeFile(fs::FS &fs, const char *path, const char *message);
        static void appendFile(fs::FS &fs, const char *path, const char *message);
        static void renameFile(fs::FS &fs, const char *path1, const char *path2);
        static void deleteFile(fs::FS &fs, const char *path);
        static void testFileIO(fs::FS &fs, const char *path);
        static bool isInitialized() { return initialized; }

        static void ensureNFCFolderExists();
        static bool hasCardBeenScanned(uint32_t cardId);
        static void recordCardScan(uint32_t cardId);

        static uint64_t getCardSize();
        static uint64_t getTotalSpace();
        static uint64_t getUsedSpace();

    private:
        static bool initialized;
        static uint8_t cardType;
        static const char* NFC_FOLDER;
        static const char* SCANNED_CARDS_FILE;
        static uint64_t cardSize;
        static uint64_t totalSpace;
        static uint64_t usedSpace;
    };

}
#endif // SDMANAGER_H#ifndef ROVER_BEHAVIOR_MANAGER_H
#define ROVER_BEHAVIOR_H

#include "../CorpusCallosum/SynapticPathways.h"

namespace PrefrontalCortex 
{
    using namespace CorpusCallosum;  // Keep this for other utilities
    
    class RoverBehaviorManager 
    {
    public:
        static void init();
        static void update();
        static RoverTypes::BehaviorState getCurrentState();
        static void setState(RoverTypes::BehaviorState state);
        static RoverTypes::LoadingPhase getLoadingPhase();
        static void setLoadingPhase(RoverTypes::LoadingPhase phase);
        static const char* getStatusMessage();
        static void triggerFatalError(uint32_t errorCode, const char* errorMessage);
        static void handleFatalError();
        static void triggerError(uint32_t errorCode, const char* errorMessage, ErrorType type);
        static void handleWarning();
        static bool IsInitialized();
        static bool isErrorFatal(uint32_t errorCode);
        static int getCurrentBootStep();
        static void updateWarningCountdown();
        static constexpr unsigned long WARNING_DURATION = 120000;  // 2 minutes
        static constexpr unsigned long FATAL_REBOOT_DELAY = 60000;  // 1 minute
        static bool isWarningCountdownActive() { return isCountingDown; }
        static int getRemainingWarningSeconds();
        static unsigned long getWarningStartTime() { return warningStartTime; }

    private:
        static RoverTypes::BehaviorState currentState;
        static RoverTypes::LoadingPhase loadingPhase;
        static const char* currentStatusMessage;
        static RoverTypes::BehaviorState previousState;
        static unsigned long warningStartTime;
        static bool isCountingDown;
        static int currentBootStep;
        static unsigned long fatalErrorStartTime;
        static bool isFatalError;
        static VisualTypes::VisualPattern pattern;
        static String statusMessage;

        static void handleLoading();
        static void handleHome();
        static void handleMenu();
        static void handleApp();
        static void handleError();
        static void handleBooting();
        static void handleWiFiConnection();
        static void handleTimeSync();
        static bool initialized;
    };
}

#endif // ROVER_BEHAVIOR_H 
#pragma once

#include <Arduino.h>
#include <SPI.h>
#include "../PrefrontalCortex/utilities.h"
#include "../MotorCortex/PinDefinitions.h"
#include "../CorpusCallosum/SynapticPathways.h"

using namespace CorpusCallosum;

namespace PrefrontalCortex 
{

    class SPIManager {
    public:
        // Check if SPIManager is initialized
        static bool isInitialized();

        // Initialize SPI and CS pins
        static void init();

        // Select a specific SPI device
        static void selectDevice(uint8_t deviceCS);

        // Deselect all SPI devices
        static void deselectAll();

    private:
        static bool initialized;
    };

}#include "SDManager.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "../VisualCortex/LEDManager.h"
#include "../VisualCortex/RoverManager.h"
#include "../PrefrontalCortex/RoverBehaviorManager.h"
#include "../PrefrontalCortex/utilities.h"
#include "../MotorCortex/PinDefinitions.h"
#include "../PrefrontalCortex/SPIManager.h"

namespace PrefrontalCortex 
{
    bool SDManager::initialized = false;
    const char* SDManager::NFC_FOLDER = "/NFC";
    const char* SDManager::SCANNED_CARDS_FILE = "/NFC/scannedcards.inf";

    uint8_t SDManager::cardType;
    uint64_t SDManager::cardSize = 0;
    uint64_t SDManager::totalSpace = 0;
    uint64_t SDManager::usedSpace = 0;


    uint64_t SDManager::getTotalSpace() {
        return SDManager::totalSpace;
    }

    uint64_t SDManager::getUsedSpace() {
        return SDManager::usedSpace;
    }

    uint64_t SDManager::getCardSize() {
        return SDManager::cardSize;
    }

    #define REASSIGN_PINS
    int sck =  BOARD_SD_SCK;
    int miso =  BOARD_SD_MISO;
    int mosi =  BOARD_SD_MOSI;
    int cs =    BOARD_SD_CS;

    void SDManager::listDir(fs::FS &fs, const char * dirname, uint8_t levels){
        Utilities::LOG_DEBUG("Listing directory: %s\n", dirname);

        File root = fs.open(dirname);
        if(!root){
            Utilities::LOG_ERROR("Failed to open directory");
            return;
        }
        if(!root.isDirectory()){
            Utilities::LOG_ERROR("Not a directory");
            return;
        }

        File file = root.openNextFile();
        while(file){
            if(file.isDirectory()){
                Utilities::LOG_DEBUG("  DIR : %s", file.name());
                if(levels){
                    listDir(fs, file.path(), levels -1);
                }
            } else {
                Utilities::LOG_DEBUG("  FILE: %s  SIZE: %llu", file.name(), file.size());
            }
            file = root.openNextFile();
        }
    }

    void SDManager::createDir(fs::FS &fs, const char * path){
        Serial.printf("Creating Dir: %s\n", path);
        if(fs.mkdir(path)){
            Serial.println("Dir created");
        } else {
            Serial.println("mkdir failed");
        }
    }

    void SDManager::removeDir(fs::FS &fs, const char * path){
        Serial.printf("Removing Dir: %s\n", path);
        if(fs.rmdir(path)){
            Serial.println("Dir removed");
        } else {
            Serial.println("rmdir failed");
        }
    }

    void SDManager::readFile(fs::FS &fs, const char * path){
        Serial.printf("Reading file: %s\n", path);

        File file = fs.open(path);
        if(!file){
            Serial.println("Failed to open file for reading");
            return;
        }

        Serial.print("Read from file: ");
        while(file.available()){
            Serial.write(file.read());
        }
        file.close();
    }

    void SDManager::writeFile(fs::FS &fs, const char * path, const char * message){
        Serial.printf("Writing file: %s\n", path);

        File file = fs.open(path, FILE_WRITE);
        if(!file){
            Serial.println("Failed to open file for writing");
            return;
        }
        if(file.print(message)){
            Serial.println("File written");
        } else {
            Serial.println("Write failed");
        }
        file.close();
    }

    void SDManager::appendFile(fs::FS &fs, const char * path, const char * message){
        Serial.printf("Appending to file: %s\n", path);

        File file = fs.open(path, FILE_APPEND);
        if(!file){
            Serial.println("Failed to open file for appending");
            return;
        }
        if(file.print(message)){
            Serial.println("Message appended");
        } else {
            Serial.println("Append failed");
        }
        file.close();
    }

    void SDManager::renameFile(fs::FS &fs, const char * path1, const char * path2){
        Serial.printf("Renaming file %s to %s\n", path1, path2);
        if (fs.rename(path1, path2)) {
            Serial.println("File renamed");
        } else {
            Serial.println("Rename failed");
        }
    }

    void SDManager::deleteFile(fs::FS &fs, const char * path){
        Serial.printf("Deleting file: %s\n", path);
        if(fs.remove(path)){
            Serial.println("File deleted");
        } else {
            Serial.println("Delete failed");
        }
    }

    void SDManager::testFileIO(fs::FS &fs, const char * path){
        File file = fs.open(path);
        static uint8_t buf[512];
        size_t len = 0;
        uint32_t start = millis();
        uint32_t end = start;
        if(file){
            len = file.size();
            size_t flen = len;
            start = millis();
            while(len){
                size_t toRead = len;
                if(toRead > 512){
                    toRead = 512;
                }
                file.read(buf, toRead);
                len -= toRead;
            }
            end = millis() - start;
            Serial.printf("%u bytes read for %lu ms\n", flen, end);
            file.close();
        } else {
            Serial.println("Failed to open file for reading");
        }


        file = fs.open(path, FILE_WRITE);
        if(!file){
            Serial.println("Failed to open file for writing");
            return;
        }

        size_t i;
        start = millis();
        for(i=0; i<2048; i++){
            file.write(buf, 512);
        }
        end = millis() - start;
        Serial.printf("%u bytes written for %lu ms\n", 2048 * 512, end);
        file.close();
        delete[] buf;
    }

    void SDManager::init(uint8_t cs) {
        if (initialized) return;

        // Initialize SD card
        if (!SD.begin(cs)) {
            Utilities::LOG_ERROR("SD Card initialization failed!");
            return;
        }

        // Check card type
        cardType = SD.cardType();
        if (cardType == CARD_NONE) {
            Utilities::LOG_ERROR("No SD Card detected!");
            return;
        }

        // Log card information
        cardSize = (uint64_t)SD.cardSize() / (1024 * 1024);
        totalSpace = (uint64_t)SD.totalBytes() / (1024 * 1024);
        usedSpace = (uint64_t)SD.usedBytes() / (1024 * 1024);

        const char* cardTypeStr = 
            cardType == CARD_MMC ? "MMC" :
            cardType == CARD_SD ? "SDSC" :
            cardType == CARD_SDHC ? "SDHC" : "UNKNOWN";
        
        Utilities::LOG_DEBUG("SD Card Type: %s", cardTypeStr);
        Utilities::LOG_DEBUG("SD Card Size: %llu MB", cardSize);
        Utilities::LOG_DEBUG("Total space: %llu MB", totalSpace);
        Utilities::LOG_DEBUG("Used space: %llu MB", usedSpace);
        
        initialized = true;
    }



    void SDManager::ensureNFCFolderExists() {
        if (!SD.exists(NFC_FOLDER)) {
            SD.mkdir(NFC_FOLDER);
        }
    }

    bool SDManager::hasCardBeenScanned(uint32_t cardId) {
        return false;
        /*
        if (!initialized) return false;
        
        File file = SD.open(SCANNED_CARDS_FILE, FILE_READ);
        if (!file) return false;
        
        uint32_t storedId;
        bool found = false;
        
        while (file.available() >= sizeof(uint32_t)) {
            file.read((uint8_t*)&storedId, sizeof(uint32_t));
            if (storedId == cardId) {
                found = true;
                break;
            }
        }
        
        file.close();
        return found;
        */
    }

    void SDManager::recordCardScan(uint32_t cardId) {
        return;
        /*
        if (!initialized) return;
        
        ensureNFCFolderExists();
        
        File file = SD.open(SCANNED_CARDS_FILE, FILE_APPEND);
        if (!file) {
            Utilities::LOG_ERROR("Failed to open scanned cards file for writing");
            return;
        }
        
        file.write((uint8_t*)&cardId, sizeof(uint32_t));
        file.close();
        */
    } 

}