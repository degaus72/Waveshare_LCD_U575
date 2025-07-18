#include "dht11.h"
#include "fonts.h"
#include "st7789.h"

// --- Helper Functions for Pin Mode Switching and Microsecond Delay ---

extern TIM_HandleTypeDef htim2; // Ensure this matches your timer handle

// Function to set DHT11 Data Pin as Output
void Set_DHT11_Output(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DHT11_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL; // Using external pull-up
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStruct);
}

// Function to set DHT11 Data Pin as Input
void Set_DHT11_Input(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DHT11_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL; // Using external pull-up
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStruct);
}

// Microsecond Delay Function
void Delay_us(uint32_t us) {
    uint32_t start_time = __HAL_TIM_GET_COUNTER(&htim2);
    while ((__HAL_TIM_GET_COUNTER(&htim2) - start_time) < us);
}

// --- DHT11 Protocol Implementation ---

void DHT11_Init(void) {
    Set_DHT11_Output();
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_SET); // Keep high initially, cooperating with pull-up
    Delay_us(100); // Small delay to stabilize
}

// Function to read data from DHT11
// Returns 0 on success, 1 on error
uint8_t DHT11_Read_Data(float *temperature, float *humidity, uint8_t raw_data[5]) {
    uint8_t i, j;
    uint32_t timeout_counter;

    // DEBUG: Indicate start of read function
    ST7789_FillScreen(ST7789_BLACK); // Clear screen for fresh debug output
    ST7789_WriteString(10, 10, "Read Start", &Font12, ST7789_YELLOW, ST7789_BLACK);
    HAL_Delay(500); // Increased delay

    // 1. Send Start Signal from MCU
    Set_DHT11_Output();
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_RESET); // Pull low
    Delay_us(18000); // Pull low for at least 18ms
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_SET);   // Pull high
    Delay_us(20);    // Pull high for 20-40us

    // DEBUG: After MCU sends start signal
    ST7789_WriteString(10, 25, "MCU Signal OK", &Font12, ST7789_YELLOW, ST7789_BLACK);
    HAL_Delay(500);

    // IMPORTANT NEW DEBUG MESSAGE: Check pin state before switching to input
    if(HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_SET) {
        ST7789_WriteString(10, 35, "Pin HIGH Before Input", &Font12, ST7789_CYAN, ST7789_BLACK);
    } else {
        ST7789_WriteString(10, 35, "Pin LOW Before Input", &Font12, ST7789_MAGENTA, ST7789_BLACK); // Larger font for critical error
    }
    HAL_Delay(500);


    // 2. Switch to Input mode and wait for DHT11 response
    Set_DHT11_Input();

    // Wait for DHT11 to pull low (Response: 80us LOW)
    timeout_counter = 0;
    while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_SET) {
        Delay_us(1);
        timeout_counter++;
        if (timeout_counter > 100) {
            ST7789_WriteString(10, 50, "Timeout 1 (No Low)", &Font12, ST7789_RED, ST7789_BLACK);
            HAL_Delay(500);
            return 1;
        }
    }
    ST7789_WriteString(10, 40, "DHT11 Low OK", &Font12, ST7789_GREEN, ST7789_BLACK);
    HAL_Delay(500);


    // Wait for DHT11 to pull high (Response: 80us HIGH)
    timeout_counter = 0;
    while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_RESET) {
        Delay_us(1);
        timeout_counter++;
        if (timeout_counter > 100) {
            ST7789_WriteString(10, 55, "Timeout 2 (No High)", &Font12, ST7789_RED, ST7789_BLACK);
            HAL_Delay(500);
            return 1;
        }
    }
    ST7789_WriteString(10, 55, "DHT11 High OK", &Font12, ST7789_GREEN, ST7789_BLACK);
    HAL_Delay(500);

    // Wait for DHT11 to pull low again (End of response, start of data: 50us LOW)
    timeout_counter = 0;
    while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_SET) {
        Delay_us(1);
        timeout_counter++;
        if (timeout_counter > 100) {
            ST7789_WriteString(10, 70, "Timeout 3 (Data Start)", &Font12, ST7789_RED, ST7789_BLACK);
            HAL_Delay(500);
            return 1;
        }
    }
    ST7789_WriteString(10, 70, "Data Starts OK", &Font12, ST7789_GREEN, ST7789_BLACK);
    HAL_Delay(500);

    // 3. Read 40 bits (5 bytes) of data
    for (i = 0; i < 5; i++) { // Loop for 5 bytes
        raw_data[i] = 0; // Clear byte before receiving bits
        for (j = 0; j < 8; j++) { // Loop for 8 bits per byte
            // Wait for pin to go high (start of data pulse)
            timeout_counter = 0;
            while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_RESET) {
                Delay_us(1);
                timeout_counter++;
                if (timeout_counter > 60) {
                    ST7789_WriteString(10, 85 + (j%2)*15, "Timeout 4 (Bit Start)", &Font12, ST7789_RED, ST7789_BLACK); // Offset to prevent overlap
                    HAL_Delay(500);
                    return 1;
                }
            }

            // Measure the duration of the HIGH pulse
            __HAL_TIM_SET_COUNTER(&htim2, 0); // Reset timer for pulse measurement
            timeout_counter = 0; // Safeguard for this loop
            while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_SET) {
                timeout_counter++;
                if(timeout_counter > 90) {
                    ST7789_WriteString(10, 85 + (j%2)*15, "Timeout 5 (Bit High)", &Font12, ST7789_RED, ST7789_BLACK); // Offset to prevent overlap
                    HAL_Delay(500);
                    return 1;
                }
            }
            uint16_t pulse_duration = __HAL_TIM_GET_COUNTER(&htim2); // Get pulse duration

            // Determine bit (0 or 1) based on pulse duration
            raw_data[i] <<= 1; // Shift left for next bit
            if (pulse_duration > 40) { // If HIGH pulse is longer than 40us, it's a '1'
                raw_data[i] |= 1; // Set the LSB to 1
            }
        }
    }

    ST7789_WriteString(10, 100, "Data Read OK", &Font12, ST7789_GREEN, ST7789_BLACK);
    HAL_Delay(500);

    // 4. Checksum Verification
    uint8_t checksum_calc = raw_data[0] + raw_data[1] + raw_data[2] + raw_data[3];
    if (raw_data[4] == checksum_calc) {
        *humidity = (float)raw_data[0] + ((float)raw_data[1] / 10.0); // Use original calculation
        *temperature = (float)raw_data[2] + ((float)raw_data[3] / 10.0); // Use original calculation

        ST7789_WriteString(10, 115, "Checksum OK", &Font12, ST7789_GREEN, ST7789_BLACK);
        HAL_Delay(500);
        return 0; // Success
    } else {
        ST7789_WriteString(10, 115, "Checksum ERR", &Font12, ST7789_RED, ST7789_BLACK);
        HAL_Delay(500);
        return 1; // Checksum error
    }
}
