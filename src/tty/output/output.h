#ifndef TTY_OUT
#define TTY_OUT

#include <stdint.h>
#include <utils.h>
#include <stdarg.h>
#include <memory/string.h>

#include <tty/render/fonts.h>
#include <tty/render/render.h>

#define TTY_W 80
#define TTY_H 60
#define TAB_SIZE 4

void putch(const uint8_t ch, uint8_t (*output)[TTY_H][TTY_W], uint32_t *x, uint32_t *y);
void display(void);
void putchar(const uint8_t ch);
int vsnprintf(char *str, size_t size, const char *fmt, va_list args);
int snprintf(char *str, size_t size, const char *fmt, ...);
void printf(const char *fmt, ...);
void clear(void);

#endif
