#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include "app_types.h"

/* Threads */
#include "sensors/temperature.h"
#include "sensors/humidity.h"
#include "sensors/light.h"
#include "sensors/pressure.h"
#include "control/sys_control.h"
#include "storage/storage.h"
#include "watchdog/wdt.h"

LOG_MODULE_REGISTER(app, LOG_LEVEL_INF);

/* Shared environment data */
struct shared_env_data g_env;

/* Log queue for JSON records */
K_MSGQ_DEFINE(log_q, sizeof(struct env_record), 8, 4);

/* Thread stacks and TCBs */
K_THREAD_STACK_DEFINE(temp_stack, 1024);
K_THREAD_STACK_DEFINE(humi_stack, 1024);
K_THREAD_STACK_DEFINE(light_stack, 1024);
K_THREAD_STACK_DEFINE(press_stack, 1024);
K_THREAD_STACK_DEFINE(ctrl_stack, 1024);
K_THREAD_STACK_DEFINE(stor_stack, 1536);
K_THREAD_STACK_DEFINE(wdt_stack, 768);

static struct k_thread temp_tcb, humi_tcb, light_tcb, press_tcb, ctrl_tcb, stor_tcb, wdt_tcb;

void main(void)
{
	LOG_INF("EnvMon Zephyr start (stm32f4_disco)");

	k_mutex_init(&g_env.lock);
	g_env.temperature = 0;
	g_env.humidity = 0;
	g_env.light = 0;
	g_env.pressure = 0;

	/* Start threads with mapped priorities (lower number = higher prio) */
	k_thread_create(&temp_tcb, temp_stack, K_THREAD_STACK_SIZEOF(temp_stack),
	                temperature_thread, NULL, NULL, NULL, K_PRIO_PREEMPT(5), 0, K_NO_WAIT);

	k_thread_create(&humi_tcb, humi_stack, K_THREAD_STACK_SIZEOF(humi_stack),
	                humidity_thread, NULL, NULL, NULL, K_PRIO_PREEMPT(5), 0, K_NO_WAIT);

	k_thread_create(&light_tcb, light_stack, K_THREAD_STACK_SIZEOF(light_stack),
	                light_thread, NULL, NULL, NULL, K_PRIO_PREEMPT(5), 0, K_NO_WAIT);

	k_thread_create(&press_tcb, press_stack, K_THREAD_STACK_SIZEOF(press_stack),
	                pressure_thread, NULL, NULL, NULL, K_PRIO_PREEMPT(5), 0, K_NO_WAIT);

	k_thread_create(&ctrl_tcb, ctrl_stack, K_THREAD_STACK_SIZEOF(ctrl_stack),
	                sys_control_thread, NULL, NULL, NULL, K_PRIO_PREEMPT(2), 0, K_NO_WAIT);

	k_thread_create(&stor_tcb, stor_stack, K_THREAD_STACK_SIZEOF(stor_stack),
	                storage_thread, NULL, NULL, NULL, K_PRIO_PREEMPT(6), 0, K_NO_WAIT);

	k_thread_create(&wdt_tcb, wdt_stack, K_THREAD_STACK_SIZEOF(wdt_stack),
	                watchdog_thread, NULL, NULL, NULL, K_PRIO_PREEMPT(7), 0, K_NO_WAIT);
}
