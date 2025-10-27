#include <init/gdt.h>

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

struct tss_entry
{
        uint32_t prev_tss;   // Previous TSS link
        uint32_t esp0;       // Kernel stack pointer
        uint32_t ss0;        // Kernel stack segment
        uint32_t esp1;       // Unused
        uint32_t ss1;        // Unused
        uint32_t esp2;       // Unused
        uint32_t ss2;        // Unused
        uint32_t cr3;        // Unused
        uint32_t eip;        // Unused
        uint32_t eflags;     // Unused
        uint32_t eax;        // Unused
        uint32_t ecx;        // Unused
        uint32_t edx;        // Unused
        uint32_t ebx;        // Unused
        uint32_t esp;        // Unused
        uint32_t ebp;        // Unused
        uint32_t esi;        // Unused
        uint32_t edi;        // Unused
        uint32_t es;         // Unused
        uint32_t cs;         // Unused
        uint32_t ss;         // Unused
        uint32_t ds;         // Unused
        uint32_t fs;         // Unused
        uint32_t gs;         // Unused
        uint32_t ldt;        // Unused
        uint16_t trap;       // Unused
        uint16_t iomap_base; // I/O Map Base Address
} __attribute__((packed));

struct tss_entry tss;

void set_tss_entry(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran)
{
        gdt[num].base_low = (base & 0xFFFF);
        gdt[num].base_middle = (base >> 16) & 0xFF;
        gdt[num].base_high = (base >> 24) & 0xFF;
        gdt[num].limit_low = (limit & 0xFFFF);
        gdt[num].granularity = (limit >> 16) & 0x0F;
        gdt[num].granularity |= (gran & 0xF0);
        gdt[num].access = access;
}

void init_tss()
{
        memset(&tss, 0, sizeof(tss));
        tss.esp0 = 0x10000;
        tss.ss0 = 0x10;
        tss.iomap_base = sizeof(tss);
}

void gdt_init(void)
{
        gdtp.limit = sizeof(gdt) - 1;
        gdtp.base = (uint32_t)&gdt;
        gdt_set_entry(0, 0, 0, 0, 0);
        gdt_set_entry(1, 0x00000000, 0xffffffff, 0x9A, 0xCF);
        gdt_set_entry(2, 0x00000000, 0xffffffff, 0x92, 0xCF);
        set_tss_entry(5, (uint32_t)&tss, sizeof(tss) - 1, 0x89, 0x40);

        __asm volatile("lgdt (%0)" : : "r"(&gdtp));
        asm volatile("ltr %w0" : : "r"((uint16_t)0x28));

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
