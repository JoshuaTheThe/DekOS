#ifndef UTILS_H
#define UTILS_H

#ifndef NULL
#define NULL (void*)(0)
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

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

typedef unsigned long long SIZE;
typedef unsigned long long SIZE_T;
typedef void *RAWPTR;

typedef bool BOOL;

#define TRUE (1)
#define FALSE (0)

typedef enum
{
        RESULT_OK,
        RESULT_FAILED_TO_ALLOCATE,
        RESULT_FAILED_TO_MAP, /* malloc but it returns a structure */
        RESULT_FAILED_TO_UNALLOCATE,
        RESULT_FAILED_TO_UMAP,
        RESULT_INVALID_ARGUMENTS,
        RESULT_PERMISSIONS_ERROR,
        RESULT_HAS_CHILDREN,
        RESULT_ALREADY_UNLINKED,
        RESULT_CORRUPTED,
        RESULT_CORRUPTED_SCHEDULER, /* the holy fuck error */
        RESULT_OK_WITH_ERRORS,
        RESULT_TODO, /* NOT implemented */
        RESULT_ARGUMENT_DOES_NOT_EXIST,
        RESULT_NO_OPERATION,
        RESULT_BOUNDING_OFFSET_ERROR,
        RESULT_BOUNDING_OVERFLOW,
        RESULT_BOUNDING_UNDERFLOW,
        RESULT_UNKNOWN_ERROR,
} RESULT;

extern uint32_t text_start, text_end;
extern uint32_t rodata_start, rodata_end;
extern uint32_t data_start, data_end;
extern uint32_t bss_start, bss_end;

#endif
