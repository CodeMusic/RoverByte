#ifndef TUNES_H
#define TUNES_H

#include "../CorpusCallosum/SynapticPathways.h"
#include <Arduino.h>
#include <vector>
#include "PitchPerception.h"
#include "../PrefrontalCortex/utilities.h"

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
        // Tune retrieval and manipulation methods
        
        /**
         * Retrieves a specific tune based on the provided type
         * @param type The type of tune to retrieve
         * @return The requested Tune structure
         */
        static Tune getTune(TunesTypes type);

        /**
         * Gets the length (number of notes) in a specific tune
         * @param type The type of tune to measure
         * @return The number of notes in the tune
         */
        static int getTuneLength(TunesTypes type);

        /**
         * Calculates the delay in milliseconds for a given time signature
         * @param timeSignature The musical time signature to calculate for
         * @return The delay in milliseconds
         */
        static int timeSignatureToDelay(TimeSignature timeSignature);

    private:
        // Predefined musical compositions
        static const Tune ROVERBYTE_JINGLE;
        static const Tune JINGLE_BELLS;
        static const Tune AULD_LANG_SYNE;
        static const Tune LOVE_SONG;
        static const Tune HAPPY_BIRTHDAY;
        static const Tune EASTER_SONG;
        static const Tune MOTHERS_SONG;
        static const Tune FATHERS_SONG;
        static const Tune CANADA_SONG;
        static const Tune USA_SONG;
        static const Tune CIVIC_SONG;
        static const Tune WAKE_ME_UP_WHEN_SEPTEMBER_ENDS;
        static const Tune HALLOWEEN_SONG;
        static const Tune THANKSGIVING_SONG;
        static const Tune CHRISTMAS_SONG;
    };
}

#endif // TUNES_H