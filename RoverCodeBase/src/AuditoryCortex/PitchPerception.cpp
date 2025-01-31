/**
 * @file PitchPerception.cpp
 * @brief Implementation of the PitchPerception class for auditory-cognitive processing
 */

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
}