#include <init/pde.h>
#include <memory/string.h>
#include <init/pmm.h>

pde_t *page_directory = (pde_t *)PAGE_DIRECTORY_ADDRESS;
page_table_t *page_tables = (page_table_t *)PAGE_TABLES_ADDRESS;

__attribute__((unused))
static void identity_map_4mb(uint32_t virt_start, uint32_t phys_start)
{
        for (uint32_t i = 0; i < 1024; i++)
        {
                PDEMap(virt_start + i * PAGE_SIZE_4KB,
                       phys_start + i * PAGE_SIZE_4KB,
                       PAGE_PRESENT | PAGE_WRITABLE);
        }
}

__attribute__((unused))
static void map_pages(uint32_t virt_start, uint32_t phys_start, uint32_t count, uint32_t flags)
{
        PDEPages(virt_start, phys_start, count, flags);
}

void PDEInit(void)
{
        uint32_t pd_phys = alloc_physical_page();
        pde_t *pd_virt = (pde_t *)(pd_phys + KERNEL_VIRTUAL_BASE);

        uint32_t pt_phys[4];
        for (int i = 0; i < 4; i++)
        {
                pt_phys[i] = alloc_physical_page();
        }

        memset(pd_virt, 0, 4096);
        for (int i = 0; i < 4; i++)
        {
                memset((void *)(pt_phys[i] + KERNEL_VIRTUAL_BASE), 0, 4096);
        }

        for (int i = 0; i < 4; i++)
        {
                pd_virt[i].present = 1;
                pd_virt[i].rw = 1;
                pd_virt[i].page_table_addr = pt_phys[i] >> 12;
        }

        pte_t *pt0 = (pte_t *)(pt_phys[0] + KERNEL_VIRTUAL_BASE);
        for (int i = 0; i < 1024; i++)
        {
                pt0[i].present = 1;
                pt0[i].rw = 1;
                pt0[i].page_addr = i;
        }

        pte_t *pt1 = (pte_t *)(pt_phys[1] + KERNEL_VIRTUAL_BASE);
        for (int i = 0; i < 1024; i++)
        {
                pt1[i].present = 1;
                pt1[i].rw = 1;
                pt1[i].page_addr = i;
        }

        page_directory = pd_virt;
        page_tables = (page_table_t *)(pt_phys[0] + KERNEL_VIRTUAL_BASE);
        load_page_directory(pd_phys);
        invalidate_tlb();
}

void PDEMap(uint32_t virt, uint32_t phys, uint32_t flags)
{
        uint32_t pd_index = virt >> 22;
        uint32_t pt_index = (virt >> 12) & 0x3FF;

        if (!(page_directory[pd_index].present))
        {
                uint32_t pt_phys = alloc_physical_page();
                page_table_t *new_table = (page_table_t *)(pt_phys + KERNEL_VIRTUAL_BASE);
                memset(new_table, 0, 4096);
                for (size_t i = 0; i < 1024; i++)
                {
                        page_tables[pd_index][i] = (*new_table)[i];
                }
                page_directory[pd_index].present = 1;
                page_directory[pd_index].rw = 1;
                page_directory[pd_index].page_table_addr = pt_phys >> 12;
        }

        page_table_t *pt = &page_tables[pd_index];

        (*pt)[pt_index].present = 1;
        (*pt)[pt_index].rw = (flags & PAGE_WRITABLE) ? 1 : 0;
        (*pt)[pt_index].user = (flags & PAGE_USER) ? 1 : 0;
        (*pt)[pt_index].page_addr = phys >> 12;

        invlpg(virt);
}

void PDEPages(uint32_t virt_start, uint32_t phys_start, uint32_t count, uint32_t flags)
{
        for (uint32_t i = 0; i < count; i++)
        {
                PDEMap(virt_start + i * PAGE_SIZE_4KB,
                       phys_start + i * PAGE_SIZE_4KB,
                       flags);
        }
}

uint32_t PDEVToP(uint32_t virt)
{
        uint32_t pd_index = virt >> 22;
        uint32_t pt_index = (virt >> 12) & 0x3FF;
        uint32_t offset = virt & 0xFFF;

        if (!page_directory[pd_index].present)
        {
                return 0xFFFFFFFF;
        }

        page_table_t *pt = &page_tables[pd_index];

        if (!(*pt)[pt_index].present)
        {
                return 0xFFFFFFFF;
        }

        uint32_t phys = ((*pt)[pt_index].page_addr << 12) | offset;
        return phys;
}

uint32_t PDEPToV(uint32_t phys, uint32_t start_virt, uint32_t size)
{
        for (uint32_t virt = start_virt; virt < start_virt + size; virt += PAGE_SIZE_4KB)
        {
                if (PDEVToP(virt) == phys)
                {
                        return virt;
                }
        }
        return 0xFFFFFFFF;
}