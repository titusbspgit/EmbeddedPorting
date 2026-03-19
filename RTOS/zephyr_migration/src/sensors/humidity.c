#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/sensor.h>
#include "humidity.h"
#include "avg_list.h"
#include "app_types.h"

LOG_MODULE_REGISTER(s_humi, LOG_LEVEL_INF);

void humidity_thread(void *a, void *b, void *c)
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
	LOG_WRN("SHT3x driver not present; using stub 45%%RH");
#endif

	while (1) {
		int32_t rh = 45; /* stub default */
		if (dev && device_is_ready(dev)) {
			if (sensor_sample_fetch(dev) == 0) {
				struct sensor_value h;
				if (sensor_channel_get(dev, SENSOR_CHAN_HUMIDITY, &h) == 0) {
					/* h.val1 is integer %, val2 is fractional 10^-6 */
					rh = h.val1;
				}
			}
		}
		avg5_add(&avg, rh);
		int32_t mean_rh = avg5_mean(&avg);

		k_mutex_lock(&g_env.lock, K_FOREVER);
		g_env.humidity = mean_rh;
		k_mutex_unlock(&g_env.lock);

		k_msleep(1000);
	}
}
