/* STM32F407xx startup for GCC */
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
  /* Copy .data from flash to SRAM */
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
  /* Zero .bss */
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
  bl  __libc_init_array
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

.extern NMI_Handler
.extern HardFault_Handler
.extern MemManage_Handler
.extern BusFault_Handler
.extern UsageFault_Handler
.extern SVC_Handler
.extern DebugMon_Handler
.extern PendSV_Handler
.extern SysTick_Handler
.extern TIM3_IRQHandler

.g_pfnVectors:
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
  /* External interrupts (subset; rest default) */
  .word  WWDG_IRQHandler
  .word  PVD_IRQHandler
  .word  TAMP_STAMP_IRQHandler
  .word  RTC_WKUP_IRQHandler
  .word  FLASH_IRQHandler
  .word  RCC_IRQHandler
  .word  EXTI0_IRQHandler
  .word  EXTI1_IRQHandler
  .word  EXTI2_IRQHandler
  .word  EXTI3_IRQHandler
  .word  EXTI4_IRQHandler
  .word  DMA1_Stream0_IRQHandler
  .word  DMA1_Stream1_IRQHandler
  .word  DMA1_Stream2_IRQHandler
  .word  DMA1_Stream3_IRQHandler
  .word  DMA1_Stream4_IRQHandler
  .word  DMA1_Stream5_IRQHandler
  .word  DMA1_Stream6_IRQHandler
  .word  ADC_IRQHandler
  .word  CAN1_TX_IRQHandler
  .word  CAN1_RX0_IRQHandler
  .word  CAN1_RX1_IRQHandler
  .word  CAN1_SCE_IRQHandler
  .word  EXTI9_5_IRQHandler
  .word  TIM1_BRK_TIM9_IRQHandler
  .word  TIM1_UP_TIM10_IRQHandler
  .word  TIM1_TRG_COM_TIM11_IRQHandler
  .word  TIM1_CC_IRQHandler
  .word  TIM2_IRQHandler
  .word  TIM3_IRQHandler
  .word  TIM4_IRQHandler
  .word  I2C1_EV_IRQHandler
  .word  I2C1_ER_IRQHandler
  .word  I2C2_EV_IRQHandler
  .word  I2C2_ER_IRQHandler
  .word  SPI1_IRQHandler
  .word  SPI2_IRQHandler
  .word  USART1_IRQHandler
  .word  USART2_IRQHandler
  .word  USART3_IRQHandler
  .word  EXTI15_10_IRQHandler
  .word  RTC_Alarm_IRQHandler
  .word  OTG_FS_WKUP_IRQHandler
  .word  TIM8_BRK_TIM12_IRQHandler
  .word  TIM8_UP_TIM13_IRQHandler
  .word  TIM8_TRG_COM_TIM14_IRQHandler
  .word  TIM8_CC_IRQHandler
  .word  DMA1_Stream7_IRQHandler
  .word  FSMC_IRQHandler
  .word  SDIO_IRQHandler
  .word  TIM5_IRQHandler
  .word  SPI3_IRQHandler
  .word  UART4_IRQHandler
  .word  UART5_IRQHandler
  .word  TIM6_DAC_IRQHandler
  .word  TIM7_IRQHandler
  .word  DMA2_Stream0_IRQHandler
  .word  DMA2_Stream1_IRQHandler
  .word  DMA2_Stream2_IRQHandler
  .word  DMA2_Stream3_IRQHandler
  .word  DMA2_Stream4_IRQHandler
  .word  ETH_IRQHandler
  .word  ETH_WKUP_IRQHandler
  .word  CAN2_TX_IRQHandler
  .word  CAN2_RX0_IRQHandler
  .word  CAN2_RX1_IRQHandler
  .word  CAN2_SCE_IRQHandler
  .word  OTG_FS_IRQHandler
  .word  DMA2_Stream5_IRQHandler
  .word  DMA2_Stream6_IRQHandler
  .word  DMA2_Stream7_IRQHandler
  .word  USART6_IRQHandler
  .word  I2C3_EV_IRQHandler
  .word  I2C3_ER_IRQHandler
  .word  OTG_HS_EP1_OUT_IRQHandler
  .word  OTG_HS_EP1_IN_IRQHandler
  .word  OTG_HS_WKUP_IRQHandler
  .word  OTG_HS_IRQHandler
  .word  DCMI_IRQHandler
  .word  CRYP_IRQHandler
  .word  HASH_RNG_IRQHandler
  .word  FPU_IRQHandler

/* Weak aliases to Default_Handler for unimplemented ISRs */
.macro WEAK handler
  .weak \handler
  .set  \handler, Default_Handler
.endm

WEAK(NMI_Handler)
WEAK(HardFault_Handler)
WEAK(MemManage_Handler)
WEAK(BusFault_Handler)
WEAK(UsageFault_Handler)
WEAK(SVC_Handler)
WEAK(DebugMon_Handler)
WEAK(PendSV_Handler)
WEAK(SysTick_Handler)
WEAK(WWDG_IRQHandler)
WEAK(PVD_IRQHandler)
/* ... other IRQs weak as needed ... */
WEAK(TIM3_IRQHandler)
