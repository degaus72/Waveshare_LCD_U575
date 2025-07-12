/*
 * cst816t.c
 *
 *  Created on: Jul 11, 2025
 *      Author: mgosi
 */

#include "cst816t.h"
#include "st7789.h"
#include <stdbool.h> // For true/false

I2C_HandleTypeDef *hi2c_cst816t;
volatile bool touch_event_pending = false; // Flag for touch interrupt

void CST816T_Init(I2C_HandleTypeDef *hi2c) {
    hi2c_cst816t = hi2c;

    // --- Hardware Reset for CST816T ---
	HAL_GPIO_WritePin(CST816T_RST_GPIO_Port, CST816T_RST_Pin, GPIO_PIN_RESET); // Pull low to reset
	HAL_Delay(10); // Hold low for a short period
	HAL_GPIO_WritePin(CST816T_RST_GPIO_Port, CST816T_RST_Pin, GPIO_PIN_SET);   // Pull high for normal operation
	HAL_Delay(100); // Wait for the touch controller to come out of reset

    // Optional: Configure interrupt pin to input with pull-up in CubeIDE.
    // This is handled in the .ioc file, but ensure it's correct.

    // No specific initialization commands needed for CST816T typically,
    // as it wakes up and starts reporting automatically.
    // Just verify communication.
    uint8_t chip_id = CST816T_WhoAmI();
    if (chip_id == 0xB4) {
        // Chip ID is correct
        // You might want to print a success message here for debugging
        // printf("CST816T found, Chip ID: 0x%02X\r\n", chip_id);
    } else {
        // Error: Chip ID mismatch
        // printf("CST816T not found or incorrect Chip ID: 0x%02X\r\n", chip_id);
    }
}

uint8_t CST816T_ReadTouch(TS_State_t *ts_state) {
    uint8_t data[6]; // To read Gesture ID, Finger Num, X/Y coords
    HAL_StatusTypeDef status;

    // Read multiple registers starting from GESTURE_ID (0x01)
    status = HAL_I2C_Mem_Read(hi2c_cst816t, CST816T_I2C_ADDR_READ, CST816T_GESTURE_ID, I2C_MEMADD_SIZE_8BIT, data, 6, HAL_MAX_DELAY);

    if (status == HAL_OK) {
        ts_state->gesture_id = data[0];
        ts_state->touch_detected = (data[1] & 0x0F) > 0; // Finger Num is lower 4 bits of data[1]

        if (ts_state->touch_detected) {
            ts_state->x = ((data[2] & 0x0F) << 8) | data[3]; // X coords from data[2] (4 bits) and data[3] (8 bits)
            ts_state->y = ((data[4] & 0x0F) << 8) | data[5]; // Y coords from data[4] (4 bits) and data[5] (8 bits)

            // Adjust coordinates for the 1.69" 170x320 display if necessary
            // The CST816T might report 240x280 or other resolutions
            // You may need to scale or offset:
            // For example, if touch is 240x280 and display is 170x320:
            // ts_state->x = map(ts_state->x, 0, 240, 0, 170);
            // ts_state->y = map(ts_state->y, 0, 280, 0, 320);
            // The 1.69 inch display may have an internal X-offset of 35 pixels
            // or 50 pixels (or 0, 35, 50, etc.). This needs testing.
            // Example:
            // ts_state->x -= 35; // If it reports X from 35-205 for 170 width
            // ts_state->y -= 0;  // If Y is fine

            // This specific Waveshare 1.69inch model (ST7789V2) often has a 35 pixel offset in X
            // and the Y-axis might be inverted or rotated relative to the touch.
            // It's common for 1.69" displays to be 240x320 internally, but only 170 pixels of X are shown.
            // So, reported X might be 0-239, but active is 35-204 (170 pixels).
            // A typical touch panel might report 240x280 (X-max 239, Y-max 279)
            // Let's assume the screen's X range is 170 from its 240 pixels (starting at 35 offset)
            // and the touch reports 240.
            // If the touch reports 0-239 for X, and display shows 170 pixels (0-169)
            // with an actual offset of 35:
            // ts_state->x = ts_state->x > 35 ? ts_state->x - 35 : 0; // Adjust for typical offset
            // If the touch is rotated/mirrored compared to display:
            // int temp_x = ts_state->x;
            // ts_state->x = ts_state->y;
            // ts_state->y = ST7789_HEIGHT - temp_x; // Example for 90 deg rotation & Y-flip
            // This needs empirical testing!

            // For a 1.69inch 170x320 display, touch coordinates are often 240x280.
            // The actual display X range is 35-204.
            // A simple remapping for portrait mode (0-169 for X, 0-319 for Y):
            // The CST816T can sometimes read values beyond the reported resolution.
            // It's crucial to map the touch coordinates (which might be 240x280 or 320x240)
            // to the actual display resolution (170x320).
            // This is a common point of confusion and requires testing.
            // Let's assume CST816T reports 240x280 internally.
            // For a 170x320 portrait display:
            // X-axis mapping: 0-239 (touch) -> 0-169 (display) with a 35 pixel offset on the display.
            // Y-axis mapping: 0-279 (touch) -> 0-319 (display). Often Y is inverted.
            //
            // Corrected mapping often looks like:
            int raw_x = ts_state->x;
            int raw_y = ts_state->y;

            // Example mapping for 1.69" 170x320:
            // Assuming touch reports 240x280 (width x height)
            // And display is 170x320 (width x height)
            // Usually, the X-axis of the touch is shifted by 35 or 40 pixels.
            // And Y-axis is inverted.
            ts_state->x = (raw_x * ST7789_WIDTH) / 240; // Scale X
            ts_state->y = ST7789_HEIGHT - (raw_y * ST7789_HEIGHT) / 280; // Scale Y and invert

            // Refinement for the 35-pixel X offset of ST7789
            // This is empirical and depends on the specific panel.
            // If the touch point appears shifted left, you might need:
            // ts_state->x = (raw_x - 35) * ST7789_WIDTH / 170; // If raw X starts at 35 for 0 display
            // OR simpler:
            // ts_state->x = (raw_x * ST7789_WIDTH) / 240; // Scale X
            // ts_state->y = (raw_y * ST7789_HEIGHT) / 280; // Scale Y

            // If the touch is rotated 90 degrees compared to portrait display:
            // int temp_x = ts_state->x;
            // ts_state->x = ts_state->y;
            // ts_state->y = ST7789_WIDTH - temp_x; // Swap and invert one axis for 90deg rotation
            // This is the most tricky part and needs testing.
            // For now, let's just use raw scaled coordinates.
            // You will almost certainly need to adjust these `ts_state->x` and `ts_state->y` lines
            // after initial testing to align touch with display.

            // Ensure coordinates are within display bounds
            if (ts_state->x >= ST7789_WIDTH) ts_state->x = ST7789_WIDTH - 1;
            if (ts_state->y >= ST7789_HEIGHT) ts_state->y = ST7789_HEIGHT - 1;
        }
        return ts_state->touch_detected;
    } else {
        ts_state->touch_detected = 0;
        return 0; // Error or no touch
    }
}

uint8_t CST816T_WhoAmI(void) {
    uint8_t chip_id = 0;
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(hi2c_cst816t, CST816T_I2C_ADDR_READ, CST816T_CHIP_ID, I2C_MEMADD_SIZE_8BIT, &chip_id, 1, HAL_MAX_DELAY);
    if (status == HAL_OK) {
        return chip_id;
    }
    return 0x00; // Error
}

// Callback for EXTI line interrupt (defined in stm32g4xx_it.c)
void CST816T_INT_EXTI_Callback(void) {
    touch_event_pending = true; // Set flag
    // In a real application, you might want to disable the EXTI here
    // and re-enable it after processing the touch event to debounce.
}
