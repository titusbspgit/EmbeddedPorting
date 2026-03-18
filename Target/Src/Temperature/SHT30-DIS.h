#ifndef TEMP_SHT30_DIS_H
#define TEMP_SHT30_DIS_H

#include "stm32f4xx_hal.h"

void TEMP_SHT30_Init(I2C_HandleTypeDef *hi2c);
float TEMP_GetLatestAvg(void);
void TEMP_TaskStep(void);

#endif /* TEMP_SHT30_DIS_H */
