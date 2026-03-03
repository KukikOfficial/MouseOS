ENTRY(start)

SECTIONS {
    . = 1M; /* Адрес, с которого GRUB загружает ядро */

    .text : {
        *(.text)
    }

    .rodata : {
        *(.rodata*)
    }

    .data : {
        *(.data)
    }

    .bss : {
        *(.bss)
        *(COMMON) 
    }

    end = .; 
}
