#pragma pack(1)
#include <stdint.h>
#include <stdbool.h>
#include <utils.h>

#include <init/idt.h>
#include <init/gdt.h>

#include <memory/alloc.h>
#include <memory/string.h>

#include <tty/render/render.h>

void main(uint32_t magic, uint32_t mbinfo_ptr)
{
        cli();
        while (magic != 0x2BADB002)
        {
                hlt();
        }
        
        multiboot_info_t *mbi = (multiboot_info_t *)mbinfo_ptr;
        framebuffer_t framebuffer;
        framebuffer.buffer = (uint32_t *)mbi->framebuffer_addr;
        framebuffer.dimensions[0] = mbi->framebuffer_width;
        framebuffer.dimensions[1] = mbi->framebuffer_height;
        setframebuffer(framebuffer);

        gdtInit();
        idtInit();
        memInit(mbi->mem_upper * 1024 + mbi->mem_lower * 1024);

        //mx = framebuffer_width / 2;
        //my = framebuffer_height / 2;
        //memset(system_output, 0, sizeof(system_output));
        //
        ///* Init e.g. GDT, IDT, PIT, etc. */
        //set_active_font(&font_8x8);
        //memory_init(memory_size);
        //bool fs_valid = iso9660_init();
        //if (fs_valid)
        //{
        //        k_print("file system is valid\n");
        //}
        //else
        //{
        //        k_print("file system is invalid\n");
        //        hang();
        //}
        //
        //bool keyboardq = is_keyboard_present();
        //bool mouseq = is_mouse_present();
        //if (!keyboardq)
        //{
        //        k_print("no keyboard device, plug one in and reboot");
        //        hang();
        //}
        //if (!mouseq)
        //{
        //        k_print("no mouse device, plug one in and reboot");
        //        hang();
        //}
        //pciEnumerateDevices(register_device);
        //ps2_initialize_mouse();
        //
        //int px = mx, py = my;
        //uint8_t buttons;
        //
        //iso9660_directory fil;
        //iso9660_find_file("/boot/grub/grub.cfg", &fil);
        //k_print("config size=%d\n", fil.data_length[0]);
        //char *data = iso9660_read_file(&fil);
        //data[fil.data_length[0]] = 0;
        //k_print("config:\n%s", data);
        //free(data);
        //
        //uint8_t shell_stack[8192*2];
        //StartProcess("shell", NULL, 0, (void*)shell, 0, shell_stack, 8192*2);
        //sti();

        while (1)
        {
                //if (buttons & MOUSE_LEFT_BUTTON)
                //{
                //        restore_pixels(prev_save_x, prev_save_y);
                //        put_character('!', mx, my, 0xFF000000, 0xFFFFFFFF);
                //        save_pixels(prev_save_x, prev_save_y);
                //}
                //mouse_get(&mx, &my, &px, &py, &buttons);
        }
}
