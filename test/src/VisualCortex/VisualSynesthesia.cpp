#include "VisualSynesthesia.h"
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

}