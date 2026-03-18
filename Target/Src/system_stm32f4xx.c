/* Minimal system_stm32f4xx.c ensuring SystemCoreClock=168MHz configured in SystemClock_Config */
#include "stm32f4xx.h"

uint32_t SystemCoreClock = 16000000U; /* default HSI */

void SystemInit(void)
{
  /* FPU settings */
#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
  SCB->CPACR |= ((3UL << 10*2)|(3UL << 11*2));
#endif
  /* Reset the RCC clock configuration to the default reset state */
  RCC->CR |= (uint32_t)0x00000001U;
  RCC->CFGR = 0x00000000U;
  RCC->CR &= (uint32_t)0xFEF6FFFFU;
  RCC->PLLCFGR = 0x24003010U;
  RCC->CR &= (uint32_t)0xFFFBFFFFU;
  RCC->CIR = 0x00000000U;
}

void SystemCoreClockUpdate(void)
{
  /* Rely on HAL RCC state set in SystemClock_Config. Read back HCLK. */
  SystemCoreClock = HAL_RCC_GetHCLKFreq();
}
