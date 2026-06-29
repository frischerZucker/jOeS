/*!
    @file vmm.h

    @brief Virtual memory manager.

    @author frischerZucker
*/

#ifndef VMM_H
#define VMM_H

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

vmm_error_codes_t vmm_init_kernel_vmm(struct vmm *kernel_vmm, union page_table_entry_t *kernel_page_table);

[[nodiscard("It will be quite hard to free memory if u don't remember its address.")]] void *vmm_alloc(struct vmm *vmm, size_t length);

vmm_error_codes_t vmm_free(struct vmm *test, void *address);

#endif // VMM_H