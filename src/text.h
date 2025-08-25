#ifndef TEXT_H
#define TEXT_H

#include <stdint.h>
#include <stdbool.h>
#include <fonts.h>
#include <stddef.h>

extern uint32_t *framebuffer;
extern uint32_t  framebuffer_height;
extern uint32_t  framebuffer_width;
extern void put_character(char chr, int px, int py);
extern void set_active_font(font_t *);
extern void write(const char *text, int length, int px, int py);

#endif
