#include "BME280.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <stdlib.h>
extern I2C_HandleTypeDef hi2c1;
typedef struct PressNode { float pressure; struct PressNode *next; } PressNode;
static PressNode *PressureControlList=NULL; static float latestPress=0.0f; static SemaphoreHandle_t pressMutex;
float Pressure_GetLatest(void){ float v; if(pressMutex!=NULL){ xSemaphoreTake(pressMutex,portMAX_DELAY);} v=latestPress; if(pressMutex!=NULL){ (void)xSemaphoreGive(pressMutex);} return v; }
void PressureMonitorTask(void *params)
{ (void)params; pressMutex=xSemaphoreCreateMutex(); const TickType_t delay=pdMS_TO_TICKS(1000U); float buffer[5]={0}; int idx=0; for(;;){ uint8_t data[3]={0}; (void)HAL_I2C_Mem_Read(&hi2c1,(0x76U<<1),0xF7U,I2C_MEMADD_SIZE_8BIT,data,3U,100U);
  uint32_t rawPress=((uint32_t)data[0]<<12)|((uint32_t)data[1]<<4)|(data[2]>>4); float press=(float)rawPress; buffer[idx++]=press; if(idx>=5){ idx=0; float sum=0.0f; for(int i=0;i<5;i++){ sum+=buffer[i]; }
  float avg=sum/5.0f; PressNode *node=(PressNode*)pvPortMalloc(sizeof(PressNode)); if(node!=NULL){ node->pressure=avg; node->next=PressureControlList; PressureControlList=node; } if(pressMutex!=NULL){ xSemaphoreTake(pressMutex,portMAX_DELAY);} latestPress=avg; if(pressMutex!=NULL){ (void)xSemaphoreGive(pressMutex);} }
  vTaskDelay(delay);} }
