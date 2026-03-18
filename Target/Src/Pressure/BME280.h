#ifndef BME280_H
#define BME280_H

#include "stm32f4xx_hal.h"

void PRESS_BME280_Init(I2C_HandleTypeDef *hi2c);
float PRESS_GetLatestAvg(void);
void PRESS_TaskStep(void);

#endif /* BME280_H */
