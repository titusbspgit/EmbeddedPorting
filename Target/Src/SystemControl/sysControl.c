#include "sysControl.h"
#include "main.h"
#include "Storage/W25Q64.h"
#include "FreeRTOS.h"
#include "task.h"

/* Thresholds per HighLevelrequirements.txt */
static bool in_green(float t, float h, float l, float p)
{
  return ((t >= 10.0f && t <= 20.0f) &&
          (h >= 30.0f && h <= 40.0f) &&
          (p >= 100.0f && p <= 200.0f) &&
          (l >= 23000.0f && l <= 32000.0f));
}

static bool in_yellow_any(float t, float h, float l, float p)
{
  return ((t >= 21.0f && t <= 30.0f) ||
          (h >= 41.0f && h <= 50.0f) ||
          (p >= 110.0f && p <= 120.0f) ||
          (l >= 32000.0f && l <= 49000.0f));
}

static bool in_red_any(float t, float h, float l, float p)
{
  return ((t >= 51.0f && t <= 60.0f) ||
          (h >= 51.0f && h <= 60.0f) ||
          (p >= 121.0f && p <= 130.0f) ||
          (l >= 50000.0f && l <= 88000.0f));
}

static TickType_t violation_since = 0U;
static bool violation_active = false;

void SYSCTRL_Init(void)
{
  /* Default LED state off */
  HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LED_YELLOW_GPIO_Port, LED_YELLOW_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin, GPIO_PIN_RESET);
  violation_since = 0U;
  violation_active = false;
}

/* External getters from modules */
extern float TEMP_GetLatestAvg(void);
extern float HUM_GetLatestAvg(void);
extern float LIGHT_GetLatestAvg(void);
extern float PRESS_GetLatestAvg(void);

void SYSCTRL_LogViolation(float t, float h, float l, float p)
{
  W25_LogJSON(t, h, l, p);
}

void SYSCTRL_Update(void)
{
  float t = TEMP_GetLatestAvg();
  float h = HUM_GetLatestAvg();
  float l = LIGHT_GetLatestAvg();
  float p = PRESS_GetLatestAvg();

  if (in_green(t, h, l, p))
  {
    HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED_YELLOW_GPIO_Port, LED_YELLOW_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_RESET);
    violation_active = false;
    violation_since = 0U;
  }
  else if (in_red_any(t, h, l, p))
  {
    HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_YELLOW_GPIO_Port, LED_YELLOW_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_SET);

    if (!violation_active)
    {
      violation_active = true;
      violation_since = xTaskGetTickCount();
      SYSCTRL_LogViolation(t, h, l, p);
    }
  }
  else if (in_yellow_any(t, h, l, p))
  {
    HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_YELLOW_GPIO_Port, LED_YELLOW_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_RESET);

    if (!violation_active)
    {
      violation_active = true;
      violation_since = xTaskGetTickCount();
      SYSCTRL_LogViolation(t, h, l, p);
    }
  }
  else
  {
    /* Out of specified ranges but not matching yellow/red explicitly: treat as violation */
    HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET);
    HAL_GPIO_TogglePin(LED_YELLOW_GPIO_Port, LED_YELLOW_Pin);
    if (!violation_active)
    {
      violation_active = true;
      violation_since = xTaskGetTickCount();
      SYSCTRL_LogViolation(t, h, l, p);
    }
  }

  if (violation_active)
  {
    TickType_t elapsed = xTaskGetTickCount() - violation_since;
    if (elapsed >= pdMS_TO_TICKS(60000U))
    {
      /* Blink fault LED at 300 ms */
      static TickType_t last = 0U;
      if ((xTaskGetTickCount() - last) >= pdMS_TO_TICKS(300U))
      {
        HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);
        last = xTaskGetTickCount();
      }
    }
    else if (elapsed >= pdMS_TO_TICKS(30000U))
    {
      /* Blink fault LED at 1 s */
      static TickType_t last1 = 0U;
      if ((xTaskGetTickCount() - last1) >= pdMS_TO_TICKS(1000U))
      {
        HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);
        last1 = xTaskGetTickCount();
      }
    }
  }
}
