#include "SoundFxManager.h"
#include "Arduino.h"
#include <time.h>
#include <SPIFFS.h>

// Initialize static members
int SoundFxManager::currentNote = 0;
unsigned long SoundFxManager::lastNoteTime = 0;
bool SoundFxManager::jinglePlaying = false;

const SoundFxManager::Note SoundFxManager::ROVERBYTE_JINGLE[] = {
    // Opening flourish
    {NOTE_G4, 150, 50},   // G
    {NOTE_B4, 150, 50},   // B
    {NOTE_D5, 150, 50},   // D
    {NOTE_G5, 250, 100},  // High G
    
    // Playful melody
    {NOTE_E5, 200, 50},   // E
    {NOTE_C5, 200, 50},   // C
    {NOTE_D5, 200, 50},   // D
    {NOTE_B4, 200, 100},  // B
    
    // Bridge
    {NOTE_G4, 150, 50},   // G
    {NOTE_A4, 150, 50},   // A
    {NOTE_B4, 150, 50},   // B
    {NOTE_C5, 250, 100},  // C
    
    // Final part
    {NOTE_D5, 200, 50},   // D
    {NOTE_E5, 200, 50},   // E
    {NOTE_G5, 300, 100},  // High G
    {NOTE_G4, 400, 0}     // End on low G
};

const int SoundFxManager::JINGLE_LENGTH = sizeof(ROVERBYTE_JINGLE) / sizeof(Note);

const SoundFxManager::Note SoundFxManager::STARTUP_JINGLE[] = {
    {NOTE_G5, 50, 30},   // High G
    {NOTE_B5, 50, 30},   // Higher B
    {NOTE_D6, 50, 30},   // Even higher D
    {NOTE_G6, 100, 0}    // Highest G - final note
};

const int SoundFxManager::STARTUP_JINGLE_LENGTH = sizeof(STARTUP_JINGLE) / sizeof(Note);

const SoundFxManager::Note SoundFxManager::ROVER_JINGLE[] = {
    {NOTE_G4, 100, 0},    // G
    {NOTE_E5, 100, 0},    // E
    {NOTE_G5, 100, 0},    // G
    {NOTE_B5, 200, 50},   // B (held longer)
    {NOTE_A5, 100, 0},    // A
    {NOTE_G5, 200, 0}     // G (final note)
};
const int SoundFxManager::ROVER_JINGLE_LENGTH = 6;

void SoundFxManager::playTone(int frequency, int duration) {
    ledcSetup(TONE_PWM_CHANNEL, frequency, 8);
    ledcAttachPin(BOARD_VOICE_DIN, TONE_PWM_CHANNEL);
    ledcWrite(TONE_PWM_CHANNEL, 127);
    delay(duration);
    ledcWrite(TONE_PWM_CHANNEL, 0);
    ledcDetachPin(BOARD_VOICE_DIN);
}

void SoundFxManager::startJingle() {
    currentNote = 0;
    jinglePlaying = true;
    lastNoteTime = 0;
}

void SoundFxManager::updateJingle() {
    if (!jinglePlaying) return;
    
    unsigned long currentTime = millis();
    
    if (lastNoteTime == 0 || 
        (currentTime - lastNoteTime >= ROVERBYTE_JINGLE[currentNote].duration + 
                                     ROVERBYTE_JINGLE[currentNote].delay)) {
        
        playTone(ROVERBYTE_JINGLE[currentNote].pitch, 
                ROVERBYTE_JINGLE[currentNote].duration);
        
        lastNoteTime = currentTime;
        currentNote++;
        
        if (currentNote >= JINGLE_LENGTH) {
            jinglePlaying = false;
            currentNote = 0;
        }
    }
}

bool SoundFxManager::isJinglePlaying() {
    return jinglePlaying;
}

void SoundFxManager::playSuccessSound() {
    playTone(NOTE_C5, 100);
    delay(50);
    playTone(NOTE_E5, 100);
    delay(50);
    playTone(NOTE_G5, 200);
}

void SoundFxManager::playRotaryPressSound(int mode) {  // 0=Full, 1=Week, 2=Timer
    time_t now = time(nullptr);
    struct tm* timeInfo = localtime(&now);
    int dayOfWeek = timeInfo->tm_wday;  // 0-6 (Sunday-Saturday)
    
    // Base notes for each day (C through B)
    const int dayNotes[] = {
        NOTE_C4,  // Sunday
        NOTE_D4,  // Monday
        NOTE_E4,  // Tuesday
        NOTE_F4,  // Wednesday
        NOTE_G4,  // Thursday
        NOTE_A4,  // Friday
        NOTE_B4   // Saturday
    };
    
    // Get base note from current day
    int baseNote = dayNotes[dayOfWeek];
    
    // Adjust octave based on mode
    switch(mode) {
        case 0:  // Full mode - base octave
            playTone(baseNote, 100);
            break;
        case 1:  // Week mode - octave up
            playTone(baseNote * 2, 100);  // Multiply by 2 to go up an octave
            break;
        case 2:  // Timer mode - octave up + fifth
            playTone(baseNote * 3, 100);  // Multiply by 3 for octave + fifth
            break;
    }
}

void SoundFxManager::playRotaryTurnSound(bool clockwise) {
    if (clockwise) {
        playTone(getDayBaseNote4(), 50);
        playTone(getDayBaseNote5(), 50);
    } else {
        playTone(getDayBaseNote5(), 50);
        playTone(getDayBaseNote4(), 50);
    }
}

void SoundFxManager::playSideButtonSound(bool start) {
    if (start) {
        playTone(getDayBaseNote4(), 50);
        playTone(getDayBaseNote4(), 50);
    } else {
        playTone(getDayBaseNote5(), 100);
        int baseNote = getDayBaseNote5();
        playTone(getNoteMinus2(baseNote), 100);
    }
}

void SoundFxManager::playErrorSound(int type) {
    switch(type) {
        case 1: // Recording error
            playTone(NOTE_B5, 200);
            delay(100);
            playTone(NOTE_G5, 200);
            delay(100);
            playTone(NOTE_D5, 400);
            break;
            
        case 2: // SD card error
            playTone(NOTE_G5, 200);
            delay(100);
            playTone(NOTE_G5, 200);
            delay(100);
            playTone(NOTE_G4, 400);
            break;
            
        case 3: // Playback error
            playTone(NOTE_D5, 200);
            delay(100);
            playTone(NOTE_D5, 200);
            delay(100);
            playTone(NOTE_D4, 400);
            break;
    }
}

void SoundFxManager::playStartupSound() {
    if (!SPIFFS.exists("/initialized.txt")) {
        // Only play on first boot
        for (int i = 0; i < STARTUP_JINGLE_LENGTH; i++) {
            playTone(STARTUP_JINGLE[i].pitch, STARTUP_JINGLE[i].duration);
            if (STARTUP_JINGLE[i].delay > 0) {
                delay(STARTUP_JINGLE[i].delay);
            }
        }
        
        // Create initialization file
        File f = SPIFFS.open("/initialized.txt", "w");
        if (f) {
            f.println("initialized");
            f.close();
        }
    }
}

void SoundFxManager::playJingle() {
    for (int i = 0; i < ROVER_JINGLE_LENGTH; i++) {
        playTone(ROVER_JINGLE[i].pitch, ROVER_JINGLE[i].duration);
        if (ROVER_JINGLE[i].delay > 0) {
            delay(ROVER_JINGLE[i].delay);
        }
    }
} 