/**
  ******************************************************************************
  * @file      startup_stm32f429xx.s
  * @brief     STM32F429xx Devices vector table for GCC based toolchains.
  ******************************************************************************
  */
  .syntax unified
  .cpu cortex-m4
  .fpu softvfp
  .thumb
.global  g_pfnVectors
.global  Default_Handler
.word  _sidata
.word  _sdata
.word  _edata
.word  _sbss
.word  _ebss
  .section  .text.Reset_Handler
  .weak  Reset_Handler
  .type  Reset_Handler, %function
Reset_Handler:
  ldr   sp, =_estack
  ldr r0, =_sdata
  ldr r1, =_edata
  ldr r2, =_sidata
  movs r3, #0
1:
  adds r4, r0, r3
  cmp r4, r1
  bcc 2f
  b 3f
2:
  ldr r5, [r2, r3]
  str r5, [r0, r3]
  adds r3, r3, #4
  b 1b
3:
  ldr r2, =_sbss
  ldr r4, =_ebss
  movs r3, #0
4:
  cmp r2, r4
  bcc 5f
  b 6f
5:
  str  r3, [r2]
  adds r2, r2, #4
  b 4b
6:
  bl  SystemInit
  bl __libc_init_array
  bl  main
  bx  lr
.size  Reset_Handler, .-Reset_Handler
  .section  .text.Default_Handler,"ax",%progbits
Default_Handler:
Infinite_Loop:
  b  Infinite_Loop
  .size  Default_Handler, .-Default_Handler
  .section  .isr_vector,"a",%progbits
  .type  g_pfnVectors, %object
  .size  g_pfnVectors, .-g_pfnVectors
g_pfnVectors:
  .word  _estack
  .word  Reset_Handler
  .word  NMI_Handler
  .word  HardFault_Handler
  .word  MemManage_Handler
  .word  BusFault_Handler
  .word  UsageFault_Handler
  .word  0,0,0,0
  .word  SVC_Handler
  .word  DebugMon_Handler
  .word  0
  .word  PendSV_Handler
  .word  SysTick_Handler
  .word  WWDG_IRQHandler, PVD_IRQHandler, TAMP_STAMP_IRQHandler, RTC_WKUP_IRQHandler
  .word  FLASH_IRQHandler, RCC_IRQHandler, EXTI0_IRQHandler, EXTI1_IRQHandler, EXTI2_IRQHandler, EXTI3_IRQHandler, EXTI4_IRQHandler
  .word  DMA1_Stream0_IRQHandler, DMA1_Stream1_IRQHandler, DMA1_Stream2_IRQHandler, DMA1_Stream3_IRQHandler, DMA1_Stream4_IRQHandler, DMA1_Stream5_IRQHandler, DMA1_Stream6_IRQHandler
  .word  ADC_IRQHandler, 0,0,0,0, EXTI9_5_IRQHandler, TIM1_BRK_TIM9_IRQHandler, TIM1_UP_TIM10_IRQHandler, TIM1_TRG_COM_TIM11_IRQHandler, TIM1_CC_IRQHandler
  .word  TIM2_IRQHandler, TIM3_IRQHandler, TIM4_IRQHandler, I2C1_EV_IRQHandler, I2C1_ER_IRQHandler, I2C2_EV_IRQHandler, I2C2_ER_IRQHandler
  .word  SPI1_IRQHandler, SPI2_IRQHandler, USART1_IRQHandler, USART2_IRQHandler, EXTI15_10_IRQHandler, RTC_Alarm_IRQHandler, OTG_FS_WKUP_IRQHandler
  .word  DMA1_Stream7_IRQHandler, SDIO_IRQHandler, TIM5_IRQHandler, SPI3_IRQHandler, DMA2_Stream0_IRQHandler, DMA2_Stream1_IRQHandler, DMA2_Stream2_IRQHandler, DMA2_Stream3_IRQHandler, DMA2_Stream4_IRQHandler
  .word  OTG_FS_IRQHandler, DMA2_Stream5_IRQHandler, DMA2_Stream6_IRQHandler, DMA2_Stream7_IRQHandler, USART6_IRQHandler, I2C3_EV_IRQHandler, I2C3_ER_IRQHandler, FPU_IRQHandler, SPI4_IRQHandler
