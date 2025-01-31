#include "ProtoPerceptions.h"

namespace PrefrontalCortex 
{
    namespace ColorPerceptionTypes 
    {
        const CRGB BASE_8_COLORS[8] = {
            CRGB::Black,          // 0 = Off/Unconscious
            CRGB(255, 0, 0),      // 1 = Pure Red/Primal
            CRGB(255, 140, 0),    // 2 = Orange/Creative
            CRGB::Yellow,         // 3 = Yellow/Intellectual
            CRGB::Green,          // 4 = Green/Emotional
            CRGB::Blue,           // 5 = Blue/Intuitive
            CRGB(75, 0, 130),     // 6 = Indigo/Spiritual
            CRGB(148, 0, 211)     // 7 = Violet/Transcendent
        };

        const CRGB MONTH_COLORS[12][2] = {
            {CRGB(255, 0, 0), CRGB(255, 0, 0)},       // January   - Red/Red
            {CRGB(255, 0, 0), CRGB(255, 140, 0)},     // February  - Red/Orange
            {CRGB(255, 140, 0), CRGB(255, 140, 0)},   // March     - Orange/Orange
            {CRGB(255, 140, 0), CRGB::Yellow},        // April     - Orange/Yellow
            {CRGB::Yellow, CRGB::Yellow},             // May       - Yellow/Yellow
            {CRGB::Green, CRGB::Green},               // June      - Green/Green
            {CRGB::Green, CRGB::Blue},                // July      - Green/Blue
            {CRGB::Blue, CRGB::Blue},                 // August    - Blue/Blue
            {CRGB::Blue, CRGB(75, 0, 130)},           // September - Blue/Indigo
            {CRGB(75, 0, 130), CRGB(75, 0, 130)},     // October   - Indigo/Indigo
            {CRGB(75, 0, 130), CRGB(148, 0, 211)},    // November  - Indigo/Violet
            {CRGB(148, 0, 211), CRGB(148, 0, 211)}    // December  - Violet/Violet
        };

        const CRGB DAY_COLORS[7] = {
            CRGB::Red,           // Sunday
            CRGB(255, 140, 0),   // Monday    - Orange
            CRGB::Yellow,        // Tuesday
            CRGB::Green,         // Wednesday
            CRGB::Blue,          // Thursday
            CRGB(75, 0, 130),    // Friday    - Indigo
            CRGB(148, 0, 211)    // Saturday  - Violet
        };

        const CRGB CHROMATIC_COLORS[12][2] = {
            {CRGB::Red, CRGB::Red},                   // C
            {CRGB::Red, CRGB(255, 140, 0)},          // C#/Db
            {CRGB(255, 140, 0), CRGB::Yellow},       // D
            {CRGB::Yellow, CRGB::Yellow},            // D#/Eb
            {CRGB::Yellow, CRGB::Green},             // E
            {CRGB::Green, CRGB::Green},              // F
            {CRGB::Green, CRGB::Blue},               // F#/Gb
            {CRGB::Blue, CRGB::Blue},                // G
            {CRGB::Blue, CRGB(75, 0, 130)},          // G#/Ab
            {CRGB(75, 0, 130), CRGB::Blue},          // A
            {CRGB(75, 0, 130), CRGB(148, 0, 211)},   // A#/Bb
            {CRGB(148, 0, 211), CRGB(148, 0, 211)}   // B
        };
    }
} 