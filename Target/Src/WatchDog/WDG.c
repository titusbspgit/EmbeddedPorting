#include "WDG.h"
#include "FreeRTOS.h"
#include "task.h"
void WatchdogTask(void *params)
{
    (void)params;
    for(;;){ HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_9); vTaskDelay(pdMS_TO_TICKS(1000U)); }
}
