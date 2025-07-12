#ifndef CST816T_H
#define CST816T_H

#include "main.h" // For HAL types
#include "i2c.h"  // For I2C handle
#include "gpio.h" // For GPIO control

// --- CST816T I2C Address ---
#define CST816T_I2C_ADDR_WRITE  0xBA // (0x5D << 1) for write
#define CST816T_I2C_ADDR_READ   0xBB // (0x5D << 1) | 0x01 for read

// --- CST816T Registers ---
#define CST816T_GESTURE_ID      0x01
#define CST816T_FINGER_NUM      0x02
#define CST816T_XPOS_H          0x03
#define CST816T_XPOS_L          0x04
#define CST816T_YPOS_H          0x05
#define CST816T_YPOS_L          0x06
#define CST816T_CHIP_ID         0xA7 // Should read 0xB4

// --- Pin Definitions (Adjust based on your CubeIDE configuration) ---
#define CST816T_INT_GPIO_Port   GPIOC
#define CST816T_INT_Pin         GPIO_PIN_1

#define CST816T_RST_GPIO_Port   GPIOC // Assuming PC2 for TP_RST
#define CST816T_RST_Pin         GPIO_PIN_2

typedef struct {
    uint8_t  touch_detected;
    uint16_t x;
    uint16_t y;
    uint8_t  gesture_id;
} TS_State_t;

// --- Function Prototypes ---
void CST816T_Init(I2C_HandleTypeDef *hi2c);
uint8_t CST816T_ReadTouch(TS_State_t *ts_state);
uint8_t CST816T_WhoAmI(void); // Read chip ID for verification

// External interrupt handler for touch
void CST816T_INT_EXTI_Callback(void);

#endif /* CST816T_H */
