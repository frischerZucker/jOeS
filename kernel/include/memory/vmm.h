/*!
    @file vmm.h

    @brief Virtual memory manager.

    @author frischerZucker
*/

#ifndef VMM_H
#define VMM_H

#include "stdbool.h"
#include "stddef.h"
#include "stdint.h"

#include "paging.h"

typedef enum
{
    VMM_OK,
    VMM_ERROR
} vmm_error_codes_t;

struct vmm_blob_t
{
    uintptr_t base_address;
    size_t length;
    uint64_t flags;
    
    struct vmm_blob_t *next_blob;
};

struct vmm
{
    union page_table_entry_t *page_table;
    
    struct vmm_blob_t *used_list;
};

extern bool vmm_initialized;

/*!
    @brief Initialize the kernel VMM.

    Initializes the free list for kernel VMM entries.
    Creates a VMM object to be used as the kernels VMM.
    Adds entries for memory used by the kernel, PMM and page tables.

    @param kernel_vmm Pointer to the VMM object to initialize.
    @param kernel_page_table Pointer to the page table used by the kernel.

    @returns VMM_OK if the VMM is setup correctly, otherwise VMM_ERROR.
*/
vmm_error_codes_t vmm_init_kernel_vmm(struct vmm *kernel_vmm, union page_table_entry_t *kernel_page_table);

/*!
    @brief Dump information about a VMM object.

    Logs information about each entry of the VMM object (base address, length, flags) and the number of entries.

    @param vmm VMM object which information shall be dumped.
*/
[[maybe_unused]] void vmm_dump(struct vmm vmm);

[[nodiscard("It will be quite hard to free memory if u don't remember its address.")]] void *vmm_alloc(struct vmm *vmm, size_t length);

vmm_error_codes_t vmm_free(struct vmm *test, void *address);

#endif // VMM_H