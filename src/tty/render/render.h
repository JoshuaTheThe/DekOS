#ifndef TEXT_H
#define TEXT_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include <tty/render/fonts.h>
#include <memory/string.h>
#include <utils.h>

#define MAX_TEXT_LENGTH 1024

typedef struct framebuffer_t
{
        uint32_t *buffer;
        uint32_t dimensions[2];
} framebuffer_t;

typedef enum alignType_t
{
        ALIGN_CENTER,
        ALIGN_LEFT_TOP, /* LEFT OR TOP */
        ALIGN_RIGHT_BOTTOM, /* RIGHT OR BOTTOM */

        ALIGN_TOP=ALIGN_LEFT_TOP,
        ALIGN_LEFT=ALIGN_LEFT_TOP,
        ALIGN_BOTTOM=ALIGN_RIGHT_BOTTOM,
        ALIGN_RIGHT=ALIGN_RIGHT_BOTTOM
} alignType_t;

/* Getters && Setters */
void setframebuffer(framebuffer_t fb);
framebuffer_t getframebuffer(void);
void setfont(font_t *new_font);
font_t getfont(void);

/* functions */
void displaychar(const unsigned char chr, const uint32_t px, const uint32_t py, const uint32_t bg, const uint32_t fg);
void print(const unsigned char *text, const uint32_t px, const uint32_t py, const uint32_t bg, const uint32_t fg);
void setscale(int new_scale);
int getscale(void);
void align(const char *text, uint32_t *px, uint32_t *py, int marginX, int marginY, alignType_t alignX, alignType_t alignY);

#endif
