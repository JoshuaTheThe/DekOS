#include <drivers/pit.h>

extern int tick_counter;

void pitInit(uint32_t targetFreq)
{
        uint32_t divisor = 1193180 / targetFreq;
        outb(PIT_COMMAND, 0x36);
        outb(PIT_CHANNEL0, divisor & 0xFF);
        outb(PIT_CHANNEL0, (divisor >> 8) & 0xFF);
}

void pitDelay(uint32_t ticks)
{
        int start = tick_counter;
        while ((tick_counter - start) < ticks)
        { asm("pause"); }
}
