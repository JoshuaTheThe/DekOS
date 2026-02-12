#include <memory/alloc.h>
#include <memory/string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <tty/output.h>

static memory_information_t minfo;

uintptr_t _heap_end = 0, _heap_start = 0;
uintptr_t _heap_map_end = 0, _heap_map_start = 0;

void memInit(size_t memory_size)
{
        if (memory_size >= (1 << 29))
        {
                memory_size = (1 << 29);
        }

        extern uint8_t _kernel_start[];
        extern uint8_t _kernel_end[];

        uintptr_t kernel_start_addr = (uintptr_t)&_kernel_start;
        uintptr_t kernel_end_addr = (uintptr_t)&_kernel_end;

        const size_t sizeof_kernel = kernel_end_addr - kernel_start_addr;
        const size_t reserved = 1 * 1024 * 1024;
        size_t available = memory_size - sizeof_kernel - reserved;
        size_t heap_size = (available * 7) / 10;

        heap_size = (heap_size / ALLOC_ALIGNMENT) * ALLOC_ALIGNMENT;

        uint32_t total_bits = heap_size / ALLOC_ALIGNMENT;
        uint32_t map_bytes_needed = (total_bits + 7) / 8;

        if (map_bytes_needed > available - heap_size)
        {
                heap_size = available - map_bytes_needed;
                heap_size = (heap_size / ALLOC_ALIGNMENT) * ALLOC_ALIGNMENT;
                total_bits = heap_size / ALLOC_ALIGNMENT;
                map_bytes_needed = (total_bits + 7) / 8;
        }

        _heap_start = (uintptr_t)&_kernel_end;
        _heap_end = _heap_start + heap_size;
        _heap_map_start = _heap_end;
        _heap_map_end = _heap_map_start + map_bytes_needed;

        minfo.heap_size = heap_size;
        minfo.map = (uint8_t *)_heap_map_start;
        minfo.raw = (uint8_t *)_heap_start;
        minfo.mem_size = memory_size;

        memset(minfo.map, 0, map_bytes_needed);

        printf(" [INFO] %dMB of memory\n", memory_size / (1024*1024));
        printf(" [INFO] kernel is %dKB\n", sizeof_kernel / 1024);
        printf(" [INFO] %dMB of heap space\n", (_heap_end - _heap_start) / (1024*1024));
}

region_t m_map(size_t size)
{
        region_t region;
        uint32_t aligned, num_bits, i, j;
        bool found;

        region.ptr = NULL;
        region.size = 0;

        if (size == 0)
        {
                return region;
        }

        aligned = ALLOC_ALIGN(size);
        num_bits = aligned / ALLOC_ALIGNMENT;

        uint32_t total_bits = minfo.heap_size / ALLOC_ALIGNMENT;

        for (i = 0; i <= total_bits - num_bits; i++)
        {
                if ((minfo.map[i / 8] & (1 << (i % 8))))
                {
                        continue;
                }

                found = true;

                for (j = 0; j < num_bits; j++)
                {
                        uint32_t bit_index = i + j;
                        if (bit_index >= total_bits ||
                            (minfo.map[bit_index / 8] & (1 << (bit_index % 8))))
                        {
                                found = false;
                                break;
                        }
                }

                if (found)
                {
                        for (j = 0; j < num_bits; j++)
                        {
                                uint32_t bit_index = i + j;
                                minfo.map[bit_index / 8] |= (1 << (bit_index % 8));
                        }

                        region.ptr = (uint8_t *)(minfo.raw + (i * ALLOC_ALIGNMENT));
                        region.size = aligned;
                        return region;
                }
        }

        return region;
}

void u_map(region_t region)
{
        uint32_t index, nbits, i;

        if (!region.ptr || region.size == 0)
        {
                return;
        }

        index = (uint32_t)((uint8_t *)region.ptr - minfo.raw) / ALLOC_ALIGNMENT;
        nbits = region.size / ALLOC_ALIGNMENT;

        for (i = 0; i < nbits; i++)
        {
                uint32_t bit_index = index + i;
                minfo.map[bit_index / 8] &= ~(1 << (bit_index % 8));
        }
}

void *malloc(size_t size)
{
        size_t header_size = sizeof(size_t);
        if (header_size < ALLOC_ALIGNMENT)
        {
                header_size = ALLOC_ALIGNMENT;
        }

        region_t region = m_map(size + header_size);

        if (region.size == 0)
        {
                return NULL;
        }

        *(size_t *)region.ptr = size;
        return (void *)((uint8_t *)region.ptr + header_size);
}

void free(void *p)
{
        if (p == NULL)
        {
                return;
        }

        uint8_t *header = (uint8_t *)p - ALLOC_ALIGNMENT;

        region_t region = {
            .ptr = header,
            .size = *(size_t *)header + ALLOC_ALIGNMENT,
        };
        u_map(region);
}

void *calloc(size_t num, size_t size)
{
        size_t total_size = num * size;
        if (size != 0 && total_size / size != num)
        {
                return NULL;
        }

        void *ptr = malloc(total_size);
        if (ptr != NULL)
        {
                memset(ptr, 0, total_size);
        }
        return ptr;
}

void *realloc(void *ptr, size_t new_size)
{
        if (ptr == NULL)
        {
                return malloc(new_size);
        }

        if (new_size == 0)
        {
                free(ptr);
                return NULL;
        }

        void *new_ptr = malloc(new_size);
        if (new_ptr == NULL)
        {
                return NULL;
        }

        size_t copy_size = (new_size < *(size_t *)(ptr - sizeof(size_t))) ? new_size : *(size_t *)(ptr - sizeof(size_t));
        memcpy(new_ptr, ptr, copy_size);

        free(ptr);

        return new_ptr;
}