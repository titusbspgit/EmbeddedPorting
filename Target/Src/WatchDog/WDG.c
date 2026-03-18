#include "WDG.h"
#include "main.h"

void WDG_Init(void)
{
  /* Nothing extra; pin configured in MX_GPIO_Init */
}

void WDG_Kick(void)
{
  /* Pulse PD8 */
  HAL_GPIO_WritePin(WDG_TRIG_GPIO_Port, WDG_TRIG_Pin, GPIO_PIN_SET);
  for (volatile uint32_t i = 0; i < 1000U; ++i) { __NOP(); }
  HAL_GPIO_WritePin(WDG_TRIG_GPIO_Port, WDG_TRIG_Pin, GPIO_PIN_RESET);
}

void WDG_SetReload5s(void)
{
  extern TIM_HandleTypeDef htim3;
  __disable_irq();
  __HAL_TIM_SET_AUTORELOAD(&htim3, 4999U); /* With 1 kHz tick -> 5 s */
  __HAL_TIM_SET_COUNTER(&htim3, 0U);
  __enable_irq();
}
