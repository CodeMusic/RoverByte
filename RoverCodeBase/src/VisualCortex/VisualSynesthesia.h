/**
 * @brief Cross-modal sensory integration and color-emotion mapping system
 * 
 * Manages synesthetic relationships between:
 * - Sound and color (chromesthetic mapping)
 * - Emotion and color (mood-visual correlation)
 * - Time and color (temporal-chromatic association)
 * - Energy and color (vitality visualization)
 * 
 * Core synesthetic functions:
 * - Musical note to color translation
 * - Emotional state visualization
 * - Temporal pattern recognition
 * - Cross-modal sensory binding
 * - Neural pathway integration
 */

#ifndef VISUAL_SYNESTHESIA_H
#define VISUAL_SYNESTHESIA_H

#include <FastLED.h>
#include "PrefrontalCortex/ProtoPerceptions.h"
#include "CorpusCallosum/SynapticPathways.h"

namespace VisualCortex 
{
    namespace PC = PrefrontalCortex;
    using namespace CorpusCallosum;
    using PC::AudioTypes::NoteInfo;
    using PC::ColorPerceptionTypes::ChromaticContext;
    using PC::ColorPerceptionTypes::EmotionalColor;
    using PC::ColorPerceptionTypes::ColorIntensity;
    using PC::ColorPerceptionTypes::ColorIndex;

    class VisualSynesthesia 
    {
    public:
        // Color perception arrays for different sensory mappings
        static const CRGB BASE_8_COLORS[8];  // Fundamental color perceptions
        static const CRGB MONTH_COLORS[12][2];  // Temporal-chromatic associations
        static const CRGB DAY_COLORS[7];  // Circadian color mappings
        static const CRGB CHROMATIC_COLORS[12][2];  // Musical-visual correlations

        // Sensory translation methods
        static CRGB getBase8Color(uint8_t cognitiveValue);
        static CRGB getDayColor(uint8_t circadianIndex);
        static void getMonthColors(uint8_t temporalIndex, CRGB& primaryPerception, CRGB& secondaryPerception);
        static void getHourColors(uint8_t circadianHour, CRGB& primaryPerception, CRGB& secondaryPerception);
        static CRGB getBatteryColor(uint8_t energyLevel);
        
        // Audio-visual synesthesia methods
        static CRGB getColorForFrequency(uint16_t frequency);
        static CRGB getNoteColorBlended(const NoteInfo& musicalPerception);
        static CRGB blendNoteColors(const NoteInfo& musicalPerception);
        
        // Cross-modal integration methods
        static uint16_t convertToRGB565(CRGB neuralColor);
        static void playNFCCardData(const char* sensoryStimulusData);
        static void playVisualChord(uint16_t fundamentalFreq, 
                                  CRGB& rootPerception, 
                                  CRGB& thirdPerception, 
                                  CRGB& fifthPerception);

        // Add new methods using the new types
        static ChromaticContext getChromaticContext(uint16_t frequency);
        static EmotionalColor getEmotionalColor(PC::RoverTypes::Expression emotion);
        static ColorIntensity calculateIntensity(uint8_t rawValue);
        static ColorIndex getColorIndex(uint8_t value);
        
        // Update existing method signatures
        static void getMonthColors(uint8_t temporalIndex, ChromaticContext& context);
        static void playVisualChord(uint16_t fundamentalFreq, 
                                   ChromaticContext& rootContext,
                                   ChromaticContext& thirdContext,
                                   ChromaticContext& fifthContext);
    };
}

#endif // VISUAL_SYNESTHESIA_H