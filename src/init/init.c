#pragma pack(1)
#include <stdint.h>
#include <stdbool.h>
#include <utils.h>

#include <init/idt.h>
#include <init/gdt.h>

#include <memory/alloc.h>
#include <memory/string.h>

#include <programs/scheduler.h>
#include <programs/shell.h>

#include <tty/input/input.h>
#include <tty/output/output.h>
#include <tty/render/render.h>

#include <drivers/pit.h>
#include <drivers/iso9660.h>
#include <drivers/speaker.h>
#include <drivers/sb16.h>

#include <isr/system.h>

#include <pci/pci.h>
#include <resource/main.h>

#include <symbols.h>

#include <tty/render/render.h>
#include <tty/render/fonts.h>

#include <wm/main.h>

extern schedProcess_t processes[MAX_PROCS];
extern bool tty_needs_flushing;
extern RID rdFrameRID;
extern KRNLRES *fbRes;

void deleteTask(size_t i)
{
        if (processes[i].stack)
        {
                free(processes[i].stack);
        }

        if (processes[i].program)
        {
                free(processes[i].program);
        }

        memset(&processes[i], 0, sizeof(schedProcess_t));
}

/* Post-Init kernel stuff, e.g. managing procs, and communication */
void kernelTask(multiboot_info_t *mbi)
{
        (void)mbi;
        DWORD mx = 0, my = 0, pmx = 0, pmy = 0, mbuttons = 0;
        size_t stack_size = 8192;
        uint8_t *stack = malloc(stack_size);
        cli();
        schedPid_t pid = schedCreateProcess("shell", NULL, 0, (uint8_t *)shell, 0, stack, stack_size, (schedPid_t){.num = 0, .valid = 1});
        printf("Created proc with id : %d\n", pid.num);

        // speakerPlay(300);
        // pitDelay(10);
        // speakerStop();

        PROCID WMId = WMInit();
        RESULT Result = ResourceHandoverK(fbRes, WMId);
        printf("Handover Result: %d, fbRes=%x\n", Result, fbRes->Region.ptr);
        WMCreateWindow("Kernel Window", 10, 10, 256, 128);
        sti();

        while (true)
        {
                for (int i = 0; i < MAX_PROCS; ++i)
                {
                        if (processes[i].delete)
                        {
                                cli();
                                deleteTask(i);
                                sti();
                        }
                }

                if (tty_needs_flushing)
                {
                        display();
                        tty_needs_flushing = false;
                }

                mouseFetch(&mx, &my, &pmx, &pmy, &mbuttons);
                hlt();
        }
}

/* Initialize the System */
void main(uint32_t magic, uint32_t mbinfo_ptr)
{
        multiboot_info_t *mbi;

        cli();
        while (magic != 0x2BADB002)
        {
                hlt();
        }

        mbi = (multiboot_info_t *)mbinfo_ptr;
        // setframebuffer(framebuffer);
        // setfont(&cascadia); /* bitmap */
        RenderSetFont(&cascadia);

        gdtInit();
        idtInit();
        memInit(mbi->mem_upper * 1024 + mbi->mem_lower * 1024);
        pitInit(250);
        schedInit();

        cli();
        fbRes = ResourceCreateK(NULL, RESOURCE_TYPE_RAW_FAR, 0, schedGetKernelPid(), NULL);
        fbRes->Region.ptr = (uint32_t*)mbi->framebuffer_addr;
        fbRes->Region.size = mbi->framebuffer_width * mbi->framebuffer_height * (mbi->framebuffer_bpp / 8);

        int nDim[3] = {mbi->framebuffer_width, mbi->framebuffer_height, 1};
        BOOL aDim[3] = {TRUE, TRUE, TRUE};
        RenderSetDim(nDim, aDim);

        if (!iso9660Init())
        {
                printf("Could not initialize ISO9660 Driver\n");
                display();
                sysHang();
        }

        if (!keyboardIsPresent())
        {
                printf("Keyboard is not present, please plug one in\n");
                display();
                while (!keyboardIsPresent())
                {
                        sti();
                        hlt();
                }
        }

        if (!mouseIsPresent())
        {
                printf("Mouse is not present, please plug one in\n");
                display();
                while (!mouseIsPresent())
                {
                        sti();
                        hlt();
                }
        }

        /* Stinky inconsistent naming for PS/2 stuff */
        ps2_initialize_mouse();
        ps2_initialize_keyboard();

        display();

        /* so we dont get
           nuked by the scheduler */
        cli();

        /* enumerate devices and save info to array */
        pciEnumerateDevices(pciRegister);

        iso9660Dir_t fil;
        iso9660FindFile("/boot/grub/grub.cfg", &fil);

        char *data = iso9660ReadFile(&fil);
        data[fil.data_length[0]] = 0;
        printf("Grub Configuration:\n%s\n", data);
        free(data);
        display();
        malloc(8192);

        /* Finally; reset the sound blaster */
        sti();
        // for (int i = 0; i < kernel_symbols_count; ++i)
        //{
        //	printf("FUNCTION: %s, %x\n", kernel_symbols[i].name, kernel_symbols[i].address);
        // }

        kernelTask(mbi);
        cli();
        while (1)
        {
                hlt();
        }
}
