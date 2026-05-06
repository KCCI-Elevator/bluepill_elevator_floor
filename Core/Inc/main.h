/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "hw_def.h"
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

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define DEF_LED_Pin GPIO_PIN_13
#define DEF_LED_GPIO_Port GPIOC
#define TS0224_A0_Pin GPIO_PIN_1
#define TS0224_A0_GPIO_Port GPIOA
#define TS0224_D0_Pin GPIO_PIN_2
#define TS0224_D0_GPIO_Port GPIOA
#define FLOOR_1_Pin GPIO_PIN_5
#define FLOOR_1_GPIO_Port GPIOA
#define FLOOR_2_Pin GPIO_PIN_6
#define FLOOR_2_GPIO_Port GPIOA
#define FLOOR_3_Pin GPIO_PIN_7
#define FLOOR_3_GPIO_Port GPIOA
#define OLED_D0_Pin GPIO_PIN_12
#define OLED_D0_GPIO_Port GPIOB
#define OLED_D1_Pin GPIO_PIN_13
#define OLED_D1_GPIO_Port GPIOB
#define OLED_RES_Pin GPIO_PIN_14
#define OLED_RES_GPIO_Port GPIOB
#define OLED_DC_Pin GPIO_PIN_15
#define OLED_DC_GPIO_Port GPIOB
#define OLED_CS_Pin GPIO_PIN_8
#define OLED_CS_GPIO_Port GPIOA
#define CAN_VCC_Pin GPIO_PIN_6
#define CAN_VCC_GPIO_Port GPIOB
#define CAN_GND_Pin GPIO_PIN_7
#define CAN_GND_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
