#include "dht11.h"

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
    __HAL_TIM_SET_COUNTER(&htim2, 0); // Reset the counter
    while (__HAL_TIM_GET_COUNTER(&htim2) < us); // Wait until counter reaches 'us'
}

// --- DHT11 Protocol Implementation ---
void DHT11_Init(void) {
    // Basic initialization for the sensor if needed, otherwise just prepare pin
    // We assume the pin is configured by CubeMX as output initially, but we'll ensure it here
    Set_DHT11_Output();
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_SET); // Keep high initially
    Delay_us(18000); // Small delay to stabilize
}

// Function to read data from DHT11
// Returns 0 on success, 1 on error
uint8_t DHT11_Read_Data(float *temperature, float *humidity) {
    uint8_t data[5] = {0, 0, 0, 0, 0}; // Raw data: H_int, H_dec, T_int, T_dec, Checksum
    uint8_t checksum = 0;
    uint8_t i, j;

    // 1. Send Start Signal from MCU
    Set_DHT11_Output();
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_RESET); // Pull low
    Delay_us(18000); // Pull low for at least 18ms (e.g., 18ms for DHT11)
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_SET);   // Pull high
    Delay_us(20);    // Pull high for 20-40us

    // 2. Switch to Input mode and wait for DHT11 response
    Set_DHT11_Input();

    // Wait for DHT11 to pull low (Response: 80us LOW)
    // Changed to use timer for better accuracy
    __HAL_TIM_SET_COUNTER(&htim2, 0); // Start measuring this phase
    while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_SET) {
        if (__HAL_TIM_GET_COUNTER(&htim2) > 100) { // Timeout after 100us
            *temperature = -1; *humidity = -1; // Set error values
            return DHT11_ERROR_TIMEOUT_1; // No response from DHT11
        }
    }

    // Wait for DHT11 to pull high (Response: 80us HIGH)
    __HAL_TIM_SET_COUNTER(&htim2, 0); // Start measuring this phase
    while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_RESET) {
        if (__HAL_TIM_GET_COUNTER(&htim2) > 100) { // Timeout after 100us
            *temperature = -1; *humidity = -1;
            return DHT11_ERROR_TIMEOUT_2; // DHT11 stuck low
        }
    }

    // Wait for DHT11 to pull low again (End of response, start of data: 50us LOW)
    __HAL_TIM_SET_COUNTER(&htim2, 0); // Start measuring this phase
    while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_SET) {
        if (__HAL_TIM_GET_COUNTER(&htim2) > 100) { // Timeout after 100us
            *temperature = -1; *humidity = -1;
            return DHT11_ERROR_TIMEOUT_3; // DHT11 stuck high (should be low for data start)
        }
    }

    // 3. Read 40 bits (5 bytes) of data
    for (i = 0; i < 5; i++) { // Loop for 5 bytes
        for (j = 0; j < 8; j++) { // Loop for 8 bits per byte
            // Wait for pin to go high (start of data pulse)
            __HAL_TIM_SET_COUNTER(&htim2, 0); // Measure duration of low part of bit
            while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_RESET) {
                if (__HAL_TIM_GET_COUNTER(&htim2) > 100) { // Timeout if stuck low
                    *temperature = -1; *humidity = -1;
                    return DHT11_ERROR_TIMEOUT_3; // Re-using this code for data bit low timeout
                }
            }
            // Pin is now HIGH. Start measuring HIGH pulse duration.
            __HAL_TIM_SET_COUNTER(&htim2, 0); // Reset timer for HIGH pulse measurement

            while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_SET) {
                if (__HAL_TIM_GET_COUNTER(&htim2) > 100) { // Timeout for HIGH pulse measurement
                    *temperature = -1; *humidity = -1;
                    return DHT11_ERROR_TIMEOUT_4; // Timeout if stuck high
                }
            }
            uint16_t pulse_duration = __HAL_TIM_GET_COUNTER(&htim2); // Get pulse duration

            // Determine bit (0 or 1) based on pulse duration
            // '0' bit: HIGH for ~26-28us
            // '1' bit: HIGH for ~70us
            data[i] <<= 1; // Shift left for next bit
            if (pulse_duration > 40) { // If HIGH pulse is longer than 40us, it's a '1'
                data[i] |= 1; // Set the LSB to 1
            }
        }
    }

    // 4. Checksum Verification
    checksum = data[0] + data[1] + data[2] + data[3];
    if (data[4] == checksum) {
        *humidity = (float)data[0] + ((float)data[1] / 100.0); // DHT11 humidity usually integer part only, but can have decimal
        *temperature = (float)data[2] + ((float)data[3] / 100.0); // DHT11 temperature usually integer part only
        return DHT11_OK; // Success
    } else {
        *temperature = -1; *humidity = -1;
        return DHT11_ERROR_CHECKSUM; // Checksum error
    }
}

    void DHT11_Error(uint8_t error_code) {
        // Assuming you have a way to print to serial (UART/ITM) or display on LCD
        // If using printf, ensure it's redirected to your desired output (UART/ITM)
        // This example uses a simple LED blink or a generic "Error" message
        // You should replace this with your actual error indication mechanism.

        // For debugging, a breakpoint here and checking 'error_code' is best.
        // For a proper user experience, you might blink an LED in a pattern
        // or display on the LCD.
        switch (error_code) {
            case DHT11_ERROR_TIMEOUT_1:
                // printf("DHT11 Error: Timeout 1 (No response from sensor or stuck high)\r\n");
                break;
            case DHT11_ERROR_TIMEOUT_2:
                // printf("DHT11 Error: Timeout 2 (Sensor stuck low after response start)\r\n");
                break;
            case DHT11_ERROR_TIMEOUT_3:
                // printf("DHT11 Error: Timeout 3 (Data bit low pulse stuck high)\r\n");
                break;
            case DHT11_ERROR_TIMEOUT_4:
                // printf("DHT11 Error: Timeout 4 (Data bit high pulse stuck low)\r\n");
                break;
            case DHT11_ERROR_CHECKSUM:
                // printf("DHT11 Error: Checksum Mismatch\r\n");
                break;
            default:
                // printf("DHT11 Error: Unknown (%d)\r\n", error_code);
                break;
        }
        // Optionally, set dummy values to temperature/humidity on error
        // This function cannot directly modify *temperature and *humidity as it doesn't receive them
        // These should be handled by the caller of DHT11_Read_Data.
    }
