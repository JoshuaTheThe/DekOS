#pragma pack(1)
#include <stdint.h>
#include <stdbool.h>
#include <text.h>
#include <idt.h>
#include <gdt.h>
#include <input.h>

#define cli() __asm("cli")
#define sti() __asm("sti")
#define hlt() __asm("hlt")

typedef struct
{
        uint32_t flags;
        uint32_t mem_lower;   // Amount of lower memory in KB
        uint32_t mem_upper;   // Amount of upper memory in KB
        uint32_t boot_device; // BIOS disk device the OS image was loaded from
        uint32_t cmdline;     // Pointer to kernel command line string
        uint32_t mods_count;  // Number of boot modules loaded
        uint32_t mods_addr;   // Address of the first boot module structure
        uint32_t num;
        uint32_t size;
        uint32_t addr;
        uint32_t shndx;
        uint32_t mmap_length;
        uint32_t mmap_addr;
        uint32_t drives_length;
        uint32_t drives_addr;
        uint32_t config_table;
        uint32_t boot_loader_name;
        uint32_t apm_table;
        uint32_t vbe_control_info;
        uint32_t vbe_mode_info;
        uint16_t vbe_mode;
        uint16_t vbe_interface_seg;
        uint16_t vbe_interface_off;
        uint16_t vbe_interface_len;
        uint64_t framebuffer_addr;
        uint32_t framebuffer_pitch;
        uint32_t framebuffer_width;
        uint32_t framebuffer_height;
        uint8_t framebuffer_bpp;
        uint8_t framebuffer_type;
        union
        {
                struct
                {
                        uint8_t red_field_position;
                        uint8_t red_mask_size;
                        uint8_t green_field_position;
                        uint8_t green_mask_size;
                        uint8_t blue_field_position;
                        uint8_t blue_mask_size;
                } rgb_info;
                struct
                {
                        uint8_t reserved;
                        uint8_t memory_model;
                        uint8_t reserved1[3];
                } indexed_info;
        } color_info;
} multiboot_info_t;

void hang()
{
        cli();
        while (1)
        {
                hlt();
        }
}

int abs(int x)
{
        if (x < 0)
                return -x;
        return x;
}

#define M_PI 3.14159265358979323846

// Custom math functions if not available in your environment
float sqrtf(float x)
{
        // Simple approximation using Newton's method
        if (x < 0)
                return 0;
        if (x == 0)
                return 0;

        float guess = x;
        for (int i = 0; i < 10; i++)
        {
                guess = 0.5f * (guess + x / guess);
        }
        return guess;
}

float expf(float x)
{
        // Taylor series approximation for exp(x)
        float result = 1.0f;
        float term = 1.0f;
        for (int i = 1; i < 10; i++)
        {
                term *= x / i;
                result += term;
        }
        return result;
}

float fminf(float a, float b)
{
        return (a < b) ? a : b;
}

float fabsf(float x)
{
        return (x < 0) ? -x : x;
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
        /* Init e.g. GDT, IDT, PIT, etc. */
        gdt_init();
        idt_init();
        set_active_font(&font_8x8);
        // write("Hello, World!", 13, 16, 16);
        bool keyboardq = is_keyboard_present();
        bool mouseq = is_mouse_present();
        if (!keyboardq)
        {
                print("no keyboard device, plug one in and reboot", 0, 0);
                hang();
        }
        if (!mouseq)
        {
                print("no mouse device, plug one in and reboot", 0, 0);
                hang();
        }
        ps2_initialize_mouse();
        int x=framebuffer_width/2,y=framebuffer_height/2;
        int px=x,py=y;
        int buttons;
        while(1)
        {
                if (buttons & MOUSE_LEFT_BUTTON)
                {
                        restore_pixels(prev_save_x, prev_save_y);
                        put_character('!', x, y, 0xFF000000, 0xFFFFFFFF);
                        save_pixels(prev_save_x, prev_save_y);
                }
                mouse_get(&x,&y,&px,&py,&buttons);
        }
}
