#ifndef GDT_H
#define GDT_H

#include <stdint.h>
#include <stddef.h>
#include <memory/string.h>
#include <io.h>

#define MAX_PROCS 512
#define GDT_SIZE 3 + MAX_PROCS

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
        uint32_t prev_tss;
        uint32_t esp0;
        uint32_t ss0;
        uint32_t esp1;
        uint32_t ss1;
        uint32_t esp2;
        uint32_t ss2;
        uint32_t cr3;
        uint32_t eip;
        uint32_t eflags;
        uint32_t eax;
        uint32_t ecx;
        uint32_t edx;
        uint32_t ebx;
        uint32_t esp;
        uint32_t ebp;
        uint32_t esi;
        uint32_t edi;
        uint32_t es;
        uint32_t cs;
        uint32_t ss;
        uint32_t ds;
        uint32_t fs;
        uint32_t gs;
        uint32_t ldt;
        uint16_t trap;
        uint16_t iomap_base;
} gdtTssEntry_t;

void gdtInit(void);
void gdtTssInit(void);
void gdtSetTssEntry(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);
void gdtSetEntry(uint32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);
gdtTssEntry_t *gdtGetTssEntries();
void gdtInitTssForTask(int task_num, uint32_t esp0, uint32_t eip, uint32_t esp, uint32_t cr3);

#endif
