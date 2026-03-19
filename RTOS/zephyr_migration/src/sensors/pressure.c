#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/sensor.h>
#include "pressure.h"
#include "avg_list.h"
#include "app_types.h"

LOG_MODULE_REGISTER(s_press, LOG_LEVEL_INF);

void pressure_thread(void *a, void *b, void *c)
{
	ARG_UNUSED(a); ARG_UNUSED(b); ARG_UNUSED(c);
	avg5_t avg; avg5_init(&avg);

#if DT_NODE_HAS_COMPAT_STATUS(DT_INST(0, bosch_bme280), compatible, okay)
	const struct device *dev = DEVICE_DT_GET(DT_INST(0, bosch_bme280));
	if (!device_is_ready(dev)) {
		LOG_WRN("BME280 not ready");
	}
#else
	const struct device *dev = NULL;
	LOG_WRN("BME280 driver not present; stub 115");
#endif

	while (1) {
		int32_t p_kpa = 115; /* stub; thresholds are example values */
		if (dev && device_is_ready(dev)) {
			if (sensor_sample_fetch(dev) == 0) {
				struct sensor_value p;
				if (sensor_channel_get(dev, SENSOR_CHAN_PRESS, &p) == 0) {
					/* Convert Pa to kPa */
					int64_t pa = (int64_t)p.val1 * 1000000LL + p.val2; /* val2 in micro */
					p_kpa = (int32_t)(pa / 1000LL);
				}
			}
		}
		avg5_add(&avg, p_kpa);
		int32_t mean_p = avg5_mean(&avg);

		k_mutex_lock(&g_env.lock, K_FOREVER);
		g_env.pressure = mean_p;
		k_mutex_unlock(&g_env.lock);

		k_msleep(1000);
	}
}
