#ifndef HUM_SHT30_DIS_H
#define HUM_SHT30_DIS_H

#include "stm32f4xx_hal.h"

void HUM_SHT30_Init(I2C_HandleTypeDef *hi2c);
float HUM_GetLatestAvg(void);
void HUM_TaskStep(void);

#endif /* HUM_SHT30_DIS_H */
