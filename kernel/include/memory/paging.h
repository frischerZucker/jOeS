#ifndef PAGING_H
#define PAGING_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum
{
    PAGING_OK = 0,
    PAGING_ERROR
} paging_error_codes_t;

typedef enum
{
    PAGE_SIZE_4KB,
    PAGE_SIZE_2MB,
    PAGE_SIZE_1GB
} page_size_t;

#define PAGING_FLAG_PRESENT (1 << 0)
#define PAGING_FLAG_WRITABLE (1 << 1)
#define PAGING_FLAG_USER_LEVEL (1 << 2)
#define PAGING_FLAG_PWT (1 << 3)
#define PAGING_FLAG_PCD (1 << 4)
#define PAGING_FLAG_PAGE_SIZE (1 << 7)
#define PAGING_FLAG_GLOBAL (1 << 8)
#define PAGING_FLAG_DISABLE_EXECUTION (1 << 63)

struct paging_flags_t
{
    bool writable;
    bool user_level;
    bool page_write_through;
    bool page_cache_disable;
    bool page_attribute_table;
    bool page_size;
    bool global;
    bool disable_execution;
    uint8_t protection_key;
};

/*!
    @brief Page table levels.
*/
typedef enum
{
    PML4,
    PDPR,
    PD,
    PT
} page_table_level_t;

/*!
    @brief Page table entry types.

    The entry can either be a pointer to another table or describing a page.
*/
typedef enum
{
    PAGING_ENTRY_POINTER,
    PAGING_ENTRY_PAGE
} page_table_entry_type_t;

union pml4_entry_t
{
    struct
    {
        uint64_t present: 1;
        uint64_t writable: 1;
        uint64_t user_level: 1;
        uint64_t page_write_through: 1;
        uint64_t page_cache_disable: 1;
        uint64_t accessed: 1;
        uint64_t available_1: 1;
        uint64_t reserved_1: 1;
        uint64_t available_2: 4;
        uint64_t base_address: 28;
        uint64_t reserved_2: 12;
        uint64_t available_3: 11;
        uint64_t disable_execution: 1;
    } __attribute__((packed)) pointer_fields;
};

union pdpr_entry_t
{
    struct
    {
        uint64_t present: 1;
        uint64_t writable: 1;
        uint64_t user_level: 1;
        uint64_t page_write_through: 1;
        uint64_t page_cache_disable: 1;
        uint64_t accessed: 1;
        uint64_t dirty: 1;
        uint64_t page_size: 1;
        uint64_t available_1: 4;
        uint64_t base_address: 28;
        uint64_t reserved: 12;
        uint64_t available_2: 11;
        uint64_t disable_execution: 1;
    } __attribute__((packed)) pointer_fields;
    struct
    {
        uint64_t present: 1;
        uint64_t writable: 1;
        uint64_t user_level: 1;
        uint64_t page_write_through: 1;
        uint64_t page_cache_disable: 1;
        uint64_t accessed: 1;
        uint64_t dirty: 1;
        uint64_t page_size: 1;
        uint64_t global: 1;
        uint64_t available_1: 3;
        uint64_t page_attribute_table: 1;
        uint64_t reserved_1: 17;
        uint64_t base_address: 10;
        uint64_t reserved_2: 12;
        uint64_t available_2: 7;
        uint64_t protection_key: 4;
        uint64_t disable_execution: 1;
    } __attribute__((packed)) page_fields;
};

union pd_entry_t
{
    struct
    {
        uint64_t present: 1;
        uint64_t writable: 1;
        uint64_t user_level: 1;
        uint64_t page_write_through: 1;
        uint64_t page_cache_disable: 1;
        uint64_t accessed: 1;
        uint64_t dirty: 1;
        uint64_t page_size: 1;
        uint64_t available_1: 4;
        uint64_t base_address: 28;
        uint64_t reserved: 12;
        uint64_t available_2: 11;
        uint64_t disable_execution: 1;
    } __attribute__((packed)) pointer_fields;
    struct
    {
        uint64_t present: 1;
        uint64_t writable: 1;
        uint64_t user_level: 1;
        uint64_t page_write_through: 1;
        uint64_t page_cache_disable: 1;
        uint64_t accessed: 1;
        uint64_t dirty: 1;
        uint64_t page_size: 1;
        uint64_t global: 1;
        uint64_t available_1: 3;
        uint64_t page_attribute_table: 1;
        uint64_t reserved_1: 8;
        uint64_t base_address: 19;
        uint64_t reserved_2: 12;
        uint64_t available_2: 7;
        uint64_t protection_key: 4;
        uint64_t disable_execution: 1;
    } __attribute__((packed)) page_fields;
};

union pt_entry_t
{
    struct
    {
        uint64_t present: 1;
        uint64_t writable: 1;
        uint64_t user_level: 1;
        uint64_t page_write_through: 1;
        uint64_t page_cache_disable: 1;
        uint64_t accessed: 1;
        uint64_t dirty: 1;
        uint64_t page_attribute_table: 1;
        uint64_t global: 1;
        uint64_t available_1: 3;
        uint64_t base_address: 28;
        uint64_t reserved: 12;
        uint64_t available_2: 7;
        uint64_t protection_key: 4;
        uint64_t disable_execution: 1;
    } __attribute__((packed)) page_fields;
};

/*!
    @brief Union with all the possible page table entries.
*/
union page_table_entry_t
{
    union pml4_entry_t pml4;
    union pdpr_entry_t pdpr;
    union pd_entry_t pd;
    union pt_entry_t pt;
    uint64_t raw;
};

/*!
    @brief Initializes the global HHDM offset.

    @param hhdm_offset HHDM offset used for address translation.
*/
void paging_init(ptrdiff_t hhdm_offset);

/*!
    @brief Recursively traverse and log the structure of a page table.

    Walks through all entries of a given page table (PML4, PDPR, PD or PT) and prints information about each non-empty entry.
    For entries that reference lower-level tables, the function recursively descends into the next level.
    Pages larger than 4kB are detected and logged without further recursion.
    
    Output format:
        PML4@[pml4_idx]: [pml4_entry]\n
        PDPR-[pml4_idx]@[pdpr_idx: [pdpr_entry]\n
        PD-[pml4_idx]-[pdpr_idx]@[pd_idx]: [pd_entry]\n
        PT-[pml4_idx]-[pdpr_idx]-[pd_idx]@[pt_idx]: [pt_entry]\n

    @param page_table Pointer to the current page table.
    @param hhdm_offset Offset for phys<->virt address translation.
    @param level Current page table level (PML4, PDPR, PD, PT).
*/
void paging_dump_page_table(union page_table_entry_t *page_table, page_table_level_t level);

/*!
    @brief Map a virtual address to a physical address in the page table and invalidates its TLB entry.

    Traverses the page table hierarchy until the leaf is reached to map a physical address to a virtual address.
    Supports 4kB, 2MB and 1GB pages.
    Creates entries and allocates memory for page tables as needed.
    Invalidates the pages TLB entry if the page was successfully mapped.

    Returns an error if allocating memory fails or if the leaf is already used (present flag is set).

    @param pml4 PML4 in which the page should be mapped.
    @param phys Physical address to which the virtual address should be mapped.
    @param virt Virtual address that should be mapped to the physical address.
    @param page_size Size of the page to map (4kB, 2MB or 1GB).
    @param flags Flags for the page.

    @returns PAGING_OK on success, PAGING_ERROR if allocating memory failed or the leafs present bit is set.
*/
paging_error_codes_t paging_map_page(union page_table_entry_t *pml4, uintptr_t phys, uintptr_t virt, page_size_t page_size, uint64_t flags);

/*!
    @brief Unmap a page and invalidate its TLB entry.

    Unmaps a page by clearing the present bit in its entry.
    Supports 4kB, 2MB and 1GB pages.
    After clearing the pages present bit, its page table and its parents are checked for emptiness and deletes them if they are no longer required.

    Returns an error if a non-leaf entry is not present.

    @param pml4 Page table in which the page should be unmapped.
    @param virt Virtual address to unmap.
    @param page_size Size of the page to unmap.

    @returns PAGING_OK on success, PAGING_ERROR if a non-leaf entry is not present.
*/
paging_error_codes_t paging_unmap_page(union page_table_entry_t *pml4, uintptr_t virt, page_size_t page_size);

/*!
    @brief Resolve a virtual address to a physical address.

    Walks through the page table hierarchy until a leaf is reached and returns the base address of it.
    Returns NULL if an entry along the way is not present .

    @param pml4 Page table from that the physical address should be retrieved.
    @param virt Virtual address to translate.
    
    @returns Physical address as a uintptr_t.
*/
uintptr_t paging_resolve_virtual_address(union page_table_entry_t *pml4, uintptr_t virt);

/*!
    @brief Recursively walk a page table and clone mapped pages to another page table.

    Recursively walks through all entries of a given page table (PML4, PDPR, PD or PT).
    If a leaf is reached, its flags and the physical address are retrieved and the virtual address is calculated from the indices.
    Maps the page in another page table WITHOUT invalidating its TLB entry.

    @param old_page_table Pointer to the current level of page table that should be cloned.
    @param new_pml4 Pointer to the PML4 of the page table that the entries should be cloned to.
    @param level Current page table level (PML4, PDPR, PD, PT).

    @returns PAGING_OK on success, PAGING_ERROR when allocating memory for the PML4 fails.
*/
paging_error_codes_t paging_clone_page_table(union page_table_entry_t *old_pml4, union page_table_entry_t **new_pml4, page_table_level_t level);

#endif // PAGING_H