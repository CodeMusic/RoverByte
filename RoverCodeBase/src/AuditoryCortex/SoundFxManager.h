#ifndef SOUND_FX_MANAGER_H
#define SOUND_FX_MANAGER_H

#include "../AuditoryCortex/PitchPerception.h"
#include "../PrefrontalCortex/utilities.h"
#include "../VisualCortex/RoverManager.h"
#include <time.h>
#include <SPIFFS.h>
#include <FS.h>
#include "Audio.h"
#include <SD.h>

// Define PWM channel for tone generation
#define TONE_PWM_CHANNEL 2

// Audio pins
#define I2S_BCK_PIN  BOARD_VOICE_BCLK   // Pin 46
#define I2S_WS_PIN   BOARD_VOICE_LRCLK  // Pin 40
#define I2S_DOUT_PIN BOARD_VOICE_DIN    // Pin 7

// Recording constants
#define EXAMPLE_I2S_CH      0
#define EXAMPLE_SAMPLE_RATE 44100
#define EXAMPLE_BIT_SAMPLE  16
#define NUM_CHANNELS        1
#define SAMPLE_SIZE         (EXAMPLE_BIT_SAMPLE * 1024)
#define BYTE_RATE          (EXAMPLE_SAMPLE_RATE * (EXAMPLE_BIT_SAMPLE / 8)) * NUM_CHANNELS
const int WAVE_HEADER_SIZE = 44;

class SoundFxManager {
private:
    struct Note {
        int pitch;
        int duration;
        int delay;
    };
    
    // Jingle data
    static const Note ROVERBYTE_JINGLE[];
    static const int JINGLE_LENGTH;
    static int currentNote;
    static unsigned long lastNoteTime;
    static bool jinglePlaying;
    
    // Recording state
    static bool isRecording;
    static File recordFile;
    static Audio audio;
    static bool isPlayingSound;
    static const char* RECORD_FILENAME;
    static bool isJingleActive;
    static unsigned long jingleStartTime;
    static int currentJingleNote;
    

    static void init_microphone();
    static void initializeAudio();
    static void generate_wav_header(char* wav_header, uint32_t wav_size, uint32_t sample_rate);
    

public:
    // Core functionality
    static void init();
    static void playErrorSound(int type);
    static void playJingle();
    static void playRotaryPressSound(int mode = 0);
    static void playRotaryTurnSound(bool clockwise);
    static void playSideButtonSound(bool start = false);
    static void playStartupSound();
    static void playSuccessSound();
    static void playTimerDropSound(CRGB color);
    static void playTone(int frequency, int duration, int position = 0); // Update this line
    
    // Jingle control
    static void startJingle();
    static void updateJingle();
    static bool isJinglePlaying();
    
    // Recording functionality
    static void startRecording();
    static void stopRecording();
    static void playRecording();
    static bool isCurrentlyRecording() { return isRecording; }
    static bool isCurrentlyPlaying() { return isPlayingSound; }
    static bool isPlaying() { return isPlayingSound; }
    static void audio_eof_mp3(const char* info);
      


    static void stopJingle();

    static void playVoiceLine(const char* line, uint32_t cardId = 0);
    static void playCardMelody(uint32_t cardId);

    static void update() {
        if (jinglePlaying) {
            updateJingle();
        }
        if (isPlayingSound) {
            audio_eof_mp3("update");
        }
    }
};

#endif 