#ifndef TSL25911_H
#define TSL25911_H
#include "stm32f4xx_hal.h"
void LightMonitorTask(void *params);
float Light_GetLatest(void);
#endif
