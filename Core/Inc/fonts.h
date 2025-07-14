// Core/Inc/fonts.h

#ifndef INC_FONTS_H_
#define INC_FONTS_H_

#include <stdint.h> // For uint8_t, uint16_t etc.

// Structure for a font definition
typedef struct {
    const uint8_t *data;    // Pointer to the font data array
    uint8_t width;          // Character width in pixels (e.g., 8)
    uint8_t height;         // Character height in pixels (e.g., 8)
    uint8_t firstChar;      // ASCII value of the first character in the font data
    uint8_t lastChar;       // ASCII value of the last character in the font data
} FontDef_t;

// Declare the raw data array for the 8x8 Basic Font
extern const FontDef_t Font_7x11;
extern const FontDef_t Font_8x14;
extern const FontDef_t Font_15x32;

#endif /* INC_FONTS_H_ */
