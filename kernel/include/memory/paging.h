#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>

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
    @brief Page table entry.
*/
union page_table_entry_t
{
    struct page_table_entry_bits
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
        uint64_t base_address: 28;
        uint64_t reserved: 11;
        uint64_t available_2: 7;
        uint64_t protection_key: 4;
        uint64_t disable_execution: 1;
    } fields;
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

#endif // PAGING_H