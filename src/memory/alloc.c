#include <memory/alloc.h>

memory_information_t minfo;

// Initialize memory allocator
void memInit(uint32_t memory_size)
{
        size_t heap_size = (size_t)(_heap_end - _heap_start);
        minfo.heap_size = heap_size;
        minfo.map = _heap_map_start;
        minfo.raw = _heap_start;
        minfo.mem_size = memory_size;
        memset(_allocations, 0, 0x20000);
}

region_t m_map(uint32_t size)
{
        region_t empty;
        empty.ptr = NULL;
        empty.size = 0;
        uint32_t aligned = align(size);
        uint32_t num_bits = aligned / alignment;
        for (uint32_t i = 0; i < (minfo.heap_size / 8); i++)
        {
                if ((minfo.map[i / 8] & (1 << (i % 8))))
                        continue;
                bool found = true;
                for (uint32_t j = 0; j < num_bits; j++)
                {
                        if (i + j >= (minfo.heap_size / 8) || (minfo.map[(i + j) / 8] & (1 << ((i + j) % 8))))
                        {
                                found = false;
                                break;
                        }
                }

                if (found)
                {
                        for (uint32_t j = 0; j < num_bits; j++)
                        {
                                minfo.map[(i + j) / 8] |= (1 << ((i + j) % 8));
                        }

                        region_t region;
                        region.ptr = (uint8_t *)(minfo.raw + i * alignment);
                        region.size = aligned;
                        return region;
                }
        }

        return empty;
}

void u_map(region_t region)
{
        if (!region.ptr)
                return;

        uint32_t index = (int)((uint8_t *)region.ptr - minfo.raw) / alignment;
        uint32_t nbits = region.size / alignment;

        for (uint32_t i = 0; i < nbits; i++)
        {
                minfo.map[(index + i) / 8] &= ~(1 << ((index + i) % 8));
        }
}

region_t *find_empty_allocation(void)
{
        region_t *alloc = &_allocations[0];
        int max = 0x20000 / sizeof(region_t);
        while ((alloc->ptr && alloc->size) && max)
        {
                alloc += sizeof(region_t);
                max--;
        }

        return alloc;
}

region_t *find_allocation_from_address(void *p)
{
        if (p == NULL) return NULL;
        region_t *alloc = &_allocations[0];
        int max = 0x20000 / sizeof(region_t);
        while ((alloc->ptr != p) && max)
        {
                alloc += sizeof(region_t);
                max--;
        }

        return alloc;
}

void *malloc(int size)
{
        region_t region = m_map(size);
        if (region.size == 0) return NULL;
        region_t *dest = find_empty_allocation();
        *dest = region;
        return region.ptr;
}

void free(void *p)
{
        region_t *src = find_allocation_from_address(p);
        if (src == NULL) return;
        u_map(*src);
}
