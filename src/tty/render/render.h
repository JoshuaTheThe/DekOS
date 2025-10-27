#ifndef TEXT_H
#define TEXT_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include <tty/render/fonts.h>
#include <utils.h>

typedef struct framebuffer_t
{
        uint32_t *buffer;
        uint32_t dimensions[2];
} framebuffer_t;

/* Getters && Setters */
void setframebuffer(framebuffer_t fb);
framebuffer_t getframebuffer(void);
void setfont(font_t *new_font);
font_t getfont(void);

/* functions */
void displaychar(const char chr, const int px, const int py, const int bg, const int fg);
void write(const char *text, const int length, const int px, const int py);
void print(const char *text, const int px, const int py);

#endif
