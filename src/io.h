#ifndef IO_H
#define IO_H

#include <stdint.h>
#include <utils.h>
#include <stdarg.h>
#include <fonts.h>
#include <text.h>
#define TTY_W 80
#define TTY_H 60
#define TAB_SIZE 4

extern uint8_t inb(uint16_t port);
extern uint16_t inw(uint16_t port);
extern uint32_t inl(uint16_t port);
extern void outb(uint16_t port, uint8_t value);
extern void outw(uint16_t port, uint16_t value);
extern void outl(uint16_t port, uint32_t value);
extern void insl(uint16_t port, void *addr, uint32_t count);
extern void insw(uint16_t port, void *addr, uint32_t count);
extern void outsw(uint16_t port, const void *addr, unsigned long n);
extern void k_putch(const char);
extern void k_print(const char *fmt, ...);
extern void k_display(void);

extern uint8_t system_output[TTY_H][TTY_W];
extern uint32_t tty_y, tty_x;
extern uint32_t tty_bg;
extern uint32_t tty_fg;

#endif
