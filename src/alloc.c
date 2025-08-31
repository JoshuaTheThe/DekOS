#include <alloc.h>

memory_information_t minfo;

size_t strlen(const char *x)
{
        const char *p = x;
        while (*p)
                ++p;
        return (size_t)p-(size_t)x;
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

void memset(void *d, uint8_t v, uint32_t len)
{
        for (int i = 0; i < len; ++i)
        {
                ((char *)d)[i] = v;
        }
}

// Initialize memory allocator
void memory_init(uint32_t memory_size)
{
        size_t heap_size = (size_t)(_heap_end - _heap_start);
        minfo.heap_size = heap_size;
        minfo.map = _heap_map_start;
        minfo.raw = _heap_start;
        minfo.mem_size = memory_size;
        memset(_allocations, 0, 0x20000);
}

region_t m_map(uint32_t size)
{
        region_t empty;
        empty.ptr = NULL;
        empty.size = 0;
        uint32_t aligned = align(size);
        uint32_t num_bits = aligned / alignment;
        for (uint32_t i = 0; i < (minfo.heap_size / 8); i++)
        {
                if ((minfo.map[i / 8] & (1 << (i % 8))))
                        continue;
                bool found = true;
                for (uint32_t j = 0; j < num_bits; j++)
                {
                        if (i + j >= (minfo.heap_size / 8) || (minfo.map[(i + j) / 8] & (1 << ((i + j) % 8))))
                        {
                                found = false;
                                break;
                        }
                }

                if (found)
                {
                        for (uint32_t j = 0; j < num_bits; j++)
                        {
                                minfo.map[(i + j) / 8] |= (1 << ((i + j) % 8));
                        }

                        region_t region;
                        region.ptr = (uint8_t *)(minfo.raw + i * alignment);
                        region.size = aligned;
                        return region;
                }
        }

        return empty;
}

void u_map(region_t region)
{
        if (!region.ptr)
                return;

        uint32_t index = (int)((uint8_t *)region.ptr - minfo.raw) / alignment;
        uint32_t nbits = region.size / alignment;

        for (uint32_t i = 0; i < nbits; i++)
        {
                minfo.map[(index + i) / 8] &= ~(1 << ((index + i) % 8));
        }
}

region_t *find_empty_allocation(void)
{
        region_t *alloc = &_allocations[0];
        int max = 0x20000 / sizeof(region_t);
        while ((alloc->ptr && alloc->size) && max)
        {
                alloc += sizeof(region_t);
                max--;
        }

        return alloc;
}

region_t *find_allocation_from_address(void *p)
{
        if (p == NULL) return NULL;
        region_t *alloc = &_allocations[0];
        int max = 0x20000 / sizeof(region_t);
        while ((alloc->ptr != p) && max)
        {
                alloc += sizeof(region_t);
                max--;
        }

        return alloc;
}

void *malloc(int size)
{
        region_t region = m_map(size);
        if (region.size == 0) return NULL;
        region_t *dest = find_empty_allocation();
        *dest = region;
        return region.ptr;
}

void free(void *p)
{
        region_t *src = find_allocation_from_address(p);
        if (src == NULL) return;
        u_map(*src);
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
