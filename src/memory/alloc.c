#include <memory/alloc.h>

static memory_information_t minfo;

// Initialize memory allocator
void memInit(size_t memory_size)
{
        size_t heap_size = (size_t)(_heap_end - _heap_start);
        minfo.heap_size = heap_size;
        minfo.map = _heap_map_start;
        minfo.raw = _heap_start;
        minfo.mem_size = memory_size;
        memset(_allocations, 0, 0x20000);
}

region_t m_map(size_t size)
{
        region_t region;
        uint32_t aligned, num_bits, i, j;
        bool found;

        region.ptr = NULL;
        region.size = 0;
        aligned = alloc_align(size);
        num_bits = aligned / alloc_alignment;

        for (i = 0; i < (minfo.heap_size / 8); i++)
        {
                if ((minfo.map[i / 8] & (1 << (i % 8))))
                        continue;
                found = true;
                for (j = 0; j < num_bits; j++)
                {
                        if (i + j >= (minfo.heap_size / 8) || (minfo.map[(i + j) / 8] & (1 << ((i + j) % 8))))
                        {
                                found = false;
                                break;
                        }
                }

                if (found)
                {
                        for (j = 0; j < num_bits; j++)
                        {
                                minfo.map[(i + j) / 8] |= (1 << ((i + j) % 8));
                        }

                        region.ptr = (uint8_t *)(minfo.raw + i * alloc_alignment);
                        region.size = aligned;
                        return region;
                }
        }

        return region;
}

void u_map(region_t region)
{
        uint32_t index, nbits, i;
        if (!region.ptr)
                return;

        index = (uint32_t)((uint8_t *)region.ptr - minfo.raw) / alloc_alignment;
        nbits = region.size / alloc_alignment;

        for (i = 0; i < nbits; i++)
        {
                minfo.map[(index + i) / 8] &= ~(1 << ((index + i) % 8));
        }
}

region_t *find_empty_allocation(void)
{
        region_t *alloc = &_allocations[0];
        uint32_t max = 0x20000 / sizeof(region_t);

        for (uint32_t i = 0; i < max; i++, alloc++)
        {
                if (alloc->ptr == NULL && alloc->size == 0)
                {
                        return alloc;
                }
        }
        return NULL; // No space left
}

region_t *find_allocation_from_address(void *p)
{
        if (p == NULL)
                return NULL;

        region_t *alloc = &_allocations[0];
        uint32_t max = 0x20000 / sizeof(region_t);

        for (uint32_t i = 0; i < max; i++, alloc++)
        {
                if (alloc->ptr == p)
                {
                        return alloc;
                }
        }
        return NULL; // Not found
}

void *malloc(size_t size)
{
        region_t region = m_map(size), *dest;
        if (region.size == 0)
                return NULL;
        dest = find_empty_allocation();
        *dest = region;
        return region.ptr;
}

void free(void *p)
{
        region_t *src = find_allocation_from_address(p);
        if (src == NULL)
                return;
        u_map(*src);
}
