ENTRY(reset_handler)

SECTIONS
{
    . = 0x00000000;

    .appmeta : {
    KEEP(*(.appmeta))
    }

    .text : ALIGN(4) {
    KEEP(*(.isr_vector))
    KEEP(*(.text))
    KEEP(*(.rodata))
    }

    .got : ALIGN(4) {
    PROVIDE(_got_start = .);
    *(.got.plt)
    *(.got)
    PROVIDE(_got_end = .);
    }

    .data : ALIGN(4) {
    PROVIDE(_data_start = .);
    KEEP(*(.data))
    PROVIDE(_data_end = .);
    }

    .bss (NOLOAD) : ALIGN(4) {
    PROVIDE(_bss_start = .);
    KEEP(*(.bss))
    *(COMMON)
    PROVIDE(_bss_end = .);
    }

    PROVIDE(_stack_top = 0x00010000);
}