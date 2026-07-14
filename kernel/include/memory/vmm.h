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

struct vmm_entry_t
{
    uintptr_t base_address;
    size_t length;
    uint64_t flags;
    
    struct vmm_entry_t *next_blob;
};

struct vmm
{
    union page_table_entry_t *page_table;
    
    struct vmm_entry_t *used_list;
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

/*!
    @brief Allocate virtual memory.

    Searches the VMMs used list for a gap larger then the requested size.
    If a matching gap is found physical physical pages are allocated and mapped to this region.
    Inserts a new VMM entry into the the used list.
    Returns NULL if something goes wrong.

    @param vmm Pointer to the VMM object for which memory shall be allocated.
    @param length Size of the requested memory region in bytes.

    @returns Base address of the allocated memory region if everything is ok, NULL otherwise.
*/
[[nodiscard("It will be quite hard to free memory if u don't remember its address.")]] void *vmm_alloc(struct vmm *vmm, size_t length);

/*!
    @brief Free virtual memory.

    Searches the VMM objects used list for an entry with a base address matching the address.
    If one is found:
    - frees the regions physical pages
    - unmaps the pages
    Returns an error if no matching entry is found or either freeing physical memory or unmapping pages failed.

    @param vmm VMM object for which the memory shall be freed.
    @param address Base address of the memory region.

    @returns VMM_OK if the memory was freed successfully, VMM_ERROR otherwise.
*/
vmm_error_codes_t vmm_free(struct vmm *vmm, void *address);

#endif // VMM_H