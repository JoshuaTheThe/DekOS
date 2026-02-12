#include <memory/string.h>

size_t strlen(const char *x)
{
        const char *p = x;
        while (*p)
                ++p;
        return (size_t)p - (size_t)x;
}

size_t strnlen(const char *x, size_t max)
{
        const char *p = x;
        while (*p && max--)
                ++p;
        return (size_t)p - (size_t)x;
}

int memcmp(const void *s1, const void *s2, int n)
{
        const unsigned char *p1 = (unsigned char *)s1;
        const unsigned char *p2 = (unsigned char *)s2;

        for (int i = 0; i < n; i++)
        {
                if (p1[i] != p2[i])
                {
                        return p1[i] - p2[i];
                }
        }
        return 0;
}

int memcpy(void *dest, const void *src, int n)
{
        unsigned char *d = (unsigned char *)dest;
        const unsigned char *s = (unsigned char *)src;

        for (int i = 0; i < n; i++)
        {
                d[i] = s[i];
        }
        return n;
}

int strncmp(const void *s1, const void *s2, int n)
{
        const unsigned char *p1 = (unsigned char *)s1;
        const unsigned char *p2 = (unsigned char *)s2;

        for (int i = 0; i < n; i++)
        {
                if (p1[i] != p2[i] || p1[i] == '\0')
                {
                        return p1[i] - p2[i];
                }
        }
        return 0;
}

void memset(void *d, uint8_t v, size_t len)
{
        for (size_t i = 0; i < len; ++i)
        {
                ((uint8_t *)d)[i] = v;
        }
}

void memsetdw(void *d, uint32_t v, size_t len)
{
        for (size_t i = 0; i < len; ++i)
        {
                ((uint32_t *)d)[i] = v;
        }
}

char *strtok(char *str, const char *delim)
{
        static char *save_ptr = NULL;
        char *token_start;

        if (str == NULL)
        {
                if (save_ptr == NULL)
                {
                        return NULL;
                }
                str = save_ptr;
        }

        while (*str != '\0' && strchr(delim, *str) != NULL)
        {
                str++;
        }

        if (*str == '\0')
        {
                save_ptr = NULL;
                return NULL;
        }

        token_start = str;

        while (*str != '\0' && strchr(delim, *str) == NULL)
        {
                str++;
        }

        if (*str != '\0')
        {
                *str = '\0';
                save_ptr = str + 1;
        }
        else
        {
                save_ptr = NULL;
        }

        return token_start;
}

char *strchr(const char *str, int c)
{
        char target = (char)c;
        while (*str != '\0')
        {
                if (*str == target)
                {
                        return (char *)str;
                }
                str++;
        }

        if (target == '\0')
        {
                return (char *)str;
        }

        return NULL;
}

char *strrchr(const char *s, int c)
{
        const char *last = NULL;
        while (*s)
        {
                if (*s == (char)c)
                {
                        last = s;
                }
                s++;
        }
        if ((char)c == '\0')
        {
                return (char *)s;
        }
        return (char *)last;
}

char *strncpy(char *dest, const char *src, size_t n)
{
        size_t i;
        for (i = 0; i < n && src[i] != '\0'; i++)
        {
                dest[i] = src[i];
        }
        for (; i < n; i++)
        {
                dest[i] = '\0';
        }
        return dest;
}

int strcmp(const char *s1, const char *s2)
{
        while (*s1 && (*s1 == *s2))
        {
                s1++;
                s2++;
        }
        return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

int toupper(int c)
{
        if (c >= 'a' && c <= 'z')
        {
                return c - ('a' - 'A');
        }
        return c;
}

int tolower(int c)
{
        if (c >= 'A' && c <= 'Z')
        {
                return c + ('a' - 'A');
        }
        return c;
}

int ncsstrncmp(const char *restrict s1, const char *restrict s2, size_t len)
{
        const unsigned char *p1 = (unsigned char *)s1;
        const unsigned char *p2 = (unsigned char *)s2;

        for (size_t i = 0; i < len; i++)
        {
                if (tolower(p1[i]) != tolower(p2[i]) || p1[i] == '\0')
                {
                        return p1[i] - p2[i];
                }
        }
        return 0;
}

void itos(int value, char *str, int base)
{
        char *ptr = str;
        char *start = str;
        int is_negative = 0;

        if (value < 0 && base == 10)
        {
                is_negative = 1;
                value = -value;
        }

        do
        {
                int digit = value % base;
                *ptr++ = (digit > 9) ? (digit - 10) + 'a' : digit + '0';
                value /= base;
        } while (value != 0);

        if (is_negative)
        {
                *ptr++ = '-';
        }

        *ptr = '\0';

        char *end = ptr - 1;
        while (start < end)
        {
                char temp = *start;
                *start++ = *end;
                *end-- = temp;
        }
}

bool isdigit(char chr)
{
        return chr >= '0' && chr <= '9';
}

int atoi(const char *const str, size_t *len)
{
        int k = 0, i = 0;
        while (str[i] >= '0' && str[i] <= '9')
        {
                k *= 10;
                k += str[i++] - '0';
        }

        if (len)
        {
                *len = i;
        }

        return k;
}

uint32_t strthex(const char *const c_str)
{
        uint32_t h = 0;
        for (size_t i = 0; c_str[i]; ++i)
        {
                h <<= 4;
                if (c_str[i] >= 'A' && c_str[i] <= 'F')
                {
                        h |= c_str[i] - ('A' - 10);
                }
                else if (c_str[i] >= '0' && c_str[i] <= '9')
                {
                        h |= c_str[i] - '0';
                }
                else
                {
                        break;
                }
        }

        return h;
}
