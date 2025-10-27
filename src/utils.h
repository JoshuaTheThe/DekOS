#ifndef UTILS_H
#define UTILS_H

#ifndef NULL
#define NULL (void*)(0)
#endif

#define cli() __asm("cli")
#define sti() __asm("sti")
#define hlt() __asm("hlt")

#define rgb(r,g,b) (0xFF000000 | (r << 16) | (g << 8) | (b))

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

#endif
