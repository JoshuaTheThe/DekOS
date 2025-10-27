#ifndef IDT_H
#define IDT_H

#include <stdint.h>
#include <stdbool.h>
#include <io.h>

#define IDT_ENTRIES 256

typedef struct __attribute__((__packed__))
{
        uint16_t base_low;
        uint16_t selector;
        uint8_t always_0;
        uint8_t flags;
        uint16_t base_high;
} idtEntry_t;

typedef struct __attribute__((__packed__))
{
        uint16_t limit;
        uint32_t base;
} idtPtr_t;

void idtInit(void);

extern void idtSysCall(void);
extern void idtInvalidOpcodeHandler(void);
extern void idtTimer(void);
extern void idtDivideByZeroHandler(void);
extern void idtGeneralProtectionFaultHandler(void);
extern void idtPageFault(void);
extern void idtDefaultHandler(void);

#endif
