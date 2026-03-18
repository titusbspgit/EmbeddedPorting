#ifndef WDG_H
#define WDG_H

#include <stdint.h>

void WDG_Init(void);
void WDG_Kick(void);
void WDG_SetReload5s(void);

#endif /* WDG_H */
