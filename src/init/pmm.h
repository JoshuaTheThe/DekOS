#ifndef PMM_H
#define PMM_H

#include <stdint.h>

void pmm_init(uint32_t total_memory_kb);
uint32_t alloc_physical_page(void);
void free_physical_page(uint32_t phys);

#endif
