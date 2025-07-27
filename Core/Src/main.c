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
#include "i2c.h"
#include "icache.h"
#include "spi.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "i2c.h"         // For I2C_HandleTypeDef (hi2c1, hi2c3)
#include "spi.h"         // For SPI_HandleTypeDef (hspi1, etc.) - required for ST7789 Init
#include "bme69x.h"      // BME69x driver definitions
#include "bme69x_defs.h" // IMPORTANT: Explicitly include for BME69X_VALID_MSK definitions (or similar macros)
#include "bme69x_user.h" // User-defined BME69x interface functions and handle
#include "ST7789.h"      // Your ST7789 LCD driver header
#include "fonts.h"       // Your font definitions (e.g., Font_7x10)
#include <stdio.h>       // For sprintf/snprintf and printf for debugging
#include <string.h>      // For strlen
#include "stm32u5xx_hal.h" // Include for HAL functions, often needed by SystemClock_Config
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
// Define BME69x I2C address (7-bit address)
#define BME69X_I2C_ADDR_PRIM 0x76

// FIX: Increased buffer size to prevent truncation warning
#define LCD_LINE_MAX_LEN 40 // Increased for longer messages like error codes
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

COM_InitTypeDef BspCOMInit;

/* USER CODE BEGIN PV */
struct bme69x_dev bme69x_sensor;
struct bme69x_data bme69x_data;
struct bme69x_conf bme69x_conf;       // Separate structure for TPH configuration
struct bme69x_heatr_conf heatr_config; // Separate structure for gas heater configuration

bme69x_i2c_user_handle_t bme69x_i2c_user_handle; // User-defined I2C handle for BME69x
char lcd_buffer[LCD_LINE_MAX_LEN + 1]; // Buffer for LCD display

// Assume your main SPI handle for the LCD is hspi1. Adjust if different.
extern SPI_HandleTypeDef hspi1;

// Global variable to keep track of the current line for printing
static uint8_t current_lcd_row = 0;
// Using Font_7x10 as a default, adjust if your fonts.h uses a different default or you prefer another font.
const sFONT* current_font = &Font16; // Point to the desired font from fonts.h
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void SystemPower_Config(void);
/* USER CODE BEGIN PFP */
void LCD_Init(void);
void LCD_Clear(void);
void LCD_SetCursor(uint8_t row, uint8_t col);
void LCD_PrintString(const char *str);

void SystemClock_Config(void);
void MX_GPIO_Init(void);
// Declare I2C initialization functions from i2c.c or CubeMX generated files
extern void MX_I2C1_Init(void); // For LCD (if using I2C based LCD)
extern void MX_I2C3_Init(void); // For BME690
extern void MX_SPI1_Init(void); // For ST7789 (if using SPI based LCD)
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void LCD_Init(void)
{
    // Initialize your ST7789 LCD using its dedicated function
    // Assuming the ST7789_Init function takes the SPI handle.
    ST7789_Init(&hspi1); // Pass your SPI handle (e.g., hspi1 from spi.h)

    // Set a default rotation (adjust as per your display orientation)
    // ST7789_SetRotation takes uint8_t 0,1,2,3 for 0,90,180,270 degrees.
    ST7789_SetRotation(0); // Set to default portrait mode (adjust if your display needs rotation)

    LCD_Clear(); // Call to LCD_Clear should now be recognized
    printf("ST7789 LCD Initialized!\r\n");
    current_lcd_row = 0; // Reset line counter on init
}

void LCD_Clear(void)
{
    // Clears the entire screen to the background color
    ST7789_FillScreen(ST7789_BLACK); // Use ST7789_BLACK or any other background color
    current_lcd_row = 0; // Reset row counter when screen is cleared
}

void LCD_SetCursor(uint8_t row, uint8_t col)
{
    // For graphical LCDs like ST7789, a "cursor" is often managed by the drawing functions.
    // We'll update our internal `current_lcd_row` for `LCD_PrintString`.
    if (row * current_font->Height < ST7789_HEIGHT)
    {
        current_lcd_row = row;
    }
    (void)col; // Suppress unused parameter warning
}

void LCD_PrintString(const char *str)
{
    // Calculate the Y position for the current line based on font height
    uint16_t y_pos = current_lcd_row * current_font->Height;
    uint16_t x_pos = 0; // Start printing from the left edge

    // Ensure we don't write outside the screen height
    if (y_pos >= ST7789_HEIGHT)
    {
        // If we exceed screen height, wrap around to the top and clear
        current_lcd_row = 0;
        y_pos = 0;
        LCD_Clear();
    }

    // Use your ST7789_WriteString function
    // Assuming it takes x, y, string, font pointer, text color, background color
    ST7789_WriteString(x_pos, y_pos, (char*)str, current_font, ST7789_WHITE, ST7789_BLACK); // Text color WHITE, background BLACK

    // Move to the next line for the next print
    current_lcd_row++;

    printf("LCD_PrintString: %s\r\n", str); // For serial debugging
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  int8_t rslt;
  uint32_t meas_period_us; // Measurement period in microseconds
  uint8_t n_fields; // Variable to store number of fields from get_data
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the System Power */
  SystemPower_Config();

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ICACHE_Init();
  MX_SPI1_Init();
  MX_TIM2_Init();
  MX_I2C1_Init();
  MX_I2C3_Init();
  /* USER CODE BEGIN 2 */

  // Initialize ST7789 LCD
  LCD_Init(); // This will call ST7789_Init() internally

  // Initialize user-defined I2C handle for BME690
  bme69x_i2c_user_handle.hi2c = &hi2c3; // BME690 uses I2C3
  bme69x_i2c_user_handle.i2c_addr = BME69X_I2C_ADDR_PRIM;

  // Assign BME69x sensor structure parameters (using the functions from bme69x_user.c)
  bme69x_sensor.intf_ptr = &bme69x_i2c_user_handle;
  bme69x_sensor.read = bme69x_i2c_read;
  bme69x_sensor.write = bme69x_i2c_write;
  bme69x_sensor.delay_us = bme69x_delay_us;
  bme69x_sensor.intf = BME69X_I2C_INTF;

  // Removed chip_id and temp_offset as they are not members of bme69x_dev in your API
  // bme69x_sensor.chip_id = BME69X_I2C_ADDR_PRIM;
  // bme69x_sensor.temp_offset = 0;

  // Initialize BME69x sensor
  rslt = bme69x_init(&bme69x_sensor);
  if (rslt != BME69X_OK)
  {
      printf("BME69X Init Failed: %d\r\n", rslt);
      snprintf(lcd_buffer, LCD_LINE_MAX_LEN + 1, "BME69X Init Fail: %d", rslt);
      LCD_SetCursor(0, 0); // Display error on LCD
      LCD_PrintString(lcd_buffer);
      Error_Handler();
  }
  else
  {
      printf("BME69X Init Success!\r\n");
      snprintf(lcd_buffer, LCD_LINE_MAX_LEN + 1, "BME69X Init OK");
      LCD_SetCursor(7, 0); // Display success on LCD
      LCD_PrintString(lcd_buffer);
      HAL_Delay(500); // Small delay to show init message
  }

  // TPH Sensor configuration for bme69x_conf (separate struct)
  bme69x_conf.os_hum = BME69X_OS_2X;
  bme69x_conf.os_pres = BME69X_OS_16X;
  bme69x_conf.os_temp = BME69X_OS_4X;
  bme69x_conf.filter = BME69X_FILTER_SIZE_3;
  bme69x_conf.odr = BME69X_ODR_500_MS; // Output Data Rate

  // Set the TPH sensor configuration
  rslt = bme69x_set_conf(&bme69x_conf, &bme69x_sensor);
  if (rslt != BME69X_OK)
  {
      printf("BME69X Set TPH Settings Failed: %d\r\n", rslt);
      snprintf(lcd_buffer, LCD_LINE_MAX_LEN + 1, "BME69X TPH Conf Fail: %d", rslt);
      LCD_SetCursor(1, 0); // Display error on LCD
      LCD_PrintString(lcd_buffer);
      Error_Handler();
  }

  // Populate the bme69x_heatr_conf struct
  heatr_config.heatr_temp = 320; // Degree Celsius
  heatr_config.heatr_dur = 150; // Millisecond
  heatr_config.enable = BME69X_ENABLE_GAS_MEAS; // Assuming 'enable' is the field name for running gas

  // Call bme69x_set_heatr_conf with the correct arguments: op_mode, pointer to heatr_config, and device pointer
  rslt = bme69x_set_heatr_conf(BME69X_FORCED_MODE, &heatr_config, &bme69x_sensor);
  if (rslt != BME69X_OK)
  {
      printf("BME69X Set Heater Config Failed: %d\r\n", rslt);
      snprintf(lcd_buffer, LCD_LINE_MAX_LEN + 1, "BME69X Heater Fail: %d", rslt);
      LCD_SetCursor(2, 0); // Display error on LCD
      LCD_PrintString(lcd_buffer);
      Error_Handler();
  }

  // Set the power mode to forced mode
  rslt = bme69x_set_op_mode(BME69X_FORCED_MODE, &bme69x_sensor);
  if (rslt != BME69X_OK)
  {
      printf("BME69X Set Op Mode Failed: %d\r\n", rslt);
      snprintf(lcd_buffer, LCD_LINE_MAX_LEN + 1, "BME69X Mode Fail: %d", rslt);
      LCD_SetCursor(3, 0); // Display error on LCD
      LCD_PrintString(lcd_buffer);
      Error_Handler();
  }

  // Get the recommended measurement period for forced mode in microseconds
  meas_period_us = bme69x_get_meas_dur(BME69X_FORCED_MODE, &bme69x_conf, &bme69x_sensor) * 1000;

  LCD_Clear(); // Clear display before starting main loop measurements
  /* USER CODE END 2 */

  /* Initialize leds */
  BSP_LED_Init(LED_GREEN);
  BSP_LED_Init(LED_BLUE);
  BSP_LED_Init(LED_RED);

  /* Initialize USER push-button, will be used to trigger an interrupt each time it's pressed.*/
  BSP_PB_Init(BUTTON_USER, BUTTON_MODE_EXTI);

  /* Initialize COM1 port (115200, 8 bits (7-bit data + 1 stop bit), no parity */
  BspCOMInit.BaudRate   = 115200;
  BspCOMInit.WordLength = COM_WORDLENGTH_8B;
  BspCOMInit.StopBits   = COM_STOPBITS_1;
  BspCOMInit.Parity     = COM_PARITY_NONE;
  BspCOMInit.HwFlowCtl  = COM_HWCONTROL_NONE;
  if (BSP_COM_Init(COM1, &BspCOMInit) != BSP_ERROR_NONE)
  {
    Error_Handler();
  }

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    // Delay for the measurement to complete
    bme69x_delay_us(meas_period_us, bme69x_sensor.intf_ptr);

    // Set sensor to forced mode again before reading
    rslt = bme69x_set_op_mode(BME69X_FORCED_MODE, &bme69x_sensor);
    if (rslt != BME69X_OK)
    {
        printf("BME69X Set Op Mode Failed (loop start): %d\r\n", rslt);
        snprintf(lcd_buffer, LCD_LINE_MAX_LEN + 1, "Mode Err (loop): %d", rslt);
        LCD_SetCursor(0, 0);
        LCD_PrintString(lcd_buffer);
        HAL_Delay(2000); // Wait before retrying
        continue; // Skip data read if mode setting failed
    }

    // Get sensor data. The 'n_fields' parameter is still required.
    rslt = bme69x_get_data(BME69X_FORCED_MODE, &bme69x_data, &n_fields, &bme69x_sensor);

    LCD_Clear(); // Clear LCD before printing new data in each loop iteration

    if (rslt == BME69X_OK)
    {
        // Removed specific validity mask checks, as they are undeclared.
        // Data is displayed if the overall read was successful.
        // The BME69X_GASM_VALID_MSK check is retained as it appeared to be declared.

        // Print Temperature
        snprintf(lcd_buffer, LCD_LINE_MAX_LEN + 1, "Temp: %.2f C", bme69x_data.temperature);
        LCD_SetCursor(3, 0); // Row 0, Col 0
        LCD_PrintString(lcd_buffer);
        printf("%s\r\n", lcd_buffer); // For serial debugging

        // Print Humidity
        snprintf(lcd_buffer, LCD_LINE_MAX_LEN + 1, "Hum : %.2f %%RH", bme69x_data.humidity);
        LCD_SetCursor(4, 0); // Row 1, Col 0
        LCD_PrintString(lcd_buffer);
        printf("%s\r\n", lcd_buffer);

        // Print Pressure
        snprintf(lcd_buffer, LCD_LINE_MAX_LEN + 1, "Pres: %.2f hPa", bme69x_data.pressure / 100);
        LCD_SetCursor(5, 0); // Row 2, Col 0
        LCD_PrintString(lcd_buffer);
        printf("%s\r\n", lcd_buffer);

        // Print Gas Resistance (keeping this check as BME69X_GASM_VALID_MSK seemed defined)
        if (bme69x_data.status & BME69X_GASM_VALID_MSK)
        {
            snprintf(lcd_buffer, LCD_LINE_MAX_LEN + 1, "Gas : %lu Ohm", (long unsigned int)bme69x_data.gas_resistance);
            LCD_SetCursor(6, 0); // Row 3, Col 0
            LCD_PrintString(lcd_buffer);
            printf("%s\r\n", lcd_buffer);
        } else {
             snprintf(lcd_buffer, LCD_LINE_MAX_LEN + 1, "Gas : N/A"); // No gas measurement or invalid
             LCD_SetCursor(6, 0);
             LCD_PrintString(lcd_buffer);
             printf("%s\r\n", lcd_buffer);
        }
    }
    else
    {
        printf("BME69X Get Data Failed: %d\r\n", rslt);
        snprintf(lcd_buffer, LCD_LINE_MAX_LEN + 1, "BME69X Data Error: %d", rslt);
        LCD_SetCursor(0, 0);
        LCD_PrintString(lcd_buffer);
    }

    HAL_Delay(2000); // Delay for 2 seconds before next measurement cycle
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMBOOST = RCC_PLLMBOOST_DIV1;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 10;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 1;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLLVCIRANGE_1;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_PCLK3;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Power Configuration
  * @retval None
  */
static void SystemPower_Config(void)
{
  HAL_PWREx_EnableVddIO2();

  /*
   * Disable the internal Pull-Up in Dead Battery pins of UCPD peripheral
   */
  HAL_PWREx_DisableUCPDDeadBattery();

  /*
   * Switch to SMPS regulator instead of LDO
   */
  if (HAL_PWREx_ConfigSupply(PWR_SMPS_SUPPLY) != HAL_OK)
  {
    Error_Handler();
  }
/* USER CODE BEGIN PWR */
/* USER CODE END PWR */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM17 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM17)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
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
    // Optionally, toggle an LED to indicate error
    // HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
    // HAL_Delay(100);
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
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
