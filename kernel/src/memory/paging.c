#include "memory/paging.h"

#include "string.h"

#include "logging.h"
#include "memory/pmm.h"
#include <stdint.h>

#define PAGE_TABLE_NUM_ENTRIES 512

// For now I just use a global offset for virtual to physical translation.
static ptrdiff_t g_hhdm_offset = (ptrdiff_t)NULL;

/*!
    @brief Invalidate a cached page entry in TLB.

    Invalidates a cached page entry, so that changes in the page tables can take effect.
    
    @param virt Virtual address to invalidate.
*/
static inline void invalidate_tlb(uintptr_t virt)
{
    asm volatile (
        "invlpg (%0)"
        :
        : "r" (virt)
        : "memory"
    );
}

/*!
    @brief Check if a page table is empty and if so, delete it.

    Checks all entries in a page table.
    If all entries are empty, its entry in the parent table is deleted and the tables memory is freed.

    @param table Page table that is checked for emptiness.
    @param parent_table Parent table that points to [table].
    @param idx_in_parent_table [table]s index in [parent_table].
*/
static void paging_check_for_empty_table(union page_table_entry_t *table, union page_table_entry_t *parent_table, uint64_t idx_in_parent_table)
{
    bool table_empty = true;
    for (size_t idx = 0; idx < PAGE_TABLE_NUM_ENTRIES; idx++)
    {
        if (table[idx].pml4.pointer_fields.present != 0)
        {
            table_empty = false;
            break;
        }
    }

    if (table_empty)
    {
        LOG_DEBUG("Table is empty. Remove table @%d from parent table.", idx_in_parent_table);
        parent_table[idx_in_parent_table].pml4.pointer_fields.present = 0;
        pmm_free(((void *)table) - g_hhdm_offset);
    }
}

/*!
    @brief Calculates a virtual address from page table indices.

    @param pml4_idx Index in the PML4.
    @param pdpr_idx Index in the PDPR.
    @param pd_idx Index in the PD.
    @param pt_idx Index in the PT.

    @returns Virtual address as uintptr_t.
*/
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

/*!
    @brief Create a page table entry.

    Creates a new page table entry.
    Sets the flags and base address depending on the entries level and type. 

    @param base_address Physical address the entry should point to. Can be the address of the next table or of a page.
    @param flags Flags describing the pages attributes.
    @param level Level of the entry in the page table hierarchy.
    @param type Decides if the entry points to another page table or is a "leaf"-entry.

    @returns A newly createt page_table_entry_t.
*/
static union page_table_entry_t paging_create_entry(uintptr_t base_address, uint64_t flags, page_table_level_t level, page_table_entry_type_t type)
{
    union page_table_entry_t new_entry = {0};

    new_entry.raw = flags;

    // Well, my page_table_entry_t has different fields for all page table levels and types. I need to use the right one, so there is this mess..
    switch (level)
    {
    case PML4:
        new_entry.pml4.pointer_fields.base_address = (base_address >> 12);
        break;
    case PDPR:
        if (type == PAGING_ENTRY_PAGE)
        {
            new_entry.pdpr.page_fields.base_address = (base_address >> 30);
        }
        else
        {
            new_entry.pdpr.pointer_fields.base_address = (base_address >> 12);
        }
        break;
    case PD:
        if (type == PAGING_ENTRY_PAGE)
        {
            new_entry.pd.page_fields.base_address = (base_address >> 21);
        }
        else
        {
            new_entry.pd.pointer_fields.base_address = (base_address >> 12);
        }
        break;
    case PT:
        new_entry.pt.page_fields.base_address = (base_address >> 12);
        break;
    }

    return new_entry;
}

/*!
    @brief Extract flags from a page table entry.

    Extracts flags using a bitmask.
    Which mask is used depends on the entries level in the page table hierarchy and its type (pointer or leaf).

    @param entry Entry which flags should be extracted.
    @param level Level of the entry in the page table hierarchy.
    @param type Type of the entry (pointer or lead).

    @returns uint64_t with the flags.
*/
static uint64_t paging_get_flags_from_entry(union page_table_entry_t entry, page_table_level_t level, page_table_entry_type_t type)
{
    uint64_t flags = entry.raw;
    
    uint64_t mask = 0;
    if (type == PAGING_ENTRY_POINTER || level == PT)
    {
        mask = ~(0xfffffff000);
    } 
    else if (level == PDPR)
    {
        mask = ~(0xffc0000000);
    } 
    else if (level == PD)
    {
        mask = ~(0xffffe00000);
    }
    else
    {
        LOG_ERROR("Unexpected combination of level and type. level=%d, type=%d", level, type);
    }

    flags = flags & mask;

    return flags;
}

/*!
    @brief Map a virtual address to a physical address in the page table.

    Traverses the page table hierarchy until the leaf is reached to map a physical address to a virtual address.
    Supports 4kB, 2MB and 1GB pages.
    Creates entries and allocates memory for page tables as needed.

    Returns an error if allocating memory fails or if the leaf is already used (present flag is set).

    @param pml4 PML4 in which the page should be mapped.
    @param phys Physical address to which the virtual address should be mapped.
    @param virt Virtual address that should be mapped to the physical address.
    @param page_size Size of the page to map (4kB, 2MB or 1GB).
    @param flags Flags for the page.

    @returns PAGING_OK on success, PAGING_ERROR if allocating memory failed or the leafs present bit is set.
*/
static paging_error_codes_t paging_map_page_without_tlb_invalidation(union page_table_entry_t *pml4, uintptr_t phys, uintptr_t virt, page_size_t page_size, uint64_t flags)
{
    uint64_t pml4_idx = (virt >> 39) & 0x1ff;
    uint64_t pdpr_idx = (virt >> 30) & 0x1ff;
    uint64_t pd_idx = (virt >> 21) & 0x1ff;
    uint64_t pt_idx = (virt >> 12) & 0x1ff;
    
    LOG_DEBUG("Mapping %p (virt) to %p (phys)", virt, phys);
    LOG_DEBUG("Indices: pml4=%d, pdpr=%d, pd=%d, pt=%d", pml4_idx, pdpr_idx, pd_idx, pt_idx);

    union page_table_entry_t *pdpr = NULL;
    if (pml4[pml4_idx].pml4.pointer_fields.present == 0)
    {
        pdpr = pmm_alloc() + g_hhdm_offset;
        if (pdpr == NULL)
        {
            LOG_ERROR("Failed to allocate memory for the PDPR.");
            return PAGING_ERROR;
        }
        memset(pdpr, 0, 0x1000);
        
        pml4[pml4_idx] = paging_create_entry((uintptr_t)pdpr - g_hhdm_offset, PAGING_FLAG_PRESENT | PAGING_FLAG_WRITABLE, PML4, PAGING_ENTRY_POINTER);
    }
    else
    {
        pdpr = (union page_table_entry_t *) (((uintptr_t)pml4[pml4_idx].pml4.pointer_fields.base_address << 12) + g_hhdm_offset);
    }
    
    if (page_size == PAGE_SIZE_1GB)
    {
        if (pdpr[pdpr_idx].pdpr.page_fields.present != 0)
        {
            LOG_ERROR("Virtual address is already in use: %p", virt);
            return PAGING_ERROR;
        }
        pdpr[pdpr_idx] = paging_create_entry(phys, flags, PDPR, PAGING_ENTRY_PAGE);
        return PAGING_OK;
    }

    union page_table_entry_t *pd = NULL;
    if (pdpr[pdpr_idx].pdpr.pointer_fields.present == 0)
    {
        pd = pmm_alloc() + g_hhdm_offset;
        if (pd == NULL)
        {
            LOG_ERROR("Failed to allocate memory for the PD.");
            return PAGING_ERROR;
        }
        memset(pd, 0, 0x1000);

        pdpr[pdpr_idx] = paging_create_entry((uintptr_t)pd - g_hhdm_offset, PAGING_FLAG_PRESENT | PAGING_FLAG_WRITABLE, PDPR, PAGING_ENTRY_POINTER);
    }
    else
    {
        pd = (union page_table_entry_t *) (((uintptr_t)pdpr[pdpr_idx].pdpr.pointer_fields.base_address << 12) + g_hhdm_offset);
    }
    
    if (page_size == PAGE_SIZE_2MB)
    {
        if (pd[pd_idx].pd.page_fields.present != 0)
        {
            LOG_ERROR("Virtual address is already in use: %p", virt);
            return PAGING_ERROR;
        }
        pd[pd_idx] = paging_create_entry(phys, flags, PD, PAGING_ENTRY_PAGE);
        return PAGING_OK;
    }

    union page_table_entry_t *pt = NULL;
    if (pd[pd_idx].pd.pointer_fields.present == 0)
    {
        pt = pmm_alloc() + g_hhdm_offset;
        if (pt == NULL)
        {
            LOG_ERROR("Failed to allocate memory for the PT.");
            return PAGING_ERROR;
        }
        memset(pt, 0, 0x1000);
        
        pd[pd_idx] = paging_create_entry((uintptr_t)pt - g_hhdm_offset, PAGING_FLAG_PRESENT | PAGING_FLAG_WRITABLE, PD, PAGING_ENTRY_POINTER);
    }
    else
    {
        pt = (union page_table_entry_t *) (((uintptr_t)pd[pd_idx].pd.pointer_fields.base_address << 12) + g_hhdm_offset);
    }

    if (pt[pt_idx].pt.page_fields.present != 0)
    {
        LOG_ERROR("Virtual address is already in use: %p", virt);
        return PAGING_ERROR;
    }
    pt[pt_idx] = paging_create_entry(phys, flags, PT, PAGING_ENTRY_PAGE);

    return PAGING_OK;
}

/*!
    @brief Resolve a virtual address to a physical address.

    Walks through the page table hierarchy until a leaf is reached and returns the base address of it.
    Returns NULL if an entry along the way is not present .

    @param pml4 Page table from that the physical address should be retrieved.
    @param virt Virtual address to translate.
    
    @returns Physical address as a uintptr_t.
*/
uintptr_t paging_resolve_virtual_address(union page_table_entry_t *pml4, uintptr_t virt)
{
    uint64_t pml4_idx = (virt >> 39) & 0x1ff;
    uint64_t pdpr_idx = (virt >> 30) & 0x1ff;
    uint64_t pd_idx = (virt >> 21) & 0x1ff;
    uint64_t pt_idx = (virt >> 12) & 0x1ff;

    if (pml4[pml4_idx].pml4.pointer_fields.present == 0)
    {
        return (uintptr_t) NULL;
    }

    union page_table_entry_t *pdpr = (union page_table_entry_t *) (((uintptr_t)pml4[pml4_idx].pml4.pointer_fields.base_address << 12) + g_hhdm_offset);

    if (pdpr[pdpr_idx].pdpr.pointer_fields.present == 0)
    {
        return (uintptr_t) NULL;
    }
    if (pdpr[pdpr_idx].pdpr.pointer_fields.page_size != 0)
    {
        return (uintptr_t)pdpr[pdpr_idx].pdpr.page_fields.base_address << 30;
    }

    union page_table_entry_t *pd = (union page_table_entry_t *) (((uintptr_t)pdpr[pdpr_idx].pdpr.pointer_fields.base_address << 12) + g_hhdm_offset);

    if (pd[pd_idx].pd.pointer_fields.present == 0)
    {
        return (uintptr_t) NULL;
    }
    if (pd[pd_idx].pd.pointer_fields.page_size != 0)
    {
        return (uintptr_t)(pd[pd_idx].pd.page_fields.base_address) << 21;
    }

    union page_table_entry_t *pt = (union page_table_entry_t *) (((uintptr_t)pd[pd_idx].pd.pointer_fields.base_address << 12) + g_hhdm_offset);
    
    if (pt[pt_idx].pt.page_fields.present == 0)
    {
        return (uintptr_t) NULL;
    }

    return (uintptr_t)pt[pt_idx].pt.page_fields.base_address << 12;
}

/*!
    @brief Initializes the global HHDM offset.

    @param hhdm_offset HHDM offset used for address translation.
*/
void paging_init(ptrdiff_t hhdm_offset)
{
    g_hhdm_offset = hhdm_offset;
}

/*!
    @brief Recursively traverse and log the structure of a page table.

    Walks through all entries of a given page table (PML4, PDPR, PD or PT) and prints information about each non-empty entry.
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
void paging_dump_page_table(union page_table_entry_t *page_table, page_table_level_t level)
{
    static int16_t pml4_idx = -1;
    static int16_t pdpr_idx = -1;
    static int16_t pd_idx = -1;
    static int16_t pt_idx = -1;

    for (int16_t idx = 0; idx < PAGE_TABLE_NUM_ENTRIES; idx++)
    {
        // Skip empty entries.
        if (page_table[idx].pml4.pointer_fields.present == 0)
        {
            continue;
        }
        
        switch (level)
        {
        case PML4:
            pml4_idx = idx;
            LOG_INFO("PML4@%d: %x", pml4_idx, page_table[pml4_idx].raw);
            union page_table_entry_t *pdpr = (union page_table_entry_t *) ((page_table[pml4_idx].pml4.pointer_fields.base_address << 12) + g_hhdm_offset);
            paging_dump_page_table(pdpr, PDPR);
            break;

        case PDPR:
            pdpr_idx = idx;

            if (page_table[pdpr_idx].pdpr.pointer_fields.page_size != 0)
            {
                LOG_INFO("PDPR-%d@%d: %x (1GB-PAGE)", pml4_idx, pdpr_idx, page_table[pdpr_idx].raw);
                break;
            }

            LOG_INFO("PDPR-%d@%d: %x", pml4_idx, pdpr_idx, page_table[pdpr_idx].raw);
            union page_table_entry_t *pd = (union page_table_entry_t *) ((page_table[pdpr_idx].pdpr.pointer_fields.base_address << 12) + g_hhdm_offset);
            paging_dump_page_table(pd, PD);
            break;

        case PD:
            pd_idx = idx;

            if (page_table[pd_idx].pd.pointer_fields.page_size != 0)
            {
                LOG_INFO("PD-%d-%d@%d: %x (2MB-PAGE)", pml4_idx, pdpr_idx, pd_idx, page_table[pd_idx].raw);
                break;
            }

            LOG_INFO("PD-%d-%d@%d: %x", pml4_idx, pdpr_idx, pd_idx, page_table[pd_idx].raw);
            union page_table_entry_t *pt = (union page_table_entry_t *) ((page_table[pd_idx].pd.pointer_fields.base_address << 12) + g_hhdm_offset);
            paging_dump_page_table(pt, PT);
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

/*!
    @brief Recursively walk a page table and clone mapped pages to another page table.

    Recursively walks through all entries of a given page table (PML4, PDPR, PD or PT).
    If a leaf is reached, its flags and the physical address are retrieved and the virtual address is calculated from the indices.
    Maps the page in another page table WITHOUT invalidating its TLB entry.

    @param old_page_table Pointer to the current level of page table that should be cloned.
    @param new_pml4 Pointer to the PML4 of the page table that the entries should be cloned to.
    @param level Current page table level (PML4, PDPR, PD, PT).

    @returns PAGING_OK on success, PAGING_ERROR when allocating memory for the PML4 fails.
*/
paging_error_codes_t paging_clone_page_table(union page_table_entry_t *old_page_table, union page_table_entry_t **new_pml4, page_table_level_t level)
{
    static uint64_t pml4_idx = 0;
    static uint64_t pdpr_idx = 0;
    static uint64_t pd_idx = 0;
    static uint64_t pt_idx = 0;

    // Allocate memory for the PML4 if necessary.
    if (level == PML4 && *new_pml4 == NULL)
    {
        *new_pml4 = pmm_alloc() + g_hhdm_offset;
        if (*new_pml4 == NULL)
        {
            LOG_ERROR("Failed to allocate new pml4.");
            return PAGING_ERROR;
        }
    }

    for (uint64_t idx = 0; idx < PAGE_TABLE_NUM_ENTRIES; idx++)
    {
        // Skip empty entries.
        if (old_page_table[idx].pml4.pointer_fields.present == 0)
        {
            continue;
        }
        
        switch (level)
        {
        case PML4:
            pml4_idx = idx;
            LOG_DEBUG("PML4@%d: %x", pml4_idx, old_page_table[pml4_idx].raw);
            union page_table_entry_t *pdpr = (union page_table_entry_t *) (((uintptr_t)old_page_table[pml4_idx].pml4.pointer_fields.base_address << 12) + g_hhdm_offset);
            paging_clone_page_table(pdpr, new_pml4, PDPR);
            break;

        case PDPR:
            pdpr_idx = idx;

            if (old_page_table[pdpr_idx].pdpr.pointer_fields.page_size != 0)
            {
                LOG_DEBUG("PDPR-%d@%d: %x (1GB-PAGE)", pml4_idx, pdpr_idx, old_page_table[pdpr_idx].raw);
                uintptr_t phys = (uintptr_t)old_page_table[pdpr_idx].pdpr.page_fields.base_address << 30;
                uintptr_t virt = paging_get_virt_address_from_indices(pml4_idx, pdpr_idx, 0, 0);
                uint64_t flags = paging_get_flags_from_entry(old_page_table[pdpr_idx], PDPR, PAGING_ENTRY_PAGE);
                paging_map_page_without_tlb_invalidation(*new_pml4, phys, virt, PAGE_SIZE_1GB, flags);
                break;
            }

            LOG_DEBUG("PDPR-%d@%d: %x", pml4_idx, pdpr_idx, old_page_table[pdpr_idx].raw);
            union page_table_entry_t *pd = (union page_table_entry_t *) (((uintptr_t)old_page_table[pdpr_idx].pdpr.pointer_fields.base_address << 12) + g_hhdm_offset);
            paging_clone_page_table(pd, new_pml4, PD);
            break;

        case PD:
            pd_idx = idx;

            if (old_page_table[pd_idx].pd.pointer_fields.page_size != 0)
            {
                LOG_DEBUG("PD-%d-%d@%d: %x (2MB-PAGE)", pml4_idx, pdpr_idx, pd_idx, old_page_table[pd_idx].raw);
                uintptr_t phys = (uintptr_t)old_page_table[pd_idx].pd.page_fields.base_address << 21;
                uintptr_t virt = paging_get_virt_address_from_indices(pml4_idx, pdpr_idx, pd_idx, 0);
                uint64_t flags = paging_get_flags_from_entry(old_page_table[pd_idx], PD, PAGING_ENTRY_PAGE);
                paging_map_page_without_tlb_invalidation(*new_pml4, phys, virt, PAGE_SIZE_2MB, flags);
                break;
            }

            LOG_DEBUG("PD-%d-%d@%d: %x", pml4_idx, pdpr_idx, pd_idx, old_page_table[pd_idx].raw);
            union page_table_entry_t *pt = (union page_table_entry_t *) (((uintptr_t)old_page_table[pd_idx].pd.pointer_fields.base_address << 12) + g_hhdm_offset);
            paging_clone_page_table(pt, new_pml4, PT);
            break;

        case PT:
            pt_idx = idx;
            LOG_DEBUG("PT-%d-%d-%d@%d: %x", pml4_idx, pdpr_idx, pd_idx, pt_idx, old_page_table[pt_idx].raw);
            uintptr_t phys = ((uintptr_t)old_page_table[pt_idx].pt.page_fields.base_address << 12);
            uintptr_t virt = paging_get_virt_address_from_indices(pml4_idx, pdpr_idx, pd_idx, pt_idx);
            uint64_t flags = paging_get_flags_from_entry(old_page_table[pt_idx], PT, PAGING_ENTRY_PAGE);
            paging_map_page_without_tlb_invalidation(*new_pml4, phys, virt, PAGE_SIZE_4KB, flags);
            break;

        default:
            break;
        }
    }

    return PAGING_OK;
}

/*!
    @brief Unmap a page and invalidate its TLB entry.

    Unmaps a page by clearing the present bit in its entry.
    Supports 4kB, 2MB and 1GB pages.
    After clearing the pages present bit, its page table and its parents are checked for emptiness and deletes them if they are no longer required.

    Returns an error if a non-leaf entry is not present.

    @param pml4 Page table in which the page should be unmapped.
    @param virt Virtual address to unmap.
    @param page_size Size of the page to unmap.

    @returns PAGING_OK on success, PAGING_ERROR if a non-leaf entry is not present.
*/
paging_error_codes_t paging_unmap_page(union page_table_entry_t *pml4, uintptr_t virt, page_size_t page_size)
{
    uint64_t pml4_idx = (virt >> 39) & 0x1ff;
    uint64_t pdpr_idx = (virt >> 30) & 0x1ff;
    uint64_t pd_idx = (virt >> 21) & 0x1ff;
    uint64_t pt_idx = (virt >> 12) & 0x1ff;

    LOG_DEBUG("Unmapping virt=%p", virt);
    LOG_DEBUG("Indices: pml4=%d, pdpr=%d, pd=%d, pt=%d", pml4_idx, pdpr_idx, pd_idx, pt_idx);

    if (pml4[pml4_idx].pml4.pointer_fields.present == 0)
    {
        LOG_ERROR("Failed to unmap virt=%p. PML4 entry not present in entry: %p", virt, pml4[pml4_idx].raw);
        return PAGING_ERROR;
    }

    union page_table_entry_t *pdpr = (union page_table_entry_t *) (((uintptr_t)pml4[pml4_idx].pml4.pointer_fields.base_address << 12) + g_hhdm_offset);
    if (pdpr[pdpr_idx].pdpr.pointer_fields.present == 0)
    {
        LOG_ERROR("Failed to unmap virt=%p. PDPR entry not present.", virt);
        return PAGING_ERROR;
    }
    if (page_size == PAGE_SIZE_1GB)
    {
        pdpr[pdpr_idx].pdpr.page_fields.present = 0;
        goto CHECK_FOR_EMPTY_PDPR;
    }

    union page_table_entry_t *pd = (union page_table_entry_t *) (((uintptr_t)pdpr[pdpr_idx].pdpr.pointer_fields.base_address << 12) + g_hhdm_offset);
    if (pd[pd_idx].pd.pointer_fields.present == 0)
    {
        LOG_ERROR("Failed to unmap virt=%p. PD entry not present.", virt);
        return PAGING_ERROR;
    }
    if (page_size == PAGE_SIZE_2MB)
    {
        pd[pd_idx].pd.page_fields.present = 0;
        goto CHECK_FOR_EMPTY_PD;
    }

    union page_table_entry_t *pt = (union page_table_entry_t *) (((uintptr_t)pd[pd_idx].pd.pointer_fields.base_address << 12) + g_hhdm_offset);
    pt[pt_idx].pt.page_fields.present = 0;

    // Free empty tables.
    // TODO: Only check parent tables if table was empty.
    paging_check_for_empty_table(pt, pd, pd_idx);
    CHECK_FOR_EMPTY_PD:
    paging_check_for_empty_table(pd, pdpr, pdpr_idx);
    CHECK_FOR_EMPTY_PDPR:
    paging_check_for_empty_table(pdpr, pml4, pml4_idx);

    invalidate_tlb(virt);

    return PAGING_OK;
}

/*!
    @brief Map a virtual address to a physical address in the page table and invalidates its TLB entry.

    Traverses the page table hierarchy until the leaf is reached to map a physical address to a virtual address.
    Supports 4kB, 2MB and 1GB pages.
    Creates entries and allocates memory for page tables as needed.
    Invalidates the pages TLB entry if the page was successfully mapped.

    Returns an error if allocating memory fails or if the leaf is already used (present flag is set).

    @param pml4 PML4 in which the page should be mapped.
    @param phys Physical address to which the virtual address should be mapped.
    @param virt Virtual address that should be mapped to the physical address.
    @param page_size Size of the page to map (4kB, 2MB or 1GB).
    @param flags Flags for the page.

    @returns PAGING_OK on success, PAGING_ERROR if allocating memory failed or the leafs present bit is set.
*/
paging_error_codes_t paging_map_page(union page_table_entry_t *pml4, uintptr_t phys, uintptr_t virt, page_size_t page_size, uint64_t flags)
{
    paging_error_codes_t success = paging_map_page_without_tlb_invalidation(pml4, phys, virt, page_size, flags);
    if (success != PAGING_OK)
    {
        LOG_ERROR("Failed to map page. virt=%p, phys=%p, error=%d", virt, phys, success);
        return success;
    }

    invalidate_tlb(virt);

    return PAGING_OK;
}