#ifndef IO_H
#define IO_H

#include <stdint.h>
#include <stdarg.h>
#include <utils.h>

uint8_t inb(uint16_t port);
uint16_t inw(uint16_t port);
uint32_t inl(uint16_t port);
void outb(uint16_t port, uint8_t value);
void outw(uint16_t port, uint16_t value);
void outl(uint16_t port, uint32_t value);
void insl(uint16_t port, void *addr, uint32_t count);
void insw(uint16_t port, void *addr, uint32_t count);
void outsw(uint16_t port, const void *addr, unsigned long n);

#endif
