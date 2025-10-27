#include <drivers/pit.h>

void pitInit(uint32_t targetFreq)
{
        uint32_t divisor = 1193180 / targetFreq;
        outb(PIT_COMMAND, 0x36);
        outb(PIT_CHANNEL0, divisor & 0xFF);
        outb(PIT_CHANNEL0, (divisor >> 8) & 0xFF);
}
