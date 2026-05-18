#include "memory/paging.h"

#include <stddef.h>

#include "logging.h"

#define PAGE_TABLE_NUM_ENTRIES 512

void dump_page_table(union page_table_entry_t *page_table, uint64_t hhdm_offset, page_table_level_t level)
{
    static int16_t pml4_idx = -1;
    static int16_t pdpr_idx = -1;
    static int16_t pd_idx = -1;
    static int16_t pt_idx = -1;

    for (int16_t idx = 0; idx < PAGE_TABLE_NUM_ENTRIES; idx++)
    {
        // Skip empty entries.
        if (page_table[idx].raw == 0)
        {
            continue;
        }
        
        switch (level)
        {
        case PML4:
            pml4_idx = idx;
            LOG_INFO("PML4@%d: %x", pml4_idx, page_table[pml4_idx].raw);
            union page_table_entry_t *pdpr = (union page_table_entry_t *) ((void *)(page_table[pml4_idx].fields.base_address << 12) + hhdm_offset);
            dump_page_table(pdpr, hhdm_offset, PDPR);
            break;

        case PDPR:
            pdpr_idx = idx;

            if (page_table[pdpr_idx].fields.page_size != 0)
            {
                LOG_INFO("PDPR-%d@%d: %x (1GB-PAGE)", pml4_idx, pdpr_idx, page_table[pdpr_idx].raw);
                break;
            }

            LOG_INFO("PDPR-%d@%d: %x", pml4_idx, pdpr_idx, page_table[pdpr_idx].raw);
            union page_table_entry_t *pd = (union page_table_entry_t *) ((void *)(page_table[pdpr_idx].fields.base_address << 12) + hhdm_offset);
            dump_page_table(pd, hhdm_offset, PD);
            break;

        case PD:
            pd_idx = idx;

            if (page_table[pd_idx].fields.page_size != 0)
            {
                LOG_INFO("PD-%d-%d@%d: %x (2MB-PAGE)", pml4_idx, pdpr_idx, pd_idx, page_table[pd_idx].raw);
                break;
            }

            LOG_INFO("PD-%d-%d@%d: %x", pml4_idx, pdpr_idx, pd_idx, page_table[pd_idx].raw);
            union page_table_entry_t *pt = (union page_table_entry_t *) ((void *)(page_table[pd_idx].fields.base_address << 12) + hhdm_offset);
            dump_page_table(pt, hhdm_offset, PT);
            break;

        case PT:
            pt_idx = idx;
            LOG_INFO("PT-%d-%d-%d@%d: %x", pml4_idx, pdpr_idx, pd_idx, pt_idx, page_table[pt_idx].raw);
            break;

        default:
            break;
        }
    }
}