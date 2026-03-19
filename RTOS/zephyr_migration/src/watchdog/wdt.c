#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/watchdog.h>
#include "wdt.h"

LOG_MODULE_REGISTER(wdt_mod, LOG_LEVEL_INF);

void watchdog_thread(void *a, void *b, void *c)
{
	ARG_UNUSED(a); ARG_UNUSED(b); ARG_UNUSED(c);

	const struct device *wdt = DEVICE_DT_GET_OR_NULL(DT_NODELABEL(iwdg));
	if (!wdt || !device_is_ready(wdt)) {
		LOG_WRN("WDT not ready; skipping");
		while (1) { k_msleep(1000); }
	}

	struct wdt_timeout_cfg cfg = {
		.flags = WDT_FLAG_RESET_SOC,
		.window = {
			.min = 0U,
			.max = 5000U,
		},
		.callback = NULL,
	};

	int ch = wdt_install_timeout(wdt, &cfg);
	if (ch < 0) {
		LOG_ERR("wdt_install_timeout: %d", ch);
		while (1) { k_msleep(1000); }
	}

	if (wdt_setup(wdt, 0) != 0) {
		LOG_ERR("wdt_setup failed");
		while (1) { k_msleep(1000); }
	}

	while (1) {
		(void)wdt_feed(wdt, ch);
		k_msleep(1000);
	}
}
