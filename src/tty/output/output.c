#include <tty/output/output.h>

uint8_t system_output[TTY_H][TTY_W] = {0};
uint32_t tty_y = 0, tty_x = 0;
uint32_t tty_bg = rgb(128, 0, 128);
uint32_t tty_fg = rgb(255, 128, 255);

void display(void)
{
        setfont(&font_8x8);
        for (int y = 0; y < TTY_H; ++y)
        {
                for (int x = 0; x < TTY_W; ++x)
                {
                        displaychar(system_output[y][x], x * font_8x8.char_width, y * font_8x8.char_height, tty_bg, tty_fg);
                }
        }
}

void putch(const char ch, char **output, int *tty_x, int *tty_y)
{
        if (ch == '\n')
        {
                *tty_x = 0;
                ++(*tty_y);
        }
        else if (ch == '\b' && tty_x > 0)
        {
                (*tty_x)--;
                output[*tty_y][*tty_x] = ' ';
        }
        else if (ch == '\b' && tty_y > 0)
        {
                (*tty_y)--;
                *tty_x = TTY_W - 1;
                output[*tty_y][*tty_x] = ' ';
        }
        else if (ch == '\t')
                for (int i = 0; i < TAB_SIZE; ++i)
        {
                putch(' ', output, tty_x, tty_y);
        }
        else
        {
                output[*tty_y][(*tty_x)++] = ch;
        }

        if (*tty_x >= TTY_W)
        {
                *tty_x = 0;
                ++(*tty_y);
        }
        if (*tty_y >= TTY_H)
        {
                for (int y = 1; y < TTY_H; y++)
                {
                        for (int x = 0; x < TTY_W; x++)
                        {
                                output[y - 1][x] = output[y][x];
                        }
                }
                for (int x = 0; x < TTY_W; x++)
                {
                        output[TTY_H - 1][x] = ' ';
                }
                *tty_y = TTY_H - 1;
        }
}

void putchar(const char ch)
{
        putch(ch, system_output, &tty_x, &tty_y);
}

static int vsnprintf_helper(char *str, size_t size, const char *fmt, va_list args)
{
        size_t pos = 0;
        const char *s = fmt;

        while (*s && pos < size)
        {
                if (*s == '%')
                {
                        s++;

                        // Check for format flags
                        int width = 0;
                        int precision = -1;
                        bool left_align = false;
                        bool zero_pad = false;

                        // Parse flags
                        while (*s == '-' || *s == '0')
                        {
                                if (*s == '-')
                                        left_align = true;
                                if (*s == '0')
                                        zero_pad = true;
                                s++;
                        }

                        // Parse width
                        if (*s >= '0' && *s <= '9')
                        {
                                width = 0;
                                while (*s >= '0' && *s <= '9')
                                {
                                        width = width * 10 + (*s - '0');
                                        s++;
                                }
                        }
                        else if (*s == '*')
                        {
                                width = va_arg(args, int);
                                s++;
                        }

                        // Parse precision
                        if (*s == '.')
                        {
                                s++;
                                precision = 0;
                                if (*s >= '0' && *s <= '9')
                                {
                                        precision = 0;
                                        while (*s >= '0' && *s <= '9')
                                        {
                                                precision = precision * 10 + (*s - '0');
                                                s++;
                                        }
                                }
                                else if (*s == '*')
                                {
                                        precision = va_arg(args, int);
                                        s++;
                                }
                        }

                        switch (*s)
                        {
                        case 'd':
                        {
                                int num = va_arg(args, int);
                                char buffer[32];
                                int i = 0;
                                bool negative = false;

                                if (num < 0)
                                {
                                        negative = true;
                                        num = -num;
                                }
                                else if (num == 0)
                                {
                                        buffer[i++] = '0';
                                }

                                while (num > 0)
                                {
                                        buffer[i++] = '0' + (num % 10);
                                        num /= 10;
                                }

                                // Handle precision for zero value
                                if (precision == 0 && i == 0)
                                {
                                        if (width > 0)
                                        {
                                                for (int j = 0; j < width && pos < size; j++)
                                                {
                                                        if (str)
                                                                str[pos] = ' ';
                                                        pos++;
                                                }
                                        }
                                        break;
                                }

                                // Apply precision (minimum digits)
                                if (precision > i)
                                {
                                        int padding = precision - i;
                                        for (int j = 0; j < padding; j++)
                                                buffer[i++] = '0';
                                }

                                int total_len = i + (negative ? 1 : 0);

                                // Right align with padding
                                if (!left_align && width > total_len)
                                {
                                        char pad_char = zero_pad ? '0' : ' ';
                                        for (int j = 0; j < width - total_len && pos < size; j++)
                                        {
                                                if (str)
                                                        str[pos] = pad_char;
                                                pos++;
                                        }
                                }

                                // Print sign
                                if (negative && pos < size)
                                {
                                        if (str)
                                                str[pos] = '-';
                                        pos++;
                                }

                                // Print digits
                                while (i > 0 && pos < size)
                                {
                                        if (str)
                                                str[pos] = buffer[--i];
                                        pos++;
                                }

                                // Left align with space padding
                                if (left_align && width > total_len)
                                {
                                        for (int j = 0; j < width - total_len && pos < size; j++)
                                        {
                                                if (str)
                                                        str[pos] = ' ';
                                                pos++;
                                        }
                                }
                                break;
                        }

                        case 's':
                        {
                                char *str_arg = va_arg(args, char *);
                                if (str_arg == NULL)
                                        str_arg = "(null)";

                                int len = 0;
                                const char *p = str_arg;
                                while (*p && (precision == -1 || len < precision))
                                {
                                        len++;
                                        p++;
                                }

                                if (!left_align && width > len)
                                {
                                        for (int j = 0; j < width - len && pos < size; j++)
                                        {
                                                if (str)
                                                        str[pos] = ' ';
                                                pos++;
                                        }
                                }

                                p = str_arg;
                                int printed = 0;
                                while (*p && (precision == -1 || printed < precision) && pos < size)
                                {
                                        if (str)
                                                str[pos] = *p;
                                        pos++;
                                        p++;
                                        printed++;
                                }

                                if (left_align && width > len)
                                {
                                        for (int j = 0; j < width - len && pos < size; j++)
                                        {
                                                if (str)
                                                        str[pos] = ' ';
                                                pos++;
                                        }
                                }
                                break;
                        }

                        case 'c':
                        {
                                char c = (char)va_arg(args, int);

                                if (!left_align && width > 1)
                                {
                                        for (int j = 0; j < width - 1 && pos < size; j++)
                                        {
                                                if (str)
                                                        str[pos] = ' ';
                                                pos++;
                                        }
                                }

                                if (pos < size)
                                {
                                        if (str)
                                                str[pos] = c;
                                        pos++;
                                }

                                if (left_align && width > 1)
                                {
                                        for (int j = 0; j < width - 1 && pos < size; j++)
                                        {
                                                if (str)
                                                        str[pos] = ' ';
                                                pos++;
                                        }
                                }
                                break;
                        }

                        case 'x':
                        case 'X':
                        {
                                unsigned int num = va_arg(args, unsigned int);
                                char buffer[32];
                                int i = 0;
                                const char *digits = (*s == 'X') ? "0123456789ABCDEF" : "0123456789abcdef";

                                if (num == 0)
                                {
                                        buffer[i++] = '0';
                                }

                                while (num > 0)
                                {
                                        buffer[i++] = digits[num & 0xF];
                                        num >>= 4;
                                }

                                // Handle precision for zero value
                                if (precision == 0 && i == 0)
                                {
                                        if (width > 0)
                                        {
                                                for (int j = 0; j < width && pos < size; j++)
                                                {
                                                        if (str)
                                                                str[pos] = ' ';
                                                        pos++;
                                                }
                                        }
                                        break;
                                }

                                // Apply precision
                                if (precision > i)
                                {
                                        int padding = precision - i;
                                        for (int j = 0; j < padding; j++)
                                                buffer[i++] = '0';
                                }

                                int total_len = i;

                                // Right align with padding
                                if (!left_align && width > total_len)
                                {
                                        char pad_char = zero_pad ? '0' : ' ';
                                        for (int j = 0; j < width - total_len && pos < size; j++)
                                        {
                                                if (str)
                                                        str[pos] = pad_char;
                                                pos++;
                                        }
                                }

                                // Print digits
                                while (i > 0 && pos < size)
                                {
                                        if (str)
                                                str[pos] = buffer[--i];
                                        pos++;
                                }

                                // Left align with space padding
                                if (left_align && width > total_len)
                                {
                                        for (int j = 0; j < width - total_len && pos < size; j++)
                                        {
                                                if (str)
                                                        str[pos] = ' ';
                                                pos++;
                                        }
                                }
                                break;
                        }

                        case '%':
                        {
                                if (pos < size)
                                {
                                        if (str)
                                                str[pos] = '%';
                                        pos++;
                                }
                                break;
                        }

                        default:
                        {
                                if (pos < size)
                                {
                                        if (str)
                                                str[pos] = '%';
                                        pos++;
                                }
                                if (pos < size)
                                {
                                        if (str)
                                                str[pos] = *s;
                                        pos++;
                                }
                                break;
                        }
                        }
                }
                else
                {
                        if (str)
                                str[pos] = *s;
                        pos++;
                }

                s++;
        }

        if (pos < size)
        {
                if (str)
                        str[pos] = '\0';
        }
        else if (size > 0)
        {
                if (str)
                        str[size - 1] = '\0';
        }

        return pos;
}

int vsnprintf(char *str, size_t size, const char *fmt, va_list args)
{
        if (str == NULL || size == 0 || fmt == NULL)
        {
                return 0;
        }

        va_list args_copy;
        va_copy(args_copy, args);

        int required_len = vsnprintf_helper(NULL, 0, fmt, args_copy);
        va_end(args_copy);
        int written = vsnprintf_helper(str, size, fmt, args);

        return (written < required_len) ? required_len : written;
}

int snprintf(char *str, size_t size, const char *fmt, ...)
{
        va_list args;
        va_start(args, fmt);
        int result = vsnprintf(str, size, fmt, args);
        va_end(args);
        return result;
}

int printf(const char *fmt, ...)
{
        va_list args;
        va_start(args, fmt);
        int result = vsnprintf(system_output, sizeof(system_output), fmt, args);
        va_end(args);
        return result;
}
