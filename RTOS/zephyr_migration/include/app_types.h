#ifndef APP_TYPES_H
#define APP_TYPES_H

#include <zephyr/kernel.h>
#include <stdint.h>

struct env_record {
	int32_t temperature;
	int32_t humidity;
	int32_t light;
	int32_t pressure;
};

/* Shared environment values updated by sensor threads, read by control/log */
struct shared_env_data {
	struct k_mutex lock;
	int32_t temperature;
	int32_t humidity;
	int32_t light;
	int32_t pressure;
};

extern struct shared_env_data g_env;

/* Message queue for logging requests (produced by sys_control, consumed by storage) */
extern struct k_msgq log_q;

#endif /* APP_TYPES_H */
