#include <init/idt.h>

idtEntry_t idt[256];
idtPtr_t idtp;

void idtSetEntry(uint8_t n, void *handler, idtEntry_t *idt)
{
        idt[n].base_low = (uint32_t)handler & 0xFFFF;
        idt[n].base_high = ((uint32_t)handler >> 16) & 0xFFFF;
        idt[n].selector = 8;
        idt[n].always_0 = 0;
        idt[n].flags = 0x8E;
}

void idtDefault(void)
{
        outb(0x20, 0x20);
}

void idtInit(void)
{
        cli();
        for (uint32_t i = 0; i < IDT_ENTRIES; ++i)
        {
                idtSetEntry(i, (void *)idtDefaultHandler, idt);
        }

        idtSetEntry(0x80, (void *)idtSysCall, idt);
        idtSetEntry(0x20, (void *)idtTimer, idt);
        idtSetEntry(0x00, (void *)idtDivideByZeroHandler, idt);
        idtSetEntry(0x0E, (void *)idtPageFault, idt);
        idtSetEntry(0x0D, (void *)idtGeneralProtectionFaultHandler, idt);
        idtSetEntry(0x06, (void *)idtInvalidOpcodeHandler, idt);

        idtp.limit = (sizeof(idtEntry_t) * IDT_ENTRIES) - 1;
        idtp.base = (uint32_t)idt;
        __asm("lidt (%0)" : : "r"(&idtp));

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
        outb(0xA1, inb(0xA1) & ~(1 << 4));
}
