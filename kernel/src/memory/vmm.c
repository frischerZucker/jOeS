#include "memory/vmm.h"

#include "logging.h"
#include "memory/paging.h"
#include "memory/pmm.h"
#include <stdint.h>

extern uint8_t _KERNEL_START[];
extern uint8_t _KERNEL_END[];

extern uint8_t *_KERNEL_PMM_START;
extern uint8_t *_KERNEL_PMM_END;

extern uint8_t _KERNEL_VMM_START[];
extern uint8_t _KERNEL_VMM_END[];

static struct vmm_blob_t *vmm_blob_free_list = NULL;

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

[[maybe_unused]] static void vmm_free_blob(struct vmm_blob_t *blob)
{
    // Set the lists current head as the blobs next blob.
    blob->next_blob = vmm_blob_free_list;
    // Insert the blob at the lists head.
    vmm_blob_free_list = blob;
}

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

    // TODO: Add blobs for used memory
    // Add the first entry for the kernels memory.
    kernel_vmm->used_list = vmm_alloc_blob();
    kernel_vmm->used_list->base_address = (uintptr_t)_KERNEL_START;
    kernel_vmm->used_list->length = (uintptr_t)(_KERNEL_END - _KERNEL_START);
    kernel_vmm->used_list->next_blob = NULL;
    kernel_vmm->used_list->flags = 0;

    struct vmm_blob_t *pmm_region = vmm_alloc_blob();
    pmm_region->base_address = (uintptr_t)_KERNEL_PMM_START;
    pmm_region->length = (uintptr_t)(_KERNEL_PMM_END - _KERNEL_PMM_START);
    pmm_region->next_blob = NULL;
    pmm_region->flags = 0;
    vmm_insert_blob(kernel_vmm, pmm_region);

    LOG_INFO("PMM region: %p to %p", pmm_region->base_address, pmm_region->length);

    return VMM_OK;
}

void *vmm_alloc(struct vmm *vmm, size_t length)
{
    void *address = NULL;

    return address;
}