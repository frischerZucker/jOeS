#include <limits.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>

#include "stdio.h"
#include "string.h"

#include "charset.h"
#include "gdt.h"
#include "hcf.h"
#include "idt.h"
#include "terminal.h"

// set limine base revision to 3
__attribute__((used, section(".limine_requests"))) static volatile LIMINE_BASE_REVISION(3);

__attribute__((used, section(".limine_requests"))) static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
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

    terminal_write_string("Joe Biden\n", strlen("Joe Biden\n"));
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

    idt_init();
    idt_install(idt);
    // It's the same as with the GDT.. I think if this code is printed, the IDT was loaded correctly.
    terminal_write_string("> IDT loaded successfully.\n", strlen("> IDT loaded successfully.\n"));

    printf("> Testing printf():\nchar: %c\nescaping the format character: %%\nstring:%s\nint > 0: %d\nint < 0:%d\n", 'a', "no service", 187420, -161);

    // This triggers a breakpoint interrupt.
    // asm("int3");
    // This should fail and trigger a "Division by 0" exception.
    // The volatile stuff is needed to force a division at runtime. 
    // Otherwise the compiler generates an "un2" isntruction and I get an Invalid Opcode exception instead of a Divide Error.
    // volatile int a = 1;
    // volatile int b = 0;
    // volatile int c = a / b;
    // printf("%d", c);

    hcf();
}