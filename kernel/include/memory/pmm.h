/*!
    @file pmm.h
    @addtogroup memory

    @brief TESTTEST.

    @author frischerZucker
*/

#ifndef PMM_H
#define PMM_H

#include <limine.h>

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
    PMM_INIT_FAILED
} pmm_error_codes_t;

pmm_error_codes_t pmm_init(struct limine_memmap_response *memmap, uint64_t hhdm_offset);

#endif // PMM_H