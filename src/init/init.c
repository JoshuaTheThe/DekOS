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

#include <isr/system.h>

#include <pci/pci.h>

void main(uint32_t magic, uint32_t mbinfo_ptr)
{
        multiboot_info_t *mbi;
        framebuffer_t framebuffer;

        cli();
        while (magic != 0x2BADB002)
        {
                hlt();
        }
        
        mbi = (multiboot_info_t *)mbinfo_ptr;
        framebuffer.buffer = (uint32_t *)mbi->framebuffer_addr;
        framebuffer.dimensions[0] = mbi->framebuffer_width;
        framebuffer.dimensions[1] = mbi->framebuffer_height;
        setframebuffer(framebuffer);
        setfont(&font_8x8);

        gdtInit();
        idtInit();
        memInit(mbi->mem_upper * 1024 + mbi->mem_lower * 1024);
        pitInit(25);
        schedInit();

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

        uint8_t stack[8192];
        schedCreateProcess("Shell", NULL, 0, (void*)shell, 0, stack, sizeof(stack));

        int mouse_x = framebuffer.dimensions[0]/2, mouse_y = framebuffer.dimensions[1]/2, prev_x=-1, prev_y=-1;
        uint8_t mouse_buttons = 0;
        sti();

        while (1)
        {
                mouseFetch(&mouse_x, &mouse_y, &prev_x, &prev_y, &mouse_buttons);
        }
}
