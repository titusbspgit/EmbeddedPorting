#include "W25Q64.h"
#include "main.h"
#include <stdio.h>
#include <string.h>

#define CMD_READ_ID   0x9FU
#define CMD_WREN      0x06U
#define CMD_PP        0x02U
#define CMD_READ      0x03U
#define CMD_RDSR      0x05U

static uint32_t g_total_bytes = 8UL * 1024UL * 1024UL; /* default 64Mbit */
static uint32_t g_write_ptr = 0U;

static void cs_low(void)  { HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_RESET); }
static void cs_high(void) { HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_SET); }

static void spi_tx(const uint8_t *buf, uint16_t len)
{
  (void)HAL_SPI_Transmit(&hspi1, (uint8_t*)buf, len, 100U);
}

static void spi_rx(uint8_t *buf, uint16_t len)
{
  (void)HAL_SPI_Receive(&hspi1, buf, len, 100U);
}

static void write_enable(void)
{
  uint8_t cmd = CMD_WREN; cs_low(); spi_tx(&cmd, 1U); cs_high();
}

static void wait_wip_clear(void)
{
  uint8_t cmd = CMD_RDSR; uint8_t sr = 0x01U;
  do {
    cs_low(); spi_tx(&cmd, 1U); spi_rx(&sr, 1U); cs_high();
  } while ((sr & 0x01U) != 0U);
}

void W25_ReadJEDEC(uint8_t *mid, uint8_t *memtype, uint8_t *capacity)
{
  uint8_t cmd = CMD_READ_ID;
  uint8_t id[3] = {0};
  cs_low(); spi_tx(&cmd, 1U); spi_rx(id, 3U); cs_high();
  if (mid) { *mid = id[0]; }
  if (memtype) { *memtype = id[1]; }
  if (capacity) { *capacity = id[2]; }
  /* Capacity decode: 0x17 -> 64Mbit (8MB), 0x18 -> 128Mbit (16MB) */
  if (id[2] == 0x18U) { g_total_bytes = 16UL * 1024UL * 1024UL; } else { g_total_bytes = 8UL * 1024UL * 1024UL; }
}

uint32_t W25_TotalSizeBytes(void)
{
  return g_total_bytes;
}

void W25_Init(void)
{
  uint8_t m=0, t=0, c=0; W25_ReadJEDEC(&m, &t, &c);
  (void)m; (void)t; (void)c;
  g_write_ptr = 0U;
}

static void page_program(uint32_t addr, const uint8_t *data, uint16_t len)
{
  uint8_t cmd[4];
  cmd[0] = CMD_PP;
  cmd[1] = (uint8_t)(addr >> 16);
  cmd[2] = (uint8_t)(addr >> 8);
  cmd[3] = (uint8_t)(addr);
  write_enable();
  cs_low(); spi_tx(cmd, 4U); spi_tx(data, len); cs_high();
  wait_wip_clear();
}

int W25_LogJSON(float t, float h, float l, float p)
{
  char line[96];
  int n = snprintf(line, sizeof line, "{ \"temperature\": %.2f, \"humidity\": %.2f, \"light\": %.2f, \"pressure\": %.2f }\n", t, h, l, p);
  if (n <= 0) { return -1; }
  if ((uint32_t)n > g_total_bytes) { return -2; }
  if (g_write_ptr + (uint32_t)n >= g_total_bytes) { g_write_ptr = 0U; }
  page_program(g_write_ptr, (const uint8_t*)line, (uint16_t)n);
  g_write_ptr += (uint32_t)n;
  return 0;
}
