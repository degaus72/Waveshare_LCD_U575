/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32u5xx_hal.h"
#include "stm32u5xx_nucleo.h"
#include <stdio.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void Delay_us(uint32_t us); // Add this line
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define ST7789_RST_Pin GPIO_PIN_0
#define ST7789_RST_GPIO_Port GPIOC
#define INT_Pin GPIO_PIN_1
#define INT_GPIO_Port GPIOC
#define INT_EXTI_IRQn EXTI1_IRQn
#define TP_RST_Pin GPIO_PIN_2
#define TP_RST_GPIO_Port GPIOC
#define ST7789_CS_Pin GPIO_PIN_4
#define ST7789_CS_GPIO_Port GPIOA
#define ST7789_DC_Pin GPIO_PIN_0
#define ST7789_DC_GPIO_Port GPIOB
#define ST7789_BL_Pin GPIO_PIN_1
#define ST7789_BL_GPIO_Port GPIOB
#define DHT11_DATA_Pin GPIO_PIN_8
#define DHT11_DATA_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
