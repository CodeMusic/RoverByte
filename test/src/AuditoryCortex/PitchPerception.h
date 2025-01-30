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
