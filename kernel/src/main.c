#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>

#include "libc/include/string.h"

#include "terminal.h"

// set limine base revision to 3
__attribute__((used, section(".limine_requests"))) static volatile LIMINE_BASE_REVISION(3);

__attribute__((used, section(".limine_requests"))) static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0};

__attribute__((used, section(".limine_requests_start"))) static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end"))) static volatile LIMINE_REQUESTS_END_MARKER;

/*
    Halt-And-Catch-Fire function.

    Stops all action and does nothing.
*/
static void hcf(void)
{
    while (1)
    {
        asm("hlt");
    }
}

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

    terminal_init(framebuffer, 12, 18);

    terminal_write_string("Joe Biden\n", strlen("Joe Biden\n"));
    terminal_set_color(0xaa0000);
    terminal_write_string("\tObamnaprism\n", strlen("\tObamnaprism\n"));
    terminal_set_color(0x00aa00);
    terminal_put_char('o');
    terminal_put_char('\t');
    terminal_set_color(0xaa00aa);
    terminal_put_char('o');
    terminal_put_char('\n');

    // Test if the automatic line break when reaching the right end of the screen works.
    uint8_t c = 33;
    for (size_t i = 0; i < 200; i++)
    {
        terminal_put_char(c);
        c = c + 1;
        if (c > 126)
            c = 33;
    }

    hcf();
}