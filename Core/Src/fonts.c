// Core/Src/fonts.c

#include "fonts.h"

// 8x8 Basic Font Data
// Format: Column-major, LSB (bit 0) is bottom pixel, MSB (bit 7) is top pixel.
// Each character is 8 bytes (8 columns * 1 byte/column for 8 pixel height)
const uint8_t Font_8x8_Basic_data[] = {
    // ASCII 32: ' ' (Space) - 8 bytes
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    // ASCII 33: '!' (Exclamation Mark) - 8 bytes (Standard 8x8 column-major, LSB-bottom)
    // Pixels (X from left, Y from bottom)
    // . . X . . . . . (Y=7) (MSB)
    // . . X . . . . . (Y=6)
    // . . X . . . . . (Y=5)
    // . . X . . . . . (Y=4)
    // . . . . . . . . (Y=3)
    // . . X . . . . . (Y=2)
    // . . . . . . . . (Y=1)
    // . . X . . . . . (Y=0) (LSB)
    // Column 2 (0xF5 = 11110101):
    // 0:1, 1:0, 2:1, 3:0, 4:1, 5:1, 6:1, 7:1 (from bottom to top)
    0x00, 0x00, 0xF5, 0x00, 0x00, 0x00, 0x00, 0x00, // Corrected '!' data

    // ASCII 34: '"' (Double Quote) - 8 bytes
    // . X . X . . . .
    // . X . X . . . .
    // . . . . . . . .
    // . . . . . . . .
    // . . . . . . . .
    // . . . . . . . .
    // . . . . . . . .
    // . . . . . . . .
    0x00, 0xC0, 0x00, 0xC0, 0x00, 0x00, 0x00, 0x00,

    // ASCII 35: '#' (Hash/Pound) - 8 bytes
    // . X . X . X . .
    // X X X X X X X .
    // . X . X . X . .
    // X X X X X X X .
    // . X . X . X . .
    // X X X X X X X .
    // . X . X . X . .
    // . . . . . . . .
    0x2A, 0x7F, 0x2A, 0x7F, 0x2A, 0x7F, 0x2A, 0x00,

    // ASCII 36: '$' (Dollar Sign) - 8 bytes (example)
    0x00, 0x20, 0x5E, 0x54, 0x7E, 0x50, 0x20, 0x00,


    // ... (you will need to fill in other characters up to 126. Each character must be 8 bytes!) ...

    // ASCII 65: 'A' (Capital A) - 8 bytes (Standard 8x8 column-major, LSB-bottom)
    // . . X . . . . .
    // . X . X . . . .
    // . X . X . . . .
    // X X X X X X X .
    // X . . . . . X .
    // X . . . . . X .
    // X . . . . . X .
    // . . . . . . . .
    0x00, 0xFE, 0x11, 0x11, 0x11, 0x7E, 0x00, 0x00, // Corrected 'A' data

    // ASCII 66: 'B' (Capital B) - 8 bytes (Standard 8x8 column-major, LSB-bottom)
    // X X X X X X . .
    // X . . . . . X .
    // X . . . . . X .
    // X X X X X X . .
    // X . . . . . X .
    // X . . . . . X .
    // X X X X X X . .
    // . . . . . . . .
    0x00, 0x7F, 0x49, 0x49, 0x49, 0x36, 0x00, 0x00, // Corrected 'B' data

    // ASCII 67: 'C' (Capital C) - 8 bytes (Standard 8x8 column-major, LSB-bottom)
    // . . X X X X . .
    // . X . . . . X .
    // X . . . . . . .
    // X . . . . . . .
    // X . . . . . . .
    // X . . . . . . .
    // . X . . . . X .
    // . . X X X X . .
    0x00, 0x3E, 0x41, 0x41, 0x41, 0x22, 0x00, 0x00, // Corrected 'C' data

    // It is critical that your Font_8x8_Basic_data array contains exactly
    // (font->lastChar - font->firstChar + 1) * font->width bytes.
    // So for firstChar = 32, lastChar = 126, width = 8:
    // (126 - 32 + 1) * 8 = 95 characters * 8 bytes/char = 760 bytes total.
    // If you don't have all characters, pad with 0x00, 0x00, ... for each missing character's 8 bytes.
    // ...
};

// Font Definition (this part remains the same)
const FontDef_t Font_8x8_Basic = {
    .data = Font_8x8_Basic_data,
    .width = 8,
    .height = 8,
    .firstChar = 32,
    .lastChar = 126
};
