#include <tty/render/render.h>

static framebuffer_t framebuffer;
static font_t *font = NULL;
static int scale = 1;

void setscale(int new_scale)
{
        scale = new_scale;
}

int getscale(void)
{
        return scale;
}

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

font_t *getfont(void)
{
        return font;
}

uint32_t blend_colors(uint32_t bg, uint32_t fg, uint8_t intensity, uint8_t bpp)
{
        if (bpp == 1)
                return intensity ? fg : bg;

        uint8_t alpha;
        switch (bpp)
        {
        case 2:
                alpha = intensity * 85;
                break;
        case 4:
                alpha = intensity * 17;
                break;
        default:
                alpha = intensity ? 255 : 0;
        }

        if (alpha == 0)
                return bg;
        if (alpha == 255)
                return fg;

        uint8_t bg_r = (bg >> 16) & 0xFF;
        uint8_t bg_g = (bg >> 8) & 0xFF;
        uint8_t bg_b = bg & 0xFF;

        uint8_t fg_r = (fg >> 16) & 0xFF;
        uint8_t fg_g = (fg >> 8) & 0xFF;
        uint8_t fg_b = fg & 0xFF;

        uint8_t r = ((fg_r * alpha) + (bg_r * (255 - alpha))) / 255;
        uint8_t g = ((fg_g * alpha) + (bg_g * (255 - alpha))) / 255;
        uint8_t b = ((fg_b * alpha) + (bg_b * (255 - alpha))) / 255;

        return (r << 16) | (g << 8) | b;
}

void displaychar(unsigned char chr, uint32_t px, uint32_t py, uint32_t bg, uint32_t fg)
{
        if (framebuffer.dimensions[1] == 0 || framebuffer.dimensions[0] == 0)
                return;
        if (!framebuffer.buffer || !font)
                return;

        if (px >= framebuffer.dimensions[0] || py >= framebuffer.dimensions[1])
                return;

        // Treat unknown characters as space (ASCII 32)
        if (chr < font->first_char || chr > font->last_char)
        {
                chr = 32; // Space character
        }

        // Calculate bytes per row based on BPP
        uint32_t pixels_per_byte = 8 / font->bpp;
        uint32_t bytes_per_row = (font->char_width + pixels_per_byte - 1) / pixels_per_byte;
        uint32_t bytes_per_char = bytes_per_row * font->char_height;
        uint32_t char_offset = (chr - font->first_char) * bytes_per_char;

        bool skip_bg = (bg & 0xFF000000) == 0x00000000;
        bool skip_fg = (fg & 0xFF000000) == 0x00000000;

        for (uint32_t y = 0; y < font->char_height * scale; y++)
        {
                uint32_t current_y = py + y;
                if (current_y >= framebuffer.dimensions[1])
                        continue;

                uint32_t src_y = y / scale;
                uint32_t row_offset = char_offset + src_y * bytes_per_row;

                for (uint32_t x = 0; x < font->char_width * scale; x++)
                {
                        uint32_t current_x = px + x;
                        if (current_x >= framebuffer.dimensions[0])
                                continue;

                        uint32_t src_x = x / scale;

                        // Get the pixel value based on BPP
                        uint32_t byte_index = src_x / pixels_per_byte;
                        uint32_t bit_offset = (pixels_per_byte - 1 - (src_x % pixels_per_byte)) * font->bpp;
                        uint8_t bitmap_byte = font->font_bitmap[row_offset + byte_index];
                        uint8_t pixel_value = (bitmap_byte >> bit_offset) & ((1 << font->bpp) - 1);

                        if (font->bpp == 1)
                        {
                                // Monochrome: 0=bg, 1=fg
                                if ((pixel_value && skip_fg) || (!pixel_value && skip_bg))
                                        continue;
                                framebuffer.buffer[current_y * framebuffer.dimensions[0] + current_x] =
                                    pixel_value ? fg : bg;
                        }
                        else
                        {
                                // Grayscale: blend or use gradient
                                if (pixel_value == 0 && skip_bg)
                                        continue;

                                // Simple implementation: use fg color with alpha based on pixel value
                                uint32_t color = blend_colors(bg, fg, pixel_value, font->bpp);
                                framebuffer.buffer[current_y * framebuffer.dimensions[0] + current_x] = color;
                        }
                }
        }
}

void print(const unsigned char *text, uint32_t px, uint32_t py, uint32_t bg, uint32_t fg)
{
        uint32_t i;

        i = 0;
        if (!text)
                return;
        while (text[i])
        {
                displaychar(text[i], px + (i * font->char_width * scale), py, bg, fg);
                i += 1;
        }
}

void center(const char *text, uint32_t *px, uint32_t *py)
{
        framebuffer_t frame = getframebuffer();
        int length = strnlen(text, 1000);
        *px = frame.dimensions[0] / 2 - length / 2;
        *py = frame.dimensions[1] / 2;
}

void align(const char *text, uint32_t *px, uint32_t *py, int marginX, int marginY,
           alignType_t alignX, alignType_t alignY)
{
        if (!text || !px || !py)
                return;

        const framebuffer_t frame = getframebuffer();
        const size_t len = strnlen(text, MAX_TEXT_LENGTH);
        const uint32_t width = frame.dimensions[0];
        const uint32_t height = frame.dimensions[1];

        font_t *font = getfont();
        const uint32_t text_width = len * font->char_width * scale;
        const uint32_t text_height = font->char_height * scale;

        switch (alignX)
        {
        case ALIGN_LEFT_TOP:
                *px = marginX;
                break;
        case ALIGN_CENTER:
                *px = (width >= text_width + marginX) ? (width - text_width - marginX) / 2 : 0;
                break;
        case ALIGN_RIGHT_BOTTOM:
                *px = (width >= text_width + marginX) ? width - text_width - marginX : 0;
                break;
        default:
                *px = marginX;
        }

        switch (alignY)
        {
        case ALIGN_LEFT_TOP:
                *py = marginY;
                break;
        case ALIGN_CENTER:
                *py = (height >= text_height + marginY) ? (height - text_height - marginY) / 2 : 0;
                break;
        case ALIGN_RIGHT_BOTTOM:
                *py = (height >= text_height + marginY) ? height - text_height - marginY : 0;
                break;
        default:
                *py = marginY;
        }
}
