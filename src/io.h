#ifndef IO_H
#define IO_H

#include <stdint.h>
#include <stdarg.h>
#include <utils.h>

extern uint8_t inb(uint16_t port);
extern uint16_t inw(uint16_t port);
extern uint32_t inl(uint16_t port);
extern void outb(uint16_t port, uint8_t value);
extern void outw(uint16_t port, uint16_t value);
extern void outl(uint16_t port, uint32_t value);
extern void insl(uint16_t port, void *addr, uint32_t count);
extern void insw(uint16_t port, void *addr, uint32_t count);
extern void outsw(uint16_t port, const void *addr, unsigned long n);

#endif
