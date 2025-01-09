#ifndef SOUND_FX_MANAGER_H
#define SOUND_FX_MANAGER_H

#include "pitches.h"
#include "utilities.h"
#include <time.h>
#include <SPIFFS.h>
#include <FS.h>

// Define PWM channel for tone generation
#define TONE_PWM_CHANNEL 2

class SoundFxManager {
public:
    // Jingle control
    static void startJingle();
    static void updateJingle();
    static bool isJinglePlaying();
    
    // Button and interaction sounds
    static void playRotaryPressSound(int mode = 0);
    static void playRotaryTurnSound(bool clockwise);
    static void playSideButtonSound(bool start = false);
    static void playSuccessSound();
    static void playErrorSound(int type);
    
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

    // Add new startup sound method
    static void playStartupSound();

    // Add with other public method declarations
    static void playJingle();

private:
    struct Note {
        int pitch;
        int duration;
        int delay;
    };
    
    static const Note ROVERBYTE_JINGLE[];
    static const int JINGLE_LENGTH;
    static int currentNote;
    static unsigned long lastNoteTime;
    static bool jinglePlaying;
    
    static void playTone(int frequency, int duration);

    // Add new startup jingle members
    static const Note STARTUP_JINGLE[];
    static const int STARTUP_JINGLE_LENGTH;

    // Add with other static members
    static const Note ROVER_JINGLE[];
    static const int ROVER_JINGLE_LENGTH;
};

#endif 