#ifndef PIT_H
#define PIT_H

#include <utils.h>
#include <io.h>

#define PIT_FREQUENCY 1193180
#define PIT_CHANNEL0 0x40
#define PIT_COMMAND 0x43
#define PIT_STATUS 0x61

void pitInit(uint32_t targetFreq);
void pitDelay(uint32_t ticks);

#endif
