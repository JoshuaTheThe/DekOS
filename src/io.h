#ifndef IO_H
#define IO_H

#include <stdint.h>
#include <stdarg.h>
#include <utils.h>

__attribute__((no_caller_saved_registers))
static inline uint8_t irqinb(uint16_t port)
{
        uint8_t value;
        __asm("inb %1, %0" : "=a"(value) : "Nd"(port));
        return value;
}

__attribute__((no_caller_saved_registers))
static inline void irqoutb(uint16_t port, uint8_t value)
{
        __asm("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port)
{
        uint8_t value;
        __asm("inb %1, %0" : "=a"(value) : "Nd"(port));
        return value;
}

static inline void outb(uint16_t port, uint8_t value)
{
        __asm("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint16_t inw(uint16_t port)
{
        uint16_t value;
        __asm("inw %1, %0" : "=a"(value) : "Nd"(port));
        return value;
}

static inline void outw(uint16_t port, uint16_t value)
{
        __asm("outw %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint32_t inl(uint16_t port)
{
        uint32_t value;
        __asm("inl %1, %0" : "=a"(value) : "Nd"(port));
        return value;
}

static inline void outl(uint16_t port, uint32_t value)
{
        __asm("outl %0, %1" : : "a"(value), "Nd"(port));
}

static inline void insl(uint16_t port, void *addr, uint32_t count)
{
        __asm volatile("rep insl" : "+D"(addr), "+c"(count) : "d"(port) : "memory");
}

static inline void insw(uint16_t port, void *addr, uint32_t count)
{
        __asm volatile("rep insw" : "+D"(addr), "+c"(count) : "d"(port) : "memory");
}

static inline void outsw(uint16_t port, const void *addr, unsigned long n)
{
        __asm volatile("cld; rep; outsw"
                       : "+S"(addr), "+c"(n)
                       : "d"(port));
}


#endif
