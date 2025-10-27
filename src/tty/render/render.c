#include <tty/render/render.h>

framebuffer_t framebuffer;
static font_t *font = NULL;

void setframebuffer(framebuffer_t fb)
{
        framebuffer = fb;
}

framebuffer_t getframebuffer(void)
{
        return framebuffer;
}

void setfont(font_t *new_font)
{
        font = new_font;
}

font_t getfont(void)
{
        return *font;
}

void displaychar(char chr, int px, int py, int bg, int fg)
{
        if (framebuffer.dimensions[1] == 0 || framebuffer.dimensions[0] == 0)
                return;
        if (!framebuffer.buffer || chr < font->first_char || chr > font->last_char)
                return;
        uint32_t bytes_per_row = (font->char_width + 7) / 8;

        for (uint32_t y = 0; y < font->char_height; y++)
        {
                if (py + y < 0 || py + y >= framebuffer.dimensions[1])
                        continue;

                uint32_t char_offset = (chr - font->first_char) * bytes_per_row * font->char_height;
                uint8_t bitmap_byte = font->font_bitmap[char_offset + y * bytes_per_row];

                for (uint32_t x = 0; x < font->char_width; x++)
                {
                        if (px + x < 0 || px + x >= framebuffer.dimensions[0])
                                continue;
                        bool status = bitmap_byte & (1 << (7 - (x % 8)));
                        if ((!status && bg == 0x00000000) || (status && fg == 0x00000000))
                        {
                                continue;
                        }

                        framebuffer.buffer[(py + y) * framebuffer.dimensions[0] + (px + x)] = status ? fg : bg;
                }
        }
}

void write(const char *text, int length, int px, int py)
{
        if (length == 0 || length < 0 || !text)
                return;
        int fg = 0xFFFFFFFF;
        int bg = 0xFF000000;
        for (int i = 0; i < length; ++i)
        {
                displaychar(text[i], px + (i * font->char_width), py, bg, fg);
        }
}

void print(const char *text, int px, int py)
{
        if (!text)
                return;
        int i = 0;
        int fg = 0xFFFFFFFF;
        int bg = 0xFF000000;
        while (text[i])
        {
                displaychar(*text, px + (i++ * font->char_width), py, bg, fg);
        }
}
