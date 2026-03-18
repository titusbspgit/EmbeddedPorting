#ifndef MAIN_H
#define MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include <stdint.h>

/* Peripheral handles (defined in main.c) */
extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c2;
extern SPI_HandleTypeDef hspi1;
extern TIM_HandleTypeDef htim3;

/* LED mappings: Discovery1 STM32F407VG */
#define LED_GREEN_GPIO_Port   GPIOD
#define LED_GREEN_Pin         GPIO_PIN_13
#define LED_YELLOW_GPIO_Port  GPIOD
#define LED_YELLOW_Pin        GPIO_PIN_14
#define LED_BLUE_GPIO_Port    GPIOD
#define LED_BLUE_Pin          GPIO_PIN_12
#define LED_RED_GPIO_Port     GPIOD
#define LED_RED_Pin           GPIO_PIN_15

/* Watchdog trigger GPIO */
#define WDG_TRIG_GPIO_Port    GPIOD
#define WDG_TRIG_Pin          GPIO_PIN_8

/* SPI NOR Flash Chip Select */
#define SPI1_CS_GPIO_Port     GPIOC
#define SPI1_CS_Pin           GPIO_PIN_4

/* Init functions */
void SystemClock_Config(void);
void MX_GPIO_Init(void);
void MX_I2C1_Init(void);
void MX_I2C2_Init(void);
void MX_SPI1_Init(void);
void MX_TIM3_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* MAIN_H */
