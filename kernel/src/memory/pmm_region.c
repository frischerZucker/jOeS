#include "memory/pmm_region.h"

#define PAGE_SIZE_BYTE 4096

#include "stdio.h"
#include "string.h"

/*!
    @brief Mark a page corresponding to a physical address as free.

    Clears the bit corresponding to the page and increments the number of free pages by one.

    @param region Pointer to the regions struct.
    @param phys_address Physical address of the page.
    @returns PMM_REGION_ERROR if the address is outside of the regions bounds, otherwise PMM_REGION_OK.
*/
pmm_region_error_codes_t pmm_region_mark_page_free(struct pmm_region_t *region, uintptr_t phys_address)
{
    if (phys_address < region->base || phys_address > (region->base + region->length))
    {
        printf("PMM REGION: ERROR: Address is not in region: phys address=%p, base=%p, len=%u.\n", phys_address, region->base, region->length);
        return PMM_REGION_ERROR;
    }   

    size_t page = (phys_address - region->base) / PAGE_SIZE_BYTE;

    region->bitmap[page / 8] = region->bitmap[page / 8] & ~(1 << page % 8);
    region->free_pages = region->free_pages + 1; 
    
    return PMM_REGION_OK;
}

/*!
    @brief Mark a page corresponding to a physical address as used.

    Sets the bit corresponding to the page and decrements the number of free pages by one.

    @param region Pointer to the regions struct.
    @param phys_address Physical address of the page.
    @returns PMM_REGION_ERROR if the address is outside of the regions bounds, otherwise PMM_REGION_OK.
*/
pmm_region_error_codes_t pmm_region_mark_page_used(struct pmm_region_t *region, uintptr_t phys_address)
{
    if (phys_address < region->base || phys_address > (region->base + region->length))
    {
        printf("PMM REGION: ERROR: Address is not in region: phys address=%p, base=%p, len=%u.\n", phys_address, region->base, region->length);
        return PMM_REGION_ERROR;
    }   

    size_t page = (phys_address - region->base) / PAGE_SIZE_BYTE;

    region->bitmap[page / 8] = region->bitmap[page / 8] | (1 << page % 8); 
    region->free_pages = region->free_pages - 1;
    
    return PMM_REGION_OK;
}

/*!
    @brief Initialize a struct for a region.

    @param region Pointer to the region struct.
    @param bitmap_base Pointer where the regions bitmap should be stored.
    @param region_base Physical base address of the region.
    @param region_length The regions lenght in byte.
    @param region_type Type of memory the region consists of.
*/
void pmm_region_init(struct pmm_region_t *region, uint8_t *bitmap_base, uintptr_t region_base, ptrdiff_t region_length, pmm_memory_types_t region_type)
{
    region->base = region_base;
    region->length = region_length;
    region->type = region_type;

    // Calculate how many pages are in the region. 
    // The result is rounded down, in case the end is not page aligned.
    region->free_pages = region_length / PAGE_SIZE_BYTE;
    
    // Calculate how large the bitmap needs to be to store information about all pages.
    // The result is rounded up, as otherwise a number of pages thats not divisible by eight would lead to a too small bitmap.
    region->bitmap_size = ((region->free_pages + 7) / 8);

    // Set the pointer for the bitmap.
    region->bitmap = bitmap_base;
    
    if (region->type == MEMMAP_TYPE_USABLE)
    {
        // If the regions memory is usable, mark all pages as free.
        memset(region->bitmap, PMM_REGION_BITMAP_FREE, region->bitmap_size);
    }
    else
    {
        // If the regions memory is not usable, mark all pages as used.
        memset(region->bitmap, UINT8_MAX, region->bitmap_size);
        region->free_pages = 0;
    }      
}