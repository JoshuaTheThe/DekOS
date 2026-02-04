#include <init/idt.h>
#include <tty/render/fonts.h>
#include <tty/render/render.h>
#include <memory/string.h>
#include <memory/alloc.h>
#include <tty/output/output.h>
#include <tty/input/input.h>
#include <drivers/dev/storage/ide.h>

static idtEntry_t idt[256];
static idtPtr_t idtp;

extern KRNLRES *fbRes;

void idtSetEntry(uint8_t n, void *handler, idtEntry_t *idtEntries)
{
        idtEntries[n].base_low = (uint32_t)handler & 0xFFFF;
        idtEntries[n].base_high = ((uint32_t)handler >> 16) & 0xFFFF;
        idtEntries[n].selector = 8;
        idtEntries[n].always_0 = 0;
        idtEntries[n].flags = 0x8E;
}

void idtDefault(int code, int eip, int cs)
{
        cli();
        if (code == 7 || code == 15 || code == 0xfe)
        {
                outb(0x20, 0x0B);
                if (!(inb(0x20) & 0x80))
                {
                        outb(0x20, 0x20);
                }
                return;
        }
        font_t *font = RenderGetFont();
        outb(0x20, 0x20);
        clear();

        int iDim[3];
        if (!fbRes || !fbRes->Region.ptr)
        {
                return;
        }

        RenderGetDim(iDim);
        RenderSetFont(&cascadia);

        memsetdw(fbRes->Region.ptr, rgb(0, 0, 128), iDim[0] * iDim[1]);
        uint32_t px, py;

        /* Messages */
        char *messages[] =
            {
                "An error has occurred. To continue:",
                "Press Enter to return to DekOS, or",
                "Press Escape to restart your computer. If you do this,",
                "you will lose any unsaved information in all open applications.",
            };
        int mc = sizeof(messages) / sizeof(*messages);

        int base_y = iDim[1] / 2 - 64;

        for (int i = 0; i < mc; ++i)
        {
                RenderAlign(messages[i], &px, &py, 32, base_y + 32 + i * font->char_height, ALIGN_LEFT, ALIGN_TOP);
                RenderPrint((unsigned char *)messages[i], px, py, rgb(0, 0, 128), rgb(255, 255, 255));
        }

        /* Other stuff */
        const char *os_name = "  DekOS  ";

        int nDim[3] = {0, 0, 2};
        BOOL nDimM[3] = {0, 0, 1};
        RenderSetDim(nDim, nDimM);
        RenderAlign(os_name, &px, &py, 32, base_y, ALIGN_CENTER, ALIGN_TOP);
        RenderPrint((unsigned char *)os_name, px, py, rgb(255, 255, 255), rgb(0, 0, 128));

        /* Debug */
        nDim[2] = 1;
        RenderSetDim(nDim, nDimM);
        char debug_info[1024];
        snprintf(debug_info, sizeof(debug_info), "Error: %02x : %08x : %08x", code, cs, eip);
        RenderAlign(debug_info, &px, &py, 32, base_y + 32 + (mc + 1) * font->char_height, ALIGN_LEFT, ALIGN_TOP);
        RenderPrint((unsigned char *)debug_info, px, py, rgb(0, 0, 128), rgb(255, 255, 255));

        char x = getchar();
        switch (x)
        {
        case '\n':
        case '\e':
                while (inb(0x64) & 0x02)
                        ;
                outb(0x64, 0xFE);
                for (;;)
                        asm volatile("hlt");
                break;
        }
}

void idtInit(void)
{
        cli();
        for (size_t i = 0; i < IDT_ENTRIES; ++i)
        {
                idtSetEntry((uint8_t)i, (void *)idtDefaultHandler, idt);
        }

        idtSetEntry(0x00, (void *)idtDivideByZeroHandler, idt);
        idtSetEntry(0x01, (void *)idtDebugHandler, idt);
        idtSetEntry(0x02, (void *)idtNMIHandler, idt);
        idtSetEntry(0x03, (void *)idtBreakpointHandler, idt);
        idtSetEntry(0x04, (void *)idtOverflowHandler, idt);
        idtSetEntry(0x05, (void *)idtBoundRangeHandler, idt);
        idtSetEntry(0x06, (void *)idtInvalidOpcodeHandler, idt);
        idtSetEntry(0x07, (void *)idtDeviceNotAvailableHandler, idt);
        idtSetEntry(0x08, (void *)idtDoubleFaultHandler, idt);
        idtSetEntry(0x09, (void *)idtCoprocessorSegmentOverrunHandler, idt);
        idtSetEntry(0x0A, (void *)idtInvalidTSSHandler, idt);
        idtSetEntry(0x0B, (void *)idtSegmentNotPresentHandler, idt);
        idtSetEntry(0x0C, (void *)idtStackSegmentFaultHandler, idt);
        idtSetEntry(0x0D, (void *)idtGeneralProtectionFaultHandler, idt);
        idtSetEntry(0x0E, (void *)idtPageFaultHandler, idt);
        idtSetEntry(0x10, (void *)idtFloatingPointHandler, idt);
        idtSetEntry(0x11, (void *)idtAlignmentCheckHandler, idt);
        idtSetEntry(0x12, (void *)idtMachineCheckHandler, idt);
        idtSetEntry(0x13, (void *)idtSIMDFloatingPointHandler, idt);
        idtSetEntry(0x14, (void *)idtVirtualizationHandler, idt);
        idtSetEntry(0x15, (void *)idtControlProtectionHandler, idt);

        idtSetEntry(0x80, (void *)idtSysCall, idt);

        idtSetEntry(0x20, (void *)idtTimer, idt);
        idtSetEntry(0x21, (void *)idtKeyboardHandler, idt);
        idtSetEntry(0x22, (void *)idtCascadeHandler, idt);
        idtSetEntry(0x23, (void *)idtCOM2Handler, idt);
        idtSetEntry(0x24, (void *)idtCOM1Handler, idt);
        idtSetEntry(0x25, (void *)idtLPT2Handler, idt);
        idtSetEntry(0x26, (void *)idtFloppyHandler, idt);
        idtSetEntry(0x27, (void *)idtLPT1Handler, idt); // Spurious
        idtSetEntry(0x28, (void *)idtRTCHandler, idt);
        idtSetEntry(0x2A, (void *)idtSB16Handler, idt);
        idtSetEntry(0x2C, (void *)idtMouseHandler, idt);

        idtSetEntry(0x2E, (void *)idtIDEHandler, idt);
        idtSetEntry(0x2F, (void *)idtIDEHandler, idt);

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
        outb(0x21, 0xFA);
        outb(0xA1, 0x3F);
}
