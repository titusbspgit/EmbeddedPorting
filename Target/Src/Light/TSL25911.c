#include "TSL25911.h"

#define TSL2591_ADDR (0x29U << 1)
#define CMD 0xA0U
#define ENABLE 0x00U
#define CONTROL 0x01U
#define CHAN0_LOW 0x14U
#define CHAN0_HIGH 0x15U

static I2C_HandleTypeDef *s_hi2c = NULL;
static float s_buf[5] = {0};
static uint8_t s_idx = 0U;
static float s_latest_avg = 0.0f;

static HAL_StatusTypeDef tsl2591_read_lux(float *lux)
{
  /* Minimalistic read: power on and read channel 0 low/high, convert roughly */
  uint8_t data[2];
  data[0] = CMD | ENABLE; data[1] = 0x03U; /* PON | AEN */
  if (HAL_I2C_Master_Transmit(s_hi2c, TSL2591_ADDR, data, 2U, 20U) != HAL_OK) { return HAL_ERROR; }
  HAL_Delay(110U); /* integration time default */
  uint8_t reg = CMD | CHAN0_LOW;
  if (HAL_I2C_Master_Transmit(s_hi2c, TSL2591_ADDR, &reg, 1U, 20U) != HAL_OK) { return HAL_ERROR; }
  uint8_t rx[2] = {0};
  if (HAL_I2C_Master_Receive(s_hi2c, TSL2591_ADDR, rx, 2U, 20U) != HAL_OK) { return HAL_ERROR; }
  uint16_t ch0 = (uint16_t)(rx[0] | ((uint16_t)rx[1] << 8));
  /* Very rough conversion */
  *lux = (float)ch0 * 1.0f;
  return HAL_OK;
}

void LIGHT_TSL25911_Init(I2C_HandleTypeDef *hi2c)
{
  s_hi2c = hi2c;
}

void LIGHT_TaskStep(void)
{
  if (s_hi2c == NULL) { return; }
  float lux = s_latest_avg;
  (void)tsl2591_read_lux(&lux);
  s_buf[s_idx] = lux;
  s_idx = (uint8_t)((s_idx + 1U) % 5U);
  float sum = 0.0f; for (uint8_t i = 0U; i < 5U; ++i) { sum += s_buf[i]; }
  s_latest_avg = sum / 5.0f;
}

float LIGHT_GetLatestAvg(void)
{
  return s_latest_avg;
}
