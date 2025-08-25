#ifndef IDT_H
#define IDT_H

#include <stdint.h>
#include <stdbool.h>
#include <io.h>
#include <text.h>

#define IDT_ENTRIES 256

extern void idt_init(void);

typedef struct __attribute__((__packed__))
{
        uint16_t base_low;
        uint16_t selector;
        uint8_t always_0;
        uint8_t flags;
        uint16_t base_high;
} idt_entry;

typedef struct __attribute__((__packed__))
{
        uint16_t limit;
        uint32_t base;
} idt_ptr;

#endif
