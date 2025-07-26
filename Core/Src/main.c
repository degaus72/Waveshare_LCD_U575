/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "i2c.h"         // For I2C_HandleTypeDef (hi2c1, hi2c3)
#include "bme69x.h"      // BME69x driver definitions
#include "bme69x_user.h" // User-defined BME69x interface functions and handle
#include <stdio.h>       // For printf
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
// Define BME69x I2C address (7-bit address)
#define BME69X_I2C_ADDR_PRIM 0x76
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
struct bme69x_dev bme69x_sensor;
struct bme69x_data bme69x_data;
bme69x_i2c_user_handle_t bme69x_i2c_user_handle; // User-defined I2C handle for BME69x
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
// Declare I2C initialization functions from i2c.c or CubeMX generated files
extern void MX_I2C1_Init(void);
extern void MX_I2C3_Init(void);
/* USER CODE BEGIN PFP */
// No need to declare bme69x_i2c_read/write/delay_us here, as they are in bme69x_user.h
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  int8_t rslt;
  uint16_t meas_period;
  uint8_t n_fields;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, semaphores to the ISR */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores to the ISR */
  /* USER CODE END RTOS_SEMAPHORES */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init(); // Initialize I2C1 for LCD (as per your clarification)
  MX_I2C3_Init(); // Initialize I2C3 for BME690

  /* USER CODE BEGIN 2 */

  // Initialize user-defined I2C handle for BME690
  bme69x_i2c_user_handle.hi2c = &hi2c3; // BME690 uses I2C3
  bme69x_i2c_user_handle.i2c_addr = BME69X_I2C_ADDR_PRIM;

  // Assign BME69x sensor structure parameters (using the functions from bme69x_user.c)
  bme69x_sensor.intf_ptr = &bme69x_i2c_user_handle;
  bme69x_sensor.read = bme69x_i2c_read;
  bme69x_sensor.write = bme69x_i2c_write;
  bme69x_sensor.delay_us = bme69x_delay_us;
  bme69x_sensor.intf = BME69X_I2C_INTF;

  // Assign dev_id (important for proper driver function)
  bme69x_sensor.dev_id = BME69X_I2C_ADDR_PRIM;

  // Initialize BME69x sensor
  rslt = bme69x_init(&bme69x_sensor);
  if (rslt != BME69X_OK)
  {
      printf("BME69X Init Failed: %d\r\n", rslt);
      Error_Handler();
  }
  else
  {
      printf("BME69X Init Success!\r\n");
  }

  // Sensor settings (BME69x API v1.0.1)
  bme69x_sensor.conf.os_hum = BME69X_OS_2X;
  bme69x_sensor.conf.os_pres = BME69X_OS_16X;
  bme69x_sensor.conf.os_temp = BME69X_OS_4X;
  bme69x_sensor.conf.filter = BME69X_FILTER_SIZE_3;
  bme69x_sensor.conf.odr = BME69X_ODR_500_MS; // Corrected from standby_time to odr

  bme69x_sensor.conf.run_gas = BME69X_ENABLE_GAS_MEAS;
  bme69x_sensor.conf.heatr_temp = 320; // Degree Celsius
  bme69x_sensor.conf.heatr_dur = 150; // Millisecond
  // In BME69x API v1.0.1, temp_offset is a member of bme69x_dev
  bme69x_sensor.temp_offset = 0;

  // Set the sensor configuration
  rslt = bme69x_set_conf(&bme69x_sensor.conf, &bme69x_sensor);
  if (rslt != BME69X_OK)
  {
      printf("BME69X Set Settings Failed: %d\r\n", rslt);
      Error_Handler();
  }

  // Set the power mode to forced mode
  rslt = bme69x_set_op_mode(BME69X_FORCED_MODE, &bme69x_sensor);
  if (rslt != BME69X_OK)
  {
      printf("BME69X Set Op Mode Failed: %d\r\n", rslt);
      Error_Handler();
  }

  // Get the recommended measurement period for forced mode
  meas_period = bme69x_get_meas_dur(BME69X_FORCED_MODE, &bme69x_sensor.conf, &bme69x_sensor);

/* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    // Delay for the measurement to complete
    bme69x_delay_us(meas_period * 1000, bme69x_sensor.intf_ptr); // Convert ms to us for delay_us function

    // Get sensor data
    rslt = bme69x_get_data(BME69X_FORCED_MODE, &bme69x_data, &n_fields, &bme69x_sensor);

    if (rslt == BME69X_OK)
    {
        // Print sensor data using the correct validity masks
        if (n_fields & BME69X_TEMP_VALID_MSK)
        {
            printf("Temperature: %.2f degC\r\n", bme69x_data.temperature / 100.0);
        }
        if (n_fields & BME69X_PRES_VALID_MSK)
        {
            printf("Pressure: %.2f hPa\r\n", bme69x_data.pressure / 100.0);
        }
        if (n_fields & BME69X_HUM_VALID_MSK)
        {
            printf("Humidity: %.2f %%RH\r\n", bme69x_data.humidity / 1000.0);
        }
        if (n_fields & BME69X_GASM_VALID_MSK)
        {
            // Check if gas measurement is valid
            if (bme69x_data.gas_sense_valid != 0)
            {
                printf("Gas Resistance: %lu Ohm\r\n", (long unsigned int)bme69x_data.gas_resistance);
            }
        }
    }
    else
    {
        printf("BME69X Get Data Failed: %d\r\n", rslt);
    }

    HAL_Delay(1000); // Delay for 1 second before next measurement
  }
  /* USER CODE END 3 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  * where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
