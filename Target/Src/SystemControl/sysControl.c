#include "sysControl.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdbool.h>
float Temperature_GetLatest(void); float Humidity_GetLatest(void); float Light_GetLatest(void); float Pressure_GetLatest(void);
static bool in_green(float t,float h,float p,float l){ return (t>=10.0f&&t<=20.0f&&h>=30.0f&&h<=40.0f&&p>=100.0f&&p<=200.0f&&l>=23000.0f&&l<=32000.0f);} 
static bool in_yellow(float t,float h,float p,float l){ return (t>=21.0f&&t<=30.0f&&h>=41.0f&&h<=50.0f&&p>=110.0f&&p<=120.0f&&l>=32000.0f&&l<=49000.0f);} 
static bool in_red(float t,float h,float p,float l){ return (t>=51.0f&&t<=60.0f&&h>=51.0f&&h<=60.0f&&p>=121.0f&&p<=130.0f&&l>=50000.0f&&l<=88000.0f);} 
void SystemControlTask(void *params)
{ (void)params; TickType_t vioStart=0U; bool inViolation=false; for(;;){ float t=Temperature_GetLatest(); float h=Humidity_GetLatest(); float l=Light_GetLatest(); float p=Pressure_GetLatest(); bool green=in_green(t,h,p,l); bool yellow=in_yellow(t,h,p,l); bool red=in_red(t,h,p,l); bool violation=!(green||yellow||red);
  if(violation){ if(!inViolation){ inViolation=true; vioStart=xTaskGetTickCount(); } TickType_t dt=xTaskGetTickCount()-vioStart; if(dt>pdMS_TO_TICKS(60000U)){ HAL_GPIO_TogglePin(GPIOD,GPIO_PIN_15); vTaskDelay(pdMS_TO_TICKS(300U)); } else if(dt>pdMS_TO_TICKS(30000U)){ HAL_GPIO_TogglePin(GPIOD,GPIO_PIN_15); vTaskDelay(pdMS_TO_TICKS(1000U)); } else { vTaskDelay(pdMS_TO_TICKS(200U)); } }
  else { inViolation=false; HAL_GPIO_WritePin(GPIOD,GPIO_PIN_15,GPIO_PIN_RESET); if(green){ HAL_GPIO_WritePin(GPIOD,GPIO_PIN_13,GPIO_PIN_SET); HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14|GPIO_PIN_12,GPIO_PIN_RESET);} else if(yellow){ HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14,GPIO_PIN_SET); HAL_GPIO_WritePin(GPIOD,GPIO_PIN_13|GPIO_PIN_12,GPIO_PIN_RESET);} else if(red){ HAL_GPIO_WritePin(GPIOD,GPIO_PIN_12,GPIO_PIN_SET); HAL_GPIO_WritePin(GPIOD,GPIO_PIN_13|GPIO_PIN_14,GPIO_PIN_RESET);} vTaskDelay(pdMS_TO_TICKS(500U)); } } }
