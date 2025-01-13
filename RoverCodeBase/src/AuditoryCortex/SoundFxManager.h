#ifndef SOUND_FX_MANAGER_H
#define SOUND_FX_MANAGER_H

#include "../AuditoryCortex/pitches.h"
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
    
    static void playTone(int frequency, int duration);
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
      
    // Note helpers
    static inline int getDayBaseNote4() {
        time_t now = time(nullptr);
        struct tm* timeInfo = localtime(&now);
        int dayOfWeek = timeInfo->tm_wday;
        
        const int dayNotes[] = {
            NOTE_C4,  // Sunday
            NOTE_D4,  // Monday
            NOTE_E4,  // Tuesday
            NOTE_F4,  // Wednesday
            NOTE_G4,  // Thursday
            NOTE_A4,  // Friday
            NOTE_B4   // Saturday
        };
        
        return dayNotes[dayOfWeek];
    }

    static inline int getDayBaseNote5() {
        return getDayBaseNote4() * 2;
    }

    static inline int getNoteMinus2(int baseNote) {
        return baseNote * 8 / 10;
    }

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