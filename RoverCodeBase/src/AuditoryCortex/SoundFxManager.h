#ifndef SOUND_FX_MANAGER_H
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
        static void playToneFx(PC::AudioTypes::Tone type);  // New method for playing system tones
        static void playTune(PC::AudioTypes::TunesTypes type);  // For playing musical pieces
        static void playTone(int frequency, int duration, int volume = 42); // Raw tone playback
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

#endif // SOUND_FX_MANAGER_H