#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/dt-bindings/gpio/gpio.h>
#include "app_types.h"
#include "sys_control.h"

LOG_MODULE_REGISTER(sys_ctl, LOG_LEVEL_INF);

/* LED GPIOs via DT aliases led0..led3 -> gpio-leds nodes */
static const struct gpio_dt_spec led_green = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec led_orange = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);
static const struct gpio_dt_spec led_red = GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);
static const struct gpio_dt_spec led_blue = GPIO_DT_SPEC_GET(DT_ALIAS(led3), gpios);

enum sev { SEV_GREEN = 0, SEV_YELLOW = 1, SEV_RED = 2 };

static struct k_timer fault_timer;
static atomic_t fault_period_ms = ATOMIC_INIT(0);

static void fault_timer_cb(struct k_timer *t)
{
	ARG_UNUSED(t);
	/* Blink BLUE as fault indicator */
	static bool on;
	on = !on;
	if (device_is_ready(led_blue.port)) {
		gpio_pin_set(led_blue.port, led_blue.pin, on ? 1 : 0);
	}
}

static enum sev eval_severity(int32_t v, int32_t g_lo, int32_t g_hi, int32_t y_lo, int32_t y_hi, int32_t r_lo, int32_t r_hi)
{
	if (v >= r_lo && v <= r_hi) return SEV_RED;
	if (v >= y_lo && v <= y_hi) return SEV_YELLOW;
	if (v >= g_lo && v <= g_hi) return SEV_GREEN;
	/* Outside known ranges -> treat as RED */
	return SEV_RED;
}

void sys_control_thread(void *a, void *b, void *c)
{
	ARG_UNUSED(a); ARG_UNUSED(b); ARG_UNUSED(c);

	/* Configure LEDs */
	const struct gpio_dt_spec *leds[] = { &led_green, &led_orange, &led_red, &led_blue };
	for (size_t i = 0; i < 4; i++) {
		if (device_is_ready(leds[i]->port)) {
			gpio_pin_configure_dt(leds[i], GPIO_OUTPUT_INACTIVE);
		}
	}

	k_timer_init(&fault_timer, fault_timer_cb, NULL);

	enum sev last_sev = SEV_GREEN;
	int64_t viol_start_ms = 0;

	while (1) {
		/* Snapshot readings */
		int32_t t, h, l, p;
		k_mutex_lock(&g_env.lock, K_FOREVER);
		t = g_env.temperature;  /* C */
		h = g_env.humidity;     /* % */
		l = g_env.light;        /* lux */
		p = g_env.pressure;     /* kPa (example) */
		k_mutex_unlock(&g_env.lock);

		/* Thresholds from Analysis Report examples */
		enum sev st = eval_severity(t, 10, 20, 21, 30, 51, 60);
		enum sev sh = eval_severity(h, 30, 40, 41, 50, 51, 60);
		enum sev sp = eval_severity(p, 100, 200, 110, 120, 121, 130);
		enum sev sl = eval_severity(l, 23000, 32000, 32000, 49000, 50000, 88000);

		enum sev overall = st;
		if (sh > overall) overall = sh;
		if (sp > overall) overall = sp;
		if (sl > overall) overall = sl;

		/* LED policy: GREEN/YELLOW/RED map to GREEN/ORANGE/RED; BLUE is fault blinker */
		if (device_is_ready(led_green.port)) {
			gpio_pin_set_dt(&led_green, overall == SEV_GREEN ? 1 : 0);
		}
		if (device_is_ready(led_orange.port)) {
			gpio_pin_set_dt(&led_orange, overall == SEV_YELLOW ? 1 : 0);
		}
		if (device_is_ready(led_red.port)) {
			gpio_pin_set_dt(&led_red, overall == SEV_RED ? 1 : 0);
		}

		/* Violation detection (non-green) */
		int64_t now = k_uptime_get();
		if (overall != SEV_GREEN) {
			if (last_sev == SEV_GREEN) {
				viol_start_ms = now;
				/* enqueue first violation record */
				struct env_record r = { .temperature = t, .humidity = h, .light = l, .pressure = p };
				(void)k_msgq_put(&log_q, &r, K_NO_WAIT);
			}
			int64_t dur = now - viol_start_ms;
			if (dur >= 60000 && atomic_get(&fault_period_ms) != 300) {
				atomic_set(&fault_period_ms, 300);
				k_timer_start(&fault_timer, K_NO_WAIT, K_MSEC(300));
			} else if (dur >= 30000 && atomic_get(&fault_period_ms) == 0) {
				atomic_set(&fault_period_ms, 1000);
				k_timer_start(&fault_timer, K_NO_WAIT, K_SECONDS(1));
			}
		} else {
			viol_start_ms = 0;
			atomic_set(&fault_period_ms, 0);
			k_timer_stop(&fault_timer);
			if (device_is_ready(led_blue.port)) {
				gpio_pin_set_dt(&led_blue, 0);
			}
		}

		if (overall != last_sev && overall != SEV_GREEN) {
			/* On transitions to YELLOW/RED, enqueue a log record */
			struct env_record r = { .temperature = t, .humidity = h, .light = l, .pressure = p };
			(void)k_msgq_put(&log_q, &r, K_NO_WAIT);
		}

		last_sev = overall;
		k_msleep(200);
	}
}
