#include <limits.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>

#include "stdio.h"
#include "string.h"

#include "charset.h"
#include "cpu/gdt.h"
#include "cpu/hcf.h"
#include "cpu/idt.h"
#include "cpu/registers.h"
#include "drivers/pic.h"
#include "drivers/pit.h"
#include "drivers/ps2.h"
#include "drivers/serial.h"
#include "logging.h"
#include "memory/paging.h"
#include "memory/pmm.h"
#include "terminal.h"

// set limine base revision to 3
__attribute__((used, section(".limine_requests"))) static volatile LIMINE_BASE_REVISION(3);

__attribute__((used, section(".limine_requests"))) static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0};

__attribute__((used, section(".limine_requests"))) static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0};

__attribute__((used, section(".limine_requests"))) static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0};

__attribute__((used, section(".limine_requests_start"))) static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end"))) static volatile LIMINE_REQUESTS_END_MARKER;

void kmain(void)
{
    // Ensure the bootloader supports the base revision.
    if (LIMINE_BASE_REVISION_SUPPORTED == false)
    {
        hcf();
    }

    // Ensure we got a framebuffer.
    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1)
    {
        hcf();
    }

    // Fetch a framebuffer.
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];

    terminal_init(framebuffer, CHARACTER_WIDTH * 1.3, CHARACTER_HEIGHT * 2.2);

    terminal_write_string("Joe\n", strlen("Joe\n"));
    terminal_set_color(0xaa0000);
    terminal_put_char('o');
    terminal_put_char('\t');
    terminal_set_color(0xaa00aa);
    terminal_put_char('o');
    terminal_put_char('\n');
    terminal_set_color(0xffffff);

    logging_set_backend(terminal_log_write, NULL);

    gdt_init();
    gdt_install(gdt);

    idt_init();
    idt_install(idt);

    if (serial_init(COM1, 38400, SERIAL_DATA_BITS_8 | SERIAL_PARITY_NONE | SERIAL_STOP_BITS_1) != SERIAL_OK)
    {
        LOG_ERROR("ERROR: Something went wrong setting up COM1 serial port!");
        hcf();
    }

    // Switch logging to serial, so I can see it in a scrollable terminal.
    int serial_config = COM1;
    logging_set_backend(serial_log_write, &serial_config);

    // Ensure we really got a memory map.
    if (memmap_request.response == NULL)
    {
        LOG_ERROR("Could not get Memory Map :(");
        hcf();
    }

    // Get the memory map.
    struct limine_memmap_response *memmap = memmap_request.response;
    
    // Ensure we got the hhdm offset.
    if (hhdm_request.response == NULL)
    {
        LOG_ERROR("Could not get HHDM :(");
        hcf();
    }
    
    struct limine_hhdm_response *hhdm_response = hhdm_request.response;
    
    if (pmm_init(memmap, hhdm_response->offset) != PMM_OK)
    {
        hcf();
    }

    paging_init((ptrdiff_t)hhdm_response->offset);

    union page_table_entry_t *old_pml4 = (union page_table_entry_t *) ((read_cr3() & ~0x7ff) + hhdm_response->offset);

    union page_table_entry_t *pml4 = NULL;

    if (paging_clone_page_table(old_pml4, &pml4, PML4) != PAGING_OK)
    {
        LOG_ERROR("Failed to clone page table.");
        hcf();
    }
    LOG_INFO("Successfully cloned the page table.");

    LOG_INFO("Before loading cr3");
    
    set_cr3(((uint64_t)pml4) - hhdm_response->offset);

    LOG_INFO("after loading cr3");

    // Initialize the PIC and enable interrupts.
    pic_init(0x20, 0x28);
    asm("sti");
    pit_init_channel(PIT_CHANNEL_0, 1000, PIT_SC_COUNTER_0 | PIT_MODE_SQUARE_WAVE);
    pic_enable_irq(0);
    
    ps2_init_controller();

    LOG_INFO("Test mapping and unmapping a page...");

    if (paging_map_page(pml4, 0x7fb000, 0x7fb000, PAGE_SIZE_4KB, PAGING_FLAG_PRESENT | PAGING_FLAG_WRITABLE))
    {
        LOG_ERROR("Failed to map page");
    }
    
    if (paging_unmap_page(pml4, 0x7fb000, PAGE_SIZE_4KB))
    {
        LOG_ERROR("Failed to unmap page");
    }

    LOG_INFO("No erros. Seems to work i guess.");

    hcf();
}