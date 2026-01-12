#ifndef SHT30_DIS_HUM_H
#define SHT30_DIS_HUM_H
#include "stm32f4xx_hal.h"

void HumidityMonitorTask(void *params);
float Humidity_GetLatest(void);

#endif