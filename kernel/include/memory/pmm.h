/*!
    @file pmm.h

    @brief Physical Memory Manager.

    @author frischerZucker
*/

#ifndef PMM_H
#define PMM_H

#include <limine.h>

#include <stdint.h>

void pmm_init(struct limine_memmap_response *memmap, uint64_t hhdm_offset);

#endif // PMM_H