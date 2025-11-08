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
#include "drivers/pic.h"
#include "drivers/pit.h"
#include "drivers/ps2.h"
#include "drivers/serial.h"
#include "logging.h"
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

    terminal_init(framebuffer, CHAR_WIDTH * 1.3, CHAR_HEIGHT * 2.2);

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

    // Get the memory map and print its entries.
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

    int *b = NULL;
    b = pmm_alloc();
    if (b != NULL)
    {
        LOG_DEBUG("Allocated physical address: %p", b);
    }

    for (size_t i = 0; i < 5; i++)
    {
        int *a = NULL;
        a = pmm_alloc();
        if (a != NULL)
        {
            LOG_DEBUG("Allocated physical address: %p", a);
        }

        pmm_free(a);
    }    

    pmm_free(b);

    b = pmm_alloc();
    if (b != NULL)
    {
        LOG_DEBUG("Allocated physical address: %p", b);
    }

    // Initialize the PIC and enable interrupts.
    pic_init(0x20, 0x28);
    asm("sti");
    pit_init_channel(PIT_CHANNEL_0, 1000, PIT_SC_COUNTER_0 | PIT_MODE_SQUARE_WAVE);
    pic_enable_irq(0);
    
    ps2_init_controller();

    hcf();
}