#pragma pack(1)
#include <stdint.h>
#include <stdbool.h>
#include <text.h>
#include <idt.h>
#include <gdt.h>
#include <input.h>
#include <rtc.h>
#include <math.h>
#include <utils.h>
#include <stdarg.h>

#define TTY_W 80
#define TTY_H 60

uint8_t system_output[TTY_H][TTY_W] = {0};
uint32_t tty_y = 0, tty_x = 0;
uint32_t tty_bg = rgb(128, 0, 128);
uint32_t tty_fg = rgb(255, 128, 255);

int mx,my;

void hang()
{
        cli();
        while (1)
        {
                hlt();
        }
}

void k_display(void)
{
        set_active_font(&font_8x8);
        for (int y = 0; y < TTY_H; ++y)
                for (int x = 0; x < TTY_W; ++x)
                {
                        put_character(system_output[y][x], x * 8, y * 8, tty_bg, tty_fg);
                }
}

void k_print(const char *fmt, ...)
{
        va_list args;
        va_start(args, fmt);

        const char *s = fmt;
        while (*s)
        {
                if (*s == '%')
                {
                        s++;

                        switch (*s)
                        {
                        case 'd':
                        {
                                int num = va_arg(args, int);
                                char buffer[12];
                                int i = 0;

                                if (num < 0)
                                {
                                        system_output[tty_y][tty_x++] = '-';
                                        if (tty_x >= TTY_W)
                                        {
                                                tty_x = 0;
                                                ++tty_y;
                                        }
                                        num = -num;
                                }

                                do
                                {
                                        buffer[i++] = '0' + (num % 10);
                                        num /= 10;
                                } while (num > 0);

                                while (i > 0)
                                {
                                        system_output[tty_y][tty_x++] = buffer[--i];
                                        if (tty_x >= TTY_W)
                                        {
                                                tty_x = 0;
                                                ++tty_y;
                                        }
                                }
                                break;
                        }

                        case 's':
                        {
                                char *str = va_arg(args, char *);
                                while (*str)
                                {
                                        if (*str == '\n')
                                        {
                                                system_output[tty_y][tty_x++] = '\n';
                                                tty_x = 0;
                                                ++tty_y;
                                        }
                                        else
                                        {
                                                system_output[tty_y][tty_x++] = *str;
                                        }

                                        if (tty_x >= TTY_W)
                                        {
                                                tty_x = 0;
                                                ++tty_y;
                                        }
                                        if (tty_y >= TTY_H)
                                        {
                                                tty_y = TTY_H - 1;
                                        }
                                        str++;
                                }
                                break;
                        }

                        case 'c':
                        {
                                char c = (char)va_arg(args, int);
                                if (c == '\n')
                                {
                                        tty_x = 0;
                                        ++tty_y;
                                }
                                else if (c == '\b')
                                {
                                        if (tty_x > 0)
                                        {
                                                tty_x--;
                                                system_output[tty_y][tty_x] = ' ';
                                        }
                                        else if (tty_y > 0)
                                        {
                                                tty_y--;
                                                tty_x = TTY_W - 1;
                                                system_output[tty_y][tty_x] = ' ';
                                        }
                                }
                                else
                                {
                                        system_output[tty_y][tty_x++] = c;
                                }
                                break;
                        }

                        case '%':
                        {
                                system_output[tty_y][tty_x++] = '%';
                                break;
                        }

                        default:
                        {
                                system_output[tty_y][tty_x++] = '%';
                                system_output[tty_y][tty_x++] = *s;
                                if (tty_x >= TTY_W)
                                {
                                        tty_x = 0;
                                        ++tty_y;
                                }
                                break;
                        }
                        }
                }
                else
                {
                        if (*s == '\n')
                        {
                                tty_x = 0;
                                ++tty_y;
                        }
                        else if (*s == '\b')
                        {
                                if (tty_x > 0)
                                {
                                        tty_x--;
                                        system_output[tty_y][tty_x] = ' ';
                                }
                                else if (tty_y > 0)
                                {
                                        tty_y--;
                                        tty_x = TTY_W - 1;
                                        system_output[tty_y][tty_x] = ' ';
                                }
                        }
                        else
                        {
                                system_output[tty_y][tty_x++] = *s;
                        }
                }

                if (tty_x >= TTY_W)
                {
                        tty_x = 0;
                        ++tty_y;
                }
                if (tty_y >= TTY_H)
                {
                        for (int y = 1; y < TTY_H; y++)
                        {
                                for (int x = 0; x < TTY_W; x++)
                                {
                                        system_output[y - 1][x] = system_output[y][x];
                                }
                        }
                        for (int x = 0; x < TTY_W; x++)
                        {
                                system_output[TTY_H - 1][x] = ' ';
                        }
                        tty_y = TTY_H - 1;
                }

                s += 1;
        }

        va_end(args);
        k_display();
}

void memset(void *d, uint8_t v, uint32_t len)
{
        for (int i = 0; i < len; ++i)
        {
                ((char *)d)[i] = v;
        }
}

void main(uint32_t magic, uint32_t mbinfo_ptr)
{
        __asm("cli;");
        while (magic != 0x2BADB002)
        {
                hlt();
        }

        multiboot_info_t *mbi = (multiboot_info_t *)mbinfo_ptr;
        uint64_t memory_size = mbi->mem_upper * 1024 + mbi->mem_lower * 1024;
        for (unsigned int y = 0; y < mbi->framebuffer_height; ++y)
                for (unsigned int x = 0; x < mbi->framebuffer_width; ++x)
                        ((uint32_t *)mbi->framebuffer_addr)[y * mbi->framebuffer_width + x] = 0xFF;
        framebuffer = (uint32_t *)mbi->framebuffer_addr;
        framebuffer_width = mbi->framebuffer_width;
        framebuffer_height = mbi->framebuffer_height;
        mx = framebuffer_width / 2;
        my = framebuffer_height / 2;
        memset(system_output, 0, sizeof(system_output));

        /* Init e.g. GDT, IDT, PIT, etc. */
        set_active_font(&font_8x8);
        gdt_init();
        idt_init();
        // write("Hello, World!", 13, 16, 16);
        bool keyboardq = is_keyboard_present();
        bool mouseq = is_mouse_present();
        if (!keyboardq)
        {
                k_print("no keyboard device, plug one in and reboot");
                hang();
        }
        if (!mouseq)
        {
                k_print("no mouse device, plug one in and reboot");
                hang();
        }
        ps2_initialize_mouse();
        int px = mx, py = my;
        int buttons;

        while (1)
        {
                bool hit;
                char ch = keyboard_get(&hit);
                if (hit)
                {
                        k_print("%c", ch);
                }

                if (buttons & MOUSE_LEFT_BUTTON)
                {
                        restore_pixels(prev_save_x, prev_save_y);
                        put_character('!', mx, my, 0xFF000000, 0xFFFFFFFF);
                        save_pixels(prev_save_x, prev_save_y);
                }
                mouse_get(&mx, &my, &px, &py, &buttons);
        }
}
