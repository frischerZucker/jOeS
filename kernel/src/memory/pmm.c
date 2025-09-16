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
        for (size_t page = 0; page < required_pages; page++)
        {
            // Calculate the pages physical address.
            // regions needs to be casted to an uintptr_t, so that the addition does not increment by regions size, but uses bytes instead.
            uintptr_t page_address = ((uintptr_t)pmm_regions - offset + page * PAGE_SIZE_BYTE);
            
            if (pmm_region_mark_page_used(&pmm_regions[i], page_address) != PMM_REGION_OK)
            {
                printf("PMM: ERROR: Could not mark page used by PMM as used!\n");
                return PMM_INIT_FAILED;
            }
        }        

        break;
    }
    
    return PMM_OK;
}

/*!
    @brief Initializes the PMM.
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
        return PMM_INIT_FAILED;
    }

    if (pmm_init_region_structs(memmap, pmm_base, phys_to_virt_offset, required_pages) != PMM_OK)
    {
        return PMM_INIT_FAILED;
    }    

    printf("PMM: PMM initialized.\n");

    return PMM_OK;
}