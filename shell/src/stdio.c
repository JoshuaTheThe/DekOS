#ifdef __WIN32
#include "stdio.h"
#include "string.h"
#else
#include <stdio.h>
#include <string.h>
#endif

#include <stdarg.h>

extern int main(void);

char getchar(void)
{
        char x = -1;
        while(x == -1)
        {
                x = getc();
        }

        return x;
}

unsigned int syscall(unsigned int num, unsigned int arg1, unsigned int arg2, unsigned int arg3)
{
        unsigned int result;
        asm volatile(
            "int $0x80"
            : "=a"(result)
            : "a"(num), "b"(arg1), "c"(arg2), "d"(arg3)
            : "memory");
        return result;
}

/* Process Management */
void exit(int status)
{
        syscall(INT80_EXIT, status, 0, 0);
        while (1)
                ;
}

int yield(void)
{
        return syscall(INT80_YIELD, 0, 0, 0);
}

int yield_to(int pid)
{
        return syscall(INT80_YIELD, pid, 0, 0);
}

int getpid(void)
{
        return syscall(INT80_GET_PID, 0, 0, 0);
}

int getppid(void)
{
        return syscall(INT80_GET_PARENT_PID, 0, 0, 0);
}

int fork(void)
{
        return syscall(INT80_FORK, 0, 0, 0);
}

int progexists(int pid)
{
        return syscall(INT80_PID_EXISTS, pid, 0, 0);
}

void sleep(unsigned int ticks)
{
        syscall(INT80_SLEEP, ticks, 0, 0);
}

/* I/O Operations */
int write(const char *str, unsigned int len)
{
        return syscall(INT80_WRITE, (unsigned int)str, len, 0);
}

int putc(char c)
{
        return syscall(INT80_PUTCH, c, 0, 0);
}

int kbhit(void)
{
        return syscall(INT80_KBHIT, 0, 0, 0);
}

char getc(void)
{
        return (char)syscall(INT80_GETCH, 0, 0, 0);
}

/* IPC */
int sendmsg(int pid, const char *msg, unsigned int size)
{
        return syscall(INT80_SENDMSG, pid, (unsigned int)msg, size);
}

int recvmsg(char *buffer, unsigned int size)
{
        return syscall(INT80_RECVMSG, (unsigned int)buffer, size, 0);
}

bool msgrecv(int sender_pid)
{
        return syscall(INT80_MSGRECV, sender_pid, 0, 0) != 0;
}

void *malloc(unsigned int size)
{
        return (void *)syscall(INT80_ALLOC, size, 0, 0);
}

void free(void *p)
{
        syscall(INT80_UNALLOC, (uint32_t)p, 0, 0);
}

void *ReadFile(const char *FilePath)
{
        Response resp;
        resp.Code = RESPONSE_READ_FILE;
        memcpy(resp.as.bytes, FilePath, strlen(FilePath));
        sendmsg(0, &resp, sizeof(Response));
        int pidn;
        while (1)
        {
                if (!msgrecv(-1))
                        continue;
                pidn = recvmsg(&resp, sizeof(resp));
                if (pidn != 0)
                        continue;
                break;
        }

        return resp.as.P;
}

PID CreateProcess(const char *FilePath)
{
        Response resp;
        resp.Code = RESPONSE_CREATE_PROC;
        memcpy(resp.as.bytes, FilePath, strlen(FilePath));
        sendmsg(0, &resp, sizeof(Response));
        PID pidn;
        while (1)
        {
                if (!msgrecv(-1))
                        continue;
                pidn = recvmsg(&resp, sizeof(resp));
                if (pidn != 0)
                        continue;
                break;
        }

        return *((uint32_t *)&resp.as.bytes[0]);
}

int gets(char *b, int max)
{
        if (!b || max <= 0)
                return 0;

        int i = 0, ch;

        while (i < max - 1)
        {
                ch = (int)getchar();

                if ((char)ch == '\b' || (char)ch == 127)
                {
                        if (i > 0)
                        {
                                i--;
                                putc('\b');
                                putc(' ');
                                putc('\b');
                                b[i] = '\0';
                        }
                }
                else if (ch == '\n' || ch == '\r')
                {
                        putc((uint8_t)ch);
                        break;
                }
                else if (ch >= 32 && ch <= 126)
                {
                        putc((uint8_t)ch);
                        b[i] = (char)ch;
                        i++;
                }
        }

        b[i] = '\0';
        return i;
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
