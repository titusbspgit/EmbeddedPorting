/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f4xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  */
/* USER CODE END Header */
#include "main.h"
#include "stm32f4xx_it.h"
#include "FreeRTOS.h"
#include "task.h"

extern DMA_HandleTypeDef hdma_i2c1_rx;
extern DMA_HandleTypeDef hdma_i2c1_tx;
extern DMA_HandleTypeDef hdma_spi1_rx;
extern DMA_HandleTypeDef hdma_spi1_tx;
extern I2C_HandleTypeDef hi2c1;
extern SPI_HandleTypeDef hspi1;

void NMI_Handler(void){ while (1){} }
void HardFault_Handler(void){ while (1){} }
void MemManage_Handler(void){ while (1){} }
void BusFault_Handler(void){ while (1){} }
void UsageFault_Handler(void){ while (1){} }
void DebugMon_Handler(void){ }

void SysTick_Handler(void)
{
  HAL_IncTick();
#if (INCLUDE_xTaskGetSchedulerState == 1 )
  if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
  {
#endif
  xPortSysTickHandler();
#if (INCLUDE_xTaskGetSchedulerState == 1 )
  }
#endif
}

/* I2C1 event/error */
void I2C1_EV_IRQHandler(void) { HAL_I2C_EV_IRQHandler(&hi2c1); }
void I2C1_ER_IRQHandler(void) { HAL_I2C_ER_IRQHandler(&hi2c1); }

/* SPI1 global */
void SPI1_IRQHandler(void) { HAL_SPI_IRQHandler(&hspi1); }

/* DMA1 Streams (I2C1) */
void DMA1_Stream0_IRQHandler(void) { HAL_DMA_IRQHandler(&hdma_i2c1_rx); }
void DMA1_Stream6_IRQHandler(void) { HAL_DMA_IRQHandler(&hdma_i2c1_tx); }

/* DMA2 Streams (SPI1) */
void DMA2_Stream2_IRQHandler(void) { HAL_DMA_IRQHandler(&hdma_spi1_rx); }
void DMA2_Stream3_IRQHandler(void) { HAL_DMA_IRQHandler(&hdma_spi1_tx); }
