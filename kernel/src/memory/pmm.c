#include "memory/pmm.h"

#include "stdio.h"
#include "string.h"

#include "cpu/hcf.h"
#include "drivers/serial.h"
#include "memory/pmm_region.h"

#define PAGE_SIZE_BYTE 4096

static size_t pmm_memory_size_pages = 0;
static size_t pmm_num_regions = 0;

static ptrdiff_t phys_to_virt_offset = 0;

static struct pmm_region_t *pmm_regions;

/*!
    @brief Convert physical to virtual addresses by adding the offset.

    @param phys Physical address.
    @param offset Offset used by the higher halt direct map.
    @returns Virtual address.
*/
static inline uintptr_t phys_to_virt(uintptr_t phys, ptrdiff_t offset)
{
    return phys + offset;
}

/*!
    @brief Gets info about available memory and memory regions from the memory map.

    Iterates over the memory map and calculates how many pages there are.
    Saves the result to pmm_memory_size_pages and the number of memory regions to pmm_num_regions.

    @param memmap Pointer to Limines memory map.
    @param num_regions Pointer to the variable where the number of regions should be stored.
    @param num_pages Pointer to the variable where the number of pages should be stored.
*/
static void pmm_detect_memory(struct limine_memmap_response *memmap, size_t *num_regions, size_t *num_pages)
{
    size_t memory_size_byte = 0;

    // Sum up the length of all entries of the memory map.
    *num_regions = memmap->entry_count;
    for (size_t i = 0; i < *num_regions; i++)
    {
        memory_size_byte = memory_size_byte + memmap->entries[i]->length;
    }

    *num_pages = memory_size_byte / PAGE_SIZE_BYTE;

    printf("PMM: Detected %u kiB ^= %u pages memory.\n", memory_size_byte / 1024, *num_pages);
}

/*!
    @brief Calculates how many pages are required to store the PMMs data.

    @param regions How many memory regions exist.
    @param pages How many pages exist.
    @returns Number of pages required to store the PMMs data.
*/
static size_t pmm_get_num_required_pages(size_t regions, size_t pages)
{
    size_t required_pages = 0;

    // Space required for the pmm_region_t structs.
    size_t required_bytes = regions * sizeof(struct pmm_region_t);
    // Space required for the bitmaps.
    required_bytes = required_bytes + ((pages + 7) / 8);

    required_pages = (required_bytes + PAGE_SIZE_BYTE - 1) / PAGE_SIZE_BYTE;

    printf("PMM: %u B / %u pages are required for the PMMs data.\n", required_bytes, required_pages);

    return required_pages;
}

/*!
    @brief Prints the memory map.

    Prints the memory map as a table with following structure:
        BASE    LENGTH  TYPE
    Rows with memory of the type MEMMAP_TYPE_USABLE get a "[usable]" postfix.
    
    @param memmap Pointer to a Limine memmap struct.
*/
static void pmm_print_memmap(struct limine_memmap_response *memmap)
{
    size_t memmap_num_entries = memmap->entry_count;
    printf("Memory Map:\nBase\tLength\tType\n");
    for (size_t i = 0; i < memmap_num_entries; i++)
    {
        printf("%p\t%p\t%d", memmap->entries[i]->base, memmap->entries[i]->length, memmap->entries[i]->type);
        if (memmap->entries[i]->type == MEMMAP_TYPE_USABLE)
        {
            printf(" [usable]");
        }
        printf("\n");
    }
}

/*!
    @brief Initializes region structs for the physical memory manager (PMM).

    Converts the physical base address to a virtual address and sets up an array of pmm_region_t structs,
    each representing a memory region from Limines memory map.
    For each region, a bitmap is initialized to track page usage.

    Assumes that the PMM metadata (region structs and bitmaps) is stored in usable memory.
    Pages used by the PMM are marked as used, to protect them of accidental overwriting.

    Assumes the global variable pmm_regions exists and uses it to store the region arrays address.

    @todo Instead of marking pages as used, I could create a seperate region for PMM data.
    @todo If there are multiple regions with a length thats not a multiple of the page size, the calculated number of required pages from pmm_get_num_required_pages() could be wrong.

    @param memmap Pointer to a Limine memory map.
    @param base Physical base address where the region array starts.
    @param offset Offset used for physical-to-virtual address translation.
    @param required_pages Number of pages used by the PMM that must be marked as used.
    @returns PMM_OK on success, PMM_INIT_FAILED if marking PMM pages as used fails.
*/
static pmm_error_codes_t pmm_init_region_structs(struct limine_memmap_response *memmap, uintptr_t base, ptrdiff_t offset, size_t required_pages)
{
    // Set the base address for the region array.
    pmm_regions = (struct pmm_region_t *) phys_to_virt(base, offset);

    // Increment base to point to the first byte behind the region array.
    // We will use this for the first regions bitmap.
    uintptr_t bitmap_base = base + pmm_num_regions * sizeof(struct pmm_region_t);

    // Initialize structs for all regions.
    for (size_t i = 0; i < pmm_num_regions; i++)
    {
        pmm_region_init(&pmm_regions[i], (uint8_t *)phys_to_virt(bitmap_base, offset), memmap->entries[i]->base, memmap->entries[i]->length, memmap->entries[i]->type);

        // Increment base to point to the first byte after the current regions bitmap.
        // We will use this for the next bitmap.
        bitmap_base = bitmap_base + pmm_regions[i].bitmap_size;
    }

    /// @todo: Instead of marking the pages as used, I could split the region in two, so that the pages used by the PMM get their own region.
    
    // Mark memory used for storing these structs as used.
    for (size_t i = 0; i < pmm_num_regions; i++)
    {
        // Skip the region if its not the region where the PMMs data is stored.
        if (pmm_regions[i].base != base)
        {
            continue;
        }
        
        // Mark the regions first pages as used.
        /// @todo If there are multiple regions with a length thats not a multiple of the page size, the calculated number of required pages from pmm_get_num_required_pages() could be wrong.
        for (size_t page = 0; page < required_pages; page++)
        {
            // Calculate the pages physical address.
            // regions needs to be casted to an uintptr_t, so that the addition does not increment by regions size, but uses bytes instead.
            uintptr_t page_address = ((uintptr_t)pmm_regions - offset + page * PAGE_SIZE_BYTE);
            
            if (pmm_region_mark_page_used(&pmm_regions[i], page_address) != PMM_REGION_OK)
            {
                printf("PMM: ERROR: Could not mark page used by PMM as used!\n");
                return PMM_ERROR_INIT_FAILED;
            }
        }        

        break;
    }
    
    return PMM_OK;
}

/*!
    @brief Searches a region containing a page.

    Uses binary search to find the region containing the pages address.
    If it is found, its index in the region array is written into the value pointed to by region_index.
    Returns an error code if no region containing the pages address was found.

    @param region_index Pointer to the variable where the regions index should be stored.
    @param ptr Pointer (physical address) off the page.
    @returns PMM_OK if a matching region was found, PMM_ERROR_ADDRESS_NOT_FOUND if not.
*/
static pmm_error_codes_t get_region_containing_page(size_t *region_index, void *ptr)
{
    size_t lower_bound = 0;
    size_t upper_bound = pmm_num_regions - 1;

    /*
        Search for up to pmm_num_regions tries. 
        If all tries failed we can be sure that no region containing 
        the address exists and the search got stuck in an endless loop,
        as binary searchs worst case is log_2(pmm_num_regions).
    */
    for (size_t i = 0; i < pmm_num_regions; i++)
    {
        // Calculate the next index to look at in the middle of the searched range.
        *region_index = lower_bound + ((upper_bound - lower_bound) / 2);

        // The address is smaller then the regions max. address.
        // The search windows upper bound is moved to the current index because the region can only be below there.
        if (pmm_regions[*region_index].base > (uintptr_t)ptr)
        {
            upper_bound = *region_index - 1;
            continue;
        }
        // The address is bigger then the regions max address.
        // The search windows lower bound is moved to the current index because the region can only be above there.
        else if ((uintptr_t)ptr > pmm_regions[*region_index].base + pmm_regions[*region_index].length - 1)
        {
            lower_bound = *region_index + 1;
            continue;
        }
        else
        {
            // The current region includes the address, so we can return from the function.
            // The value pointed to by region_index does now include the correct index.
            return PMM_OK;
        }        
    }
    
    // No region including the physical address pointed to by ptr was found.
    return PMM_ERROR_ADDRESS_NOT_FOUND;
}

/*!
    @brief Checks if a page is currently free or in use.

    Searches through all regions to find the region that includes the addresses page.
    If it is found, calculates the page number corresponding page.
    Checks its status in the bitmap and returns it.
    If no matching region is found, a special page status is returned.

    @param ptr Pointer (physical address) off the page to check.
    @returns PMM_PAGE_FREE if the page is free, PMM_PAGE_USED if it is used and PMM_PAGE_NOT_FOUND if no region matching the address is found.
*/
pmm_page_status_t pmm_check_page(void *ptr)
{
    size_t region_index;
    
    if (get_region_containing_page(&region_index, ptr) != PMM_OK)
    {
        // No region including the physical address pointed to by ptr was found.
        return PMM_PAGE_NOT_FOUND;
    }
    
    size_t page_index = ((uintptr_t)ptr - pmm_regions[region_index].base) / PAGE_SIZE_BYTE;
    
    return pmm_regions[region_index].bitmap[(uintptr_t)page_index / 8] & (1 << ((uintptr_t)page_index % 8));    
}

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
void * pmm_alloc()
{
    static size_t region_cache = 0;
    static size_t bitmap_cache = 0;

    // Search a region that has free pages left.
    for (size_t i = 0; i < pmm_num_regions; i++)
    {
        /*
            Start at the last region where a free page was found in hope that there are more free pages.
            This could avoid some iterations and therefore speed up the search a little.
            The modulo operation is there so that the search will wrap around if the end of the array is reached without a hit.
            This ensures everything, not just everything behind our cached region, is searched.
        */
        size_t region_index = (i + region_cache) % pmm_num_regions;

        // Skip the current region if it has no free pages.
        if (pmm_regions[region_index].free_pages == 0)
        {
            continue;
        }
        
        // A region with free pages was found, so the regions index is saved as a starting point for the next allocation.
        region_cache = region_index;
        
        // Search a byte with free pages in the regions bitmap.
        for (size_t j = 0; j < pmm_regions[region_index].bitmap_size; i++)
        {
            /*
                Start at the last byte where a free page was found in hope that there are more free pages.
                This could avoid some iterations and therefore speed up the search a little.
                The modulo operation is also there to make it wrap around at the end of the bitmap.
            */
            size_t bitmap_index =(j + bitmap_cache) % pmm_regions[region_index].bitmap_size;

            // Skip bytes with no free pages.
            if (pmm_regions[region_index].bitmap[bitmap_index] == UINT8_MAX)
            {
                continue;
            }
            
            // A byte with free pages was found, so its index is saved as a starting point for the next allocation.
            bitmap_cache = bitmap_index;

            // Search through the bytes bits for a free page.
            for (size_t k = 0; k < 8; k++)
            {
                if ((pmm_regions[region_index].bitmap[bitmap_index] & (1 << k)) == PMM_REGION_BITMAP_FREE)
                {
                    // Calculate the pages index in the region.
                    size_t page = (bitmap_index * 8) + k;

                    // Make ptr point to the pages physical address by adding the pages offset (page size * page index) to the regions base address.
                    void * ptr = (void *)(pmm_regions[region_index].base + page * PAGE_SIZE_BYTE);
                    // Mark the page as used.
                    pmm_region_mark_page_used(&pmm_regions[region_index], (uintptr_t)ptr);                    
                    
                    // A page was found, so all these loops can be left and the address of the page returned.
                    return ptr;
                }
            }
        }
    }
    
    // Returns NULL if no free page was found, and its address if one was found.
    return NULL;
}

/*!
    @brief Frees a single physical memory page.

    Uses get_region_containing_page() to find the region that includes the page.
    If it is found, the page is marked as free.
    If none is found, an error is returned.

    @param ptr Pointer (physical address) to the page to free.
    @returns PMM_OK on success, PMM_ERROR_ADDRESS_NOT_FOUND if no region that contains the pages address was found.
*/
pmm_error_codes_t pmm_free(void *ptr)
{
    size_t region_index;
    
    if (get_region_containing_page(&region_index, ptr) != PMM_OK)
    {
        // No region including the physical address pointed to by ptr was found, so an error is returned. 
        return PMM_ERROR_ADDRESS_NOT_FOUND;
    }
    
    // A region including ptr was found. Mark the page corresponding to the address as free.
    pmm_region_mark_page_free(&pmm_regions[region_index], (uintptr_t)ptr);
    return PMM_OK;
}

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
pmm_error_codes_t pmm_init(struct limine_memmap_response *memmap, uint64_t hhdm_offset)
{
    printf("PMM: HHDM=%p\n", hhdm_offset);
    phys_to_virt_offset = hhdm_offset;

    pmm_print_memmap(memmap);
 
    pmm_detect_memory(memmap, &pmm_num_regions, &pmm_memory_size_pages);

    // Calculate how many pages are required to store the PMMs data.
    size_t required_pages =  pmm_get_num_required_pages(pmm_num_regions, pmm_memory_size_pages);

    // Search for a place where the PMMs data can be stored.
    uintptr_t pmm_base = (uintptr_t)NULL;
    for (size_t i = 0; i < pmm_num_regions; i++)
    {
        if (memmap->entries[i]->type != MEMMAP_TYPE_USABLE)
        {
            continue;
        }
        
        if (memmap->entries[i]->length >= required_pages * PAGE_SIZE_BYTE)
        {
            pmm_base = memmap->entries[i]->base;
            break;
        }
    }
    if (pmm_base != (uintptr_t)NULL)
    {
        printf("PMM: Found space to store data at %p.\n", pmm_base);
    }
    else
    {
        printf("PMM: ERROR: Could not find space to store data!\n");
        return PMM_ERROR_INIT_FAILED;
    }

    if (pmm_init_region_structs(memmap, pmm_base, phys_to_virt_offset, required_pages) != PMM_OK)
    {
        return PMM_ERROR_INIT_FAILED;
    }        

    printf("PMM: PMM initialized.\n");    

    return PMM_OK;
}