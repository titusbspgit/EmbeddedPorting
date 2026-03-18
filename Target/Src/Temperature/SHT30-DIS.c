#include "SHT30-DIS.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <stdlib.h>
extern I2C_HandleTypeDef hi2c1;
typedef struct TempNode { float temp; struct TempNode *next; } TempNode;
static TempNode *TemperatureControlList = NULL; static float latestTemp = 0.0f; static SemaphoreHandle_t tempMutex;
HAL_StatusTypeDef SHT30_ReadTempHum(I2C_HandleTypeDef *hi2c, float *temperature, float *humidity)
{ uint8_t cmd[2]={0x2C,0x06}; uint8_t data[6]={0}; (void)HAL_I2C_Master_Transmit(hi2c,(0x44U<<1),cmd,2U,100U); HAL_Delay(15U);
  (void)HAL_I2C_Master_Receive(hi2c,(0x44U<<1),data,6U,100U); uint16_t rawT=(uint16_t)((uint16_t)data[0]<<8)|data[1]; uint16_t rawH=(uint16_t)((uint16_t)data[3]<<8)|data[4];
  *temperature = -45.0f + 175.0f * ((float)rawT/65535.0f); *humidity = 100.0f * ((float)rawH/65535.0f); return HAL_OK; }
float Temperature_GetLatest(void){ float v; if(tempMutex!=NULL){ xSemaphoreTake(tempMutex,portMAX_DELAY);} v=latestTemp; if(tempMutex!=NULL){ (void)xSemaphoreGive(tempMutex);} return v; }
void TemperatureMonitorTask(void *params)
{ (void)params; tempMutex=xSemaphoreCreateMutex(); const TickType_t delay=pdMS_TO_TICKS(1000U); float buffer[5]={0}; int idx=0; for(;;){ float t=0.0f,h=0.0f; (void)SHT30_ReadTempHum(&hi2c1,&t,&h);
  buffer[idx++]=t; if(idx>=5){ idx=0; float sum=0.0f; for(int i=0;i<5;i++){ sum+=buffer[i]; } float avg=sum/5.0f; TempNode *node=(TempNode*)pvPortMalloc(sizeof(TempNode)); if(node!=NULL){ node->temp=avg; node->next=TemperatureControlList; TemperatureControlList=node; }
  if(tempMutex!=NULL){ xSemaphoreTake(tempMutex,portMAX_DELAY);} latestTemp=avg; if(tempMutex!=NULL){ (void)xSemaphoreGive(tempMutex);} } vTaskDelay(delay);} }
