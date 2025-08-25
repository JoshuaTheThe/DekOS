#include <io.h>

uint8_t inb(uint16_t port)
{
        uint8_t value;
        __asm("inb %1, %0" : "=a"(value) : "Nd"(port));
        return value;
}

void outb(uint16_t port, uint8_t value)
{
        __asm("outb %0, %1" : : "a"(value), "Nd"(port));
}

uint16_t inw(uint16_t port)
{
        uint16_t value;
        __asm("inw %1, %0" : "=a"(value) : "Nd"(port));
        return value;
}

void outw(uint16_t port, uint16_t value)
{
        __asm("outw %0, %1" : : "a"(value), "Nd"(port));
}

uint32_t inl(uint16_t port)
{
        uint32_t value;
        __asm("inl %1, %0" : "=a"(value) : "Nd"(port));
        return value;
}

void outl(uint16_t port, uint32_t value)
{
        __asm("outl %0, %1" : : "a"(value), "Nd"(port));
}

void insl(uint16_t port, void *addr, uint32_t count)
{
        __asm volatile("rep insl" : "+D"(addr), "+c"(count) : "d"(port) : "memory");
}

void insw(uint16_t port, void *addr, uint32_t count)
{
        __asm volatile("rep insw" : "+D"(addr), "+c"(count) : "d"(port) : "memory");
}
