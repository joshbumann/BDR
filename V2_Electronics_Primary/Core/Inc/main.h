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
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "string.h"
#include "stdbool.h"

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
#define LC2_A_Pin GPIO_PIN_0
#define LC2_A_GPIO_Port GPIOC
#define LC2_B_Pin GPIO_PIN_1
#define LC2_B_GPIO_Port GPIOC
#define LC3_A_Pin GPIO_PIN_2
#define LC3_A_GPIO_Port GPIOC
#define LC3_B_Pin GPIO_PIN_3
#define LC3_B_GPIO_Port GPIOC
#define PT_Channel1_Pin GPIO_PIN_0
#define PT_Channel1_GPIO_Port GPIOA
#define PT_Channel2_Pin GPIO_PIN_1
#define PT_Channel2_GPIO_Port GPIOA
#define PT_Channel3_Pin GPIO_PIN_4
#define PT_Channel3_GPIO_Port GPIOA
#define PT_Channel4_Pin GPIO_PIN_5
#define PT_Channel4_GPIO_Port GPIOA
#define PT_Channel5_Pin GPIO_PIN_6
#define PT_Channel5_GPIO_Port GPIOA
#define PT_Channel6_Pin GPIO_PIN_7
#define PT_Channel6_GPIO_Port GPIOA
#define LC1_A_Pin GPIO_PIN_0
#define LC1_A_GPIO_Port GPIOB
#define LC1_B_Pin GPIO_PIN_1
#define LC1_B_GPIO_Port GPIOB
#define RX_EN_Pin GPIO_PIN_9
#define RX_EN_GPIO_Port GPIOC
#define TX_EN_Pin GPIO_PIN_8
#define TX_EN_GPIO_Port GPIOA
#define LOX_Purge_Vlv_Pin GPIO_PIN_11
#define LOX_Purge_Vlv_GPIO_Port GPIOC
#define Fuel_Purge_Vlv_Pin GPIO_PIN_12
#define Fuel_Purge_Vlv_GPIO_Port GPIOC
#define LOX_Vent_Vlv_Pin GPIO_PIN_2
#define LOX_Vent_Vlv_GPIO_Port GPIOD
#define Fuel_Vent_Vlv_Pin GPIO_PIN_3
#define Fuel_Vent_Vlv_GPIO_Port GPIOB
#define MOV_Vlv_Pin GPIO_PIN_4
#define MOV_Vlv_GPIO_Port GPIOB
#define MFV_Vlv_Pin GPIO_PIN_5
#define MFV_Vlv_GPIO_Port GPIOB
#define LOX_N2_Vlv_Pin GPIO_PIN_6
#define LOX_N2_Vlv_GPIO_Port GPIOB
#define Fuel_N2_Vlv_Pin GPIO_PIN_7
#define Fuel_N2_Vlv_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
#define NUM_VALVES 8

typedef enum {
	Idle = 0,
	ColdFlowState,
	HotFireState,

} HFstate;

extern uint8_t currentState; // Keeps track of current engine state

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
