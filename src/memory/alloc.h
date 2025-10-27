#ifndef MEMORY_H
#define MEMORY_H

#include <memory/string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <utils.h>

#define alignment 16
#define align(size) (((size) + (alignment - 1)) & ~(alignment - 1))

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
        int size;
} region_t;

// Heap boundaries (set by linker script)
extern uint8_t _heap_start[];
extern uint8_t _heap_end[];
extern uint8_t _heap_map_start[];
extern uint8_t _heap_map_end[];
extern region_t _allocations[];
extern uint8_t _allocations_end[];

void *malloc(int);
void free(void*);
region_t m_map(uint32_t size);
void memInit(uint32_t memory_size);
void u_map(region_t region);

#endif
