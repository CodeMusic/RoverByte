#include "RoverDisplay.h"
#include "epd_driver.h"
#include <string.h>

// -----------------------------------------------------------------------------
// SOFTWARE ROTATION (90° CCW)
// Because epd_set_rotate(...) is missing in your library, we manually rotate.
//
// We assume a 960×540 “default” hardware orientation. After rotating 90° CCW,
// the visible area becomes 540 wide × 960 tall. If your actual display size
// differs, adjust EPD_WIDTH/EPD_HEIGHT accordingly.
//
// (x, y) → rotatedX = y
//          rotatedY = (EPD_WIDTH - 1 - x)
// -----------------------------------------------------------------------------
#define EPD_WIDTH  960
#define EPD_HEIGHT 540

static inline void drawPixelRot90CCW(int x, int y, uint8_t color, uint8_t* fb) {
    int nx = y;
    int ny = EPD_WIDTH - 1 - x; 
    epd_draw_pixel(nx, ny, color, fb);
}

// A simple Bresenham line drawer for small lines. 
static void drawLineRot90CCW(int x0, int y0, int x1, int y1, uint8_t color, uint8_t* fb) {
    // Bresenham's algorithm
    int dx = (x1 >= x0) ? (x1 - x0) : (x0 - x1);
    int sx = (x0 < x1) ? 1 : -1;
    int dy = -( (y1 >= y0) ? (y1 - y0) : (y0 - y1));
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx + dy;

    while(true) {
        drawPixelRot90CCW(x0, y0, color, fb);
        if(x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if(e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if(e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}

// Draw just the outline of a triangle by connecting its three points.
static void drawTriangleRot90CCW(int x1, int y1, int x2, int y2, int x3, int y3,
                                 uint8_t color, uint8_t* fb) {
    drawLineRot90CCW(x1, y1, x2, y2, color, fb);
    drawLineRot90CCW(x2, y2, x3, y3, color, fb);
    drawLineRot90CCW(x3, y3, x1, y1, color, fb);
}

static void fillRectRot90CCW(int x0, int y0, int w, int h, uint8_t color, uint8_t* fb) {
    for(int xx = x0; xx < x0 + w; xx++) {
        for(int yy = y0; yy < y0 + h; yy++) {
            drawPixelRot90CCW(xx, yy, color, fb);
        }
    }
}

static void drawRectRot90CCW(int x0, int y0, int w, int h, uint8_t color, uint8_t* fb) {
    // Top + bottom
    for(int xx = x0; xx < x0 + w; xx++) {
        drawPixelRot90CCW(xx, y0, color, fb);
        drawPixelRot90CCW(xx, y0 + h - 1, color, fb);
    }
    // Left + right
    for(int yy = y0; yy < y0 + h; yy++) {
        drawPixelRot90CCW(x0, yy, color, fb);
        drawPixelRot90CCW(x0 + w - 1, yy, color, fb);
    }
}

static void fillCircleRot90CCW(int cx, int cy, int r, uint8_t color, uint8_t* fb) {
    int rSq = r * r;
    for(int dy = -r; dy <= r; dy++) {
        for(int dx = -r; dx <= r; dx++) {
            if(dx*dx + dy*dy <= rSq) {
                drawPixelRot90CCW(cx + dx, cy + dy, color, fb);
            }
        }
    }
}

static void fillTriangleRot90CCW(int x1, int y1, int x2, int y2, int x3, int y3, uint8_t color, uint8_t* fb) {
    // Find bounding box
    int minX = x1;
    if (x2 < minX) minX = x2;
    if (x3 < minX) minX = x3;
    
    int maxX = x1;
    if (x2 > maxX) maxX = x2;
    if (x3 > maxX) maxX = x3;
    
    int minY = y1;
    if (y2 < minY) minY = y2;
    if (y3 < minY) minY = y3;
    
    int maxY = y1;
    if (y2 > maxY) maxY = y2;
    if (y3 > maxY) maxY = y3;

    // Check each point in bounding box
    for (int y = minY; y <= maxY; y++) {
        for (int x = minX; x <= maxX; x++) {
            // Calculate barycentric coordinates
            float alpha = ((y2 - y3)*(x - x3) + (x3 - x2)*(y - y3)) /
                         (float)((y2 - y3)*(x1 - x3) + (x3 - x2)*(y1 - y3));
            float beta = ((y3 - y1)*(x - x3) + (x1 - x3)*(y - y3)) /
                        (float)((y2 - y3)*(x1 - x3) + (x3 - x2)*(y1 - y3));
            float gamma = 1.0f - alpha - beta;

            // If point is inside triangle, draw it
            if (alpha >= 0 && beta >= 0 && gamma >= 0) {
                drawPixelRot90CCW(x, y, color, fb);
            }
        }
    }
}

// -----------------------------------------------------------------------------
// 6×8 BASIC FONT covering ASCII 0x20..0x7F
// (Ensure your actual file has the complete array of 96 characters.)
// -----------------------------------------------------------------------------
static const unsigned char BASIC_FONT[96][6] = {
  // 0x20 ' '
  {0x00,0x00,0x00,0x00,0x00,0x00},
  // 0x21 '!'
  {0x00,0x00,0x5F,0x00,0x00,0x00},
  // 0x22 '"'
  {0x00,0x07,0x00,0x07,0x00,0x00},
  // 0x23 '#'
  {0x14,0x7F,0x14,0x7F,0x14,0x00},
  // 0x24 '$'
  {0x24,0x2A,0x7F,0x2A,0x12,0x00},
  // 0x25 '%'
  {0x23,0x13,0x08,0x64,0x62,0x00},
  // 0x26 '&'
  {0x36,0x49,0x55,0x22,0x50,0x00},
  // 0x27 '''
  {0x00,0x05,0x03,0x00,0x00,0x00},
  // 0x28 '('
  {0x00,0x1C,0x22,0x41,0x00,0x00},
  // 0x29 ')'
  {0x00,0x41,0x22,0x1C,0x00,0x00},
  // 0x2A '*'
  {0x14,0x08,0x3E,0x08,0x14,0x00},
  // 0x2B '+'
  {0x08,0x08,0x3E,0x08,0x08,0x00},
  // 0x2C ','
  {0x00,0x50,0x30,0x00,0x00,0x00},
  // 0x2D '-'
  {0x08,0x08,0x08,0x08,0x08,0x00},
  // 0x2E '.'
  {0x00,0x60,0x60,0x00,0x00,0x00},
  // 0x2F '/'
  {0x20,0x10,0x08,0x04,0x02,0x00},

  // 0x30 '0'
  {0x3E,0x51,0x49,0x45,0x3E,0x00},
  // 0x31 '1'
  {0x00,0x42,0x7F,0x40,0x00,0x00},
  // 0x32 '2'
  {0x72,0x49,0x49,0x49,0x46,0x00},
  // 0x33 '3'
  {0x21,0x49,0x49,0x49,0x36,0x00},
  // 0x34 '4'
  {0x18,0x14,0x12,0x7F,0x10,0x00},
  // 0x35 '5'
  {0x27,0x49,0x49,0x49,0x31,0x00},
  // 0x36 '6'
  {0x3E,0x49,0x49,0x49,0x32,0x00},
  // 0x37 '7'
  {0x01,0x71,0x09,0x05,0x03,0x00},
  // 0x38 '8'
  {0x36,0x49,0x49,0x49,0x36,0x00},
  // 0x39 '9'
  {0x26,0x49,0x49,0x49,0x3E,0x00},
  // 0x3A ':'
  {0x00,0x36,0x36,0x00,0x00,0x00},
  // 0x3B ';'
  {0x00,0x56,0x36,0x00,0x00,0x00},
  // 0x3C '<'
  {0x08,0x14,0x22,0x41,0x00,0x00},
  // 0x3D '='
  {0x14,0x14,0x14,0x14,0x14,0x00},
  // 0x3E '>'
  {0x00,0x41,0x22,0x14,0x08,0x00},
  // 0x3F '?'
  {0x02,0x01,0x59,0x09,0x06,0x00},

  // 0x40 '@'
  {0x3E,0x41,0x5D,0x59,0x4E,0x00},
  // 0x41 'A'
  {0x7E,0x11,0x11,0x11,0x7E,0x00},
  // 0x42 'B'
  {0x7F,0x49,0x49,0x49,0x36,0x00},
  // 0x43 'C'
  {0x3E,0x41,0x41,0x41,0x22,0x00},
  // 0x44 'D'
  {0x7F,0x41,0x41,0x22,0x1C,0x00},
  // 0x45 'E'
  {0x7F,0x49,0x49,0x49,0x41,0x00},
  // 0x46 'F'
  {0x7F,0x09,0x09,0x09,0x01,0x00},
  // 0x47 'G'
  {0x3E,0x41,0x49,0x49,0x7A,0x00},
  // 0x48 'H'
  {0x7F,0x08,0x08,0x08,0x7F,0x00},
  // 0x49 'I'
  {0x00,0x41,0x7F,0x41,0x00,0x00},
  // 0x4A 'J'
  {0x20,0x40,0x41,0x3F,0x01,0x00},
  // 0x4B 'K'
  {0x7F,0x08,0x14,0x22,0x41,0x00},
  // 0x4C 'L'
  {0x7F,0x40,0x40,0x40,0x40,0x00},
  // 0x4D 'M'
  {0x7F,0x02,0x04,0x02,0x7F,0x00},
  // 0x4E 'N'
  {0x7F,0x04,0x08,0x10,0x7F,0x00},
  // 0x4F 'O'
  {0x3E,0x41,0x41,0x41,0x3E,0x00},

  // 0x50 'P'
  {0x7F,0x09,0x09,0x09,0x06,0x00},
  // 0x51 'Q'
  {0x3E,0x41,0x51,0x21,0x5E,0x00},
  // 0x52 'R'
  {0x7F,0x09,0x19,0x29,0x46,0x00},
  // 0x53 'S'
  {0x26,0x49,0x49,0x49,0x32,0x00},
  // 0x54 'T'
  {0x01,0x01,0x7F,0x01,0x01,0x00},
  // 0x55 'U'
  {0x3F,0x40,0x40,0x40,0x3F,0x00},
  // 0x56 'V'
  {0x1F,0x20,0x40,0x20,0x1F,0x00},
  // 0x57 'W'
  {0x7F,0x20,0x10,0x20,0x7F,0x00},
  // 0x58 'X'
  {0x63,0x14,0x08,0x14,0x63,0x00},
  // 0x59 'Y'
  {0x03,0x04,0x78,0x04,0x03,0x00},
  // 0x5A 'Z'
  {0x61,0x51,0x49,0x45,0x43,0x00},
  // 0x5B '['
  {0x00,0x7F,0x41,0x41,0x00,0x00},
  // 0x5C '\'
  {0x02,0x04,0x08,0x10,0x20,0x00},
  // 0x5D ']'
  {0x00,0x41,0x41,0x7F,0x00,0x00},
  // 0x5E '^'
  {0x04,0x02,0x01,0x02,0x04,0x00},
  // 0x5F '_'
  {0x40,0x40,0x40,0x40,0x40,0x00},

  // 0x60 '`'
  {0x00,0x01,0x02,0x04,0x00,0x00},
  // 0x61 'a'
  {0x20,0x54,0x54,0x54,0x78,0x00},
  // 0x62 'b'
  {0x7F,0x48,0x44,0x44,0x38,0x00},
  // 0x63 'c'
  {0x38,0x44,0x44,0x44,0x20,0x00},
  // 0x64 'd'
  {0x38,0x44,0x44,0x48,0x7F,0x00},
  // 0x65 'e'
  {0x38,0x54,0x54,0x54,0x18,0x00},
  // 0x66 'f'
  {0x08,0x7E,0x09,0x01,0x02,0x00},
  // 0x67 'g'
  {0x08,0x54,0x54,0x54,0x3C,0x00},
  // 0x68 'h'
  {0x7F,0x08,0x04,0x04,0x78,0x00},
  // 0x69 'i'
  {0x00,0x44,0x7D,0x40,0x00,0x00},
  // 0x6A 'j'
  {0x20,0x40,0x44,0x3D,0x00,0x00},
  // 0x6B 'k'
  {0x7F,0x10,0x28,0x44,0x00,0x00},
  // 0x6C 'l'
  {0x00,0x41,0x7F,0x40,0x00,0x00},
  // 0x6D 'm'
  {0x7C,0x04,0x18,0x04,0x78,0x00},
  // 0x6E 'n'
  {0x7C,0x08,0x04,0x04,0x78,0x00},
  // 0x6F 'o'
  {0x38,0x44,0x44,0x44,0x38,0x00},

  // 0x70 'p'
  {0x7C,0x14,0x14,0x14,0x08,0x00},
  // 0x71 'q'
  {0x08,0x14,0x14,0x14,0x7C,0x00},
  // 0x72 'r'
  {0x7C,0x08,0x04,0x04,0x08,0x00},
  // 0x73 's'
  {0x48,0x54,0x54,0x54,0x20,0x00},
  // 0x74 't'
  {0x04,0x3F,0x44,0x40,0x20,0x00},
  // 0x75 'u'
  {0x3C,0x40,0x40,0x20,0x7C,0x00},
  // 0x76 'v'
  {0x1C,0x20,0x40,0x20,0x1C,0x00},
  // 0x77 'w'
  {0x3C,0x40,0x30,0x40,0x3C,0x00},
  // 0x78 'x'
  {0x44,0x28,0x10,0x28,0x44,0x00},
  // 0x79 'y'
  {0x0C,0x50,0x50,0x50,0x3C,0x00},
  // 0x7A 'z'
  {0x44,0x64,0x54,0x4C,0x44,0x00},
  // 0x7B '{'
  {0x00,0x08,0x36,0x41,0x00,0x00},
  // 0x7C '|'
  {0x00,0x00,0x7F,0x00,0x00,0x00},
  // 0x7D '}'
  {0x00,0x41,0x36,0x08,0x00,0x00},
  // 0x7E '~'
  {0x02,0x01,0x02,0x04,0x02,0x00},
};

static void drawCharScaledRot90CCW(int x, int y, char c, int scale, uint8_t color, uint8_t* fb) {
    if(c < 0x20 || c > 0x7F) c = 0x20;
    int idx = c - 0x20;

    for(int col = 0; col < 6; col++) {
        unsigned char colData = BASIC_FONT[idx][col];
        for(int row = 0; row < 8; row++) {
            bool bitSet = (colData & (1 << row)) != 0;
            // If bitSet => use 'color', else fill background with 0xFF (white).
            uint8_t pxColor = bitSet ? color : 0xFF;
            for(int sx = 0; sx < scale; sx++) {
                for(int sy = 0; sy < scale; sy++) {
                    drawPixelRot90CCW(x + col*scale + sx, y + row*scale + sy, pxColor, fb);
                }
            }
        }
    }
}

static void drawStringScaledRot90CCW(int x, int y, const char* text, int scale,
                                     uint8_t color, uint8_t* fb) {
    int cursorX = x;
    while(*text) {
        drawCharScaledRot90CCW(cursorX, y, *text, scale, color, fb);
        cursorX += 6 * scale;
        text++;
    }
}

// -----------------------------------------------------------------------------
// ROVER DISPLAY
// -----------------------------------------------------------------------------
void RoverDisplay::drawRover(uint8_t* framebuffer, int ignoreX, int ignoreY) {
    const int s = 3;
    int x0 = 540;
    int y0 = 100;

    // White body with black outline
    fillRectRot90CCW(x0, y0, 100*s, 70*s, 0xFF, framebuffer);
    for(int t=0; t<2; t++) {
        drawRectRot90CCW(x0 - t, y0 - t, (100*s) + 2*t, (70*s) + 2*t, 0, framebuffer);
    }

    int x = x0 + 50*s;
    int y = y0 + 35*s;

    // Ears: fill white, then outline in black
    fillTriangleRot90CCW(
        x - 40*s, y - 35*s,
        x - 25*s, y - 45*s,
        x - 45*s, y - 45*s,
        0xFF, framebuffer
    );
    fillTriangleRot90CCW(
        x + 40*s, y - 35*s,
        x + 25*s, y - 45*s,
        x + 45*s, y - 45*s,
        0xFF, framebuffer
    );
    // Black outlines for ears
    drawTriangleRot90CCW(
        x - 40*s, y - 35*s,
        x - 25*s, y - 45*s,
        x - 45*s, y - 45*s,
        0, framebuffer
    );
    drawTriangleRot90CCW(
        x + 40*s, y - 35*s,
        x + 25*s, y - 45*s,
        x + 45*s, y - 45*s,
        0, framebuffer
    );

    // Eye panel
    fillRectRot90CCW(x - 35*s, y - 25*s, 70*s, 30*s, 0xAA, framebuffer);

    // Eyes
    fillCircleRot90CCW(x - 15*s, y - 10*s, 5*s, 0, framebuffer);
    fillCircleRot90CCW(x + 15*s, y - 10*s, 5*s, 0, framebuffer);

    // Nose + vertical line to mouth
    fillCircleRot90CCW(x, y + 10*s, 2*s, 0, framebuffer);
    for(int ln = (y + 12*s); ln < (y + 25*s); ln++) {
        drawPixelRot90CCW(x, ln, 0, framebuffer);
    }

    // Smile
    int mouthWidth = 30*s;
    int mouthY = y + 25*s;
    for(int i = -mouthWidth/2; i < mouthWidth/2; i++) {
        int h = -(i*i) / (40 / s);
        drawPixelRot90CCW(x + i, mouthY + h, 0, framebuffer);
        drawPixelRot90CCW(x + i, mouthY + h - 1, 0, framebuffer);
    }

    // Move "ROVERSCRIBE" slightly left (from 560 to 540)
    drawStringScaledRot90CCW(540, 340, "ROVERSCRIBE", 4, 0, framebuffer);
}

void RoverDisplay::drawTasks(uint8_t* framebuffer, const char* tasks[], int taskCount) {
    // Keep existing ToDo list and tasks exactly as they were
    drawStringScaledRot90CCW(195, 480, "ToDo:", 5, 0, framebuffer);

    int scale = 2;
    int lineHeight = (8 * scale) + 10;
    int startY = 540;

    for(int i = 0; i < taskCount; i++) {
        int ty = startY + i * lineHeight;
        fillCircleRot90CCW(460, ty + (4 * scale), 5, 0, framebuffer);
        drawStringScaledRot90CCW(490, ty, tasks[i], scale, 0, framebuffer);
    }

    // Time stays at x=335
    drawStringScaledRot90CCW(335, 10, "12:34", 2, 0, framebuffer);

    // Move battery further right (to x=900)
    fillRectRot90CCW(900, 10, 30, 15, 0xFF, framebuffer);
    drawRectRot90CCW(900, 10, 30, 15, 0, framebuffer);
    fillRectRot90CCW(930, 12, 2, 11, 0, framebuffer);  // nub
    fillRectRot90CCW(902, 12, 10, 11, 0, framebuffer); // partial fill

    // Move both buttons further right
    int btnY = 880;

    // Left "MENU" button (move to x=485)
    fillRectRot90CCW(485, btnY, 120, 40, 0xFF, framebuffer);
    drawRectRot90CCW(485, btnY, 120, 40, 0, framebuffer);
    drawStringScaledRot90CCW(533, btnY + 10, "MENU", 2, 0, framebuffer);

    // Right "Sync RoverNet" button (move to x=655)
    fillRectRot90CCW(655, btnY, 300, 40, 0xFF, framebuffer);
    drawRectRot90CCW(655, btnY, 300, 40, 0, framebuffer);
    drawStringScaledRot90CCW(657, btnY + 10, "Sync RoverNet", 2, 0, framebuffer);
}