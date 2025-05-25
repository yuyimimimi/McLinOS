ENTRY(reset_handler)

_Min_Heap_Size = 0x800;      /* 2KB */
_Min_Stack_Size = 0x1000; 

SECTIONS
{
    . = 0x00000000;

    .appmeta : {
    KEEP(*(.appmeta))
    }

    
    .got : ALIGN(4) {
    PROVIDE(_got_start = .);
    *(.got.plt)
    *(.got)
    PROVIDE(_got_end = .);
    }



    .text : ALIGN(4) {
    KEEP(*(.isr_vector))
    KEEP(*(.text))
    KEEP(*(.rodata))
    }

    .preinit_array :
    {
        PROVIDE_HIDDEN (__preinit_array_start = .);
        KEEP (*(.preinit_array*))
        PROVIDE_HIDDEN (__preinit_array_end = .);
    } 
    .init_array :
    {
        PROVIDE_HIDDEN (__init_array_start = .);
        KEEP (*(SORT(.init_array.*)))        
        KEEP (*(.init_array*))
        PROVIDE_HIDDEN (__init_array_end = .);
    } 
    .fini_array :
    {
        PROVIDE_HIDDEN (__fini_array_start = .);
        KEEP (*(SORT(.fini_array.*)))
        KEEP (*(.fini_array*))
        PROVIDE_HIDDEN (__fini_array_end = .);
    }
  
    .data : ALIGN(4) {
    PROVIDE(_data_start = .);
    KEEP(*(.data))
    PROVIDE(_data_end = .);
    }






    .bss (NOLOAD) : ALIGN(4) {
    _sbss = .;         /* define a global symbol at bss start */
    __bss_start__ = _sbss;
    PROVIDE(_bss_start = .);
    KEEP(*(.bss))
    *(COMMON)
    PROVIDE(_bss_end = .);
    _ebss = .;         /* define a global symbol at bss end */
    __bss_end__ = _ebss;
    }


    ._user_heap_stack :
    {
        . = ALIGN(8);
        PROVIDE ( end = . );
        PROVIDE ( _end = . );
        . = . + _Min_Heap_Size;
        . = . + _Min_Stack_Size;
        . = ALIGN(8);
    } 


    __end_program = .;  
    PROVIDE(_stack_top = 0xFFFFFFFF);
}