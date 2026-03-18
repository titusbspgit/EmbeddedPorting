#ifndef SYSCTRL_H
#define SYSCTRL_H

#include <stdint.h>
#include <stdbool.h>

void SYSCTRL_Init(void);
void SYSCTRL_Update(void);

/* Violation logging hook */
void SYSCTRL_LogViolation(float t, float h, float l, float p);

#endif /* SYSCTRL_H */
