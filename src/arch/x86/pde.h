#ifndef PDE_H
#define PDE_H

#include <utils.h>

#define KERNEL_VIRTUAL_BASE  0xC0000000
#define KERNEL_PHYSICAL_BASE 0x00100000

#define PAGE_DIRECTORY_ADDRESS 0x00100000
#define PAGE_TABLES_ADDRESS    0x00101000

#define PAGE_PRESENT    (1 << 0)
#define PAGE_WRITABLE   (1 << 1)
#define PAGE_USER       (1 << 2)
#define PAGE_WRITETHROUGH (1 << 3)
#define PAGE_CACHE_DISABLE (1 << 4)
#define PAGE_ACCESSED   (1 << 5)
#define PAGE_DIRTY      (1 << 6)
#define PAGE_4MB_SIZE   (1 << 7)
#define PAGE_GLOBAL     (1 << 8)

#define PAGE_SIZE_4KB   4096
#define PAGE_SIZE_4M   (4096 * 1024)

#define PAGE_MASK_4KB   0xFFFFF000
#define PAGE_MASK_4M    0xFFC00000

typedef struct __attribute__((__packed__))
{
        uint32_t present : 1;          // Page present in memory
        uint32_t rw : 1;               // Read/write
        uint32_t user : 1;             // User/supervisor
        uint32_t writethrough : 1;     // Cache writethrough
        uint32_t cachedisable : 1;     // Cache disabled
        uint32_t accessed : 1;         // Accessed flag
        uint32_t reserved : 1;         // Must be 0
        uint32_t size : 1;             // Page size (0=4KB, 1=4MB)
        uint32_t ignored : 1;          // Ignored
        uint32_t available : 3;        // Available for OS
        uint32_t page_table_addr : 20; // Page table physical address
} pde_t;

typedef struct __attribute__((__packed__))
{
        uint32_t present : 1;
        uint32_t rw : 1;
        uint32_t user : 1;
        uint32_t writethrough : 1;
        uint32_t cachedisable : 1;
        uint32_t accessed : 1;
        uint32_t dirty : 1;  // Dirty flag
        uint32_t pat : 1;    // Page attribute table
        uint32_t global : 1; // Global page
        uint32_t available : 3;
        uint32_t page_addr : 20; // Page physical address
} pte_t;

// Enable paging in CR0
static inline void enable_paging(void)
{
        asm volatile(
            "mov %%cr0, %%eax\n"
            "or $0x80000000, %%eax\n"
            "mov %%eax, %%cr0\n"
            : : : "eax");
}

static inline void disable_paging(void)
{
        asm volatile(
            "mov %%cr0, %%eax\n"
            "and $0x7FFFFFFF, %%eax\n"
            "mov %%eax, %%cr0\n"
            : : : "eax");
}

static inline void invalidate_tlb(void)
{
        asm volatile(
            "mov %%cr3, %%eax\n"
            "mov %%eax, %%cr3\n"
            : : : "eax");
}

static inline uint32_t get_cr2(void)
{
        uint32_t cr2;
        asm volatile("mov %%cr2, %0" : "=r"(cr2));
        return cr2;
}

static inline void invlpg(uint32_t addr)
{
        asm volatile("invlpg (%0)" : : "r"(addr) : "memory");
}

static inline void load_page_directory(uint32_t addr)
{
    asm volatile("mov %0, %%cr3" : : "r"(addr));
}

typedef pte_t page_table_t[1024];

extern pde_t *page_directory;
extern page_table_t *page_tables;

void PDEInit(void);
void PDEMap(uint32_t virt, uint32_t phys, uint32_t flags);
void PDEPages(uint32_t virt_start, uint32_t phys_start, uint32_t count, uint32_t flags);
uint32_t PDEVToP(uint32_t virt);
uint32_t PDEPToV(uint32_t phys, uint32_t start_virt, uint32_t size);

uint32_t alloc_physical_page(void);
void invlpg(uint32_t addr);
void enable_paging(void);
void load_page_directory(uint32_t addr);

#endif
