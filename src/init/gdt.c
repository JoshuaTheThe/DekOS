#include <init/gdt.h>

gdtTssEntry_t tss;
gdtEntry_t gdt[GDT_SIZE];
gdtPtr_t gdtp;

void gdtSetEntry(uint32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran)
{
        gdt[num].base_low = (base & 0xFFFF);
        gdt[num].base_middle = (base >> 16) & 0xFF;
        gdt[num].base_high = (base >> 24) & 0xFF;
        gdt[num].limit_low = (limit & 0xFFFF);
        gdt[num].granularity = ((limit >> 16) & 0x0F) | (gran & 0xF0);
        gdt[num].access = access;
}

void gdtSetTssEntry(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran)
{
        gdt[num].base_low = (base & 0xFFFF);
        gdt[num].base_middle = (base >> 16) & 0xFF;
        gdt[num].base_high = (base >> 24) & 0xFF;
        gdt[num].limit_low = (limit & 0xFFFF);
        gdt[num].granularity = (limit >> 16) & 0x0F;
        gdt[num].granularity |= (gran & 0xF0);
        gdt[num].access = access;
}

void gdtTssInit(void)
{
        memset(&tss, 0, sizeof(tss));
        tss.esp0 = 0x10000;
        tss.ss0 = 0x10;
        tss.iomap_base = sizeof(tss);
}

void gdtInit(void)
{
        gdtp.limit = sizeof(gdt) - 1;
        gdtp.base = (uint32_t)&gdt;
        gdtTssInit();
        gdtSetEntry(0, 0, 0, 0, 0);
        gdtSetEntry(1, 0x00000000, 0xffffffff, 0x9A, 0xCF);
        gdtSetEntry(2, 0x00000000, 0xffffffff, 0x92, 0xCF);
        gdtSetTssEntry(5, (uint32_t)&tss, sizeof(tss) - 1, 0x89, 0x40);

        __asm volatile("lgdt (%0)" : : "r"(&gdtp));
        __asm volatile("ltr %w0" : : "r"((uint16_t)0x28));

        __asm volatile(
            "ljmp $0x08, $reload_segments \n\t"
            "reload_segments: \n\t"
            "mov $0x10, %%ax \n\t"
            "mov %%ax, %%ds \n\t"
            "mov %%ax, %%es \n\t"
            "mov %%ax, %%fs \n\t"
            "mov %%ax, %%gs \n\t"
            "mov %%ax, %%ss \n\t"
            :
            :
            : "memory", "eax");
}
