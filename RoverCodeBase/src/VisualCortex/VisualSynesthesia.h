#ifndef VISUAL_SYNESIA_H
#define VISUAL_SYNESIA_H

#include <FastLED.h>
#include "../AuditoryCortex/PitchPerception.h"

namespace AuditoryCortex 
{
    struct NoteInfo;  // Forward declaration
}

namespace VisualCortex 
{

    class VisualSynesthesia {
    public:
        // Static color arrays - moved to public and made extern
        static const CRGB BASE_8_COLORS[8];
        static const CRGB MONTH_COLORS[12][2];
        static const CRGB DAY_COLORS[7];
        static const CRGB CHROMATIC_COLORS[12][2];

        // Static methods
        static CRGB getBase8Color(uint8_t value);
        static CRGB getDayColor(uint8_t day);
        static void getMonthColors(uint8_t month, CRGB& color1, CRGB& color2);
        static void getHourColors(uint8_t hour, CRGB& color1, CRGB& color2);
        static CRGB getBatteryColor(uint8_t percentage);
        static CRGB getColorForFrequency(uint16_t freq);
        static CRGB getNoteColorBlended(const AuditoryCortex::NoteInfo& info);
        static uint16_t convertToRGB565(CRGB color);
        

        // New synesthesia-specific methods
        static CRGB blendNoteColors(const AuditoryCortex::NoteInfo& info);
        static void playNFCCardData(const char* cardData);
        
        static void playVisualChord(uint16_t baseFreq, CRGB& root, CRGB& third, CRGB& fifth);
    }; 
}

#endif // VISUAL_SYNESIA_H