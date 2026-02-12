#ifndef MEMORY_H
#define MEMORY_H

#include <utils.h>

// #include <memory/string.h>
// #include <stdint.h>
// #include <stdbool.h>
// #include <stddef.h>
// #include <utils.h>

#define ALLOC_ALIGNMENT (uint32_t)(16)
#define ALLOC_ALIGN(SIZE) (((SIZE) + (ALLOC_ALIGNMENT - 1)) & ~(ALLOC_ALIGNMENT - 1))

typedef struct
{
        uint8_t *raw;
        uint8_t *map;
        uint32_t heap_size;
        uint32_t mem_size;
} memory_information_t;

typedef struct
{
        void *ptr;
        size_t size;
} region_t;

typedef region_t HREGION;

extern uintptr_t _heap_start;
extern uintptr_t _heap_end;
extern uintptr_t _heap_map_start;
extern uintptr_t _heap_map_end;
extern region_t *_allocations;
extern region_t *_allocations_end;

void *malloc(size_t);
void free(void*);
region_t m_map(size_t size);
void memInit(size_t memory_size);
void u_map(region_t region);
region_t *find_allocation_from_address(void *p);
region_t *find_empty_allocation(void);

#endif
