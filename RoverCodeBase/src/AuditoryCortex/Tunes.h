/**
 * @brief Tunes manages the musical composition library and melody processing
 * 
 * This class handles the storage and retrieval of predefined musical compositions,
 * providing functionality for:
 * - Storing predefined musical compositions and jingles
 * - Converting musical time signatures to playback timings
 * - Managing tune retrieval and length calculations
 * - Supporting cross-modal audio-visual experiences
 */

#ifndef TUNES_H
#define TUNES_H

#include "CorpusCallosum/SynapticPathways.h"
#include <Arduino.h>
#include <vector>
#include "PitchPerception.h"
#include "PrefrontalCortex/Utilities.h"

namespace AuditoryCortex
{
    using namespace CorpusCallosum;
    using PC::AudioTypes::NoteInfo;
    using PC::AudioTypes::TimeSignature;
    using PC::AudioTypes::NoteIndex;
    using PC::AudioTypes::NoteType;
    using PC::AudioTypes::Tune;
    using PC::AudioTypes::TunesTypes;
    using PC::Utilities;

    class Tunes 
    {
    public:
        /**
         * @brief Retrieves a specific tune based on the provided type
         * @param type The type of tune to retrieve
         * @return The requested Tune structure containing the musical composition
         */
        static Tune getTune(TunesTypes type);

        /**
         * @brief Gets the length (number of notes) in a specific tune
         * @param type The type of tune to measure
         * @return The number of notes in the tune
         */
        static int getTuneLength(TunesTypes type);

        /**
         * @brief Calculates the delay in milliseconds for a given time signature
         * @param timeSignature The musical time signature to calculate for
         * @return The delay in milliseconds between notes
         */
        static int timeSignatureToDelay(TimeSignature timeSignature);

    private:
        /**
         * @brief Predefined musical compositions for various occasions and moods
         * Each tune is stored as a constant Tune structure containing the melody
         */
        static const Tune ROVERBYTE_JINGLE;      // Startup/identity jingle
        static const Tune JINGLE_BELLS;          // Holiday tune
        static const Tune AULD_LANG_SYNE;        // New Year's celebration
        static const Tune LOVE_SONG;             // Emotional expression
        static const Tune HAPPY_BIRTHDAY;        // Celebration tune
        static const Tune EASTER_SONG;           // Seasonal celebration
        static const Tune MOTHERS_SONG;          // Family celebration
        static const Tune FATHERS_SONG;          // Family celebration
        static const Tune CANADA_SONG;           // National celebration
        static const Tune USA_SONG;              // National celebration
        static const Tune CIVIC_SONG;            // Community celebration
        static const Tune WAKE_ME_UP_WHEN_SEPTEMBER_ENDS;  // Melancholic tune
        static const Tune HALLOWEEN_SONG;        // Seasonal celebration
        static const Tune THANKSGIVING_SONG;     // Holiday celebration
        static const Tune CHRISTMAS_SONG;        // Holiday celebration
    };
}

#endif // TUNES_H