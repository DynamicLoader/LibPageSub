/* sim.ld - Simple Linker Script */

ENTRY(start)

SECTIONS
{
    PROVIDE(__arg_start = 0x1000);
    PROVIDE(__arg_end = 0x2000);
    

    . = 0x2000;

    /* Text section */
    .text  : {
        *(.text)
        *(.text.*)
    }

    /* Padding section */
    .pad : {
        . = . + 16;
    }

    . = ALIGN(0x1000);
    /* Data section */
    .data : {
        *(.rodata)
        *(.rodata.*)
        *(.data)
        *(.data.*)
        *(.sdata)
        *(.sdata.*)
    }

    /* BSS section */
    .bss : {
        *(.bss)
        *(.bss.*)
        *(.sbss)
        *(.sbss.*)
    }

    /* Stack section (We need to explict alloc and load)*/
    . = ALIGN(0x1000);
    .stack : {
        . = ALIGN(16);
        _stack = .;
        . += 0x4000;
        _stack_end = .;
    }

    /* End of sections */
    /DISCARD/ : {
        *(.note.*)
        *(.comment)
    }
}