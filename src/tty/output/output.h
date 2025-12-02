#ifndef TTY_OUT
#define TTY_OUT

#define TTY_W (128)
#define TTY_H (55)
#define TAB_SIZE 8

#include <utils.h>
#include <stdarg.h>
#include <io.h>

void putch(const uint8_t ch, uint8_t (*output)[TTY_H][TTY_W], uint32_t *x, uint32_t *y);
void display(void);
void putchar(const uint8_t ch);
int vsnprintf(char *str, size_t size, const char *fmt, va_list args);
int snprintf(char *str, size_t size, const char *fmt, ...);
void printf(const char *fmt, ...);
void clear(void);

#endif
