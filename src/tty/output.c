#include <tty/output.h>
#include <tty/render/fonts.h>
#include <tty/render/render.h>
#include <memory/string.h>
#include <resource/main.h>
#include <wm/main.h>
#include <drivers/dev/serial.h>

extern KRNLRES *fbRes;

uint8_t system_output[TTY_H][TTY_W];
uint32_t tty_y = 0, tty_x = 0;
uint32_t tty_bg = rgb(30, 30, 30);
uint32_t tty_fg = rgb(230, 230, 230);
bool redraw = false;

void display(void)
{
        if (redraw)
        {
                RenderSetFont(&cascadia);
                for (size_t y = 0; y < TTY_H; ++y)
                {
                        for (size_t x = 0; x < TTY_W; ++x)
                        {
                                RenderChar(system_output[y][x], x * cascadia.char_width, y * cascadia.char_height, tty_bg, tty_fg);
                        }
                }

                redraw = false;
        }
}

void clear(void)
{
        for (int y = 0; y < TTY_H; ++y)
        {
                memset(system_output[y], 0, TTY_W);
        }
        tty_x = 0;
        tty_y = 0;
}

void putch(const uint8_t ch, uint8_t (*output)[TTY_H][TTY_W], uint32_t *x, uint32_t *y)
{
        SerialPut(ch);
        if (ch == '\n')
        {
                *x = 0;
                ++(*y);
                SerialPut('\r');
        }
        else if (ch == '\b' && *x > 0)
        {
                (*x)--;
                (*output)[*y][*x] = ' ';
                SerialPut(' ');
                SerialPut('\b');
        }
        else if (ch == '\b' && *y > 0)
        {
                (*y)--;
                *x = TTY_W - 1;
                (*output)[*y][*x] = ' ';
                SerialPut(' ');
                SerialPut('\b');
        }
        else if (ch == '\t')
        {
                (*output)[*y][*x] = ' ';
                (*x)++;
        }
        else
        {
                (*output)[*y][*x] = ch;
                (*x)++;
        }

        if (*x >= TTY_W)
        {
                *x = 0;
                ++(*y);
        }

        if (*y >= TTY_H)
        {
                for (int a = 1; a < TTY_H; a++)
                {
                        for (int b = 0; b < TTY_W; b++)
                        {
                                (*output)[a - 1][b] = (*output)[a][b];
                        }
                }

                for (int b = 0; b < TTY_W; b++)
                {
                        (*output)[TTY_H - 1][b] = ' ';
                }

                *y = TTY_H - 1;
        }

        redraw = true;
}

void putchar(const uint8_t ch)
{
        // if (ch == '\n')
        //         SerialPut('\r');
        // SerialPut(ch);
        putch(ch, &system_output, &tty_x, &tty_y);
}
