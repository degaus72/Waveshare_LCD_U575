#ifndef __DHT11_H
#define __DHT11_H

#include "main.h" // Includes HAL types and your main MCU configuration
#include "tim.h"  // For your timer handle (e.g., htim2)

// --- DHT11 Pin Definitions ---
#define DHT11_PORT GPIOB
#define DHT11_PIN  GPIO_PIN_8

// --- DHT11 Error Codes ---
#define DHT11_OK                 0
#define DHT11_ERROR_TIMEOUT_1    1 // No response from DHT11 / DHT11 stuck high at start of response
#define DHT11_ERROR_TIMEOUT_2    2 // DHT11 stuck low after pulling low (expected high)
#define DHT11_ERROR_TIMEOUT_3    3 // Timeout during data bit low pulse measurement (pin stuck high)
#define DHT11_ERROR_TIMEOUT_4    4 // Timeout during data bit high pulse measurement (pin stuck low)
#define DHT11_ERROR_CHECKSUM     5 // Checksum mismatch

// --- Function Prototypes ---
void Set_DHT11_Output(void);
void Set_DHT11_Input(void);
void Delay_us(uint32_t us);
void DHT11_Init(void);
uint8_t DHT11_Read_Data(float *temperature, float *humidity);
void DHT11_Error(uint8_t error_code); // Declare your error handling function


#endif /* __DHT11_H */
