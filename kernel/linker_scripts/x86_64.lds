/* x86_64 ELF64 output file. */
OUTPUT_FORMAT(elf64-x86-64)

/* Define the entrypoint. */
ENTRY(kmain)

/* Define the programm headers. */
PHDRS
{
    limine_requests PT_LOAD;
    text PT_LOAD;
    rodata PT_LOAD;
    data PT_LOAD;
}

SECTIONS
{
    /* Adress where the kernel is placed. -> beginning of the topmost 2GiB region */
    . = 0xffffffff80000000;

    /* Define a section for  the limine requests and assign it to its own PHDR. */
    .limine_requests : {
        KEEP(*(.limine_requests_start))
        KEEP(*(.limine_requests))
        KEEP(*(.limine_requests_end))
    } : limine_requests

    /* Move to the next memory page for .text. */
    . = ALIGN(CONSTANT(MAXPAGESIZE));

    .text : {
        *(.text .text.*)
    } : text

    /* Move to the next memory page for .rodata. */
    . = ALIGN(CONSTANT(MAXPAGESIZE));

    /* read-only data */
    .rodata : {
        *(.rodata .rodata.*)
    } : rodata
    
    /* Move to the next memory page for .data. */
    . = ALIGN(CONSTANT(MAXPAGESIZE));

    /* initialized global / static variables */
    .data : {
        *(.data .data.*)
    } : data

    /* Fills uninitialized global / static variables with Null. */
    .bss : {
        *(.bss .bss.*)
        *(COMMON)
    } : data

    /* Removes unnecessary metadata. */
    /DISCARD/ : {
        *(.eh_frame*)
        *(.note .note.*)
    }
}