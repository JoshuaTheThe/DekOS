#ifndef GDT_H
#define GDT_H

#include <stdint.h>
#include <stddef.h>
#include <memory/string.h>
#include <io.h>

#define GDT_SIZE 6

typedef struct __attribute__((packed))
{
        uint16_t limit_low;
        uint16_t base_low;
        uint8_t base_middle;
        uint8_t access;
        uint8_t granularity;
        uint8_t base_high;
} gdtEntry_t;

typedef struct __attribute__((packed))
{
        uint16_t limit;
        uint32_t base;
} gdtPtr_t;

typedef struct __attribute__((packed))
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
} gdtTssEntry_t;

void gdtInit(void);
void gdtTssInit(void);
void gdtSetTssEntry(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);
void gdtSetEntry(uint32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);

#endif
