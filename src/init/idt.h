#ifndef IDT_H
#define IDT_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <io.h>
#include <utils.h>
#include <tty/output/output.h>
#include <tty/input/input.h>

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
void idtDefault(int code, int eip, int cs);
void idtSetEntry(uint8_t n, void *handler, idtEntry_t *idt);

// CPU Exceptions
void idtDivideByZeroHandler(void);
void idtDebugHandler(void);
void idtNMIHandler(void);
void idtBreakpointHandler(void);
void idtOverflowHandler(void);
void idtBoundRangeHandler(void);
void idtInvalidOpcodeHandler(void);
void idtDeviceNotAvailableHandler(void);
void idtDoubleFaultHandler(void);
void idtCoprocessorSegmentOverrunHandler(void); // 0x09
void idtInvalidTSSHandler(void);
void idtSegmentNotPresentHandler(void);
void idtStackSegmentFaultHandler(void);
void idtGeneralProtectionFaultHandler(void);
void idtPageFaultHandler(void);
void idtReserved15Handler(void); // 0x0F
void idtFloatingPointHandler(void);
void idtAlignmentCheckHandler(void);
void idtMachineCheckHandler(void);
void idtSIMDFloatingPointHandler(void);
void idtVirtualizationHandler(void);
void idtControlProtectionHandler(void);
void idtReserved22Handler(void); // 0x16-0x1D
void idtReserved23Handler(void);
void idtReserved24Handler(void);
void idtReserved25Handler(void);
void idtReserved26Handler(void);
void idtReserved27Handler(void);
void idtReserved28Handler(void);
void idtReserved29Handler(void);
void idtSecurityExceptionHandler(void); // 0x1E
void idtReserved31Handler(void);        // 0x1F

// System Interrupts
void idtSysCall(void);
void idtTimer(void);

// IRQ Handlers
void idtKeyboardHandler(void);
void idtCascadeHandler(void);
void idtCOM2Handler(void);
void idtCOM1Handler(void);
void idtLPT2Handler(void);
void idtFloppyHandler(void);
void idtLPT1Handler(void);
void idtRTCHandler(void);
void idtReservedIRQ9Handler(void);  // 0x29
void idtReservedIRQ10Handler(void); // 0x2A
void idtReservedIRQ11Handler(void); // 0x2B
void idtMouseHandler(void);
void idtCoprocessorHandler(void);            // 0x2D
void idtPrimaryATAControllerHandler(void);   // 0x2E
void idtSecondaryATAControllerHandler(void); // 0x2F
void idtSB16Handler(void);

// Default/Spurious
void idtDefaultHandler(void);
void idtSpuriousHandler(void);

#endif
