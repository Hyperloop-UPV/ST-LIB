/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.h
 * @brief          : Header for main.c file.
 *                   This file contains the common defines of the application.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2022 STMicroelectronics.
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
#include "stm32h7xx_hal.h"

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

void HAL_TIM_MspPostInit(TIM_HandleTypeDef* htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define MCO_Pin GPIO_PIN_0
#define MCO_GPIO_Port GPIOH
#define RMII_MDC_Pin GPIO_PIN_1
#define RMII_MDC_GPIO_Port GPIOC
#define RMII_REF_CLK_Pin GPIO_PIN_1
#define RMII_REF_CLK_GPIO_Port GPIOA
#define RMII_MDIO_Pin GPIO_PIN_2
#define RMII_MDIO_GPIO_Port GPIOA
#define RMII_CRS_DV_Pin GPIO_PIN_7
#define RMII_CRS_DV_GPIO_Port GPIOA
#define RMII_RXD0_Pin GPIO_PIN_4
#define RMII_RXD0_GPIO_Port GPIOC
#define RMII_RXD1_Pin GPIO_PIN_5
#define RMII_RXD1_GPIO_Port GPIOC
#define ADC_16BITB0_Pin GPIO_PIN_0
#define ADC_16BITB0_GPIO_Port GPIOB
#define ADC_16BITB1_Pin GPIO_PIN_1
#define ADC_16BITB1_GPIO_Port GPIOB
#define ADC_16BITF11_Pin GPIO_PIN_11
#define ADC_16BITF11_GPIO_Port GPIOF
#define ADC_16BITF12_Pin GPIO_PIN_12
#define ADC_16BITF12_GPIO_Port GPIOF
#define ADC_16BITF13_Pin GPIO_PIN_13
#define ADC_16BITF13_GPIO_Port GPIOF
#define ADC_16BITF14_Pin GPIO_PIN_14
#define ADC_16BITF14_GPIO_Port GPIOF
#define PWM_OUT_NE8_Pin GPIO_PIN_8
#define PWM_OUT_NE8_GPIO_Port GPIOE
#define PWM_OUTE9_Pin GPIO_PIN_9
#define PWM_OUTE9_GPIO_Port GPIOE
#define PWM_OUT_NE10_Pin GPIO_PIN_10
#define PWM_OUT_NE10_GPIO_Port GPIOE
#define PWM_OUTE11_Pin GPIO_PIN_11
#define PWM_OUTE11_GPIO_Port GPIOE
#define PWM_OUT_NE12_Pin GPIO_PIN_12
#define PWM_OUT_NE12_GPIO_Port GPIOE
#define PWM_OUTE13_Pin GPIO_PIN_13
#define PWM_OUTE13_GPIO_Port GPIOE
#define PWM_OUTE14_Pin GPIO_PIN_14
#define PWM_OUTE14_GPIO_Port GPIOE
#define RMII_TXD1_Pin GPIO_PIN_13
#define RMII_TXD1_GPIO_Port GPIOB
#define PWM_OUTB14_Pin GPIO_PIN_14
#define PWM_OUTB14_GPIO_Port GPIOB
#define PWM_OUTB15_Pin GPIO_PIN_15
#define PWM_OUTB15_GPIO_Port GPIOB
#define PWM_OUTD12_Pin GPIO_PIN_12
#define PWM_OUTD12_GPIO_Port GPIOD
#define PWM_OUTD13_Pin GPIO_PIN_13
#define PWM_OUTD13_GPIO_Port GPIOD
#define PWM_OUTD15_Pin GPIO_PIN_15
#define PWM_OUTD15_GPIO_Port GPIOD
#define ENCODER_1_Pin GPIO_PIN_6
#define ENCODER_1_GPIO_Port GPIOC
#define ENCODER_1C7_Pin GPIO_PIN_7
#define ENCODER_1C7_GPIO_Port GPIOC
#define PWM_OUTC8_Pin GPIO_PIN_8
#define PWM_OUTC8_GPIO_Port GPIOC
#define PWM_INC9_Pin GPIO_PIN_9
#define PWM_INC9_GPIO_Port GPIOC
#define SPI3_SS_Pin GPIO_PIN_3
#define SPI3_SS_GPIO_Port GPIOD
#define RMII_TX_EN_Pin GPIO_PIN_11
#define RMII_TX_EN_GPIO_Port GPIOG
#define RMII_TXD0_Pin GPIO_PIN_13
#define RMII_TXD0_GPIO_Port GPIOG
#define PWM_OUTB4_Pin GPIO_PIN_4
#define PWM_OUTB4_GPIO_Port GPIOB
#define PWM_OUTB5_Pin GPIO_PIN_5
#define PWM_OUTB5_GPIO_Port GPIOB
#define PWM_OUT_NB6_Pin GPIO_PIN_6
#define PWM_OUT_NB6_GPIO_Port GPIOB
#define PWM_OUT_NB7_Pin GPIO_PIN_7
#define PWM_OUT_NB7_GPIO_Port GPIOB
#define PWM_OUTB8_Pin GPIO_PIN_8
#define PWM_OUTB8_GPIO_Port GPIOB
#define PWM_OUTB9_Pin GPIO_PIN_9
#define PWM_OUTB9_GPIO_Port GPIOB
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB
#define SWDIO_Pin GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define SWCLK_Pin GPIO_PIN_14
#define SWCLK_GPIO_Port GPIOA
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
