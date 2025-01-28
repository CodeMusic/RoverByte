#ifndef PITCHPERCEPTION_H
#define PITCHPERCEPTION_H
#include <Arduino.h>

#include <cstdint> // For fixed-width integer types

namespace AuditoryCortex
{
    // Enum for note types
    enum class NoteType {
        WHOLE = 0,
        HALF = 1,
        QUARTER = 2,
        EIGHTH = 3,
        SIXTEENTH = 4,
        THIRTY_SECOND = 5,
        SIXTY_FOURTH = 6,
        HUNDRED_TWENTY_EIGHTH = 7,
        DOTTED_WHOLE = 8,
        DOTTED_HALF = 9,
        DOTTED_QUARTER = 10,
        DOTTED_EIGHTH = 11,
        DOTTED_SIXTEENTH = 12,
        DOTTED_THIRTY_SECOND = 13,
        DOTTED_SIXTY_FOURTH = 14,
        DOTTED_HUNDRED_TWENTY_EIGHTH = 15
    };

    enum class NoteIndex {
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
        REST = -1 // Representing a rest
    };

    // Enum for time signatures
    enum class TimeSignature {
        TIME_2_2 = 2,   // Half note gets the beat (2/2)
        TIME_3_4 = 3,   // Quarter note gets the beat (3/4)
        TIME_4_4 = 4,   // Quarter note gets the beat (4/4)
        TIME_6_8 = 8,   // Eighth note gets the beat (6/8)
        TIME_12_16 = 16 // Sixteenth note gets the beat (12/16)
    };

    // Constants for standard note durations in milliseconds (base 4/4)
    static const uint16_t WHOLE_NOTE_MS = 1000;
    static const uint16_t HALF_NOTE_MS = 500;
    static const uint16_t QUARTER_NOTE_MS = 250;
    static const uint16_t EIGHTH_NOTE_MS = 125;
    static const uint16_t SIXTEENTH_NOTE_MS = 62;
    static const uint16_t THIRTY_SECOND_NOTE_MS = 31;
    static const uint16_t SIXTY_FOURTH_NOTE_MS = 15;
    static const uint16_t HUNDRED_TWENTY_EIGHTH_NOTE_MS = 7;

    struct NoteInfo {
        NoteIndex note;      // The note index (C, C#, D, etc.)
        uint8_t octave;      // The octave number (0-8)
        NoteType type;       // The type of note (whole, half, quarter, etc.)
        bool isDotted;       // Whether the note is dotted
        bool isSharp;        // Whether the note is sharp
        
        NoteInfo() : note(NoteIndex::REST), octave(4), type(NoteType::QUARTER), isDotted(false), isSharp(false) {}
        
        NoteInfo(NoteIndex n, uint8_t o, NoteType t, bool dotted = false) 
            : note(n)
            , octave(o)
            , type(t)
            , isDotted(dotted)
            , isSharp((static_cast<int>(n) == 1) || (static_cast<int>(n) == 3) || 
                     (static_cast<int>(n) == 6) || (static_cast<int>(n) == 8) || 
                     (static_cast<int>(n) == 10)) {}
    };

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


        // Core functionality
        static NoteInfo getNoteInfo(uint16_t frequency);
        static uint16_t getStandardFrequency(uint16_t frequency);
        static const char* getNoteName(const NoteInfo& info);
        static uint16_t getNoteFrequency(const NoteInfo& info);
        
        // Helper for SoundFxManager
        static uint16_t getOctaveUp(uint16_t baseNote) { return baseNote * 2; }
        static uint16_t getOctaveAndFifthUp(uint16_t baseNote) { return baseNote * 3; }
        static uint16_t getToneDown(uint16_t baseNote) { return baseNote * 8 / 10; }
        static uint16_t getDayBaseNote(bool is4thOctave);
        static uint16_t getDayBaseNote4();
        static uint16_t getDayBaseNote5();
        static uint16_t getNoteMinus2(uint16_t baseNote);

        uint16_t getNoteDuration(NoteType note, TimeSignature timeSignature);

        static const uint16_t* getNoteFrequencies() {
            return NOTE_FREQUENCIES;
        }

        static bool isSharp(uint16_t frequency);
        static bool isFlat(uint16_t frequency);
        static bool isInTune(uint16_t frequency);
        
        // Frequency analysis methods
        static uint16_t detectPitch();
        static uint16_t getFrequencyFromNote(uint8_t note);
        static uint8_t getNoteFromFrequency(uint16_t frequency);

    private:
        static const uint16_t NOTE_FREQUENCIES[];
        static const char* NOTE_NAMES[];
        static const uint16_t FREQUENCY_TOLERANCE; // Hz tolerance for pitch detection
    }; 

}
#endif // PITCHPERCEPTION_H
