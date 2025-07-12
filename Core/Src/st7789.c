/*
 * st7789.c
 *
 *  Created on: Jul 11, 2025
 *      Author: mgosi
 */

#include "st7789.h"
#include <string.h> // For strlen
#include <stdio.h>  // For snprintf (if needed for debugging)

// Global SPI handle pointer
SPI_HandleTypeDef *hspi_st7789;

// Internal functions
static void ST7789_Select() {
    HAL_GPIO_WritePin(ST7789_CS_GPIO_Port, ST7789_CS_Pin, GPIO_PIN_RESET);
}

static void ST7789_Unselect() {
    HAL_GPIO_WritePin(ST7789_CS_GPIO_Port, ST7789_CS_Pin, GPIO_PIN_SET);
}

void ST7789_WriteCommand(uint8_t cmd) {
    ST7789_Select();
    HAL_GPIO_WritePin(ST7789_DC_GPIO_Port, ST7789_DC_Pin, GPIO_PIN_RESET); // Command mode
    HAL_SPI_Transmit(hspi_st7789, &cmd, 1, HAL_MAX_DELAY);
    ST7789_Unselect();
}

void ST7789_WriteData(uint8_t *buff, size_t buff_size) {
    ST7789_Select();
    HAL_GPIO_WritePin(ST7789_DC_GPIO_Port, ST7789_DC_Pin, GPIO_PIN_SET); // Data mode
    HAL_SPI_Transmit(hspi_st7789, buff, buff_size, HAL_MAX_DELAY);
    ST7789_Unselect();
}

void ST7789_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {

	// --- THIS IS THE KEY MODIFICATION ---
	// For Waveshare 1.69inch 170x320, a common X-offset is 35 pixels.
	// This shifts our drawing origin to the correct visible area on the panel.
	uint16_t y_hardware_offset = 20; // Start with 35. Some panels use 50.

	y0 += y_hardware_offset;
	y1 += y_hardware_offset;
	// --- END OF KEY MODIFICATION ---

    uint8_t data[4];

    // Column Address Set (CASET)
    ST7789_WriteCommand(ST7789_CASET);
    data[0] = (x0 >> 8) & 0xFF;
    data[1] = x0 & 0xFF;
    data[2] = (x1 >> 8) & 0xFF;
    data[3] = x1 & 0xFF;
    ST7789_WriteData(data, 4);

    // Row Address Set (RASET)
    ST7789_WriteCommand(ST7789_RASET);
    data[0] = (y0 >> 8) & 0xFF;
    data[1] = y0 & 0xFF;
    data[2] = (y1 >> 8) & 0xFF;
    data[3] = y1 & 0xFF;
    ST7789_WriteData(data, 4);

    ST7789_WriteCommand(ST7789_RAMWR); // Memory Write
}

void ST7789_Init(SPI_HandleTypeDef *hspi) {
    hspi_st7789 = hspi;

    // Hardware Reset
    HAL_GPIO_WritePin(ST7789_RST_GPIO_Port, ST7789_RST_Pin, GPIO_PIN_SET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(ST7789_RST_GPIO_Port, ST7789_RST_Pin, GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(ST7789_RST_GPIO_Port, ST7789_RST_Pin, GPIO_PIN_SET);
    HAL_Delay(120); // Wait for reset to complete

    // Backlight ON
    ST7789_SetBacklight(1);
    HAL_Delay(10);

    // Initial sequence for ST7789V2 (common for 1.69inch)
    ST7789_WriteCommand(ST7789_SWRESET); // Software reset
    HAL_Delay(150);

    ST7789_WriteCommand(ST7789_SLPOUT);  // Exit sleep
    HAL_Delay(10);

    ST7789_WriteCommand(ST7789_COLMOD);  // Set color mode
    uint8_t data_colmod[] = {0x05};      // 16-bit color (RGB565)
    ST7789_WriteData(data_colmod, 1);
    HAL_Delay(10);

    ST7789_WriteCommand(ST7789_MADCTL);  // Memory Data Access Control
    uint8_t data_madctl[] = {0x00};      // Default: MX=0, MY=0, MV=0, ML=0, RGB=0 (Portrait, top-to-bottom, left-to-right)
    // For 1.69" 170x320: MV = 0x20 is common for landscape, 0x00 for portrait
    // The 1.69" might be 240x280 internally mapped to 170x320 (or vice versa due to offset).
    // Let's assume 0x00 for now and adjust with ST7789_SetRotation if needed.
    // A common MADCTL for these screens to align correctly with 170x320:
    // 0x00: Portrait (MY, MX, MV, ML, RGB, MH, X, X)
    // 0x08: BGR (instead of RGB)
    // 0x0C: MY + MX (180deg rotation)
    // 0x60: MV + MX (90deg landscape)
    // 0xA0: MY + MV (270deg landscape)
    // 0xC0: MY + MX + MV + ML (Portrait, 180deg)
    // For 1.69" often 0x00 or 0x60 (landscape)
    // Let's use 0x00 for initial portrait and add rotation function.
    ST7789_WriteData(data_madctl, 1);
    HAL_Delay(10);

    // Common for some 1.69" ST7789V2 panels: setting offset (usually 35,0 or 0,35)
    // If your display looks cut off, these might be needed:
    // ST7789_WriteCommand(0xB2); // Porch setting
    // uint8_t data_porch[] = {0x0C, 0x0C, 0x00, 0x33, 0x33};
    // ST7789_WriteData(data_porch, 5);
    //
    // ST7789_WriteCommand(0xB7); // Gate Control
    // uint8_t data_gate[] = {0x35};
    // ST7789_WriteData(data_gate, 1);

    ST7789_WriteCommand(ST7789_INVON);   // Inversion ON
    HAL_Delay(10);

    ST7789_WriteCommand(ST7789_NORON);   // Normal display mode
    HAL_Delay(10);

    ST7789_WriteCommand(ST7789_DISPON);  // Display ON
    HAL_Delay(120);

    // Initial fill to black
    ST7789_FillScreen(ST7789_BLACK);
}

void ST7789_FillScreen(uint16_t color) {
    ST7789_FillRectangle(0, 0, ST7789_WIDTH, ST7789_HEIGHT, color);
}

void ST7789_DrawPixel(int16_t x, int16_t y, uint16_t color) {
    if ((x < 0) || (x >= ST7789_WIDTH) || (y < 0) || (y >= ST7789_HEIGHT)) return;
    ST7789_SetAddressWindow(x, y, x + 1, y + 1);
    uint8_t data[2];
    data[0] = (uint8_t)(color >> 8);
    data[1] = (uint8_t)(color & 0xFF);
    ST7789_WriteData(data, 2);
}

void ST7789_FillRectangle(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if ((x >= ST7789_WIDTH) || (y >= ST7789_HEIGHT)) return;
    if ((x + w - 1) >= ST7789_WIDTH) w = ST7789_WIDTH - x;
    if ((y + h - 1) >= ST7789_HEIGHT) h = ST7789_HEIGHT - y;

    ST7789_SetAddressWindow(x, y, x + w - 1, y + h - 1);

    uint8_t data[2];
    data[0] = (uint8_t)(color >> 8);
    data[1] = (uint8_t)(color & 0xFF);

    uint32_t i = 0;
    ST7789_Select();
    HAL_GPIO_WritePin(ST7789_DC_GPIO_Port, ST7789_DC_Pin, GPIO_PIN_SET); // Data mode

    // Transmit in chunks if the buffer is large, or just directly
    for (i = 0; i < (uint32_t)w * h; i++) {
        HAL_SPI_Transmit(hspi_st7789, data, 2, HAL_MAX_DELAY);
    }
    ST7789_Unselect();
}


void ST7789_DrawRectangle(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    ST7789_DrawLine(x, y, x + w - 1, y, color);         // Top
    ST7789_DrawLine(x, y + h - 1, x + w - 1, y + h - 1, color); // Bottom
    ST7789_DrawLine(x, y, x, y + h - 1, color);         // Left
    ST7789_DrawLine(x + w - 1, y, x + w - 1, y + h - 1, color); // Right
}

// Bresenham's line algorithm
void ST7789_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    int16_t dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int16_t dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int16_t err = (dx > dy ? dx : -dy) / 2, e2;

    for (;;) {
        ST7789_DrawPixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        e2 = err;
        if (e2 > -dx) { err -= dy; x0 += sx; }
        if (e2 < dy) { err += dx; y0 += sy; }
    }
}

void ST7789_DrawChar(int16_t x, int16_t y, unsigned char c, const FontDef_t *font, uint16_t color, uint16_t bgcolor) {
    if ((x >= ST7789_WIDTH) || (y >= ST7789_HEIGHT) ||
        ((x + font->width) < 0) || ((y + font->height) < 0))
        return;

    uint8_t i, j;
    const uint8_t *glyph = &font->data[(c - font->firstChar) * font->width * ((font->height + 7) / 8)];

    for (i = 0; i < font->width; i++) {
        uint8_t line_data = 0;
        for (j = 0; j < font->height; j++) {
            if (font->height <= 8) { // 1 byte per column
                line_data = glyph[i];
                if (((line_data >> j) & 0x01) == 0x01) {
                    ST7789_DrawPixel(x + i, y + j, color);
                } else {
                    ST7789_DrawPixel(x + i, y + j, bgcolor);
                }
            } else { // Multiple bytes per column (for taller fonts)
                uint8_t byte_idx = j / 8;
                uint8_t bit_idx = j % 8;
                line_data = glyph[i + byte_idx * font->width];
                if (((line_data >> bit_idx) & 0x01) == 0x01) {
                    ST7789_DrawPixel(x + i, y + j, color);
                } else {
                    ST7789_DrawPixel(x + i, y + j, bgcolor);
                }
            }
        }
    }
}

void ST7789_WriteString(int16_t x, int16_t y, const char* str, const FontDef_t *font, uint16_t color, uint16_t bgcolor) {
    while (*str) {
        if (x + font->width >= ST7789_WIDTH) { // Wrap text if it exceeds screen width
            x = 0;
            y += font->height;
            if (y + font->height >= ST7789_HEIGHT) break; // Don't draw if going off screen
        }
        ST7789_DrawChar(x, y, *str++, font, color, bgcolor);
        x += font->width;
    }
}

void ST7789_SetRotation(uint8_t m) {
    uint8_t madctl_reg;
    switch (m) {
        case 0: // Portrait (0 degrees)
            madctl_reg = 0x00; // MY=0, MX=0, MV=0, ML=0, RGB=0
            break;
        case 1: // Landscape (90 degrees)
            madctl_reg = 0x60; // MY=0, MX=1, MV=1, ML=0, RGB=0
            break;
        case 2: // Portrait (180 degrees)
            madctl_reg = 0xC0; // MY=1, MX=1, MV=0, ML=0, RGB=0
            break;
        case 3: // Landscape (270 degrees)
            madctl_reg = 0xA0; // MY=1, MX=0, MV=1, ML=0, RGB=0
            break;
        default: // Default to 0 degrees
            madctl_reg = 0x00;
            break;
    }
    // Consider BGR if your colors are inverted
    // madctl_reg |= 0x08; // Set BGR bit if needed (0x08 for BGR, 0x00 for RGB)

    ST7789_WriteCommand(ST7789_MADCTL);
    ST7789_WriteData(&madctl_reg, 1);
}

void ST7789_DisplayOn(void) {
    ST7789_WriteCommand(ST7789_DISPON);
}

void ST7789_DisplayOff(void) {
    ST7789_WriteCommand(ST7789_DISPOFF);
}

void ST7789_SetBacklight(uint8_t state) {
    if (state) {
        HAL_GPIO_WritePin(ST7789_BL_GPIO_Port, ST7789_BL_Pin, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(ST7789_BL_GPIO_Port, ST7789_BL_Pin, GPIO_PIN_RESET);
    }
}
