#include "SHT30-DIS.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <stdlib.h>

extern I2C_HandleTypeDef hi2c2;
HAL_StatusTypeDef SHT30_ReadTempHum(I2C_HandleTypeDef *hi2c, float *temperature, float *humidity);

typedef struct HumNode {
    float hum;
    struct HumNode *next;
} HumNode;

static HumNode *HumidityControlList = NULL;
static float latestHum = 0.0f;
static SemaphoreHandle_t humMutex;

float Humidity_GetLatest(void)
{
    float v;
    if (humMutex != NULL) { xSemaphoreTake(humMutex, portMAX_DELAY); }
    v = latestHum;
    if (humMutex != NULL) { (void)xSemaphoreGive(humMutex); }
    return v;
}

void HumidityMonitorTask(void *params)
{
    (void)params;
    humMutex = xSemaphoreCreateMutex();
    const TickType_t delay = pdMS_TO_TICKS(1000U);
    float buffer[5] = {0};
    int idx = 0;
    for(;;){
        float t=0.0f,h=0.0f;
        (void)SHT30_ReadTempHum(&hi2c2, &t, &h);
        buffer[idx++] = h;
        if (idx >= 5){
            idx = 0;
            float sum = 0.0f;
            for (int i=0;i<5;i++){ sum += buffer[i]; }
            float avg = sum/5.0f;
            HumNode *node = (HumNode*)pvPortMalloc(sizeof(HumNode));
            if (node != NULL){
                node->hum = avg;
                node->next = HumidityControlList;
                HumidityControlList = node;
            }
            if (humMutex != NULL) { xSemaphoreTake(humMutex, portMAX_DELAY); }
            latestHum = avg;
            if (humMutex != NULL) { (void)xSemaphoreGive(humMutex); }
        }
        vTaskDelay(delay);
    }
}