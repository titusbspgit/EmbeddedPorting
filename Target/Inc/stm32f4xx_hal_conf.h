/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f4xx_hal_conf_template.h
  * @author  MCD Application Team
  * @brief   HAL configuration template file.
  *          This file should be copied to the application folder and renamed
  *          to stm32f4xx_hal_conf.h.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
#ifndef __STM32F4xx_HAL_CONF_H
#define __STM32F4xx_HAL_CONF_H
#ifdef __cplusplus
 extern "C" {
#endif
#define HAL_MODULE_ENABLED
/* #define HAL_ADC_MODULE_ENABLED */
#define HAL_I2C_MODULE_ENABLED
#define HAL_SPI_MODULE_ENABLED
#define HAL_TIM_MODULE_ENABLED
#define HAL_GPIO_MODULE_ENABLED
#define HAL_EXTI_MODULE_ENABLED
#define HAL_DMA_MODULE_ENABLED
#define HAL_RCC_MODULE_ENABLED
#define HAL_FLASH_MODULE_ENABLED
#define HAL_PWR_MODULE_ENABLED
#define HAL_CORTEX_MODULE_ENABLED
#if !defined  (HSE_VALUE)
  #define HSE_VALUE    8000000U
#endif
#if !defined  (HSE_STARTUP_TIMEOUT)
  #define HSE_STARTUP_TIMEOUT    100U
#endif
#if !defined  (HSI_VALUE)
  #define HSI_VALUE    ((uint32_t)16000000U)
#endif
#if !defined  (LSI_VALUE)
 #define LSI_VALUE  32000U
#endif
#if !defined  (LSE_VALUE)
 #define LSE_VALUE  32768U
#endif
#if !defined  (LSE_STARTUP_TIMEOUT)
  #define LSE_STARTUP_TIMEOUT    5000U
#endif
#if !defined  (EXTERNAL_CLOCK_VALUE)
  #define EXTERNAL_CLOCK_VALUE    12288000U
#endif
#define  VDD_VALUE		      3300U
#define  TICK_INT_PRIORITY            15U
#define  USE_RTOS                     0U
#define  PREFETCH_ENABLE              1U
#define  INSTRUCTION_CACHE_ENABLE     1U
#define  DATA_CACHE_ENABLE            1U
#define  USE_HAL_I2C_REGISTER_CALLBACKS         0U
#define  USE_HAL_SPI_REGISTER_CALLBACKS         0U
#define  USE_HAL_TIM_REGISTER_CALLBACKS         0U
#ifdef HAL_RCC_MODULE_ENABLED
  #include "stm32f4xx_hal_rcc.h"
#endif
#ifdef HAL_GPIO_MODULE_ENABLED
  #include "stm32f4xx_hal_gpio.h"
#endif
#ifdef HAL_EXTI_MODULE_ENABLED
  #include "stm32f4xx_hal_exti.h"
#endif
#ifdef HAL_DMA_MODULE_ENABLED
  #include "stm32f4xx_hal_dma.h"
#endif
#ifdef HAL_CORTEX_MODULE_ENABLED
  #include "stm32f4xx_hal_cortex.h"
#endif
#ifdef HAL_ADC_MODULE_ENABLED
  #include "stm32f4xx_hal_adc.h"
#endif
#ifdef HAL_I2C_MODULE_ENABLED
 #include "stm32f4xx_hal_i2c.h"
#endif
#ifdef HAL_SPI_MODULE_ENABLED
 #include "stm32f4xx_hal_spi.h"
#endif
#ifdef HAL_TIM_MODULE_ENABLED
 #include "stm32f4xx_hal_tim.h"
#endif
#ifdef __cplusplus
}
#endif
#endif /* __STM32F4xx_HAL_CONF_H */
