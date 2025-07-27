#ifndef BME69X_USER_H_
#define BME69X_USER_H_

#include "main.h" // For HAL_StatusTypeDef, uint8_t etc.
#include "i2c.h"  // For I2C_HandleTypeDef (e.g., hi2c3)
#include "bme69x.h" // Required for BME69X_OK, BME69X_E_COM_FAIL

// User-defined I2C handle struct to pass to the BME69x driver
typedef struct {
    I2C_HandleTypeDef *hi2c;
    uint8_t i2c_addr;
} bme69x_i2c_user_handle_t;

// Function prototypes for the BME69x API interface
// These will be implemented in bme69x_user.c
int8_t bme69x_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr);
int8_t bme69x_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr);
void bme69x_delay_us(uint32_t period, void *intf_ptr);

#endif /* BME69X_USER_H_ */
