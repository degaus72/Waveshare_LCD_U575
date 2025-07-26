#ifndef BME69X_USER_H_
#define BME69X_USER_H_

#include "main.h" // For HAL_StatusTypeDef, uint8_t etc.
#include "i2c.h"  // For I2C_HandleTypeDef

// Function prototypes for the BME69x API interface
int8_t bme69x_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr);
int8_t bme69x_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr);
void bme69x_delay_us(uint32_t period, void *intf_ptr);

// User-defined I2C handle struct for BME69x driver (to pass hi2c handle)
typedef struct {
    I2C_HandleTypeDef *hi2c;
    uint8_t i2c_addr;
} bme69x_i2c_user_handle_t; // Renamed to avoid conflict if already defined in main.c

#endif /* BME69X_USER_H_ */
