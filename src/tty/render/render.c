#include <tty/render/render.h>

static framebuffer_t framebuffer;
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

void displaychar(unsigned char chr, uint32_t px, uint32_t py, uint32_t bg, uint32_t fg)
{
        if (framebuffer.dimensions[1] == 0 || framebuffer.dimensions[0] == 0)
                return;
        if (!framebuffer.buffer || chr < font->first_char || chr > font->last_char)
                return;

        if (px >= framebuffer.dimensions[0] || py >= framebuffer.dimensions[1])
                return;

        uint32_t bytes_per_row = (font->char_width + 7) / 8;
        uint32_t char_offset = (chr - font->first_char) * bytes_per_row * font->char_height;

        bool skip_bg = (bg & 0xFF000000) == 0x00000000;
        bool skip_fg = (fg & 0xFF000000) == 0x00000000;

        for (uint32_t y = 0; y < font->char_height; y++)
        {
                uint32_t current_y = py + y;
                if (current_y >= framebuffer.dimensions[1])
                        continue;

                uint8_t bitmap_byte = font->font_bitmap[char_offset + y * bytes_per_row];

                for (uint32_t x = 0; x < font->char_width; x++)
                {
                        uint32_t current_x = px + x;
                        if (current_x >= framebuffer.dimensions[0])
                                continue;

                        bool pixel_set = bitmap_byte & (1 << (7 - (x % 8)));

                        if ((pixel_set && skip_fg) || (!pixel_set && skip_bg))
                                continue;

                        framebuffer.buffer[current_y * framebuffer.dimensions[0] + current_x] =
                            pixel_set ? fg : bg;
                }
        }
}

void write(const unsigned char *text, size_t length, uint32_t px, uint32_t py)
{
        uint32_t fg, bg;

        fg = 0xFFFFFFFF;
        bg = 0xFF000000;

        if (length == 0 || !text)
                return;
        for (size_t i = 0; i < length; ++i)
        {
                displaychar(text[i], px + (i * font->char_width), py, bg, fg);
        }
}

void print(const unsigned char *text, uint32_t px, uint32_t py)
{
        uint32_t i, fg, bg;

        i = 0;
        fg = 0xFFFFFFFF;
        bg = 0xFF000000;

        if (!text)
                return;
        while (text[i])
        {
                displaychar(*text, px + (i++ * font->char_width), py, bg, fg);
        }
}
