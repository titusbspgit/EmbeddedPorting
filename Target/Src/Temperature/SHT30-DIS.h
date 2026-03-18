#ifndef SHT30_DIS_TEMP_H
#define SHT30_DIS_TEMP_H
#include "stm32f4xx_hal.h"
HAL_StatusTypeDef SHT30_ReadTempHum(I2C_HandleTypeDef *hi2c, float *temperature, float *humidity);
void TemperatureMonitorTask(void *params);
float Temperature_GetLatest(void);
#endif
