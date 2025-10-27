#ifndef FONTS_H
#define FONTS_H

typedef struct
{
        const unsigned char *font_name;
        unsigned char *font_bitmap;
        unsigned int char_width;
        unsigned int char_height;
        unsigned char first_char;
        unsigned char last_char;
} font_t;

extern font_t font_8x8;
extern font_t cursors;

#endif
