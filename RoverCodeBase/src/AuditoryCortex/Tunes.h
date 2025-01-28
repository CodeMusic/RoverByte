#ifndef TUNES_H
#define TUNES_H

#include <Arduino.h>
#include <vector>
#include "../AuditoryCortex/PitchPerception.h"

#include "../PrefrontalCortex/utilities.h"
#include "../VisualCortex/RoverManager.h"
#include <time.h>

namespace AuditoryCortex
{

    // Tune structure
    struct Tune {
        const char* name;
        std::vector<NoteInfo> notes;       // Sequence of notes
        std::vector<uint8_t> ledAnimation; // LED animation frames
        TimeSignature timeSignature;       // Time signature
    };

    class Tunes {
    public:
        enum class TunesTypes {
            ROVERBYTE_JINGLE,
            JINGLE_BELLS,
            AULD_LANG_SYNE,
            LOVE_SONG,
            HAPPY_BIRTHDAY,
            EASTER_SONG,
            MOTHERS_SONG,
            FATHERS_SONG,
            CANADA_SONG,
            USA_SONG,
            CIVIC_SONG,
            WAKE_ME_UP_WHEN_SEPTEMBER_ENDS,
            HALLOWEEN_SONG,
            THANKSGIVING_SONG,
            CHRISTMAS_SONG
        };

        // Retrieve a tune
        static Tune getTune(TunesTypes type);
        static int getTuneLength(TunesTypes type);
        static int timeSignatureToDelay(TimeSignature timeSignature);

    private:

        // Jingle data
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

#endif