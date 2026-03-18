#ifndef BME280_H
#define BME280_H
#include "stm32f4xx_hal.h"
void PressureMonitorTask(void *params);
float Pressure_GetLatest(void);
#endif
