#include "SHT30-DIS.h"
#include <string.h>

/* SHT30 I2C address */
#define SHT30_ADDR (0x44U << 1)

static I2C_HandleTypeDef *s_hi2c = NULL;
static float s_buf[5] = {0};
static uint8_t s_idx = 0U;
static float s_latest_avg = 0.0f;

static HAL_StatusTypeDef sht30_read_raw(uint16_t *t_raw, uint16_t *rh_raw)
{
  /* Single shot high repeatability, clock stretching disabled */
  uint8_t cmd[2] = {0x24U, 0x00U};
  if (HAL_I2C_Master_Transmit(s_hi2c, SHT30_ADDR, cmd, 2U, 50U) != HAL_OK) { return HAL_ERROR; }
  HAL_Delay(15U);
  uint8_t rx[6] = {0};
  if (HAL_I2C_Master_Receive(s_hi2c, SHT30_ADDR, rx, 6U, 50U) != HAL_OK) { return HAL_ERROR; }
  *t_raw = (uint16_t)((rx[0] << 8) | rx[1]);
  *rh_raw = (uint16_t)((rx[3] << 8) | rx[4]);
  return HAL_OK;
}

void HUM_SHT30_Init(I2C_HandleTypeDef *hi2c)
{
  s_hi2c = hi2c;
}

static float compute_rh(uint16_t rh_raw)
{
  /* From datasheet: RH = 100 * raw/65535 */
  return 100.0f * ((float)rh_raw / 65535.0f);
}

void HUM_TaskStep(void)
{
  if (s_hi2c == NULL) { return; }
  uint16_t t_raw = 0U, rh_raw = 0U; (void)t_raw;
  if (sht30_read_raw(&t_raw, &rh_raw) == HAL_OK)
  {
    float rh = compute_rh(rh_raw);
    s_buf[s_idx] = rh;
    s_idx = (uint8_t)((s_idx + 1U) % 5U);
    float sum = 0.0f; for (uint8_t i = 0U; i < 5U; ++i) { sum += s_buf[i]; }
    s_latest_avg = sum / 5.0f;
  }
}

float HUM_GetLatestAvg(void)
{
  return s_latest_avg;
}
