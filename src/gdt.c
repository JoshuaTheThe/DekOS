#include <gdt.h>

gdt_entry gdt[GDT_SIZE];
gdt_ptr gdtp;

void gdt_set_entry(uint32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran)
{
        gdt[num].base_low = (base & 0xFFFF);
        gdt[num].base_middle = (base >> 16) & 0xFF;
        gdt[num].base_high = (base >> 24) & 0xFF;
        gdt[num].limit_low = (limit & 0xFFFF);
        gdt[num].granularity = ((limit >> 16) & 0x0F) | (gran & 0xF0);
        gdt[num].access = access;
}

void gdt_init(void)
{
        gdtp.limit = sizeof(gdt) - 1;
        gdtp.base = (uint32_t)&gdt;
        gdt_set_entry(0, 0, 0, 0, 0);
        gdt_set_entry(1, 0x00000000, 0xffffffff, 0x9A, 0xCF);
        gdt_set_entry(2, 0x00000000, 0xffffffff, 0x92, 0xCF);
        __asm volatile("lgdt (%0)" : : "r"(&gdtp));

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


