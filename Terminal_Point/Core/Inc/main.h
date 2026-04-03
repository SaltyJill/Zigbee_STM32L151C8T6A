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
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l1xx_hal.h"

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
#define LED_Pin GPIO_PIN_13
#define LED_GPIO_Port GPIOC
#define RST_Pin GPIO_PIN_2
#define RST_GPIO_Port GPIOB
#define CFG_Pin GPIO_PIN_3
#define CFG_GPIO_Port GPIOB
#define SLP_Pin GPIO_PIN_4
#define SLP_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
#define led_on() HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET)
#define led_off() HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET)
#define cfg_enter() HAL_GPIO_WritePin(CFG_GPIO_Port, CFG_Pin, GPIO_PIN_RESET)
#define cfg_exit() HAL_GPIO_WritePin(CFG_GPIO_Port, CFG_Pin, GPIO_PIN_SET)
#define slp_enter() HAL_GPIO_WritePin(SLP_GPIO_Port, SLP_Pin, GPIO_PIN_RESET)
#define slp_exit() HAL_GPIO_WritePin(SLP_GPIO_Port, SLP_Pin, GPIO_PIN_SET)
    /* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
