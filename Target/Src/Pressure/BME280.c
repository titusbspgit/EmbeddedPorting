#include "BME280.h"

#define BME280_ADDR (0x76U << 1)

static I2C_HandleTypeDef *s_hi2c = NULL;
static float s_buf[5] = {0};
static uint8_t s_idx = 0U;
static float s_latest_avg = 100.0f; /* start in green band */

static HAL_StatusTypeDef bme280_read_pressure(float *press)
{
  /* Placeholder: in real code, configure and read uncompensated pressure and apply calibration. */
  (void)press;
  return HAL_OK;
}

void PRESS_BME280_Init(I2C_HandleTypeDef *hi2c)
{
  s_hi2c = hi2c;
}

void PRESS_TaskStep(void)
{
  if (s_hi2c == NULL) { return; }
  float press = s_latest_avg;
  (void)bme280_read_pressure(&press);
  s_buf[s_idx] = press;
  s_idx = (uint8_t)((s_idx + 1U) % 5U);
  float sum = 0.0f; for (uint8_t i = 0U; i < 5U; ++i) { sum += s_buf[i]; }
  s_latest_avg = sum / 5.0f;
}

float PRESS_GetLatestAvg(void)
{
  return s_latest_avg;
}
