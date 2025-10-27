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
void displaychar(const unsigned char chr, const uint32_t px, const uint32_t py, const uint32_t bg, const uint32_t fg);
void write(const unsigned char *text, const size_t length, const uint32_t px, const uint32_t py);
void print(const unsigned char *text, const uint32_t px, const uint32_t py);

#endif
