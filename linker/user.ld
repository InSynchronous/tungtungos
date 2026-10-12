ENTRY(_start)
SECTIONS
{
    /* userspace programs are loaded to this virtual address */
    . = 0x00400000;

    __program_start = .;

    .text :
    {
        *(.text*)
    }

    .rodata :
    {
        *(.rodata*)
    }

    .data :
    {
        *(.data*)
    }

    /* end of flat binary */
    __program_file_end = .;

    .bss (NOLOAD) :
    {
        *(COMMON)
        *(.bss*)
    }

    /* end of memory program uses*/
    __program_mem_end = .;

    /* useless info*/
    /DISCARD/ :
    {
        *(.comment*)
        *(.note*)
        *(.eh_frame*)
    }
}
