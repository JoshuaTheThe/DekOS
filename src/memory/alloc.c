#include <memory/alloc.h>
#include <memory/string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <tty/output/output.h>

static memory_information_t minfo;

void memInit(size_t memory_size)
{
        size_t heap_size = (size_t)(_heap_end - _heap_start);

        uint32_t total_bits = heap_size / ALLOC_ALIGNMENT;
        uint32_t map_bytes_needed = (total_bits + 7) / 8;
        uint32_t map_bytes_available = (size_t)(_heap_map_end - _heap_map_start);

        if (map_bytes_needed > map_bytes_available)
        {
                total_bits = map_bytes_available * 8;
                heap_size = total_bits * ALLOC_ALIGNMENT;
        }

        minfo.heap_size = heap_size;
        minfo.map = _heap_map_start;
        minfo.raw = _heap_start;
        minfo.mem_size = memory_size;

        memset(minfo.map, 0, map_bytes_needed);

        size_t allocations_size = (size_t)(_allocations_end - (uint8_t *)_allocations);
        memset(_allocations, 0, allocations_size);
}

region_t m_map(size_t size)
{
        region_t region;
        uint32_t aligned, num_bits, i, j;
        bool found;

        region.ptr = NULL;
        region.size = 0;

        // Handle size 0
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

region_t *find_empty_allocation(void)
{
        region_t *alloc = &_allocations[0];
        size_t max = (size_t)(_allocations_end - (uint8_t *)_allocations) / sizeof(region_t);

        for (size_t i = 0; i < max; i++, alloc++)
        {
                if (alloc->ptr == NULL && alloc->size == 0)
                {
                        return alloc;
                }
        }
        return NULL;
}

region_t *find_allocation_from_address(void *p)
{
        if (p == NULL)
        {
                return NULL;
        }

        region_t *alloc = &_allocations[0];
        size_t max = (size_t)(_allocations_end - (uint8_t *)_allocations) / sizeof(region_t);

        for (size_t i = 0; i < max; i++, alloc++)
        {
                if (alloc->ptr == p)
                {
                        return alloc;
                }
        }
        return NULL;
}

void *malloc(size_t size)
{
        region_t region = m_map(size);
        region_t *dest;

        if (region.size == 0)
        {
                return NULL;
        }

        dest = find_empty_allocation();
        if (dest == NULL)
        {
                printf("Could not allocate %d bytes\n", size);
                u_map(region);
                return NULL;
        }

        *dest = region;
        return region.ptr;
}

void free(void *p)
{
        if (p == NULL)
        {
                return;
        }

        region_t *src = find_allocation_from_address(p);
        if (src == NULL)
        {
                return;
        }

        u_map(*src);

        src->ptr = NULL;
        src->size = 0;
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

        region_t *old_alloc = find_allocation_from_address(ptr);
        if (old_alloc == NULL)
        {
                return NULL;
        }

        void *new_ptr = malloc(new_size);
        if (new_ptr == NULL)
        {
                return NULL;
        }

        size_t copy_size = (new_size < old_alloc->size) ? new_size : old_alloc->size;
        memcpy(new_ptr, ptr, copy_size);

        free(ptr);

        return new_ptr;
}