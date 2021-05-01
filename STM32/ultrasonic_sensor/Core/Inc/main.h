/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
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
#include "stm32f4xx_hal.h"

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

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define Trig5_Pin GPIO_PIN_0
#define Trig5_GPIO_Port GPIOA
#define Trig6_Pin GPIO_PIN_1
#define Trig6_GPIO_Port GPIOA
#define Trig7_Pin GPIO_PIN_2
#define Trig7_GPIO_Port GPIOA
#define Trig3_Pin GPIO_PIN_0
#define Trig3_GPIO_Port GPIOB
#define Trig4_Pin GPIO_PIN_1
#define Trig4_GPIO_Port GPIOB
#define Echo1_Pin GPIO_PIN_9
#define Echo1_GPIO_Port GPIOE
#define Echo2_Pin GPIO_PIN_11
#define Echo2_GPIO_Port GPIOE
#define Echo3_Pin GPIO_PIN_13
#define Echo3_GPIO_Port GPIOE
#define Echo4_Pin GPIO_PIN_14
#define Echo4_GPIO_Port GPIOE
#define Echo5_Pin GPIO_PIN_6
#define Echo5_GPIO_Port GPIOC
#define Echo6_Pin GPIO_PIN_8
#define Echo6_GPIO_Port GPIOC
#define Echo7_Pin GPIO_PIN_9
#define Echo7_GPIO_Port GPIOC
#define Trig1_Pin GPIO_PIN_4
#define Trig1_GPIO_Port GPIOB
#define Trig2_Pin GPIO_PIN_5
#define Trig2_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
