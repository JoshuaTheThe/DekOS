#include <text.h>

uint32_t *framebuffer = NULL;
uint32_t framebuffer_height = 0;
uint32_t framebuffer_width = 0;

static font_t *font = NULL;

void set_active_font(font_t *new_font)
{
        font = new_font;
}

void put_character(char chr, int px, int py)
{
        if (framebuffer_height == 0 || framebuffer_width == 0)
                return;
        if (!framebuffer || chr < font->first_char || chr > font->last_char)
                return;
        uint32_t bytes_per_row = (font->char_width + 7) / 8;

        for (uint32_t y = 0; y < font->char_height; y++)
        {
                if (py + y < 0 || py + y >= framebuffer_height)
                        continue;

                uint32_t char_offset = (chr - font->first_char) * bytes_per_row * font->char_height;
                uint8_t bitmap_byte = font->font_bitmap[char_offset + y * bytes_per_row];

                for (uint32_t x = 0; x < font->char_width; x++)
                {
                        if (px + x < 0 || px + x >= framebuffer_width)
                                continue;
                        bool status = bitmap_byte & (1 << (7 - (x % 8)));
                        framebuffer[(py + y) * framebuffer_width + (px + x)] = status ? 0xFFFFFFFF : 0;
                }
        }
}

void write(const char *text, int length, int px, int py)
{
        if (length == 0 || length < 0 || !text)
                return;
        for (int i = 0; i < length; ++i)
        {
                put_character(text[i], px + (i * font->char_width), py);
        }
}

