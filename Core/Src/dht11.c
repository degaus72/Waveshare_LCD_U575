#include "dht11.h"
#include "fonts.h" // For LCD display functions
#include "st7789.h" // For LCD display functions

// --- Helper Functions for Pin Mode Switching and Microsecond Delay ---

// Function to set DHT11 Data Pin as Output
void Set_DHT11_Output(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DHT11_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL; // Using external pull-up resistor
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStruct);
}

// Function to set DHT11 Data Pin as Input
void Set_DHT11_Input(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DHT11_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL; // Using external pull-up resistor
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStruct);
}

// Microsecond Delay Function
void Delay_us(uint32_t us) {
    // Reset the counter to 0 for accurate measurement
    __HAL_TIM_SET_COUNTER(&htim2, 0);
    // Wait until the counter reaches the desired microsecond value
    while (__HAL_TIM_GET_COUNTER(&htim2) < us);
}

// --- DHT11 Protocol Implementation ---

void DHT11_Init(void) {
    // Initial setup for the DHT11 pin.
    // It's configured as Output High by CubeMX's MX_GPIO_Init,
    // but we ensure it here and keep it high for stability.
    Set_DHT11_Output();
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_SET); // Ensure high initially (cooperates with pull-up)
    Delay_us(18000); // Small delay for stabilization
}

// DHT11 Read Data Function
// Returns 0 on success, 1 on failure
uint8_t DHT11_Read_Data(float *temperature, float *humidity, uint8_t raw_data[5]) {
    // --- ONLY MCU Start Signal Test ---

    // DEBUG: Clear screen for fresh debug output
    ST7789_FillScreen(ST7789_BLACK);
    ST7789_WriteString(10, 10, "MCU Signal Test", &Font12, ST7789_YELLOW, ST7789_BLACK);
    HAL_Delay(500); // ADDED DELAY

    // 1. Send Start Signal from MCU
    //Set_DHT11_Output(); // Set pin as output
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_RESET); // Pull low (SHOULD GO TO 0V HERE

    // *** ADD THIS LINE ***
	__asm("nop"); // NOP instruction to force compiler to execute preceding instruction

    Delay_us(18000); // Pull low for at least 18ms
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_SET);   // Pull high
    Delay_us(20);    // Pull high for 20-40us

    // After MCU sends start signal - HOLD HERE FOR OBSERVATION
    ST7789_WriteString(10, 25, "Signal Sent. Check PB8", &Font12, ST7789_CYAN, ST7789_BLACK);
    HAL_Delay(2000); // Hold message for 2 seconds to observe PB8 with multimeter
    ST7789_WriteString(10, 40, "Test Done", &Font12, ST7789_WHITE, ST7789_BLACK);
    HAL_Delay(500);

    return 1; // Always return 1 as this is just a test mode
}
