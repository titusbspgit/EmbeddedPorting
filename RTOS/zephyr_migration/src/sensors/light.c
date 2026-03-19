#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/i2c.h>
#include "light.h"
#include "avg_list.h"
#include "app_types.h"

LOG_MODULE_REGISTER(s_light, LOG_LEVEL_INF);

/* Minimal TSL2591 stub using I2C if wired; else a fixed value */
#define TSL2591_ADDR 0x29

static int tsl2591_lux_read(const struct device *i2c, int32_t *lux_out)
{
	if (!i2c || !device_is_ready(i2c)) return -ENODEV;
	/* TODO: Implement proper TSL2591 init and read; return stub for now */
	*lux_out = 40000; /* stub */
	return 0;
}

void light_thread(void *a, void *b, void *c)
{
	ARG_UNUSED(a); ARG_UNUSED(b); ARG_UNUSED(c);
	avg5_t avg; avg5_init(&avg);
	const struct device *i2c = DEVICE_DT_GET(DT_NODELABEL(i2c1));
	if (!device_is_ready(i2c)) {
		LOG_WRN("I2C1 not ready; light sensor stub in use");
	}

	while (1) {
		int32_t lux = 30000; /* stub */
		if (i2c && device_is_ready(i2c)) {
			(void)tsl2591_lux_read(i2c, &lux);
		}
		avg5_add(&avg, lux);
		int32_t mean_lux = avg5_mean(&avg);

		k_mutex_lock(&g_env.lock, K_FOREVER);
		g_env.light = mean_lux;
		k_mutex_unlock(&g_env.lock);

		k_msleep(1000);
	}
}
