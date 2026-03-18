#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "SystemControl/sysControl.h"
#include "WatchDog/WDG.h"
#include "Temperature/SHT30-DIS.h"
#include "Humidity/SHT30-DIS.h"
#include "Light/TSL25911.h"
#include "Pressure/BME280.h"

/* Task prototypes */
static void TemperatureTask(void *arg);
static void HumidityTask(void *arg);
static void LightTask(void *arg);
static void PressureTask(void *arg);
static void SystemControlTask(void *arg);
static void WatchdogLoaderTask(void *arg);

void MX_FREERTOS_Init(void)
{
  (void)xTaskCreate(TemperatureTask, "Temp", 256U, NULL, tskIDLE_PRIORITY + 2U, NULL);
  (void)xTaskCreate(HumidityTask,    "Hum",  256U, NULL, tskIDLE_PRIORITY + 2U, NULL);
  (void)xTaskCreate(LightTask,       "Light",256U, NULL, tskIDLE_PRIORITY + 2U, NULL);
  (void)xTaskCreate(PressureTask,    "Press",256U, NULL, tskIDLE_PRIORITY + 2U, NULL);
  (void)xTaskCreate(SystemControlTask,"Sys", 384U, NULL, tskIDLE_PRIORITY + 1U, NULL);
  (void)xTaskCreate(WatchdogLoaderTask,"WDGL",256U, NULL, tskIDLE_PRIORITY + 0U, NULL);
}

static void TemperatureTask(void *arg)
{
  (void)arg;
  for(;;)
  {
    TEMP_TaskStep();
    vTaskDelay(pdMS_TO_TICKS(1000U));
  }
}

static void HumidityTask(void *arg)
{
  (void)arg;
  for(;;)
  {
    HUM_TaskStep();
    vTaskDelay(pdMS_TO_TICKS(1000U));
  }
}

static void LightTask(void *arg)
{
  (void)arg;
  for(;;)
  {
    LIGHT_TaskStep();
    vTaskDelay(pdMS_TO_TICKS(1000U));
  }
}

static void PressureTask(void *arg)
{
  (void)arg;
  for(;;)
  {
    PRESS_TaskStep();
    vTaskDelay(pdMS_TO_TICKS(1000U));
  }
}

static void SystemControlTask(void *arg)
{
  (void)arg;
  for(;;)
  {
    SYSCTRL_Update();
    vTaskDelay(pdMS_TO_TICKS(200U));
  }
}

static void WatchdogLoaderTask(void *arg)
{
  (void)arg;
  for(;;)
  {
    /* Periodically force 5 s reload configuration as a low-priority maintenance */
    WDG_SetReload5s();
    vTaskDelay(pdMS_TO_TICKS(5000U));
  }
}
