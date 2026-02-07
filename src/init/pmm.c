#include <init/pmm.h>
#include <memory/string.h>

static uint32_t *bitmap = (uint32_t *)0x00102000;
static uint32_t total_frames = 0;
static uint32_t used_frames = 0;

void pmm_init(uint32_t total_memory_kb)
{
        total_frames = (total_memory_kb * 1024) / 4096;

        uint32_t bitmap_size = (total_frames + 31) / 32;
        memset(bitmap, 0, bitmap_size * 4);

        for (uint32_t i = 0; i < 256; i++)
        {
                bitmap[i / 32] |= (1 << (i % 32));
                used_frames++;
        }
}

uint32_t alloc_physical_page(void)
{
        for (uint32_t i = 0; i < total_frames; i++)
        {
                uint32_t word = i / 32;
                uint32_t bit = i % 32;

                if (!(bitmap[word] & (1 << bit)))
                {
                        bitmap[word] |= (1 << bit);
                        used_frames++;
                        return i * 4096;
                }
        }
        return 0xFFFFFFFF;
}

void free_physical_page(uint32_t phys)
{
        uint32_t frame = phys / 4096;
        uint32_t word = frame / 32;
        uint32_t bit = frame % 32;

        bitmap[word] &= ~(1 << bit);
        used_frames--;
}
