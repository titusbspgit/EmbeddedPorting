#ifndef AVG_LIST_H
#define AVG_LIST_H
#include <stdint.h>
#include <stddef.h>

typedef struct {
	int32_t buf[5];
	size_t idx;
	size_t count;
} avg5_t;

static inline void avg5_init(avg5_t *a) { a->idx = 0; a->count = 0; }
static inline void avg5_add(avg5_t *a, int32_t v) {
	a->buf[a->idx] = v;
	a->idx = (a->idx + 1U) % 5U;
	if (a->count < 5U) { a->count++; }
}
static inline int32_t avg5_mean(const avg5_t *a) {
	if (a->count == 0U) return 0;
	int64_t s = 0;
	for (size_t i = 0; i < a->count; i++) { s += a->buf[i]; }
	return (int32_t)(s / (int32_t)a->count);
}
#endif
