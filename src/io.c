#include <io.h>

uint8_t system_output[TTY_H][TTY_W] = {0};
uint32_t tty_y = 0, tty_x = 0;
uint32_t tty_bg = rgb(128, 0, 128);
uint32_t tty_fg = rgb(255, 128, 255);

uint8_t inb(uint16_t port)
{
        uint8_t value;
        __asm("inb %1, %0" : "=a"(value) : "Nd"(port));
        return value;
}

void outb(uint16_t port, uint8_t value)
{
        __asm("outb %0, %1" : : "a"(value), "Nd"(port));
}

uint16_t inw(uint16_t port)
{
        uint16_t value;
        __asm("inw %1, %0" : "=a"(value) : "Nd"(port));
        return value;
}

void outw(uint16_t port, uint16_t value)
{
        __asm("outw %0, %1" : : "a"(value), "Nd"(port));
}

uint32_t inl(uint16_t port)
{
        uint32_t value;
        __asm("inl %1, %0" : "=a"(value) : "Nd"(port));
        return value;
}

void outl(uint16_t port, uint32_t value)
{
        __asm("outl %0, %1" : : "a"(value), "Nd"(port));
}

void insl(uint16_t port, void *addr, uint32_t count)
{
        __asm volatile("rep insl" : "+D"(addr), "+c"(count) : "d"(port) : "memory");
}

void insw(uint16_t port, void *addr, uint32_t count)
{
        __asm volatile("rep insw" : "+D"(addr), "+c"(count) : "d"(port) : "memory");
}

void outsw(uint16_t port, const void *addr, unsigned long n)
{
        __asm volatile("cld; rep; outsw"
                       : "+S"(addr), "+c"(n)
                       : "d"(port));
}

void k_display(void)
{
        set_active_font(&font_8x8);
        for (int y = 0; y < TTY_H; ++y)
                for (int x = 0; x < TTY_W; ++x)
                {
                        put_character(system_output[y][x], x * font_8x8.char_width, y * font_8x8.char_height, tty_bg, tty_fg);
                }
}

void k_putch(const char ch)
{

        if (ch == '\n')
        {
                tty_x = 0;
                ++tty_y;
        }
        else if (ch == '\b')
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
        else if (ch == '\t')
        {
                for (int i = 0; i < TAB_SIZE; ++i)
                        k_putch(' ');
        }
        else
        {
                system_output[tty_y][tty_x++] = ch;
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
                                k_print(str);
                                break;
                        }

                        case 'c':
                        {
                                char c = (char)va_arg(args, int);
                                k_putch(c);
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
                        k_putch(*s);
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
