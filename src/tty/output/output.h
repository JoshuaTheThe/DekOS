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

void putch(const char ch, char **output, int *tty_x, int *tty_y);
void display(void);
void putchar(const char);
int printf(const char *fmt, ...);

extern uint8_t system_output[TTY_H][TTY_W];
extern uint32_t tty_y, tty_x;
extern uint32_t tty_bg;
extern uint32_t tty_fg;

#endif
