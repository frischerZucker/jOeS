#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>

#include <terminal.h>

// set limine base revision to 3
__attribute__((used, section(".limine_requests"))) static volatile LIMINE_BASE_REVISION(3);

__attribute__((used, section(".limine_requests"))) static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0};

__attribute__((used, section(".limine_requests_start"))) static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end"))) static volatile LIMINE_REQUESTS_END_MARKER;

// GCC and Clang reserve the right to generate calls to the following
// 4 functions even if they are not directly called.
// Implement them as the C specification mandates.
// DO NOT remove or rename these functions, or stuff will eventually break!
// They CAN be moved to a different .c file.

void *memcpy(void *dest, const void *src, size_t n)
{
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

    for (size_t i = 0; i < n; i++)
    {
        pdest[i] = psrc[i];
    }

    return dest;
}

void *memset(void *s, int c, size_t n)
{
    uint8_t *p = (uint8_t *)s;

    for (size_t i = 0; i < n; i++)
    {
        p[i] = (uint8_t)c;
    }

    return s;
}

void *memmove(void *dest, const void *src, size_t n)
{
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

    if (src > dest)
    {
        for (size_t i = 0; i < n; i++)
        {
            pdest[i] = psrc[i];
        }
    }
    else if (src < dest)
    {
        for (size_t i = n; i > 0; i--)
        {
            pdest[i - 1] = psrc[i - 1];
        }
    }

    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;

    for (size_t i = 0; i < n; i++)
    {
        if (p1[i] != p2[i])
        {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }

    return 0;
}

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

    terminal_put_char('J');
    terminal_put_char('o');
    terminal_put_char('e');
    terminal_put_char(' ');
    terminal_put_char('B');
    terminal_put_char('i');
    terminal_put_char('d');
    terminal_put_char('e');
    terminal_put_char('n');
    terminal_put_char('\n');
    terminal_set_color(0xaa0000);
    terminal_put_char('\t');
    terminal_put_char('O');
    terminal_put_char('b');
    terminal_put_char('a');
    terminal_put_char('m');
    terminal_put_char('n');
    terminal_put_char('a');
    terminal_put_char('p');
    terminal_put_char('r');
    terminal_put_char('i');
    terminal_put_char('s');
    terminal_put_char('m');
    terminal_put_char('\n');
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

    // framebuffer_draw_char(framebuffer, 'J', 100, 100, 0xffffff);
    // framebuffer_draw_char(framebuffer, 'o', 111, 100, 0xffffff);
    // framebuffer_draw_char(framebuffer, 'e', 122, 100, 0xffffff);
    // framebuffer_draw_char(framebuffer, ' ', 133, 100, 0xffffff);
    // framebuffer_draw_char(framebuffer, 'B', 144, 100, 0xffffff);
    // framebuffer_draw_char(framebuffer, 'i', 155, 100, 0xffffff);
    // framebuffer_draw_char(framebuffer, 'd', 166, 100, 0xffffff);
    // framebuffer_draw_char(framebuffer, 'e', 177, 100, 0xffffff);
    // framebuffer_draw_char(framebuffer, 'n', 188, 100, 0xffffff);
    // framebuffer_draw_char(framebuffer, '!', 199, 100, 0xffffff);
    // framebuffer_draw_char(framebuffer, '?', 210, 100, 0xffffff);

    hcf();
}