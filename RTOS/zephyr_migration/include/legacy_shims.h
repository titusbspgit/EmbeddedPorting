#ifndef LEGACY_SHIMS_H
#define LEGACY_SHIMS_H
/* Helpers to ease mental mapping from FreeRTOS calls in original app */
#include <zephyr/kernel.h>

#define vTaskDelay(ms_ticks) k_msleep((ms_ticks))
#define vTaskDelayUntil(last, period) do { k_msleep(period); } while (0)

#endif
