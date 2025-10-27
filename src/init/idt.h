#ifndef IDT_H
#define IDT_H

#include <stdint.h>
#include <stdbool.h>
#include <io.h>

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

extern void systemcall(void);
extern void invalid_opcode_handler(void);
extern void timer_interrupt_handler(void);
extern void divide_by_zero_handler(void);
extern void general_protection_fault_handler(void);
extern void page_fault_handler(void);
extern void default_handler_wrapper(void);
extern void keyboard_handler_wrapper(void);

#endif
