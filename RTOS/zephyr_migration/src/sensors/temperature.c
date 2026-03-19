#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/sensor.h>
#include "temperature.h"
#include "avg_list.h"
#include "app_types.h"

LOG_MODULE_REGISTER(s_temp, LOG_LEVEL_INF);

void temperature_thread(void *a, void *b, void *c)
{
	ARG_UNUSED(a); ARG_UNUSED(b); ARG_UNUSED(c);
	avg5_t avg; avg5_init(&avg);

#if DT_NODE_HAS_COMPAT_STATUS(DT_INST(0, sensirion_sht3xd), compatible, okay)
	const struct device *dev = DEVICE_DT_GET(DT_INST(0, sensirion_sht3xd));
	if (!device_is_ready(dev)) {
		LOG_WRN("SHT3x not ready");
	}
#else
	const struct device *dev = NULL;
	LOG_WRN("SHT3x driver not present; using stub 25C");
#endif

	while (1) {
		int32_t milli_c = 25000; /* default stub: 25.000 C */
		if (dev && device_is_ready(dev)) {
			if (sensor_sample_fetch(dev) == 0) {
				struct sensor_value t;
				if (sensor_channel_get(dev, SENSOR_CHAN_AMBIENT_TEMP, &t) == 0) {
					milli_c = sensor_value_to_milli(t);
				}
			}
		}
		avg5_add(&avg, milli_c / 1000); /* convert to C integer for thresholds */
		int32_t mean_c = avg5_mean(&avg);

		k_mutex_lock(&g_env.lock, K_FOREVER);
		g_env.temperature = mean_c;
		k_mutex_unlock(&g_env.lock);

		k_msleep(1000);
	}
}
