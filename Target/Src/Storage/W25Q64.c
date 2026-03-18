#include "W25Q64.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

extern SPI_HandleTypeDef hspi1;

float Temperature_GetLatest(void);
float Humidity_GetLatest(void);
float Light_GetLatest(void);
float Pressure_GetLatest(void);

/* Flash CS on PA4 per dst mapping */
#define FLASH_CS_Port GPIOA
#define FLASH_CS_Pin  GPIO_PIN_4
static inline void FLASH_CS_L(void){ HAL_GPIO_WritePin(FLASH_CS_Port, FLASH_CS_Pin, GPIO_PIN_RESET); }
static inline void FLASH_CS_H(void){ HAL_GPIO_WritePin(FLASH_CS_Port, FLASH_CS_Pin, GPIO_PIN_SET); }

static void flash_write_enable(void)
{
    uint8_t cmd = 0x06U;
    FLASH_CS_L();
    (void)HAL_SPI_Transmit(&hspi1, &cmd, 1U, 100U);
    FLASH_CS_H();
}

static void flash_page_program(uint32_t addr, const uint8_t *data, uint16_t len)
{
    uint8_t hdr[4] = {0x02U, (uint8_t)(addr>>16), (uint8_t)(addr>>8), (uint8_t)(addr)};
    FLASH_CS_L();
    (void)HAL_SPI_Transmit(&hspi1, hdr, 4U, 100U);
    (void)HAL_SPI_Transmit(&hspi1, (uint8_t*)data, len, 1000U);
    FLASH_CS_H();
}

static bool in_green(float t,float h,float p,float l){
    return (t>=10.0f && t<=20.0f && h>=30.0f && h<=40.0f && p>=100.0f && p<=200.0f && l>=23000.0f && l<=32000.0f);
}
static bool in_yellow(float t,float h,float p,float l){
    return (t>=21.0f && t<=30.0f && h>=41.0f && h<=50.0f && p>=110.0f && p<=120.0f && l>=32000.0f && l<=49000.0f);
}
static bool in_red(float t,float h,float p,float l){
    return (t>=51.0f && t<=60.0f && h>=51.0f && h<=60.0f && p>=121.0f && p<=130.0f && l>=50000.0f && l<=88000.0f);
}

void StorageTask(void *params)
{
    (void)params;
    for(;;){
        float t = Temperature_GetLatest();
        float h = Humidity_GetLatest();
        float l = Light_GetLatest();
        float p = Pressure_GetLatest();

        bool safe = in_green(t,h,p,l) || in_yellow(t,h,p,l) || in_red(t,h,p,l);
        if (!safe){
            char json[160];
            (void)snprintf(json, sizeof(json),
                "{\"temperature\":%.2f,\"humidity\":%.2f,\"light\":%.0f,\"pressure\":%.2f}", t,h,l,p);
            flash_write_enable();
            flash_page_program(0x000000U, (const uint8_t*)json, (uint16_t)strlen(json));
        }
        vTaskDelay(pdMS_TO_TICKS(1000U));
    }
}
