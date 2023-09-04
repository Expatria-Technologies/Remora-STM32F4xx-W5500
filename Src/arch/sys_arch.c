

#if defined(_WIZCHIP_) && !defined(RP2040)

#include "stm32f4xx_hal.h"

/* Returns the current time in mS. This is needed for the LWIP timers */
uint32_t sys_now (void)
{
    return uwTick;
}

#endif
