#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>

#include "string.h"

#include "gdt.h"
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

    terminal_write_string("Joe Biden\n", strlen("Joe Biden2\n"));
    terminal_set_color(0xaa0000);
    terminal_put_char('o');
    terminal_put_char('\t');
    terminal_set_color(0xaa00aa);
    terminal_put_char('o');
    terminal_put_char('\n');
    terminal_set_color(0xffffff);

    gdt_init();
    gdt_install(gdt);
    // I think if this line is reached the gdt was loaded correctly????
    // I have do look up for some checks to better check it. At least it didn't crash if you can see this lol
    terminal_write_string("> GDT loaded successfully.\n", strlen("> GDT loaded successfully.\n"));

    hcf();
}