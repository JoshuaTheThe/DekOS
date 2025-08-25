#ifndef FONTS_H
#define FONTS_H

typedef struct
{
        unsigned char_width;
        unsigned char_height;
        const char *font_name;
        unsigned char first_char;
        unsigned char last_char;
        unsigned char *font_bitmap;
} font_t;

extern font_t font_8x8;

#endif
