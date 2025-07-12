/*
 * fonts.c
 *
 *  Created on: Jul 11, 2025
 *      Author: mgosi
 */

#include "fonts.h"

// Example Font_7x10 (ASCII characters 32-126)
// This is just a placeholder. You NEED to generate proper font data.
// For a 7x10 font, each character takes 7 bytes (7 columns, 10 pixels high, stored as 1 byte per column if <= 8 height)
// Or (height + 7) / 8 bytes per column if height > 8. For 10 height, it's 2 bytes per column.
// For simplicity and common ST7789 examples, assuming a font where each column is 1 byte and height <= 8.
// If height is 10, then it's 2 bytes per column, so 14 bytes per character.
// Let's adjust FontDef_t for byte per column to simplify.

// For 7x10, each character is 7 columns * 2 bytes/column = 14 bytes.
// Example for 'A' (simplified, just to show structure)
const uint8_t Font_7x10_data[] = {

// Placeholder for ' ' (space)
0x00, 0x00, 0x5F, 0x00, 0x00, 0x00, 0x00, // Column 0 (LSB=top pixel)
//0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Column 1
// ... (up to 14 bytes per character)

// Actual data for 'A' (example, you'll need a generator)
// Column 0:
//0x00, 0x00, // Byte 0, Byte 1
// Column 1:
//0x00, 0x00,
// ... and so on for 7 columns and 10 rows
// This is a complex topic. For 7x10, typical is 7 columns, each 2 bytes.
// So, for 'A', there would be 14 bytes here.

// A more practical approach for 7x10 might be:
// 7x10 means width=7, height=10.
// So, (height + 7) / 8 = (10+7)/8 = 17/8 = 2 bytes per column.
// Total bytes per char = width * bytes_per_column = 7 * 2 = 14 bytes.
// This is just a dummy array for compilation. You must replace it with actual font data.
//0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Dummy for char 32 ' '
//0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Dummy for char 33 '!'
// ... you'll need data for all characters from 32 to 126
};

FontDef_t Font_7x10 = {
    .data = Font_7x10_data,
    .width = 7,
    .height = 10,
    .firstChar = 32,
    .lastChar = 126
};

// You would define Font_11x18 and Font_16x26 similarly with their own data arrays.
// For larger fonts, the data arrays become very big.
const uint8_t Font_11x18_data[] = { /* ... actual 11x18 font data ... */ };
FontDef_t Font_11x18 = {
    .data = Font_11x18_data,
    .width = 11,
    .height = 18,
    .firstChar = 32,
    .lastChar = 126
};

const uint8_t Font_16x26_data[] = { /* ... actual 16x26 font data ... */ };
FontDef_t Font_16x26 = {
    .data = Font_16x26_data,
    .width = 16,
    .height = 26,
    .firstChar = 32,
    .lastChar = 126
};
