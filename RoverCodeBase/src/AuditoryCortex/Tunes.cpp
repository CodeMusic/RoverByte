/**
 * @file Tunes.cpp
 * @brief Implementation of the Tunes class for musical composition management
 * 
 * Contains the implementation of tune retrieval, playback timing calculations,
 * and storage of predefined musical compositions.
 */

#include "../CorpusCallosum/SynapticPathways.h"
#include "PitchPerception.h"
#include "SoundFxManager.h"
#include "Arduino.h"
#include <time.h>
#include <SPIFFS.h>
#include "../PrefrontalCortex/SDManager.h"
#include "../VisualCortex/RoverViewManager.h"
#include "../VisualCortex/RoverManager.h"
#include "../VisualCortex/LEDManager.h"
#include "Tunes.h"

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
    using VC::RoverManager;
    using VC::LEDManager;

    // RoverByte's Anthem: Quantum Tails
    const Tune Tunes::ROVERBYTE_JINGLE = {
        "Quantum Tails",
        {
            {NoteIndex::NOTE_G, 4, NoteType::QUARTER},
            {NoteIndex::NOTE_A, 4, NoteType::EIGHTH},
            {NoteIndex::NOTE_B, 4, NoteType::EIGHTH},
            {NoteIndex::REST, 0, NoteType::EIGHTH},
            {NoteIndex::NOTE_C, 5, NoteType::QUARTER},
            {NoteIndex::NOTE_E, 5, NoteType::QUARTER},
            {NoteIndex::NOTE_D, 5, NoteType::EIGHTH},
            {NoteIndex::REST, 0, NoteType::QUARTER},
            {NoteIndex::NOTE_F, 5, NoteType::HALF},
            {NoteIndex::NOTE_A, 4, NoteType::EIGHTH},
            {NoteIndex::NOTE_C, 5, NoteType::QUARTER},
            {NoteIndex::NOTE_B, 4, NoteType::EIGHTH},
            {NoteIndex::NOTE_G, 4, NoteType::QUARTER},
            {NoteIndex::REST, 0, NoteType::QUARTER},
            {NoteIndex::NOTE_D, 5, NoteType::HALF},
            {NoteIndex::NOTE_G, 4, NoteType::WHOLE}
        },
        {
            0b00000001, 0b00000010, 0b00000100, 0b00001000,
            0b00010000, 0b00100000, 0b01000000, 0b10000000,
            0b11000000, 0b01100000, 0b00110000, 0b00011000,
            0b00001100, 0b00000110, 0b00000011, 0b00000001
        },
        PC::AudioTypes::TimeSignature::TIME_6_8
    };

    // Cosmic Reflections: Painted Skies
    const Tune Tunes::CHRISTMAS_SONG = {
        "Painted Skies",
        {
            {NoteIndex::NOTE_C, 4, NoteType::QUARTER},
            {NoteIndex::NOTE_G, 4, NoteType::EIGHTH},
            {NoteIndex::NOTE_A, 4, NoteType::EIGHTH},
            {NoteIndex::REST, 0, NoteType::QUARTER},
            {NoteIndex::NOTE_B, 4, NoteType::HALF},
            {NoteIndex::NOTE_D, 5, NoteType::QUARTER},
            {NoteIndex::NOTE_E, 5, NoteType::EIGHTH},
            {NoteIndex::NOTE_C, 5, NoteType::EIGHTH},
            {NoteIndex::REST, 0, NoteType::QUARTER},
            {NoteIndex::NOTE_G, 4, NoteType::QUARTER},
            {NoteIndex::NOTE_F, 4, NoteType::QUARTER},
            {NoteIndex::NOTE_E, 4, NoteType::QUARTER},
            {NoteIndex::NOTE_C, 4, NoteType::QUARTER},
            {NoteIndex::NOTE_A, 4, NoteType::HALF}
        },
        {
            0b00000001, 0b00000011, 0b00000111, 0b00001110,
            0b00011100, 0b00111000, 0b01110000, 0b11100000,
            0b11000000, 0b10000001, 0b00000001, 0b00000111,
            0b11111111, 0b00000000, 0b00000001
        },
        PC::AudioTypes::TimeSignature::TIME_4_4
    };

    // Reflections of Unity: Symphonic Threads
    const Tune Tunes::AULD_LANG_SYNE = {
        "Symphonic Threads",
        {
            {NoteIndex::NOTE_E, 4, NoteType::EIGHTH},
            {NoteIndex::NOTE_F, 4, NoteType::EIGHTH},
            {NoteIndex::NOTE_G, 4, NoteType::QUARTER},
            {NoteIndex::REST, 0, NoteType::EIGHTH},
            {NoteIndex::NOTE_A, 4, NoteType::QUARTER},
            {NoteIndex::NOTE_B, 4, NoteType::QUARTER},
            {NoteIndex::NOTE_C, 5, NoteType::HALF},
            {NoteIndex::NOTE_D, 5, NoteType::QUARTER},
            {NoteIndex::NOTE_C, 5, NoteType::QUARTER},
            {NoteIndex::NOTE_A, 4, NoteType::HALF},
            {NoteIndex::NOTE_B, 4, NoteType::QUARTER},
            {NoteIndex::REST, 0, NoteType::QUARTER},
            {NoteIndex::NOTE_C, 5, NoteType::HALF},
            {NoteIndex::NOTE_E, 5, NoteType::QUARTER}
        },
        {
            0b00000001, 0b10000001, 0b11000011, 0b11100111,
            0b11111111, 0b11100111, 0b11000011, 0b10000001,
            0b00000001, 0b10001000, 0b11111111, 0b00000000,
            0b00001100, 0b11110000
        },
        PC::AudioTypes::TimeSignature::TIME_3_4
    };

    const Tune Tunes::JINGLE_BELLS = {
        "Winter Dance",
        {
            {NoteIndex::NOTE_E, 4, NoteType::QUARTER}, // 1
            {NoteIndex::NOTE_E, 4, NoteType::QUARTER}, // 2
            {NoteIndex::NOTE_E, 4, NoteType::HALF},    // 3
            {NoteIndex::NOTE_E, 4, NoteType::QUARTER}, // 4
            {NoteIndex::NOTE_E, 4, NoteType::QUARTER}, // 5
            {NoteIndex::NOTE_E, 4, NoteType::HALF},    // 6
            {NoteIndex::NOTE_E, 4, NoteType::QUARTER}, // 7
            {NoteIndex::NOTE_G, 4, NoteType::QUARTER}, // 8
            {NoteIndex::NOTE_C, 5, NoteType::QUARTER}, // 9
            {NoteIndex::NOTE_D, 5, NoteType::QUARTER}, // 10
            {NoteIndex::NOTE_E, 5, NoteType::WHOLE},   // 11
            {NoteIndex::NOTE_F, 5, NoteType::QUARTER}, // 12
            {NoteIndex::NOTE_F, 5, NoteType::EIGHTH},  // 13
            {NoteIndex::NOTE_F, 5, NoteType::EIGHTH},  // 14
            {NoteIndex::NOTE_F, 5, NoteType::HALF},    // 15
            {NoteIndex::NOTE_E, 5, NoteType::QUARTER}, // 16
            {NoteIndex::NOTE_D, 5, NoteType::QUARTER}, // 17
            {NoteIndex::NOTE_C, 5, NoteType::HALF},    // 18
            {NoteIndex::NOTE_G, 4, NoteType::QUARTER}, // 19
            {NoteIndex::NOTE_E, 4, NoteType::QUARTER}, // 20
            {NoteIndex::REST, 0, NoteType::HALF}       // 21
        },
        {
            0b00000001, // Swirl starts with LED 0
            0b00010001, // Pair 0/4 lights up
            0b00100010, // Pair 1/5 lights up
            0b01000100, // Pair 2/6 lights up
            0b10001000, // Pair 3/7 lights up
            0b11111111, // All LEDs on
            0b00000001, // Reset to single light
            0b00110000, // LED 4 fades in, and so on
            0b00001110, // LEDs swirl inward
            0b11100111, // Symmetric ripple outward
            0b11111111, // Bright pulse for "E5 WHOLE"
            0b00010001, // Subtle shimmer (Pair 0/4)
            0b00100010, // (Pair 1/5)
            0b01000100, // (Pair 2/6)
            0b10001000, // (Pair 3/7)
            0b11111111, // Intense flash
            0b00100010, // Pair fade
            0b00010001, // Back to Pair 0/4
            0b00000001, // Single LED on LED 0
            0b00000000  // Rest (all LEDs off)
        },
        PC::AudioTypes::TimeSignature::TIME_4_4
    };

    const Tune Tunes::LOVE_SONG = {
        "Entangled Hearts",
        {
            {NoteIndex::NOTE_A, 4, NoteType::EIGHTH},  // 1
            {NoteIndex::NOTE_C, 5, NoteType::QUARTER}, // 2
            {NoteIndex::NOTE_E, 5, NoteType::QUARTER}, // 3
            {NoteIndex::NOTE_G, 5, NoteType::EIGHTH},  // 4
            {NoteIndex::NOTE_F, 5, NoteType::EIGHTH},  // 5
            {NoteIndex::NOTE_E, 5, NoteType::QUARTER}, // 6
            {NoteIndex::NOTE_D, 5, NoteType::HALF},    // 7
            {NoteIndex::REST, 0, NoteType::EIGHTH},    // 8
            {NoteIndex::NOTE_C, 5, NoteType::QUARTER}, // 9
            {NoteIndex::NOTE_A, 4, NoteType::QUARTER}, // 10
            {NoteIndex::NOTE_B, 4, NoteType::HALF},    // 11
            {NoteIndex::NOTE_C, 5, NoteType::QUARTER}, // 12
            {NoteIndex::NOTE_E, 5, NoteType::QUARTER}, // 13
            {NoteIndex::NOTE_G, 5, NoteType::EIGHTH},  // 14
            {NoteIndex::NOTE_A, 5, NoteType::HALF},    // 15
            {NoteIndex::REST, 0, NoteType::EIGHTH},    // 16
            {NoteIndex::NOTE_F, 5, NoteType::QUARTER}, // 17
            {NoteIndex::NOTE_E, 5, NoteType::EIGHTH},  // 18
            {NoteIndex::NOTE_C, 5, NoteType::EIGHTH},  // 19
            {NoteIndex::NOTE_A, 4, NoteType::QUARTER}, // 20
            {NoteIndex::NOTE_F, 4, NoteType::HALF},    // 21
            {NoteIndex::NOTE_G, 4, NoteType::QUARTER}, // 22
            {NoteIndex::NOTE_A, 4, NoteType::HALF},    // 23
            {NoteIndex::NOTE_C, 5, NoteType::WHOLE}    // 24
        },
        {
            0b00011000, // Center glow starts
            0b00111100, // Expanding heart
            0b01111110, // Full brightness
            0b11111111, // Intense pulse
            0b01111110, // Contracting heart
            0b00111100, // Shrinking
            0b00011000, // Back to subtle glow
            0b00000000, // Rest: All off
            0b00011000, // Gentle pulse restart
            0b00111100, // Building intensity
            0b01111110, // Heartbeat synchronization
            0b11111111, // Full pulse
            0b01111110, // Retreat to calm
            0b00111100, // Gentle glow
            0b00011000, // Slow pulse
            0b00000000, // Rest: Dark pause
            0b00011000, // Restart light heartbeat
            0b00111100, // Grow again
            0b01111110, // Peak pulse
            0b11111111, // Full brightness
            0b01111110, // Dim down
            0b00111100, // Fade
            0b00011000, // Subtle light
            0b00000000  // Final rest
        },
        PC::AudioTypes::TimeSignature::TIME_6_8
    };

    const Tune Tunes::HAPPY_BIRTHDAY = {
        "Celebration Sparks",
        {
            {NoteIndex::NOTE_C, 4, NoteType::QUARTER},  // 1
            {NoteIndex::NOTE_C, 4, NoteType::EIGHTH},  // 2
            {NoteIndex::NOTE_D, 4, NoteType::QUARTER}, // 3
            {NoteIndex::NOTE_C, 4, NoteType::EIGHTH},  // 4
            {NoteIndex::NOTE_F, 4, NoteType::HALF},    // 5
            {NoteIndex::NOTE_E, 4, NoteType::QUARTER}, // 6
            {NoteIndex::NOTE_C, 4, NoteType::QUARTER}, // 7
            {NoteIndex::NOTE_C, 4, NoteType::QUARTER}, // 8
            {NoteIndex::NOTE_D, 4, NoteType::QUARTER}, // 9
            {NoteIndex::NOTE_C, 4, NoteType::EIGHTH},  // 10
            {NoteIndex::NOTE_G, 4, NoteType::HALF},    // 11
            {NoteIndex::NOTE_F, 4, NoteType::HALF},    // 12
            {NoteIndex::NOTE_C, 4, NoteType::QUARTER}, // 13
            {NoteIndex::NOTE_C, 4, NoteType::EIGHTH},  // 14
            {NoteIndex::NOTE_D, 4, NoteType::QUARTER}, // 15
            {NoteIndex::NOTE_C, 4, NoteType::EIGHTH},  // 16
            {NoteIndex::NOTE_C, 4, NoteType::HALF},    // 17
            {NoteIndex::NOTE_C, 4, NoteType::QUARTER}, // 18
            {NoteIndex::REST, 0, NoteType::HALF}       // 19
        },
        {
            0b00011000, // Heartbeat glow
            0b00111100, // Expand
            0b01111110, // Full burst
            0b11111111, // Firework explosion
            0b01111110, // Retract
            0b00111100, // Calm fade
            0b00011000, // Gentle light
            0b00111100, // Glow grows
            0b11111111, // Bright explosion
            0b01111110, // Fade again
            0b11111111, // Climax burst
            0b00011000, // Calm glow
            0b00001100, // Shrink light inward
            0b00111100, // Bright again
            0b01111110, // Another celebration burst
            0b11111111, // Flash out
            0b00111100, // Fade to dim
            0b00000000  // Rest
        },
        PC::AudioTypes::TimeSignature::TIME_3_4
    };

    const Tune Tunes::EASTER_SONG = {
        "Spring Awakening",
        {
            {NoteIndex::NOTE_E, 4, NoteType::QUARTER},  // 1
            {NoteIndex::NOTE_G, 4, NoteType::QUARTER},  // 2
            {NoteIndex::NOTE_A, 4, NoteType::EIGHTH},   // 3
            {NoteIndex::NOTE_B, 4, NoteType::EIGHTH},   // 4
            {NoteIndex::NOTE_C, 5, NoteType::HALF},     // 5
            {NoteIndex::NOTE_B, 4, NoteType::QUARTER},  // 6
            {NoteIndex::NOTE_A, 4, NoteType::QUARTER},  // 7
            {NoteIndex::REST, 0, NoteType::EIGHTH},     // 8
            {NoteIndex::NOTE_G, 4, NoteType::QUARTER},  // 9
            {NoteIndex::NOTE_E, 4, NoteType::QUARTER},  // 10
            {NoteIndex::NOTE_F, 4, NoteType::QUARTER},  // 11
            {NoteIndex::NOTE_G, 4, NoteType::HALF},     // 12
            {NoteIndex::NOTE_A, 4, NoteType::QUARTER},  // 13
            {NoteIndex::NOTE_B, 4, NoteType::QUARTER},  // 14
            {NoteIndex::NOTE_C, 5, NoteType::HALF},     // 15
            {NoteIndex::NOTE_D, 5, NoteType::QUARTER},  // 16
            {NoteIndex::NOTE_E, 5, NoteType::WHOLE}     // 17
        },
        {
            0b00000001, // Small bud (LED 0)
            0b00000011, // Bud grows outward (0/1)
            0b00000111, // Expanding (0/1/2)
            0b00001110, // More bloom (1/2/3)
            0b00011111, // Full bloom
            0b00111111, // Brighter
            0b01111110, // Calm retreat
            0b11111111, // Full light
            0b00111110, // Dim down
            0b00011100, // Almost gone
            0b00001100, // Fading bloom
            0b00000100, // Just a petal remains
            0b00000000, // Rest
            0b00000001, // Restart small bud
            0b00001111, // Faster bloom
            0b11111111, // Full spring
            0b00000000  // Rest
        },
        PC::AudioTypes::TimeSignature::TIME_6_8
    };

    const Tune Tunes::MOTHERS_SONG = {
        "Heart of the Home",
        {
            {NoteIndex::NOTE_C, 4, NoteType::HALF},     // 1
            {NoteIndex::NOTE_E, 4, NoteType::QUARTER},  // 2
            {NoteIndex::NOTE_G, 4, NoteType::QUARTER},  // 3
            {NoteIndex::NOTE_A, 4, NoteType::HALF},     // 4
            {NoteIndex::NOTE_F, 4, NoteType::QUARTER},  // 5
            {NoteIndex::NOTE_D, 4, NoteType::HALF},     // 6
            {NoteIndex::NOTE_E, 4, NoteType::QUARTER},  // 7
            {NoteIndex::NOTE_C, 4, NoteType::HALF},     // 8
            {NoteIndex::NOTE_G, 4, NoteType::QUARTER},  // 9
            {NoteIndex::REST, 0, NoteType::HALF}        // 10
        },
        {
            0b00110000, // Gentle pulse starts
            0b01111000, // Glow spreads
            0b11111100, // Full warmth
            0b11111111, // Peak heartbeat
            0b01111000, // Retract
            0b00110000, // Heartbeat fades
            0b00011000, // Smaller pulse
            0b00001100, // Retreat further
            0b00000100, // Dim light
            0b00000000  // Rest
        },
        PC::AudioTypes::TimeSignature::TIME_4_4
    };

    const Tune Tunes::FATHERS_SONG = {
        "Pillars of Strength",
        {
            {NoteIndex::NOTE_C, 4, NoteType::HALF},     // 1
            {NoteIndex::NOTE_G, 4, NoteType::QUARTER},  // 2
            {NoteIndex::NOTE_A, 4, NoteType::QUARTER},  // 3
            {NoteIndex::NOTE_F, 4, NoteType::HALF},     // 4
            {NoteIndex::NOTE_D, 4, NoteType::QUARTER},  // 5
            {NoteIndex::NOTE_E, 4, NoteType::QUARTER},  // 6
            {NoteIndex::NOTE_C, 4, NoteType::HALF},     // 7
            {NoteIndex::NOTE_G, 4, NoteType::QUARTER},  // 8
            {NoteIndex::NOTE_A, 4, NoteType::QUARTER},  // 9
            {NoteIndex::NOTE_C, 4, NoteType::WHOLE}     // 10
        },
        {
            0b00010001, // Solid base (0/4)
            0b00100010, // Pairing (1/5)
            0b01000100, // Strength builds (2/6)
            0b10001000, // Broad foundation (3/7)
            0b11111111, // Stability across all LEDs
            0b01000100, // Retreat to pillars (2/6)
            0b00100010, // Refocus (1/5)
            0b00010001, // Narrowed strength (0/4)
            0b11111111, // Full stability pulse
            0b00000000  // Rest
        },
        PC::AudioTypes::TimeSignature::TIME_4_4
    };

    const Tune Tunes::CANADA_SONG = {
        "Northern Lights",
        {
            {NoteIndex::NOTE_C, 5, NoteType::QUARTER},  // 1
            {NoteIndex::NOTE_E, 5, NoteType::QUARTER},  // 2
            {NoteIndex::NOTE_G, 5, NoteType::QUARTER},  // 3
            {NoteIndex::NOTE_F, 5, NoteType::QUARTER},  // 4
            {NoteIndex::NOTE_A, 4, NoteType::HALF},     // 5
            {NoteIndex::NOTE_B, 4, NoteType::QUARTER},  // 6
            {NoteIndex::NOTE_C, 5, NoteType::QUARTER},  // 7
            {NoteIndex::NOTE_D, 5, NoteType::HALF},     // 8
            {NoteIndex::NOTE_E, 5, NoteType::WHOLE}     // 9
        },
        {
            0b00000001, // Spark at LED 0
            0b00000011, // Wave grows (0/1)
            0b00000111, // Expands further (0/1/2)
            0b00001111, // Adding (0/1/2/3)
            0b11111111, // Aurora peak across LEDs
            0b00011111, // Retreat begins (3/2/1)
            0b00001111, // Light condenses (2/1/0)
            0b00000011, // Narrow glow
            0b00000001  // Fade to rest
        },
        PC::AudioTypes::TimeSignature::TIME_6_8
    };

    const Tune Tunes::USA_SONG = {
        "Stars and Stripes",
        {
            {NoteIndex::NOTE_G, 4, NoteType::QUARTER},  // 1
            {NoteIndex::NOTE_C, 5, NoteType::QUARTER},  // 2
            {NoteIndex::NOTE_D, 5, NoteType::QUARTER},  // 3
            {NoteIndex::NOTE_E, 5, NoteType::HALF},     // 4
            {NoteIndex::NOTE_F, 5, NoteType::QUARTER},  // 5
            {NoteIndex::NOTE_E, 5, NoteType::HALF},     // 6
            {NoteIndex::NOTE_D, 5, NoteType::QUARTER},  // 7
            {NoteIndex::NOTE_C, 5, NoteType::WHOLE}     // 8
        },
        {
            0b10101010, // Stars (pairs 0/4, 1/5, 2/6, 3/7 alternating)
            0b01010101, // Stripes flip
            0b10101010, // Alternating again
            0b11111111, // Bold full brightness
            0b10000001, // Ends with outer LEDs lit
            0b01000010, // Central symmetry grows
            0b00111100, // Flag waves inward
            0b11111111  // Bright pulse of unity
        },
        PC::AudioTypes::TimeSignature::TIME_4_4
    };

    const Tune Tunes::CIVIC_SONG = {
        "Unity in Motion",
        {
            {NoteIndex::NOTE_D, 4, NoteType::QUARTER},  // 1
            {NoteIndex::NOTE_E, 4, NoteType::QUARTER},  // 2
            {NoteIndex::NOTE_F, 4, NoteType::HALF},     // 3
            {NoteIndex::NOTE_G, 4, NoteType::QUARTER},  // 4
            {NoteIndex::NOTE_A, 4, NoteType::HALF},     // 5
            {NoteIndex::NOTE_B, 4, NoteType::QUARTER},  // 6
            {NoteIndex::NOTE_G, 4, NoteType::WHOLE}     // 7
        },
        {
            0b00000001, // LED 0 starts
            0b00000011, // Pair 0/1 grow
            0b00000111, // Circle spreads outward
            0b00001111, // Halfway expansion
            0b11111111, // Full brightness circle
            0b11110000, // Retreat backward
            0b00000000  // Rest
        },
        PC::AudioTypes::TimeSignature::TIME_4_4
    };

    const Tune Tunes::WAKE_ME_UP_WHEN_SEPTEMBER_ENDS = {
        "Autumn's Farewell",
        {
            {NoteIndex::NOTE_E, 4, NoteType::QUARTER},
            {NoteIndex::NOTE_G, 4, NoteType::QUARTER},
            {NoteIndex::NOTE_F, 4, NoteType::HALF},
            {NoteIndex::NOTE_E, 4, NoteType::QUARTER},
            {NoteIndex::REST, 0, NoteType::QUARTER},
            {NoteIndex::NOTE_D, 4, NoteType::QUARTER},
            {NoteIndex::NOTE_C, 4, NoteType::HALF},
            {NoteIndex::NOTE_E, 4, NoteType::QUARTER},
            {NoteIndex::NOTE_F, 4, NoteType::QUARTER},
            {NoteIndex::NOTE_G, 4, NoteType::HALF},
            {NoteIndex::REST, 0, NoteType::EIGHTH},
            {NoteIndex::NOTE_A, 4, NoteType::QUARTER},
            {NoteIndex::NOTE_G, 4, NoteType::QUARTER},
            {NoteIndex::NOTE_F, 4, NoteType::HALF},
            {NoteIndex::NOTE_C, 4, NoteType::QUARTER},
            {NoteIndex::NOTE_E, 4, NoteType::HALF},
            {NoteIndex::NOTE_A, 4, NoteType::QUARTER},
            {NoteIndex::NOTE_F, 4, NoteType::QUARTER},
            {NoteIndex::NOTE_G, 4, NoteType::HALF},
            {NoteIndex::REST, 0, NoteType::QUARTER},
            {NoteIndex::NOTE_D, 4, NoteType::QUARTER},
            {NoteIndex::NOTE_C, 4, NoteType::WHOLE},
            {NoteIndex::NOTE_A, 4, NoteType::HALF},
            {NoteIndex::NOTE_F, 4, NoteType::QUARTER},
            {NoteIndex::NOTE_G, 4, NoteType::QUARTER},
            {NoteIndex::NOTE_E, 4, NoteType::HALF},
            {NoteIndex::NOTE_D, 4, NoteType::QUARTER},
            {NoteIndex::NOTE_C, 4, NoteType::WHOLE},
            {NoteIndex::REST, 0, NoteType::HALF},
            {NoteIndex::NOTE_A, 4, NoteType::WHOLE}
        },
        {
            0b11111111,
            0b01111110,
            0b00111100,
            0b00011000,
            0b00000000,
            0b00010000,
            0b00111000,
            0b00011100,
            0b00001100,
            0b00000011,
            0b00000000,
            0b11110000,
            0b01111000,
            0b00011100,
            0b00001110,
            0b00000110,
            0b00000011,
            0b00000000,
            0b11111111,
            0b00111000,
            0b01111110,
            0b11111111,
            0b01111110,
            0b00011000,
            0b00000001,
            0b00000000,
            0b11111111,
            0b00000000
        },
        PC::AudioTypes::TimeSignature::TIME_4_4
    };

    const Tune Tunes::HALLOWEEN_SONG = {
        "Phantom Waltz",
        {
            {NoteIndex::NOTE_E, 5, NoteType::QUARTER},  // 1
            {NoteIndex::NOTE_D, 5, NoteType::QUARTER},  // 2
            {NoteIndex::NOTE_C, 5, NoteType::HALF},     // 3
            {NoteIndex::REST, 0, NoteType::QUARTER},    // 4
            {NoteIndex::NOTE_B, 4, NoteType::QUARTER},  // 5
            {NoteIndex::NOTE_G, 4, NoteType::EIGHTH},   // 6
            {NoteIndex::NOTE_A, 4, NoteType::EIGHTH},   // 7
            {NoteIndex::NOTE_C, 5, NoteType::QUARTER},  // 8
            {NoteIndex::NOTE_E, 5, NoteType::HALF},     // 9
            {NoteIndex::REST, 0, NoteType::HALF},       // 10
            {NoteIndex::NOTE_D, 5, NoteType::QUARTER},  // 11
            {NoteIndex::NOTE_C, 5, NoteType::QUARTER},  // 12
            {NoteIndex::NOTE_B, 4, NoteType::HALF},     // 13
            {NoteIndex::NOTE_G, 4, NoteType::QUARTER},  // 14
            {NoteIndex::NOTE_A, 4, NoteType::HALF},     // 15
            {NoteIndex::NOTE_C, 5, NoteType::WHOLE}     // 16
        },
        {
            0b00000001, // Ghostly flicker (LED 0)
            0b00000100, // LED 2 lights up
            0b00001000, // LED 3 flickers on
            0b10000000, // LED 7 appears suddenly
            0b11000000, // LEDs 6 and 7 shimmer together
            0b01100000, // LEDs 5 and 6 flicker
            0b00011000, // Focus on LEDs 3 and 4
            0b00100100, // LEDs 2 and 5 light up
            0b00010001, // Symmetry between LEDs 0 and 4
            0b11111111, // All LEDs brighten
            0b01111110, // Gradual retreat inward
            0b00011000, // Back to subtle glow
            0b00000001, // Single flicker at LED 0
            0b00000000, // Rest
            0b00111100, // Intense middle flash
            0b11111111  // Final full brightness
        },
        PC::AudioTypes::TimeSignature::TIME_3_4
    };

    const Tune Tunes::THANKSGIVING_SONG = {
        "Harvest Hymn",
        {
            {NoteIndex::NOTE_C, 4, NoteType::EIGHTH},   // 1
            {NoteIndex::NOTE_E, 4, NoteType::QUARTER},  // 2
            {NoteIndex::NOTE_G, 4, NoteType::HALF},     // 3
            {NoteIndex::NOTE_A, 4, NoteType::QUARTER},  // 4
            {NoteIndex::NOTE_F, 4, NoteType::QUARTER},  // 5
            {NoteIndex::NOTE_D, 4, NoteType::EIGHTH},   // 6
            {NoteIndex::NOTE_B, 4, NoteType::EIGHTH},   // 7
            {NoteIndex::NOTE_C, 5, NoteType::HALF},     // 8
            {NoteIndex::NOTE_D, 5, NoteType::QUARTER},  // 9
            {NoteIndex::NOTE_E, 5, NoteType::QUARTER},  // 10
            {NoteIndex::NOTE_F, 5, NoteType::HALF},     // 11
            {NoteIndex::NOTE_E, 5, NoteType::QUARTER},  // 12
            {NoteIndex::NOTE_C, 5, NoteType::QUARTER},  // 13
            {NoteIndex::NOTE_A, 4, NoteType::HALF},     // 14
            {NoteIndex::NOTE_G, 4, NoteType::WHOLE}     // 15
        },
        {
            0b00000001, // Seed planted (LED 0)
            0b00000011, // Growth outward (LEDs 0/1)
            0b00000111, // Small sprout (LEDs 0/1/2)
            0b00001110, // Bloom begins (LEDs 1/2/3)
            0b00011100, // Expands toward center
            0b00111100, // LEDs 2/3/4 shine brightly
            0b01111110, // Almost full harvest
            0b11111111, // Abundance achieved
            0b01111110, // Gradual fade
            0b00111100, // LED 3 and surroundings dim
            0b00011100, // LED 3 glows faintly
            0b00001110, // Light shifts inward
            0b00000110, // LED 2 dims gently
            0b00000011, // Light narrows
            0b00000001  // Final flicker
        },
        PC::AudioTypes::TimeSignature::TIME_6_8
    };

    int Tunes::getTuneLength(TunesTypes type) 
    {
        switch (type) 
        {
            case TunesTypes::ROVERBYTE_JINGLE: 
                return ROVERBYTE_JINGLE.notes.size();
            case TunesTypes::CHRISTMAS_SONG: 
                return CHRISTMAS_SONG.notes.size();
            case TunesTypes::AULD_LANG_SYNE: 
                return AULD_LANG_SYNE.notes.size();
            case TunesTypes::JINGLE_BELLS: 
                return JINGLE_BELLS.notes.size();
            case TunesTypes::LOVE_SONG: 
                return LOVE_SONG.notes.size();
            case TunesTypes::FATHERS_SONG: 
                return FATHERS_SONG.notes.size();
            case TunesTypes::CANADA_SONG: 
                return CANADA_SONG.notes.size();
            case TunesTypes::USA_SONG: 
                return USA_SONG.notes.size();
            case TunesTypes::CIVIC_SONG: 
                return CIVIC_SONG.notes.size();
            case TunesTypes::WAKE_ME_UP_WHEN_SEPTEMBER_ENDS: 
                return WAKE_ME_UP_WHEN_SEPTEMBER_ENDS.notes.size();
            case TunesTypes::HALLOWEEN_SONG: 
                return HALLOWEEN_SONG.notes.size();
            case TunesTypes::THANKSGIVING_SONG: 
                return THANKSGIVING_SONG.notes.size();
            default: 
                return ROVERBYTE_JINGLE.notes.size(); // Fallback
        }
    }

    // Get tune based on the type
    Tune Tunes::getTune(TunesTypes type) 
    {
        switch (type) 
        {
            case TunesTypes::CHRISTMAS_SONG: 
                return CHRISTMAS_SONG;
            case TunesTypes::AULD_LANG_SYNE: 
                return AULD_LANG_SYNE;
            case TunesTypes::JINGLE_BELLS: 
                return JINGLE_BELLS;
            case TunesTypes::LOVE_SONG: 
                return LOVE_SONG;
            case TunesTypes::FATHERS_SONG: 
                return FATHERS_SONG;
            case TunesTypes::CANADA_SONG: 
                return CANADA_SONG;
            case TunesTypes::USA_SONG: 
                return USA_SONG;
            case TunesTypes::CIVIC_SONG: 
                return CIVIC_SONG;
            case TunesTypes::WAKE_ME_UP_WHEN_SEPTEMBER_ENDS: 
                return WAKE_ME_UP_WHEN_SEPTEMBER_ENDS;
            case TunesTypes::HALLOWEEN_SONG: 
                return HALLOWEEN_SONG;
            case TunesTypes::THANKSGIVING_SONG: 
                return THANKSGIVING_SONG;
            case TunesTypes::ROVERBYTE_JINGLE:
                return ROVERBYTE_JINGLE;
            default: 
                return ROVERBYTE_JINGLE; // Fallback
        }
    }

    int Tunes::timeSignatureToDelay(TimeSignature timeSignature) 
    {
        const int DEFAULT_TEMPO_BPM = 120; // Default tempo (beats per minute)
        const int MILLISECONDS_PER_MINUTE = 60000;

        // Calculate the duration of one beat in milliseconds
        int beatDurationMs = MILLISECONDS_PER_MINUTE / DEFAULT_TEMPO_BPM;

        switch (timeSignature) 
        {
            case TimeSignature::TIME_2_2:
                // Half note gets the beat, so the delay is 2x beat duration
                return beatDurationMs * 2;

            case TimeSignature::TIME_4_4:
                // Quarter note gets the beat, delay matches beat duration
                return beatDurationMs;

            case TimeSignature::TIME_6_8:
                // Eighth note gets the beat, so divide beat duration by 2
                return beatDurationMs / 2;

            case TimeSignature::TIME_12_16:
                // Sixteenth note gets the beat, so divide beat duration by 4
                return beatDurationMs / 4;

            default:
                // Default to quarter note delay for unrecognized time signatures
                return beatDurationMs;
        }
    }
}