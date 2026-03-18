#ifndef TSL25911_H
#define TSL25911_H

#include "stm32f4xx_hal.h"

void LIGHT_TSL25911_Init(I2C_HandleTypeDef *hi2c);
float LIGHT_GetLatestAvg(void);
void LIGHT_TaskStep(void);

#endif /* TSL25911_H */
