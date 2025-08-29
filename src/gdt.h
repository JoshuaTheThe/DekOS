#ifndef GDT_H
#define GDT_H

#include <stdint.h>
#include <stddef.h>
#include <io.h>

#define GDT_SIZE 3

typedef struct __attribute__((packed))
{
        uint16_t limit_low;
        uint16_t base_low;
        uint8_t base_middle;
        uint8_t access;
        uint8_t granularity;
        uint8_t base_high;
} gdt_entry;

typedef struct __attribute__((packed))
{
        uint16_t limit;
        uint32_t base;
} gdt_ptr;

extern void gdt_init(void);

#endif
