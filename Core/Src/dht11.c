/*
 * dht11.c
 *
 *  Created on: Jul 16, 2025
 *      Author: matteogosi
 */

#include "dht11.h"

// --- Helper Functions for Pin Mode Switching and Microsecond Delay ---

// Function to set DHT11 Data Pin as Output
void Set_DHT11_Output(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DHT11_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; // Push-Pull output
    GPIO_InitStruct.Pull = GPIO_NOPULL;          // External pull-up resistor is used
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW; // Can be high, low is fine
    HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStruct);
}

// Function to set DHT11 Data Pin as Input
void Set_DHT11_Input(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DHT11_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;     // Input mode
    GPIO_InitStruct.Pull = GPIO_NOPULL;          // External pull-up resistor is used
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStruct);
}

// Microsecond Delay Function using the configured Timer
// Make sure your TIM_HandleTypeDef variable matches the one from main.c (e.g., htim2)
void Delay_us(uint32_t us) {
	uint32_t start_time = __HAL_TIM_GET_COUNTER(&htim2);
	while ((__HAL_TIM_GET_COUNTER(&htim2) - start_time) < us);
}


// --- DHT11 Protocol Implementation ---

void DHT11_Init(void) {
    // Basic initialization for the sensor if needed, otherwise just prepare pin
    // We assume the pin is configured by CubeMX as output initially, but we'll ensure it here
    Set_DHT11_Output();
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_SET); // Keep high initially
    Delay_us(100); // Small delay to stabilize
}

// Function to read data from DHT11
// Returns 0 on success, 1 on error
uint8_t DHT11_Read_Data(float *temperature, float *humidity, uint8_t raw_data[5]) {
    uint8_t data[5] = {0, 0, 0, 0, 0}; // Raw data: H_int, H_dec, T_int, T_dec, Checksum
    uint8_t checksum = 0;
    uint8_t i, j;
    uint32_t timeout_counter;

    // 1. Send Start Signal from MCU
    Set_DHT11_Output();
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_RESET); // Pull low
    Delay_us(18000); // Pull low for at least 18ms (e.g., 18ms for DHT11)
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_SET);   // Pull high
    Delay_us(20);    // Pull high for 20-40us

    // 2. Switch to Input mode and wait for DHT11 response
    Set_DHT11_Input();

    // Wait for DHT11 to pull low (Response: 80us LOW)
    timeout_counter = 0;
    while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_SET) {
        Delay_us(1); // Increment counter every microsecond
        timeout_counter++;
        if (timeout_counter > 100) { // Timeout after 100us
            return 1; // No response from DHT11
        }
    }

    // Wait for DHT11 to pull high (Response: 80us HIGH)
    timeout_counter = 0;
    while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_RESET) {
    	Delay_us(1);
        timeout_counter++;
        if (timeout_counter > 100) { // Timeout after 100us
            return 1; // DHT11 stuck low
        }
    }

    // Wait for DHT11 to pull low again (End of response, start of data: 50us LOW)
    timeout_counter = 0;
    while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_SET) {
        Delay_us(1);
    	timeout_counter++;
        if (timeout_counter > 100) { // Timeout after 100us
            return 1; // DHT11 stuck high
        }
    }

    // Read 40 bits
    for (i = 0; i < 5; i++) {
        for (j = 0; j < 8; j++) {
            // Wait for start of bit (50us LOW pulse)
            timeout_counter = 0;
            while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_RESET) {
                Delay_us(1);
                timeout_counter++;
                if (timeout_counter > 60) { // Timeout if LOW pulse too long (expected ~50us)
                    // ST7789_WriteString(10, 50, "T4", &Font16, ST7789_RED, ST7789_BLACK); // For debugging
                    return 1;
                }
            }

            // Measure HIGH pulse duration
            __HAL_TIM_SET_COUNTER(&htim2, 0);
            // Don't start/stop the timer for *every* pulse if you started it once globally.
            // If you keep the start/stop inside Delay_us, keep it this way.
            // But if you switch to the "measure current counter" method, this part changes.

            // The 'while' loop below should be fast, not using Delay_us
            timeout_counter = 0;
            while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_SET) {
                // Wait for pin to go low (end of data pulse)
                timeout_counter++;
                // Max pulse duration for '1' bit is ~70us. Add some margin.
                if(timeout_counter > 90) { // Timeout for HIGH pulse measurement
                    // ST7789_WriteString(10, 50, "T5", &Font16, ST7789_RED, ST7789_BLACK); // For debugging
                    // It's possible for this to happen if the pin just stays high.
                    return 1;
                }
            }
            uint16_t pulse_duration = __HAL_TIM_GET_COUNTER(&htim2); // Get pulse duration
            // This reads the counter value which increments every us.
            // If you started the timer once globally, this is simpler:
            // uint16_t pulse_duration = __HAL_TIM_GET_COUNTER(&htim2); // Get time taken since last reset
            // __HAL_TIM_SET_COUNTER(&htim2, 0); // Reset for next measurement

            data[i] <<= 1;
            if (pulse_duration > 40) { // Threshold: >40us is a '1', <=40us is a '0'
                data[i] |= 1;
            }
        }
    }

    // 4. Checksum Verification
    checksum = data[0] + data[1] + data[2] + data[3];
    if (data[4] == checksum) {
        *humidity = (float)data[0] + ((float)data[1] / 100.0); // DHT11 humidity usually integer part only, but can have decimal
        *temperature = (float)data[2] + ((float)data[3] / 100.0); // DHT11 temperature usually integer part only
        return 0; // Success
    } else {
        return 1; // Checksum error
    }
}
