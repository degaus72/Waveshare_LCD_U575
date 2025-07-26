#include "bme69x_user.h"
#include "bme69x.h" // Include the BME69x driver header
#include "main.h"   // For HAL_Delay, HAL_GetTick, etc.
#include "i2c.h"    // For hi2c3 if not extern'd in main.h

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
    status = HAL_I2C_Mem_Read(user_handle->hi2c, (user_handle->i2c_addr << 1), reg_addr, I2C_MEMADD_SIZE_8BIT, reg_data, (uint16_t)len, HAL_MAX_DELAY);

    return (status == HAL_OK) ? (int8_t)BME69X_OK : (int8_t)BME69X_E_COM_FAIL;
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
    status = HAL_I2C_Mem_Write(user_handle->hi2c, (user_handle->i2c_addr << 1), reg_addr, I2C_MEMADD_SIZE_8BIT, (uint8_t *)reg_data, (uint16_t)len, HAL_MAX_DELAY);

    return (status == HAL_OK) ? (int8_t)BME69X_OK : (int8_t)BME69X_E_COM_FAIL;
}

/**
 * @brief Delay function for BME69x sensor.
 *
 * @param[in] period       Delay period in microseconds.
 * @param[in] intf_ptr     Pointer to the interface (user-defined handle).
 */
void bme69x_delay_us(uint32_t period, void *intf_ptr)
{
    // A simple busy-wait loop for microsecond delays can be imprecise.
    // For more precise delays, a timer peripheral configured for microsecond resolution is recommended.
    // For typical sensor applications, milliseconds precision (using HAL_Delay) is often sufficient
    // if `period` is converted to milliseconds.
    // However, the BME69x driver sometimes requires microsecond delays, especially for shorter periods.
    // If your system clock is fast enough, a `for` loop can provide *approximate* microsecond delays.
    // Example for approximate microsecond delay (adjust N_CYCLES_PER_US based on your CPU frequency):
    // For a 100MHz CPU, 1 instruction cycle is 10ns. So 100 cycles = 1us.
    // N_CYCLES_PER_US needs to be calibrated.
    // A more robust solution involves a hardware timer (e.g., TIM) or DWT (Data Watchpoint and Trace unit).

    // For simplicity and quick testing, you can use HAL_Delay if period is large enough:
    if (period >= 1000) {
        HAL_Delay(period / 1000); // Convert microseconds to milliseconds
    } else {
        // For sub-millisecond delays, a busy-wait or timer is needed.
        // This is a crude busy-wait. Adjust `i` iterations for your clock speed.
        // For a 100MHz processor, roughly 100 cycles per microsecond.
        // A single NOP or simple loop iteration might take a few cycles.
        volatile uint32_t i;
        for (i = 0; i < period * (SystemCoreClock / 1000000U / 4); i++) // Approximate cycles per microsecond
        {
            __NOP(); // No Operation
        }
    }
}
