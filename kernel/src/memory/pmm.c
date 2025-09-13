#include "memory/pmm.h"

#include "stdio.h"
#include "string.h"

#define PAGE_SIZE_BYTE 4096

/*!
    @brief Available types of memory
*/
typedef enum {
    MEMMAP_TYPE_USABLE = 0,
    MEMMAP_TYPE_RESERVED,
    MEMMAP_TYPE_ACPI_RECLAIMABLE,
    MEMMAP_TYPE_ACPI_NON_VOLATILE,
    MEMMAP_TYPE_BAD_MEMORY,
    MEMMAP_TYPE_BOOTLOADER_RECLAIMABLE,
    MEMMAP_TYPE_KERNEL_AND_MODULES,
    MEMMAP_TYPE_FRAMEBUFFER
} pmm_memory_type_t;

enum {
    PMM_BITMAP_FREE = 0,
    PMM_BITMAP_USED,
    PMM_BITMAP_RESERVED
} pmm_bitmap_entry_t;

struct pmm_region_t {
    uint8_t *bitmap;
    uint64_t bitmap_size;

    uint64_t base;
    uint64_t length;

    uint64_t free_pages;

    pmm_memory_type_t type;
};

static uint64_t pmm_memory_size_pages = 0;
static uint64_t pmm_num_regions = 0;

static struct pmm_region_t *pmm_regions = {0};

/*!
    @brief Gets info about available memory and memory regions from the memory map.

    Iterates over the memory map, calculates how many pages there are.
    Saves the result to pmm_memory_size_pages and the number of memory regions to pmm_num_regions.

    @param memmap Pointer to Limines memory map.
*/
static void pmm_detect_memory(struct limine_memmap_response *memmap)
{
    uint64_t memory_size_byte = 0;

    // Sum up the length of all entries of the memory map.
    size_t pmm_num_regions = memmap->entry_count;
    for (size_t i = 0; i < pmm_num_regions; i++)
    {
        memory_size_byte = memory_size_byte + memmap->entries[i]->length;
    }

    pmm_memory_size_pages = memory_size_byte / PAGE_SIZE_BYTE;

    printf("PMM: Detected %u kiB ^= %u pages memory.\n", memory_size_byte / 1024, pmm_memory_size_pages);
}

/*!
    @brief Calculates how much memory is needed for the PMMs data.

    @returns Number of pages required to store the PMMs data.
*/
static size_t pmm_calc_required_space()
{
    size_t required_pages = 0;

    // Space required for the pmm_region_t structs.
    size_t required_bytes = pmm_num_regions * sizeof(struct pmm_region_t);
    // Space required for the bitmaps.
    required_bytes = required_bytes + ((pmm_memory_size_pages + 7) / 8);

    required_pages = (required_bytes + PAGE_SIZE_BYTE - 1) / PAGE_SIZE_BYTE;

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

void pmm_init(struct limine_memmap_response *memmap, uint64_t hhdm_offset)
{
    printf("HHDM: %p\n", hhdm_offset);
    pmm_print_memmap(memmap);
 
    // Detects how many pages are mapped. I don't really use this, so maybe I can remove it??
    pmm_detect_memory(memmap);

    size_t required_pages =  pmm_calc_required_space();
    printf("Pages required for PMMs data: %u\n", required_pages);

    // Contains bitmaps for all usable memory regions.
    // As of now it only lives in this functions stack.
    /// @todo This needs to find a place in memory where it can live forever.
    // struct pmm_bitmap_t *usable_regions_temp[pmm_num_regions];

    // // Create a bitmap for all usable memory regions.
    // size_t j = 0;
    // size_t memmap_num_entries = memmap->entry_count;
    // for (size_t i = 0; i < memmap_num_entries; i++)
    // {
    //     // Skip unsusual memory. It cannot be used, so it does not matter.
    //     if (memmap->entries[i]->type != MEMMAP_TYPE_USABLE)
    //     {
    //         continue;
    //     }
        
    //     printf("Usable Region @%p ^= page %u\n", memmap->entries[i]->base, memmap->entries[i]->base / PAGE_SIZE_BYTE);

    //     // Used to count how many bytes are used by the regions bitmap, so occupied pages can later be marked as USED.
    //     uint64_t bytes_used = 0;

    //     // Store the regions bitmap at its first address.
    //     usable_regions_temp[j] = memmap->entries[i]->base + hhdm_offset;

    //     bytes_used = bytes_used + sizeof(struct pmm_bitmap_t);

    //     // The regions base address.
    //     usable_regions_temp[j]->base = memmap->entries[i]->base;
    //     // The regions length in byte.
    //     usable_regions_temp[j]->length = memmap->entries[i]->length;
    //     // How many pages the region contains.
    //     usable_regions_temp[j]->free_pages = memmap->entries[i]->length / PAGE_SIZE_BYTE;

    //     // Store the bitmaps buffer right after the bitmaps struct.
    //     usable_regions_temp[j]->buffer = usable_regions_temp[j]->base + hhdm_offset + bytes_used;
    //     // How large the bitmap needs to be. Each bit contains information about one page, so (free_pages / 8) bytes are required.
    //     // The result is rounded up by using (x + 7) / 8 to ensure its large enough.
    //     usable_regions_temp[j]->bitmap_size = (usable_regions_temp[j]->free_pages + 7) / 8;
    //     bytes_used = bytes_used + usable_regions_temp[j]->bitmap_size;

    //     // Mark all pages as FREE.
    //     memset(usable_regions_temp[j]->buffer, PMM_BITMAP_FREE, usable_regions_temp[j]->bitmap_size);

    //     // How large the regions bitmap is in pages. 
    //     uint64_t pages_used = (bytes_used + PAGE_SIZE_BYTE - 1) / PAGE_SIZE_BYTE;
    //     // Mark the pages used by the regions bitmap as USED.
    //     int a = 0, b = 0;
    //     while (pages_used > 0)
    //     {
    //         usable_regions_temp[j]->buffer[a] = (PMM_BITMAP_USED << b);
    //         pages_used = pages_used - 1;

    //         b = b + 1;
    //         if (b >= 8)
    //         {
    //             a = a + 1;
    //             b = 0;
    //         }
    //     }

    //     j = j + 1;
    // }

    // for (size_t i = 0; i < usable_regions_temp[0]->bitmap_size; i++)
    // {
    //     printf("%d ", usable_regions_temp[0]->buffer[i]);
    // }
    
}