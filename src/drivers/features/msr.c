#include <drivers/features/feature.h>

void MSRGet(uint32_t msr, uint32_t *lo, uint32_t *hi)
{
        asm volatile("rdmsr" : "=a"(*lo), "=d"(*hi) : "c"(msr));
}

void MSRSet(uint32_t msr, uint32_t lo, uint32_t hi)
{
        asm volatile("wrmsr" : : "a"(lo), "d"(hi), "c"(msr));
}
