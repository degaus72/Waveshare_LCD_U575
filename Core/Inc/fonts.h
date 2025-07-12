#ifndef FONTS_H
#define FONTS_H

#include <stdint.h>

typedef struct {
    const uint8_t *data;
    uint16_t width;
    uint16_t height;
    uint16_t firstChar; // ASCII value of the first character in the font
    uint16_t lastChar;  // ASCII value of the last character in the font
} FontDef_t;

extern FontDef_t Font_7x10;
extern FontDef_t Font_11x18;
extern FontDef_t Font_16x26;

#endif /* FONTS_H */
