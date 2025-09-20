/*!
    @file pmm.h

    @brief TESTTEST.

    @author frischerZucker
*/

#ifndef PMM_H
#define PMM_H

#include <limine.h>

#include <stddef.h>
#include <stdint.h>

/*!
    @brief Available types of memory
*/
typedef enum{
    MEMMAP_TYPE_USABLE = 0,
    MEMMAP_TYPE_RESERVED,
    MEMMAP_TYPE_ACPI_RECLAIMABLE,
    MEMMAP_TYPE_ACPI_NON_VOLATILE,
    MEMMAP_TYPE_BAD_MEMORY,
    MEMMAP_TYPE_BOOTLOADER_RECLAIMABLE,
    MEMMAP_TYPE_KERNEL_AND_MODULES,
    MEMMAP_TYPE_FRAMEBUFFER
} pmm_memory_types_t;

/*!
    @brief Error codes used by the PMM.
*/
typedef enum{
    PMM_OK = 0,
    PMM_ERROR_INIT_FAILED,
    PMM_ERROR_ADDRESS_NOT_FOUND
} pmm_error_codes_t;

typedef enum {
    PMM_PAGE_FREE = 0,
    PMM_PAGE_USED = 1,
    PMM_PAGE_NOT_FOUND
} pmm_page_status_t;

/*!
    @brief Initializes the physical memory manager (PMM).

    Sets up the PMM by parsing Limines memory map, detecting usable memory regions. 
    Calculates how many pages are required to store PMM metadata (region structs, bitmaps) 
    and searches for a suitable memory region to store this data.
    Once found, initializes all region structs and marks memory used by the PMM as used to prevent reuse.

    Aksi sets the HHDM (higher half direct map) offset for physical-to-virtual address translation.
    Prints diagnostic information during setup for debugging purposes.

    @param memmap Pointer to Limines memory map.
    @param hhdm_offset Offset to convert physical to virtual addresses provided by Limine.
    @returns PMM_OK on success, PMM_INIT_FAILED if setup fails.
*/
pmm_error_codes_t pmm_init(struct limine_memmap_response *memmap, uint64_t hhdm_offset);

/*!
    @brief Checks if a page is currently free or in use.

    Searches through all regions to find the region that includes the addresses page.
    If it is found, calculates the page number corresponding page.
    Checks its status in the bitmap and returns it.
    If no matching region is found, a special page status is returned.

    @param ptr Pointer (physical address) to the page to check.
    @returns PMM_PAGE_FREE if the page is free, PMM_PAGE_USED if it is used and PMM_PAGE_NOT_FOUND if no region matching the address is found.
*/
pmm_page_status_t pmm_check_page(void *ptr);

/*!
    @brief Allocates a single free physical memory page.

    Searches through all memory regions managed by the PMM to find a free page.
    The search is optimized by using static caches (region_cache, bitmap_cache)
    to reduce iteration overhead by starting from the last successful allocated page.

    The function scans each regions bitmap for a free bit (representing a free page),
    marks the page as used, and returns its physical address.
    If no free page is found across all regions, NULL is returned.

    @returns Pointer to the allocated physical page, or NULL if no free page was found.
*/
void * pmm_alloc();

/*!
    @brief Frees a single physical memory page.

    Uses binary search to find the region that includes the page.
    If it is found, the page is marked as free.
    If none is found, an error is returned.

    @param ptr Pointer (physical address) to the page to free.
    @returns PMM_OK on success, PMM_ERROR_ADDRESS_NOT_FOUND if no region that contains the pages address was found.
*/
pmm_error_codes_t pmm_free(void *ptr);

#endif // PMM_H