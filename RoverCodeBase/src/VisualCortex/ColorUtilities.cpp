#include "ColorUtilities.h"

// Define the static arrays
const CRGB ColorUtilities::BASE_8_COLORS[] = {
    CRGB::Black,          // 0 = Off
    CRGB(255, 0, 0),     // 1 = Pure Red
    CRGB(255, 140, 0),   // 2 = Orange (increased from 100 to 140)
    CRGB::Yellow,        // 3 = Yellow
    CRGB::Green,         // 4 = Green
    CRGB::Blue,          // 5 = Blue
    CRGB(75, 0, 130),    // 6 = Indigo (adjusted to match standard indigo)
    CRGB(148, 0, 211)    // 7 = Violet (using standard violet)
};

const CRGB ColorUtilities::MONTH_COLORS[][2] = {
    {CRGB(255, 0, 0), CRGB(255, 0, 0)},      // January
    {CRGB(255, 0, 0), CRGB(255, 140, 0)},    // February (adjusted orange)
    {CRGB(255, 140, 0), CRGB(255, 140, 0)},  // March (adjusted orange)
    {CRGB(255, 140, 0), CRGB::Yellow},       // April (adjusted orange)
    {CRGB::Yellow, CRGB::Yellow},            // May
    {CRGB::Green, CRGB::Green},              // June
    {CRGB::Green, CRGB::Blue},               // July
    {CRGB::Blue, CRGB::Blue},                // August
    {CRGB::Blue, CRGB(75, 0, 130)},          // September (adjusted indigo)
    {CRGB(75, 0, 130), CRGB(75, 0, 130)},    // October (adjusted indigo)
    {CRGB(75, 0, 130), CRGB(148, 0, 211)},   // November (adjusted violet)
    {CRGB(148, 0, 211), CRGB(148, 0, 211)}   // December (adjusted violet)
};

const CRGB ColorUtilities::DAY_COLORS[] = {
    CRGB::Red,                  // Sunday
    CRGB(255, 140, 0),         // Monday (adjusted orange)
    CRGB::Yellow,              // Tuesday
    CRGB::Green,               // Wednesday
    CRGB::Blue,                // Thursday
    CRGB(75, 0, 130),          // Friday (adjusted indigo)
    CRGB(148, 0, 211)          // Saturday (adjusted violet)
};

const CRGB ColorUtilities::CHROMATIC_COLORS[][2] = {
    {CRGB::Red, CRGB::Red},           // C
    {CRGB::Red, CRGB(255, 140, 0)},   // C#/Db
    {CRGB(255, 140, 0), CRGB::Yellow}, // D
    {CRGB::Yellow, CRGB::Yellow},      // D#/Eb
    {CRGB::Yellow, CRGB::Green},       // E
    {CRGB::Green, CRGB::Green},        // F
    {CRGB::Green, CRGB::Blue},         // F#/Gb
    {CRGB::Blue, CRGB::Blue},          // G
    {CRGB::Blue, CRGB(75, 0, 130)},    // G#/Ab
    {CRGB(75, 0, 130), CRGB(75, 0, 130)}, // A
    {CRGB(75, 0, 130), CRGB(148, 0, 211)}, // A#/Bb
    {CRGB(148, 0, 211), CRGB(148, 0, 211)} // B
};

CRGB ColorUtilities::getBase8Color(uint8_t value) {
    return BASE_8_COLORS[value % 8];
}

CRGB ColorUtilities::getDayColor(uint8_t day) {
    return DAY_COLORS[(day - 1) % 7];
}

void ColorUtilities::getMonthColors(uint8_t month, CRGB& color1, CRGB& color2) {
    month = (month - 1) % 12;
    color1 = MONTH_COLORS[month][0];
    color2 = MONTH_COLORS[month][1];
}

void ColorUtilities::getHourColors(uint8_t hour, CRGB& color1, CRGB& color2) {
    hour = (hour - 1) % 12;
    color1 = CHROMATIC_COLORS[hour][0];
    color2 = CHROMATIC_COLORS[hour][1];
}

CRGB ColorUtilities::getBatteryColor(uint8_t percentage) {
    if (percentage > 75) {
        return CRGB::Green;
    } else if (percentage > 50) {
        return CRGB::Yellow;
    } else if (percentage > 25) {
        return CRGB(255, 140, 0);  // Orange
    } else {
        return CRGB::Red;
    }
}

CRGB ColorUtilities::getColorForFrequency(uint16_t freq) {
    if (freq >= NOTE_C5) return CRGB::Red;
    else if (freq >= NOTE_B4) return CRGB(148, 0, 211);  // Violet
    else if (freq >= NOTE_A4) return CRGB(75, 0, 130);   // Indigo
    else if (freq >= NOTE_G4) return CRGB::Blue;
    else if (freq >= NOTE_F4) return CRGB::Green;
    else if (freq >= NOTE_E4) return CRGB::Yellow;
    else if (freq >= NOTE_D4) return CRGB(255, 140, 0);  // Orange
    else return CRGB::Red;
}

uint16_t ColorUtilities::convertToRGB565(CRGB color) {
    return ((color.r & 0xF8) << 8) | ((color.g & 0xFC) << 3) | (color.b >> 3);
} 