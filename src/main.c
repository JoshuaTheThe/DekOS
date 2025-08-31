#pragma pack(1)
#include <stdint.h>
#include <stdbool.h>
#include <text.h>
#include <idt.h>
#include <gdt.h>
#include <input.h>
#include <rtc.h>
#include <math.h>
#include <utils.h>
#include <stdarg.h>
#include <disk.h>
#include <iso9660.h>
#include <alloc.h>
#include <shell.h>
#include <scheduler.h>

int mx, my;

extern void transfer(void);

/* called by pit */
void tick(void)
{
        outb(0x20, 0x20);
        scheduler();
}

/* int 0x80 */
int sysreply(void)
{
        return 0x40;
}

void hang(void)
{
        cli();
        while (1)
        {
                hlt();
        }
}

#define PIT_FREQUENCY 1193180
#define PIT_CHANNEL0  0x40
#define PIT_COMMAND   0x43
#define PIT_STATUS    0x61

void pit_init(uint32_t frequency)
{
        uint32_t divisor = 1193180 / frequency;
        outb(PIT_COMMAND, 0x36);
        outb(PIT_CHANNEL0, divisor & 0xFF);
        outb(PIT_CHANNEL0, (divisor >> 8) & 0xFF);
}

void main(uint32_t magic, uint32_t mbinfo_ptr)
{
        __asm("cli;");
        while (magic != 0x2BADB002)
        {
                hlt();
        }

        multiboot_info_t *mbi = (multiboot_info_t *)mbinfo_ptr;
        uint64_t memory_size = mbi->mem_upper * 1024 + mbi->mem_lower * 1024;
        for (unsigned int y = 0; y < mbi->framebuffer_height; ++y)
                for (unsigned int x = 0; x < mbi->framebuffer_width; ++x)
                        ((uint32_t *)mbi->framebuffer_addr)[y * mbi->framebuffer_width + x] = 0xFF;
        framebuffer = (uint32_t *)mbi->framebuffer_addr;
        framebuffer_width = mbi->framebuffer_width;
        framebuffer_height = mbi->framebuffer_height;
        mx = framebuffer_width / 2;
        my = framebuffer_height / 2;
        memset(system_output, 0, sizeof(system_output));

        /* Init e.g. GDT, IDT, PIT, etc. */
        set_active_font(&font_8x8);
        gdt_init();
        idt_init();
        sched_init();
        pit_init(100);
        memory_init(memory_size);
        bool fs_valid = iso9660_init();
        if (fs_valid)
        {
                k_print("file system is valid\n");
        }
        else
        {
                k_print("file system is invalid\n");
                hang();
        }

        bool keyboardq = is_keyboard_present();
        bool mouseq = is_mouse_present();
        if (!keyboardq)
        {
                k_print("no keyboard device, plug one in and reboot");
                hang();
        }
        if (!mouseq)
        {
                k_print("no mouse device, plug one in and reboot");
                hang();
        }
        ps2_initialize_mouse();
        int px = mx, py = my;
        uint8_t buttons;

        iso9660_directory fil;
        iso9660_find_file("/boot/grub/grub.cfg", &fil);
        k_print("config size=%d\n", fil.data_length[0]);
        char *data = iso9660_read_file(&fil);
        data[fil.data_length[0]] = 0;
        k_print("config:\n%s", data);
        free(data);

        sti();
        start_process("shell", NULL, 0, (void*)shell, 0, NULL, 8192);
        //shell();
        while (1)
        {
                if (buttons & MOUSE_LEFT_BUTTON)
                {
                        restore_pixels(prev_save_x, prev_save_y);
                        put_character('!', mx, my, 0xFF000000, 0xFFFFFFFF);
                        save_pixels(prev_save_x, prev_save_y);
                }
                mouse_get(&mx, &my, &px, &py, &buttons);
        }
}
