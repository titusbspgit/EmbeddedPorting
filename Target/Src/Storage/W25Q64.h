#ifndef W25Q64_H
#define W25Q64_H

#include <stdint.h>

void W25_Init(void);
void W25_ReadJEDEC(uint8_t *mid, uint8_t *memtype, uint8_t *capacity);
uint32_t W25_TotalSizeBytes(void);
int W25_LogJSON(float t, float h, float l, float p);

#endif /* W25Q64_H */
