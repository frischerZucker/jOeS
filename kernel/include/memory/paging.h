#ifndef PAGING_H
#define PAGING_H

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
#define PAGING_FLAG_ALL ((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 7) | (1 << 8) | (1L << 63))

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
    } pointer_fields;
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
    } pointer_fields;
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
    } page_fields;
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
    } pointer_fields;
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
    } page_fields;
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
    } page_fields;
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
    @brief Recursively traverse and log the structure of a page table.

    Walks through all entries of a given page table (PML4, PDPR, PD, PT) and prints information about each non-empty entry.
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
void dump_page_table(union page_table_entry_t *page_table, uint64_t hhdm_offset, page_table_level_t level);

paging_error_codes_t paging_map_page(union page_table_entry_t *pml4, uintptr_t phys, uintptr_t virt, page_size_t page_size, uint64_t flags, ptrdiff_t hhdm_offset);

paging_error_codes_t paging_clone_page_table(union page_table_entry_t *old_pml4, union page_table_entry_t **new_pml4, uint64_t hhdm_offset, page_table_level_t level);

#endif // PAGING_H