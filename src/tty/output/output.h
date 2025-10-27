#ifndef TTY_OUT
#define TTY_OUT

#include <stdint.h>
#include <utils.h>
#include <stdarg.h>

#include <tty/render/fonts.h>
#include <tty/render/render.h>

#define TTY_W 80
#define TTY_H 60
#define TAB_SIZE 4

void putch(const char ch, char **output, uint32_t *tty_x, uint32_t *tty_y);
void display(void);
void putchar(const char);
int vsnprintf(char *str, size_t size, const char *fmt, va_list args);
int snprintf(char *str, size_t size, const char *fmt, ...);
int printf(const char *fmt, ...);

#endif
