#include <stdint.h>
#include <stdbool.h>

#define cli() __asm("cli")
#define sti() __asm("sti")
#define hlt() __asm("hlt")

typedef struct
{
        DWORD flags;
        DWORD mem_lower;   // Amount of lower memory in KB
        DWORD mem_upper;   // Amount of upper memory in KB
        DWORD boot_device; // BIOS disk device the OS image was loaded from
        DWORD cmdline;     // Pointer to kernel command line string
        DWORD mods_count;  // Number of boot modules loaded
        DWORD mods_addr;   // Address of the first boot module structure
        DWORD num;
        DWORD size;
        DWORD addr;
        DWORD shndx;
        DWORD mmap_length;
        DWORD mmap_addr;
        DWORD drives_length;
        DWORD drives_addr;
        DWORD config_table;
        DWORD boot_loader_name;
        DWORD apm_table;
        DWORD vbe_control_info;
        DWORD vbe_mode_info;
        uint16_t vbe_mode;
        uint16_t vbe_interface_seg;
        uint16_t vbe_interface_off;
        uint16_t vbe_interface_len;
        QWORD framebuffer_addr;
        DWORD framebuffer_pitch;
        DWORD framebuffer_width;
        DWORD framebuffer_height;
        BYTE framebuffer_bpp;
        BYTE framebuffer_type;
        union
        {
                struct
                {
                        BYTE red_field_position;
                        BYTE red_mask_size;
                        BYTE green_field_position;
                        BYTE green_mask_size;
                        BYTE blue_field_position;
                        BYTE blue_mask_size;
                } rgb_info;
                struct
                {
                        BYTE reserved;
                        BYTE memory_model;
                        BYTE reserved1[3];
                } indexed_info;
        } color_info;
} multiboot_info_t;

void main(
        uint32_t magic,
        uint32_t  mbinfo_ptr,
) {
        cli();
        while (magic != 0x2BADB002)
        {
                hlt();
        }

        multiboot_info_t *mbi = (multiboot_info_t *)mbinfo_ptr;

        /* Init e.g. GDT, IDT, HPET, etc. */
        while(1)
        {
                hlt();
        }
}
