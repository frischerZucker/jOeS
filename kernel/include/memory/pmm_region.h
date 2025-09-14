/*!
    @file pmm_region.h

    @brief Region-level physical memory management using bitmap tracking.

    Defines data structures and functions for managing memory regions in a physical memory manager.
    Each region tracks it pages via a bitmap, allowing efficient marking of free and used pages.
    Initialization sets up metadata and marks pages based on memory type (usable or some kind of reserved).

    This module assumes 4 kiB page granularity and provides error-checked functions for updating page status based on physical addresses.

    @author frischerZucker
*/

#ifndef PMM_REGION_H
#define PMM_REGION_H

#include <stddef.h>

#include "memory/pmm.h"

/*!
    @brief Error codes used by this module.
*/
typedef enum
{
    PMM_REGION_OK = 0,
    PMM_REGION_ERROR
} pmm_region_error_codes_t;

/*!
    @brief Bitmap entries.
*/
typedef enum {
    PMM_REGION_BITMAP_FREE = 0,
    PMM_REGION_BITMAP_USED = 1
} pmm_region_bitmap_entry_t;

/*!
    @brief Struct for a memory region.

    Contains a bitmap, where each bit represents a page of the region and some metadata.
*/
struct pmm_region_t {
    uint8_t *bitmap;
    size_t bitmap_size;

    /// @brief Can be used for sanity checks of addresses.
    uintptr_t base;
    ptrdiff_t length;

    /// @brief Allows checking for free pages without the need to iterate over the whole bitmap.
    size_t free_pages;

    /// @brief Allows categorization of regions, so that unsusable regions can be skipped when searching for free pages.
    pmm_memory_types_t type;
};

/*!
    @brief Initialize a struct for a region.

    @param region Pointer to the region struct.
    @param bitmap_base Pointer where the regions bitmap should be stored.
    @param region_base Physical base address of the region.
    @param region_length The regions lenght in byte.
    @param region_type Type of memory the region consists of.
*/
void pmm_region_init(struct pmm_region_t *region, uint8_t *bitmap_base, uintptr_t region_base, ptrdiff_t region_length, pmm_memory_types_t type);

/*!
    @brief Mark a page corresponding to a physical address as free.

    Clears the bit corresponding to the page and increments the number of free pages by one.

    @param region Pointer to the regions struct.
    @param phys_address Physical address of the page.
    @returns PMM_REGION_ERROR if the address is outside of the regions bounds, otherwise PMM_REGION_OK.
*/
pmm_region_error_codes_t pmm_region_mark_page_free(struct pmm_region_t *region, uintptr_t phys_address);

/*!
    @brief Mark a page corresponding to a physical address as used.

    Sets the bit corresponding to the page and decrements the number of free pages by one.

    @param region Pointer to the regions struct.
    @param phys_address Physical address of the page.
    @returns PMM_REGION_ERROR if the address is outside of the regions bounds, otherwise PMM_REGION_OK.
*/
pmm_region_error_codes_t pmm_region_mark_page_used(struct pmm_region_t *region, uintptr_t phys_address);

#endif // PMM_REGION_H