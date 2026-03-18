#include "TSL25911.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <stdlib.h>
extern I2C_HandleTypeDef hi2c1;
typedef struct LightNode { float light; struct LightNode *next; } LightNode;
static LightNode *LightControlList=NULL; static float latestLight=0.0f; static SemaphoreHandle_t lightMutex;
float Light_GetLatest(void){ float v; if(lightMutex!=NULL){ xSemaphoreTake(lightMutex,portMAX_DELAY);} v=latestLight; if(lightMutex!=NULL){ (void)xSemaphoreGive(lightMutex);} return v; }
void LightMonitorTask(void *params)
{ (void)params; lightMutex=xSemaphoreCreateMutex(); const TickType_t delay=pdMS_TO_TICKS(1000U); float buffer[5]={0}; int idx=0; for(;;){ uint8_t data[2]={0};
  (void)HAL_I2C_Mem_Read(&hi2c1,(0x29U<<1),0x14U,I2C_MEMADD_SIZE_8BIT,data,2U,100U); uint16_t rawLux=(uint16_t)((uint16_t)data[1]<<8)|data[0]; float lux=(float)rawLux; buffer[idx++]=lux; if(idx>=5){ idx=0; float sum=0.0f; for(int i=0;i<5;i++){sum+=buffer[i];}
  float avg=sum/5.0f; LightNode *node=(LightNode*)pvPortMalloc(sizeof(LightNode)); if(node!=NULL){ node->light=avg; node->next=LightControlList; LightControlList=node; } if(lightMutex!=NULL){ xSemaphoreTake(lightMutex,portMAX_DELAY);} latestLight=avg; if(lightMutex!=NULL){ (void)xSemaphoreGive(lightMutex);} }
  vTaskDelay(delay);} }
