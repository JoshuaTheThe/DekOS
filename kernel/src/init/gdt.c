#include <init/gdt.h>
#include <memory/string.h>

static gdtEntry_t gdt[GDT_SIZE];
static gdtPtr_t gdtp;

// gdtTssEntry_t *gdtGetTssEntries(void)
// {
//         return tss;
// }

// void gdtInitTssForTask(int task_num, uint32_t esp0, uint32_t eip, uint32_t esp, uint32_t cr3)
// {
//         memset(&tss[task_num], 0, sizeof(gdtTssEntry_t));
// 
//         tss[task_num].esp0 = esp0;
//         tss[task_num].ss0 = 0x10;
//         tss[task_num].cs = 0x08;
//         tss[task_num].ss = 0x10;
//         tss[task_num].ds = 0x10;
//         tss[task_num].es = 0x10;
//         tss[task_num].fs = 0x10;
//         tss[task_num].gs = 0x10;
//         tss[task_num].eip = eip;
//         tss[task_num].esp = esp;
//         tss[task_num].eflags = 0x200;
//         tss[task_num].cr3 = cr3;
//         tss[task_num].iomap_base = 0xFFFF;  // Allow all I/O ports
// }

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
        // memset(&tss, 0, sizeof(tss));
        // tss.esp0 = 0x10000;
        // tss.ss0 = 0x10;
        // tss.iomap_base = 0xFFFF;  // Allow all I/O ports
}

void gdtInit(void)
{
        gdtp.limit = sizeof(gdt) - 1;
        gdtp.base = (uint32_t)&gdt;
        gdtSetEntry(0, 0, 0, 0, 0);
        gdtSetEntry(1, 0x00000000, 0xffffffff, 0x9A, 0xCF);
        gdtSetEntry(2, 0x00000000, 0xffffffff, 0x92, 0xCF);

        // uint32_t tss_base = (uint32_t)&tss;
        // gdtSetTssEntry(3, tss_base, sizeof(tss) - 1, 0x89, 0x40);

        __asm volatile("lgdt (%0)" : : "r"(&gdtp));
        // __asm volatile("ltr %w0" : : "r"((uint16_t)0x18)); // Load first TSS

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
