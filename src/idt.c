#include <idt.h>

extern void systemcall(void);
extern void invalid_opcode_handler(void);
extern void timer_interrupt_handler(void);
extern void divide_by_zero_handler(void);
extern void general_protection_fault_handler(void);
extern void page_fault_handler(void);

idt_entry idt[256];
idt_ptr idtp;

void idt_set_entry(uint8_t n, void *handler, idt_entry *idt)
{
        idt[n].base_low = (uint32_t)handler & 0xFFFF;
        idt[n].base_high = ((uint32_t)handler >> 16) & 0xFFFF;
        idt[n].selector = 8;
        idt[n].always_0 = 0;
        idt[n].flags = 0x8E;
}

void default_handler(void)
{
        outb(0x20, 0x20);
}

extern void default_handler_wrapper(void);
extern void keyboard_handler_wrapper(void);

__asm(
    "default_handler_wrapper:"
    "cli;"
    "pusha;"
    "call default_handler;"
    "popa;"
    "sti;"
    "iret;");

void idt_init(void)
{
        __asm("cli;");
        /* PIC Remap */
        outb(0x20, 0x11);
        outb(0xA0, 0x11);
        outb(0x21, 0x20);
        outb(0xA1, 0x28);
        outb(0x21, 0x04);
        outb(0xA1, 0x02);
        outb(0x21, 0x01);
        outb(0xA1, 0x01);
        outb(0x21, 0x0);
        outb(0xA1, 0x0);

        outb(0x21, inb(0x21) | 0x02);
        outb(0xA1, inb(0xA1) | 0x10);
        for (uint32_t i = 0; i < IDT_ENTRIES; ++i)
        {
                idt_set_entry(i, (void *)default_handler_wrapper, idt);
        }
        idt_set_entry(0x80, (void *)systemcall, idt);
        idt_set_entry(0x20, (void *)timer_interrupt_handler, idt);
        idt_set_entry(0x00, (void *)divide_by_zero_handler, idt);
        idt_set_entry(0x0E, (void *)page_fault_handler, idt);
        idt_set_entry(0x0D, (void *)general_protection_fault_handler, idt);
        idt_set_entry(0x06, (void *)invalid_opcode_handler, idt);

        idtp.limit = (sizeof(idt_entry) * IDT_ENTRIES) - 1;
        idtp.base = (uint32_t)idt;
        __asm("lidt (%0)" : : "r"(&idtp));
        k_print("idt initialized\n");
}
