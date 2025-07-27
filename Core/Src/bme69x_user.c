#include "bme69x_user.h"
#include "main.h"   // For HAL_Delay, __NOP, SystemCoreClock
#include "i2c.h"    // For access to hi2c3

/**
 * @brief I2C read function for BME69x sensor.
 *
 * @param[in] reg_addr     Register address to read from.
 * @param[out] reg_data    Pointer to the data buffer to store the read data.
 * @param[in] len          Number of bytes to read.
 * @param[in] intf_ptr     Pointer to the interface (user-defined handle).
 *
 * @return 0 for success (BME69X_OK), non-zero for failure (BME69X_E_COM_FAIL).
 */
int8_t bme69x_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    bme69x_i2c_user_handle_t *user_handle = (bme69x_i2c_user_handle_t *)intf_ptr;
    HAL_StatusTypeDef status;

    // The BME69x expects the register address to be sent first, then data is read.
    // HAL_I2C_Mem_Read handles this sequence.
    // The device address needs to be shifted left by 1 for HAL functions.
    status = HAL_I2C_Mem_Read(user_handle->hi2c, (user_handle->i2c_addr << 1), reg_addr, I2C_MEMADD_SIZE_8BIT, reg_data, (uint16_t)len, HAL_MAX_DELAY);

    return (status == HAL_OK) ? (int8_t)BME69X_OK : (int8_t)BME69X_E_COM_FAIL; // Corrected error macro
}

/**
 * @brief I2C write function for BME69x sensor.
 *
 * @param[in] reg_addr     Register address to write to.
 * @param[in] reg_data    Pointer to the data buffer containing data to be written.
 * @param[in] len          Number of bytes to write.
 * @param[in] intf_ptr     Pointer to the interface (user-defined handle).
 *
 * @return 0 for success (BME69X_OK), non-zero for failure (BME69X_E_COM_FAIL).
 */
int8_t bme69x_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    bme69x_i2c_user_handle_t *user_handle = (bme69x_i2c_user_handle_t *)intf_ptr;
    HAL_StatusTypeDef status;

    // HAL_I2C_Mem_Write handles sending register address, then data.
    // The device address needs to be shifted left by 1 for HAL functions.
    status = HAL_I2C_Mem_Write(user_handle->hi2c, (user_handle->i2c_addr << 1), reg_addr, I2C_MEMADD_SIZE_8BIT, (uint8_t *)reg_data, (uint16_t)len, HAL_MAX_DELAY);

    return (status == HAL_OK) ? (int8_t)BME69X_OK : (int8_t)BME69X_E_COM_FAIL; // Corrected error macro
}

/**
 * @brief Delay function for BME69x sensor in microseconds using busy-wait.
 *
 * @param[in] period       Delay period in microseconds.
 * @param[in] intf_ptr     Pointer to the interface (user-defined handle).
 */
void bme69x_delay_us(uint32_t period, void *intf_ptr)
{
    // This provides an *approximate* microsecond delay using a busy-wait loop.
    // The magic number '4' is an estimation for cycles per loop iteration on Cortex-M.
    // For a 160MHz CPU, (160,000,000 / 1,000,000) = 160 cycles per microsecond.
    // So, '4' means each loop iteration takes about 4 CPU cycles.
    // This value might need slight calibration for perfect accuracy on your specific board/compiler settings.
    volatile uint32_t num_cycles = period * (SystemCoreClock / 1000000U / 4U);
    while (num_cycles--);
}
