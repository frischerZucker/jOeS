#include "memory/vmm.h"

#include "logging.h"
#include "memory/paging.h"
#include "memory/pmm.h"

extern uint8_t _KERNEL_START[];
extern uint8_t _KERNEL_END[];

extern uint8_t *_KERNEL_PMM_START;
extern uint8_t *_KERNEL_PMM_END;

extern uint8_t _KERNEL_VMM_START[];
extern uint8_t _KERNEL_VMM_END[];

bool vmm_initialized = false;

static struct vmm_blob_t *vmm_blob_free_list = NULL;

/*!
    @brief Grab a VMM entry from the free list.

    @returns NULL if there are no free VMM entries left, otherwise a pointer to the VMM entry.
*/
[[nodiscard]] static struct vmm_blob_t *vmm_alloc_blob()
{
    if (vmm_blob_free_list == NULL)
    {
        LOG_ERROR("No free VMM blobs!");
        return NULL;
    }

    // Get the first free blob from the list.
    struct vmm_blob_t *new_blob = vmm_blob_free_list;
    // Move its head to the next free blob.
    vmm_blob_free_list = vmm_blob_free_list->next_blob;

    return new_blob;
}

/*!
    @brief Add a VMM entry to the free list.
*/
[[maybe_unused]] static void vmm_free_blob(struct vmm_blob_t *blob)
{
    // Set the lists current head as the blobs next blob.
    blob->next_blob = vmm_blob_free_list;
    // Insert the blob at the lists head.
    vmm_blob_free_list = blob;
}

/*!
    @brief Insert a VMM entry into a VMMs used list.

    Iterates through the used list and checks where the new entry should be inserted. 
    This means: end of last entry < base address of the new entry & base of next entry > end of the new entry.
    If a matching spot is found, the entry is inserted into the list.

    @param vmm Pointer to the VMM object into thats used list the entry shall be inserted.
    @param blob Pointer to the VMM entry to insert.

    @returns VMM_OK if the entry was successfully inserted, otherwise VMM_ERROR.
*/
static vmm_error_codes_t vmm_insert_blob(struct vmm *vmm, struct vmm_blob_t *blob)
{
    struct vmm_blob_t *entry = vmm->used_list;
    
    // Check if the new entry fits at the head of the list.
    if (blob->base_address < entry->base_address)
    {
        if (blob->base_address + blob->length > entry->base_address)
        {
            LOG_ERROR("Region does not fit into the virtual memory space. It overlaps with an already existing region.");
            return VMM_ERROR;
        }

        // Insert the new entry as head of the used list.
        blob->next_blob = entry;
        vmm->used_list = blob;

        return VMM_OK;
    }

    // Check if the new entry fits between an two neighbours in the used list.
    for (; entry->next_blob != NULL; entry = entry->next_blob)
    {
        // The current entry has to end before the new entrys base address.
        if (entry->base_address + entry->length > blob->base_address)
        {
            continue;
        }

        // The next entrys base address has to be larger then the end of the new entrys region.
        if (entry->next_blob->base_address < blob->base_address + blob->length)
        {
            continue;
        }

        // Both criteria are fulfilled -> The blob fits into the gap between the current and the next entry. Therefore insert it into the list.
        blob->next_blob = entry->next_blob;
        entry->next_blob = blob;
        return VMM_OK;
    }

    // The new entry should be behind the lists tail.
    if (entry->base_address + entry->length <= blob->base_address)
    {
        blob->next_blob = NULL;
        entry->next_blob = blob;
        return VMM_OK;
    }
    
    LOG_ERROR("Found no gap for the new entry.");
    return VMM_ERROR;
}

/*!
    @brief Dump information about a VMM object.

    Logs information about each entry of the VMM object (base address, length, flags) and the number of entries.

    @param vmm VMM object which information shall be dumped.
*/
[[maybe_unused]] void vmm_dump(struct vmm vmm)
{
    LOG_INFO("--- Start VMM dump ---");

    size_t num_entries = 0;

    struct vmm_blob_t *entry = vmm.used_list;
    while (entry != NULL)
    {
        LOG_INFO("VMM region @ %p, length=%d kB, flags=0x%x", entry->base_address, entry->length, entry->flags);
        num_entries = num_entries + 1;

        entry = entry->next_blob;
    }
    
    LOG_INFO("The VMM object has %d entries.", num_entries);
}

/*!
    @brief Initialize the kernel VMM.

    Initializes the free list for kernel VMM entries.
    Creates a VMM object to be used as the kernels VMM.
    Adds entries for memory used by the kernel, PMM and page tables.

    @param kernel_vmm Pointer to the VMM object to initialize.
    @param kernel_page_table Pointer to the page table used by the kernel.

    @returns VMM_OK if the VMM is setup correctly, otherwise VMM_ERROR.
*/
vmm_error_codes_t vmm_init_kernel_vmm(struct vmm *kernel_vmm, union page_table_entry_t *kernel_page_table)
{
    size_t available_space = _KERNEL_VMM_END - _KERNEL_VMM_START;

    LOG_INFO("\tvmm region start: %p\n\tvmm region end: %p\n\tavailable Space: %d B, enough for %d blobs", _KERNEL_VMM_START, _KERNEL_VMM_END, available_space, available_space / sizeof(struct vmm_blob_t));

    // Populate the free list with all blob-structs in the reserved region
    vmm_blob_free_list = (struct vmm_blob_t *) _KERNEL_VMM_START;
    for (size_t idx = 1; idx < available_space / sizeof(struct vmm_blob_t); idx++)
    {
        vmm_blob_free_list[idx - 1].next_blob = &vmm_blob_free_list[idx];
    }

    kernel_vmm->page_table = kernel_page_table;

    // Add the first entry for the kernels memory.
    kernel_vmm->used_list = vmm_alloc_blob();
    if (kernel_vmm->used_list == NULL)
    {
        LOG_ERROR("Failed to allocate VMM region.");
        return VMM_ERROR;
    }
    kernel_vmm->used_list->base_address = (uintptr_t)_KERNEL_START;
    kernel_vmm->used_list->length = (size_t)(_KERNEL_END - _KERNEL_START);
    kernel_vmm->used_list->next_blob = NULL;
    kernel_vmm->used_list->flags = 0;

    // Memory used to store the PMMs metadata.
    struct vmm_blob_t *pmm_region = vmm_alloc_blob();
    if (pmm_region == NULL)
    {
        LOG_ERROR("Failed to allocate PMM metadata region.");
        return VMM_ERROR;
    }
    pmm_region->base_address = (uintptr_t)_KERNEL_PMM_START;
    pmm_region->length = (size_t)(_KERNEL_PMM_END - _KERNEL_PMM_START);
    pmm_region->next_blob = NULL;
    pmm_region->flags = 0;
    vmm_insert_blob(kernel_vmm, pmm_region);

    // Add entries for the memory used for page tables.
    uintptr_t region_start = (uintptr_t)paging_used_pages_list[0];
    size_t region_len = 0x1000;
    for (int idx = 1; idx < paging_used_pages_list_idx; idx++)
    {   
        if ((uintptr_t)paging_used_pages_list[idx] - region_start == region_len)
        {
            region_len = region_len + 0x1000;
        }
        else
        {
            LOG_DEBUG("Page table region @ %p (%d kB)", region_start, region_len / 1024);
            struct vmm_blob_t *paging_region = vmm_alloc_blob();
            if (paging_region == NULL)
            {
                LOG_ERROR("Failed to allocate page table region.");
                return VMM_ERROR;
            }
            paging_region->base_address = region_start;
            paging_region->length = region_len;
            paging_region->next_blob = NULL;
            paging_region->flags = 0;
            vmm_insert_blob(kernel_vmm, paging_region);

            region_start = (uintptr_t)paging_used_pages_list[idx];
            region_len = 0x1000;
        }
    }
    LOG_DEBUG("Page table region @ %p (%d kB)", region_start, region_len / 1024);
    struct vmm_blob_t *paging_region = vmm_alloc_blob();
    if (paging_region == NULL)
    {
        LOG_ERROR("Failed to allocate page table region.");
        return VMM_ERROR;
    }
    paging_region->base_address = region_start;
    paging_region->length = region_len;
    paging_region->next_blob = NULL;
    paging_region->flags = 0;
    vmm_insert_blob(kernel_vmm, paging_region);

    vmm_initialized = true;

    return VMM_OK;
}

void *vmm_alloc(struct vmm *vmm, size_t length)
{
    void *address = NULL;

    return address;
}