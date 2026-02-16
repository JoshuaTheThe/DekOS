#ifndef FONTS_H
#define FONTS_H

#include <stdint.h>

typedef struct
{
        const unsigned char *font_name;
        uint8_t *font_bitmap;
        unsigned int char_width;
        unsigned int char_height;
        unsigned char first_char;
        unsigned char last_char;
        unsigned char bpp;
} font_t;

extern font_t font_8x8;
extern font_t cursors;
extern font_t *cascadia;

font_t *FontLoad(const char *path);
void FontFree(font_t *font);

#endif
