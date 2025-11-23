#include <tty/output/output.h>

static uint8_t system_output[TTY_H][TTY_W];
static uint32_t tty_y = 0, tty_x = 0;
static uint32_t tty_bg = rgb(30, 30, 30);
static uint32_t tty_fg = rgb(230, 230, 230);

void display(void)
{
        setfont(&cascadia);
        for (size_t y = 0; y < TTY_H; ++y)
        {
                for (size_t x = 0; x < TTY_W; ++x)
                {
                        displaychar(system_output[y][x], x * cascadia.char_width, y * cascadia.char_height, tty_bg, tty_fg);
                }
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
        outb(0xE9,ch);
        if (ch == '\n')
        {
                *x = 0;
                ++(*y);
        }
        else if (ch == '\b' && *x > 0)
        {
                (*x)--;
                (*output)[*y][*x] = ' ';
        }
        else if (ch == '\b' && *y > 0)
        {
                (*y)--;
                *x = TTY_W - 1;
                (*output)[*y][*x] = ' ';
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
}

void putchar(const uint8_t ch)
{
        putch(ch, &system_output, &tty_x, &tty_y);
}

static size_t vsnprintf_helper(char *str, size_t size, const char *fmt, va_list args)
{
        const char *s = fmt, *p, *hex_digits = "0123456789abcdef";
        int padding, width, precision, total_len, i, j, num;
        char buffer[32], pad_char, *str_arg;
        bool left_align, zero_pad, negative;
        size_t pos = 0;

        while (*s && pos < size)
        {
                if (*s == '%')
                {
                        s++;

                        width = 0;
                        precision = -1;
                        left_align = false;
                        zero_pad = false;

                        while (*s == '-' || *s == '0')
                        {
                                if (*s == '-')
                                        left_align = true;
                                if (*s == '0')
                                        zero_pad = true;
                                s++;
                        }

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

                                num = va_arg(args, int);
                                negative = false;
                                i = 0;

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

                                if (precision == 0 && i == 0)
                                {
                                        if (width > 0)
                                        {
                                                for (j = 0; j < width && pos < size; j++)
                                                {
                                                        if (str)
                                                                str[pos] = ' ';
                                                        pos++;
                                                }
                                        }
                                        break;
                                }

                                if (precision > i)
                                {
                                        padding = precision - i;
                                        for (j = 0; j < padding; j++)
                                                buffer[i++] = '0';
                                }

                                total_len = i + (negative ? 1 : 0);

                                if (!left_align && width > total_len)
                                {
                                        pad_char = zero_pad ? '0' : ' ';
                                        for (j = 0; j < width - total_len && pos < size; j++)
                                        {
                                                if (str)
                                                        str[pos] = pad_char;
                                                pos++;
                                        }
                                }

                                if (negative && pos < size)
                                {
                                        if (str)
                                                str[pos] = '-';
                                        pos++;
                                }

                                while (i > 0 && pos < size)
                                {
                                        if (str)
                                                str[pos] = buffer[--i];
                                        pos++;
                                }

                                if (left_align && width > total_len)
                                {
                                        for (j = 0; j < width - total_len && pos < size; j++)
                                        {
                                                if (str)
                                                        str[pos] = ' ';
                                                pos++;
                                        }
                                }
                                break;

                        case 's':
                        {
                                str_arg = va_arg(args, char *);
                                if (str_arg == NULL)
                                        str_arg = "(null)";

                                total_len = 0;
                                p = str_arg;
                                while (*p && (precision == -1 || total_len < precision))
                                {
                                        total_len++;
                                        p++;
                                }

                                if (!left_align && width > total_len)
                                {
                                        for (j = 0; j < width - total_len && pos < size; j++)
                                        {
                                                if (str)
                                                        str[pos] = ' ';
                                                pos++;
                                        }
                                }

                                p = str_arg;
                                num = 0;
                                while (*p && (precision == -1 || num < precision) && pos < size)
                                {
                                        if (str)
                                                str[pos] = *p;
                                        pos++;
                                        p++;
                                        num++;
                                }

                                if (left_align && width > total_len)
                                {
                                        for (j = 0; j < width - total_len && pos < size; j++)
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
                                pad_char = (char)va_arg(args, int);

                                if (!left_align && width > 1)
                                {
                                        for (j = 0; j < width - 1 && pos < size; j++)
                                        {
                                                if (str)
                                                        str[pos] = ' ';
                                                pos++;
                                        }
                                }

                                if (pos < size)
                                {
                                        if (str)
                                                str[pos] = pad_char;
                                        pos++;
                                }

                                if (left_align && width > 1)
                                {
                                        for (j = 0; j < width - 1 && pos < size; j++)
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
                                num = va_arg(args, int);
                                i = 0;

                                if (num == 0)
                                {
                                        buffer[i++] = '0';
                                }

                                while (num > 0)
                                {
                                        buffer[i++] = hex_digits[num & 0xF];
                                        num >>= 4;
                                }

                                if (precision == 0 && i == 0)
                                {
                                        if (width > 0)
                                        {
                                                for (j = 0; j < width && pos < size; j++)
                                                {
                                                        if (str)
                                                                str[pos] = ' ';
                                                        pos++;
                                                }
                                        }
                                        break;
                                }

                                if (precision > i)
                                {
                                        padding = precision - i;
                                        for (j = 0; j < padding; j++)
                                                buffer[i++] = '0';
                                }

                                total_len = i;

                                if (!left_align && width > total_len)
                                {
                                        pad_char = zero_pad ? '0' : ' ';
                                        for (j = 0; j < width - total_len && pos < size; j++)
                                        {
                                                if (str)
                                                        str[pos] = pad_char;
                                                pos++;
                                        }
                                }

                                while (i > 0 && pos < size)
                                {
                                        if (str)
                                                str[pos] = buffer[--i];
                                        pos++;
                                }

                                if (left_align && width > total_len)
                                {
                                        for (j = 0; j < width - total_len && pos < size; j++)
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
        va_list args_copy;
        int required_len, written;

        if (str == NULL || size == 0 || fmt == NULL)
        {
                return 0;
        }

        va_copy(args_copy, args);
        required_len = (int)vsnprintf_helper(NULL, 0, fmt, args_copy);
        va_end(args_copy);
        written = (int)vsnprintf_helper(str, size, fmt, args);
        return (written < required_len) ? required_len : written;
}

int snprintf(char *str, size_t size, const char *fmt, ...)
{
        int result;
        va_list args;
        va_start(args, fmt);
        result = (int)vsnprintf(str, size, fmt, args);
        va_end(args);
        return result;
}

void printf(const char *fmt, ...)
{
        va_list args;
        va_start(args, fmt);

        const char *s = fmt;
        const char *hex_digits = "0123456789abcdef";
        const char *HEX_digits = "0123456789ABCDEF";
        char buffer[32];
        int i, num;
        unsigned int unum;
        char *str_arg;
        int width, precision, zero_pad, left_justify;
        char length_modifier;

        while (*s)
        {
                if (*s == '%')
                {
                        s++;

                        // Parse flags
                        zero_pad = 0;
                        left_justify = 0;
                        if (*s == '0')
                        {
                                zero_pad = 1;
                                s++;
                        }
                        if (*s == '-')
                        {
                                left_justify = 1;
                                s++;
                        }

                        // Parse width
                        width = 0;
                        while (*s >= '0' && *s <= '9')
                        {
                                width = width * 10 + (*s - '0');
                                s++;
                        }

                        // Parse precision
                        precision = -1;
                        if (*s == '.')
                        {
                                s++;
                                precision = 0;
                                while (*s >= '0' && *s <= '9')
                                {
                                        precision = precision * 10 + (*s - '0');
                                        s++;
                                }
                        }

                        // Parse length modifier
                        length_modifier = 0;
                        if (*s == 'l' || *s == 'h')
                        {
                                length_modifier = *s;
                                s++;
                                if ((length_modifier == 'l' && *s == 'l') ||
                                    (length_modifier == 'h' && *s == 'h'))
                                {
                                        s++; // Skip second character for ll/hh
                                }
                        }

                        switch (*s)
                        {
                        case 'd':
                        case 'i':
                                if (length_modifier == 'l')
                                {
                                        long lnum = va_arg(args, long);
                                        num = (int)lnum; // Simplified for 32-bit systems
                                }
                                else
                                {
                                        num = va_arg(args, int);
                                }

                                if (num < 0)
                                {
                                        putchar('-');
                                        num = -num;
                                }
                                else if (num == 0)
                                {
                                        putchar('0');
                                }
                                else
                                {
                                        i = 0;
                                        while (num > 0)
                                        {
                                                buffer[i++] = '0' + (num % 10);
                                                num /= 10;
                                        }

                                        // Apply padding
                                        if (!left_justify && width > i)
                                        {
                                                for (int j = i; j < width; j++)
                                                {
                                                        putchar(zero_pad ? '0' : ' ');
                                                }
                                        }

                                        while (i > 0)
                                        {
                                                putchar((uint8_t)buffer[--i]);
                                        }

                                        // Right padding for left justification
                                        if (left_justify && width > i)
                                        {
                                                for (int j = i; j < width; j++)
                                                {
                                                        putchar(' ');
                                                }
                                        }
                                }
                                break;

                        case 'u': // Unsigned decimal
                                unum = va_arg(args, unsigned int);
                                i = 0;

                                if (unum == 0)
                                {
                                        putchar('0');
                                }
                                else
                                {
                                        while (unum > 0)
                                        {
                                                buffer[i++] = '0' + (unum % 10);
                                                unum /= 10;
                                        }

                                        // Apply padding
                                        if (!left_justify && width > i)
                                        {
                                                for (int j = i; j < width; j++)
                                                {
                                                        putchar(zero_pad ? '0' : ' ');
                                                }
                                        }

                                        while (i > 0)
                                        {
                                                putchar((uint8_t)buffer[--i]);
                                        }

                                        // Right padding for left justification
                                        if (left_justify && width > i)
                                        {
                                                for (int j = i; j < width; j++)
                                                {
                                                        putchar(' ');
                                                }
                                        }
                                }
                                break;

                        case 's':
                                str_arg = va_arg(args, char *);
                                if (str_arg == NULL)
                                        str_arg = "(null)";

                                // Calculate string length
                                int len = 0;
                                const char *temp = str_arg;
                                while (*temp++)
                                        len++;

                                // Apply precision if specified
                                if (precision >= 0 && precision < len)
                                {
                                        len = precision;
                                }

                                // Left padding
                                if (!left_justify && width > len)
                                {
                                        for (int j = len; j < width; j++)
                                        {
                                                putchar(' ');
                                        }
                                }

                                // Print string (up to precision)
                                for (i = 0; i < len && str_arg[i]; i++)
                                {
                                        putchar((uint8_t)str_arg[i]);
                                }

                                // Right padding
                                if (left_justify && width > len)
                                {
                                        for (int j = len; j < width; j++)
                                        {
                                                putchar(' ');
                                        }
                                }
                                break;

                        case 'c':
                                putchar((uint8_t)va_arg(args, int));
                                break;

                        case 'x': // Lowercase hex
                                unum = va_arg(args, unsigned int);
                                i = 0;

                                if (unum == 0)
                                {
                                        putchar('0');
                                }
                                else
                                {
                                        while (unum > 0)
                                        {
                                                buffer[i++] = hex_digits[unum & 0xF];
                                                unum >>= 4;
                                        }

                                        // Apply padding
                                        if (!left_justify && width > i)
                                        {
                                                for (int j = i; j < width; j++)
                                                {
                                                        putchar(zero_pad ? '0' : ' ');
                                                }
                                        }

                                        while (i > 0)
                                        {
                                                putchar((uint8_t)buffer[--i]);
                                        }

                                        // Right padding for left justification
                                        if (left_justify && width > i)
                                        {
                                                for (int j = i; j < width; j++)
                                                {
                                                        putchar(' ');
                                                }
                                        }
                                }
                                break;

                        case 'X': // Uppercase hex
                                unum = va_arg(args, unsigned int);
                                i = 0;

                                if (unum == 0)
                                {
                                        putchar('0');
                                }
                                else
                                {
                                        while (unum > 0)
                                        {
                                                buffer[i++] = HEX_digits[unum & 0xF];
                                                unum >>= 4;
                                        }

                                        // Apply padding
                                        if (!left_justify && width > i)
                                        {
                                                for (int j = i; j < width; j++)
                                                {
                                                        putchar(zero_pad ? '0' : ' ');
                                                }
                                        }

                                        while (i > 0)
                                        {
                                                putchar((uint8_t)buffer[--i]);
                                        }

                                        // Right padding for left justification
                                        if (left_justify && width > i)
                                        {
                                                for (int j = i; j < width; j++)
                                                {
                                                        putchar(' ');
                                                }
                                        }
                                }
                                break;

                        case 'p': // Pointer
                        {
                                void *ptr = va_arg(args, void *);
                                uintptr_t ptr_val = (uintptr_t)ptr;

                                putchar('0');
                                putchar('x');

                                i = 0;
                                if (ptr_val == 0)
                                {
                                        putchar('0');
                                }
                                else
                                {
                                        while (ptr_val > 0)
                                        {
                                                buffer[i++] = hex_digits[ptr_val & 0xF];
                                                ptr_val >>= 4;
                                        }
                                        while (i > 0)
                                        {
                                                putchar((uint8_t)buffer[--i]);
                                        }
                                }
                        }
                        break;

                        case 'o': // Octal
                                unum = va_arg(args, unsigned int);
                                i = 0;

                                if (unum == 0)
                                {
                                        putchar('0');
                                }
                                else
                                {
                                        while (unum > 0)
                                        {
                                                buffer[i++] = '0' + (unum & 0x7);
                                                unum >>= 3;
                                        }

                                        // Apply padding
                                        if (!left_justify && width > i)
                                        {
                                                for (int j = i; j < width; j++)
                                                {
                                                        putchar(zero_pad ? '0' : ' ');
                                                }
                                        }

                                        while (i > 0)
                                        {
                                                putchar((uint8_t)buffer[--i]);
                                        }

                                        // Right padding for left justification
                                        if (left_justify && width > i)
                                        {
                                                for (int j = i; j < width; j++)
                                                {
                                                        putchar(' ');
                                                }
                                        }
                                }
                                break;

                        case '%':
                                putchar('%');
                                break;

                        default:
                                putchar('%');
                                putchar((uint8_t)*s);
                                break;
                        }
                }
                else
                {
                        putchar((uint8_t)*s);
                }

                s++;
        }

        va_end(args);
        display();
}
