#include "WDG.h"
#include "FreeRTOS.h"
#include "task.h"

extern TIM_HandleTypeDef htim3;

void WatchdogTask(void *params)
{
    (void)params;
    const TickType_t delay = pdMS_TO_TICKS(1000U);
    for(;;){
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_8, GPIO_PIN_SET);
        __HAL_TIM_SET_AUTORELOAD(&htim3, 50000U - 1U); /* 5 seconds at 10 kHz */
        __HAL_TIM_SET_COUNTER(&htim3, 0U);
        vTaskDelay(delay);
    }
}