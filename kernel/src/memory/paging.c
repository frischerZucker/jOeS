#include "memory/paging.h"

#include "string.h"

#include "cpu/hcf.h"
#include "logging.h"
#include "memory/pmm.h"

#define PAGE_TABLE_NUM_ENTRIES 512

static uintptr_t paging_hhdm_offset = 0;

static uintptr_t paging_get_virt_address_from_indices(uint64_t pml4_idx, uint64_t pdpr_idx, uint64_t pd_idx, uint64_t pt_idx)
{
    uintptr_t virt_address = 0;

    uint64_t sign_extension = 0;
    if (pml4_idx >> 8 == 1)
    {
        sign_extension = 0xffff;
    }
    
    virt_address = (sign_extension << 48) | (pml4_idx << 39) | (pdpr_idx << 30) | (pd_idx << 21) | (pt_idx << 12);

    return virt_address;
}

static union page_table_entry_t paging_create_entry(uintptr_t base_address, uint64_t flags)
{
    union page_table_entry_t new_entry = {0};

    // new_entry.fields.present = 1;
    // // For now I make all pages writable. -> If it works I can do this correctly..
    // new_entry.fields.writable = 1;

    // if (flags & PAGING_FLAG_PAGE_SIZE)
    // {
    //     new_entry.fields.page_size = 1;
    // }
    new_entry.raw = flags;

    new_entry.fields.base_address = (base_address >> 12);

    return new_entry;
}

static uint64_t paging_get_flags_from_entry(union page_table_entry_t entry)
{
    uint64_t flags = entry.raw;
    
    flags = flags & PAGING_FLAG_ALL;

    return flags;
}

/*!
    @brief Recursively traverse and log the structure of a page table.

    Walks through all entries of a given page table (PML4, PDPR, PD, PT) and prints information about each non-empty entry.
    For entries that reference lower-level tables, the function recursively descends into the next level.
    Pages larger than 4kB are detected and logged without further recursion.
    
    Output format:
        PML4@[pml4_idx]: [pml4_entry]\n
        PDPR-[pml4_idx]@[pdpr_idx: [pdpr_entry]\n
        PD-[pml4_idx]-[pdpr_idx]@[pd_idx]: [pd_entry]\n
        PT-[pml4_idx]-[pdpr_idx]-[pd_idx]@[pt_idx]: [pt_entry]\n

    @param page_table Pointer to the current page table.
    @param hhdm_offset Offset for phys<->virt address translation.
    @param level Current page table level (PML4, PDPR, PD, PT).
*/
void dump_page_table(union page_table_entry_t *page_table, uint64_t hhdm_offset, page_table_level_t level)
{
    static int16_t pml4_idx = -1;
    static int16_t pdpr_idx = -1;
    static int16_t pd_idx = -1;
    static int16_t pt_idx = -1;

    for (int16_t idx = 0; idx < PAGE_TABLE_NUM_ENTRIES; idx++)
    {
        // Skip empty entries.
        if (page_table[idx].raw == 0)
        {
            continue;
        }
        
        switch (level)
        {
        case PML4:
            pml4_idx = idx;
            LOG_INFO("PML4@%d: %x", pml4_idx, page_table[pml4_idx].raw);
            union page_table_entry_t *pdpr = (union page_table_entry_t *) ((page_table[pml4_idx].fields.base_address << 12) + hhdm_offset);
            dump_page_table(pdpr, hhdm_offset, PDPR);
            break;

        case PDPR:
            pdpr_idx = idx;

            if (page_table[pdpr_idx].fields.page_size != 0)
            {
                LOG_INFO("PDPR-%d@%d: %x (1GB-PAGE)", pml4_idx, pdpr_idx, page_table[pdpr_idx].raw);
                break;
            }

            LOG_INFO("PDPR-%d@%d: %x", pml4_idx, pdpr_idx, page_table[pdpr_idx].raw);
            union page_table_entry_t *pd = (union page_table_entry_t *) ((page_table[pdpr_idx].fields.base_address << 12) + hhdm_offset);
            dump_page_table(pd, hhdm_offset, PD);
            break;

        case PD:
            pd_idx = idx;

            if (page_table[pd_idx].fields.page_size != 0)
            {
                LOG_INFO("PD-%d-%d@%d: %x (2MB-PAGE)", pml4_idx, pdpr_idx, pd_idx, page_table[pd_idx].raw);
                break;
            }

            LOG_INFO("PD-%d-%d@%d: %x", pml4_idx, pdpr_idx, pd_idx, page_table[pd_idx].raw);
            union page_table_entry_t *pt = (union page_table_entry_t *) ((page_table[pd_idx].fields.base_address << 12) + hhdm_offset);
            dump_page_table(pt, hhdm_offset, PT);
            break;

        case PT:
            pt_idx = idx;
            LOG_INFO("PT-%d-%d-%d@%d: %x", pml4_idx, pdpr_idx, pd_idx, pt_idx, page_table[pt_idx].raw);
            break;

        default:
            break;
        }
    }
}

paging_error_codes_t paging_clone_page_table(union page_table_entry_t *old_page_table, union page_table_entry_t **new_pml4, uint64_t hhdm_offset, page_table_level_t level)
{
    static int16_t pml4_idx = -1;
    static int16_t pdpr_idx = -1;
    static int16_t pd_idx = -1;
    static int16_t pt_idx = -1;

    // Allocate memory for the PML4 if necessary.
    if (level == PML4 && *new_pml4 == NULL)
    {
        *new_pml4 = pmm_alloc() + hhdm_offset;
        if (*new_pml4 == NULL)
        {
            LOG_ERROR("Failed to allocate new pml4.");
            return PAGING_ERROR;
        }
    }

    for (int16_t idx = 0; idx < PAGE_TABLE_NUM_ENTRIES; idx++)
    {
        // Skip empty entries.
        if (old_page_table[idx].raw == 0)
        {
            continue;
        }
        
        switch (level)
        {
        case PML4:
            pml4_idx = idx;
            LOG_DEBUG("PML4@%d: %x", pml4_idx, old_page_table[pml4_idx].raw);
            union page_table_entry_t *pdpr = (union page_table_entry_t *) ((old_page_table[pml4_idx].fields.base_address << 12) + hhdm_offset);
            paging_clone_page_table(pdpr, new_pml4, hhdm_offset, PDPR);
            break;

        case PDPR:
            pdpr_idx = idx;

            if (old_page_table[pdpr_idx].fields.page_size != 0)
            {
                LOG_DEBUG("PDPR-%d@%d: %x (1GB-PAGE)", pml4_idx, pdpr_idx, old_page_table[pdpr_idx].raw);
                uintptr_t phys = (old_page_table[pdpr_idx].fields.base_address << 12);
                uintptr_t virt = paging_get_virt_address_from_indices(pml4_idx, pdpr_idx, 0, 0);
                uint64_t flags = paging_get_flags_from_entry(old_page_table[pdpr_idx]);
                // uint64_t flags = PAGING_FLAG_WRITABLE | PAGING_FLAG_PRESENT | PAGING_FLAG_PAGE_SIZE;
                paging_map_page(*new_pml4, phys, virt, PAGE_SIZE_1GB, flags, hhdm_offset);
                break;
            }

            LOG_DEBUG("PDPR-%d@%d: %x", pml4_idx, pdpr_idx, old_page_table[pdpr_idx].raw);
            union page_table_entry_t *pd = (union page_table_entry_t *) ((old_page_table[pdpr_idx].fields.base_address << 12) + hhdm_offset);
            paging_clone_page_table(pd, new_pml4, hhdm_offset, PD);
            break;

        case PD:
            pd_idx = idx;

            if (old_page_table[pd_idx].fields.page_size != 0)
            {
                LOG_DEBUG("PD-%d-%d@%d: %x (2MB-PAGE)", pml4_idx, pdpr_idx, pd_idx, old_page_table[pd_idx].raw);
                uintptr_t phys = (old_page_table[pd_idx].fields.base_address << 12);
                uintptr_t virt = paging_get_virt_address_from_indices(pml4_idx, pdpr_idx, pd_idx, 0);
                uint64_t flags = paging_get_flags_from_entry(old_page_table[pd_idx]);
                // uint64_t flags = PAGING_FLAG_WRITABLE | PAGING_FLAG_PRESENT | PAGING_FLAG_PAGE_SIZE;
                paging_map_page(*new_pml4, phys, virt, PAGE_SIZE_2MB, flags, hhdm_offset);
                break;
            }

            LOG_DEBUG("PD-%d-%d@%d: %x", pml4_idx, pdpr_idx, pd_idx, old_page_table[pd_idx].raw);
            union page_table_entry_t *pt = (union page_table_entry_t *) ((old_page_table[pd_idx].fields.base_address << 12) + hhdm_offset);
            paging_clone_page_table(pt, new_pml4, hhdm_offset, PT);
            break;

        case PT:
            pt_idx = idx;
            LOG_DEBUG("PT-%d-%d-%d@%d: %x", pml4_idx, pdpr_idx, pd_idx, pt_idx, old_page_table[pt_idx].raw);
            uintptr_t phys = (old_page_table[pt_idx].fields.base_address << 12);
            uintptr_t virt = paging_get_virt_address_from_indices(pml4_idx, pdpr_idx, pd_idx, pt_idx);
            uint64_t flags = paging_get_flags_from_entry(old_page_table[pt_idx]);
            // uint64_t flags = PAGING_FLAG_WRITABLE | PAGING_FLAG_PRESENT;
            paging_map_page(*new_pml4, phys, virt, PAGE_SIZE_4KB, flags, hhdm_offset);
            break;

        default:
            break;
        }
    }

    return PAGING_OK;
}

paging_error_codes_t paging_map_page(union page_table_entry_t *pml4, uintptr_t phys, uintptr_t virt, page_size_t page_size, uint64_t flags, ptrdiff_t hhdm_offset)
{
    uint64_t pml4_idx = (virt >> 39) & 0x1ff;
    uint64_t pdpr_idx = (virt >> 30) & 0x1ff;
    uint64_t pd_idx = (virt >> 21) & 0x1ff;
    uint64_t pt_idx = (virt >> 12) & 0x1ff;
    
    // LOG_DEBUG("Mapping %p (virt) to %p (phys)", virt, phys);
    // LOG_DEBUG("Indices: pml4=%d, pdpr=%d, pd=%d, pt=%d", pml4_idx, pdpr_idx, pd_idx, pt_idx);

    union page_table_entry_t *pdpr = NULL;
    if (pml4[pml4_idx].fields.present == 0)
    {
        pdpr = pmm_alloc() + hhdm_offset;
        if (pdpr == NULL)
        {
            LOG_ERROR("Failed to allocate memory for the PDPR.");
            return PAGING_ERROR;
        }
        memset(pdpr, 0, 0x1000);
        
        pml4[pml4_idx] = paging_create_entry((uint64_t)pdpr - hhdm_offset, PAGING_FLAG_PRESENT | PAGING_FLAG_WRITABLE);
    }
    else
    {
        pdpr = (pml4[pml4_idx].fields.base_address << 12) + hhdm_offset;
    }
    
    if (page_size == PAGE_SIZE_1GB)
    {
        pdpr[pdpr_idx] = paging_create_entry(phys, flags);
        return PAGING_OK;
    }

    union page_table_entry_t *pd = NULL;
    if (pdpr[pdpr_idx].fields.present == 0)
    {
        pd = pmm_alloc() + hhdm_offset;
        if (pd == NULL)
        {
            LOG_ERROR("Failed to allocate memory for the PD.");
            return PAGING_ERROR;
        }
        memset(pd, 0, 0x1000);

        pdpr[pdpr_idx] = paging_create_entry((uint64_t)pd - hhdm_offset, PAGING_FLAG_PRESENT | PAGING_FLAG_WRITABLE);
    }
    else
    {
        pd = (pdpr[pdpr_idx].fields.base_address << 12) + hhdm_offset;
    }
    
    if (page_size == PAGE_SIZE_2MB)
    {
        pd[pd_idx] = paging_create_entry(phys, flags);
        return PAGING_OK;
    }

    union page_table_entry_t *pt = NULL;
    if (pd[pd_idx].fields.present == 0)
    {
        pt = pmm_alloc() + hhdm_offset;
        if (pt == NULL)
        {
            LOG_ERROR("Failed to allocate memory for the PT.");
            return PAGING_ERROR;
        }
        memset(pt, 0, 0x1000);
        
        pd[pd_idx] = paging_create_entry((uint64_t)pt - hhdm_offset, PAGING_FLAG_PRESENT | PAGING_FLAG_WRITABLE);
    }
    else
    {
        pt = (pd[pd_idx].fields.base_address << 12) + hhdm_offset;
    }

    if (pt[pt_idx].fields.present != 0)
    {
        LOG_ERROR("Virtual address is already in use: %p", virt);
        // For now I will let this error slip by.
        return PAGING_ERROR;
    }
    pt[pt_idx] = paging_create_entry(phys, flags);

    return PAGING_OK;
}