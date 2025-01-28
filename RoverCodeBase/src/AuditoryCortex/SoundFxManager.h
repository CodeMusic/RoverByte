#ifndef SOUND_FX_MANAGER_H
#define SOUND_FX_MANAGER_H

#include "../AuditoryCortex/PitchPerception.h"
#include "../PrefrontalCortex/utilities.h"
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
    // Represents an error tone with frequency and duration
    struct ErrorTone {
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

    class SoundFxManager {
    private:
        /* ========================== Private Members ========================== */
        
        // Jingle-related state
        static const int JINGLE_LENGTH;
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
        static Tunes::TunesTypes selectedSong;
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
        static void playTune(Tunes::TunesTypes type);
        static void playTone(int frequency, int duration, int position = 0); // Custom tone playback
        static void playRotaryPressSound(int mode = 0);
        static void playRotaryTurnSound(bool clockwise);
        static void playSideButtonSound(bool start = false);
        static void playStartupSound(){ playTune(Tunes::TunesTypes::ROVERBYTE_JINGLE); }
        static void playSuccessSound();
        static void playTimerDropSound(CRGB color);
        static void playMenuCloseSound();
        static void playMenuOpenSound();
        static void playMenuSelectSound();
        static void playVoiceLine(const char* line, uint32_t cardId = 0);
        static void playCardMelody(uint32_t cardId);

        /* ========================== Jingle Control ========================== */
        static void startTune(Tunes::TunesTypes type);
        static void updateTune();
        static bool isTunePlaying();
        static void stopJingle();

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
            if (isTunePlaying) {
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

#endif // SOUND_FX_MANAGER_H