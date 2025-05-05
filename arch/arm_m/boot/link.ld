# 0 "arch/arm_m/boot/ld/stm32f4.ld.in"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "arch/arm_m/boot/ld/stm32f4.ld.in"
# 1 "./include/generated/autoconf.h" 1
# 2 "arch/arm_m/boot/ld/stm32f4.ld.in" 2
# 1 "arch/arm_m/boot/ld/../start.ld.in" 1
ENTRY(Reset_Handler)
_estack = ORIGIN(RAM) + LENGTH(RAM);
# 3 "arch/arm_m/boot/ld/stm32f4.ld.in" 2

MEMORY
{
    RAM (xrw) : ORIGIN = 0x20000000 , LENGTH = 128 * 1k
    FLASH (rx) : ORIGIN = 0x08000000 , LENGTH = 1024 * 1k
}


# 1 "arch/arm_m/boot/ld/../kernel.ld.in" 1


SECTIONS
{

  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector))
    . = ALIGN(4);
  } >FLASH


  .text :
  {
    . = ALIGN(4);
    *(.text)
    *(.text*)
    *(.glue_7)
    *(.glue_7t)
    *(.eh_frame)

    KEEP (*(.init))
    KEEP (*(.fini))

    . = ALIGN(4);
    _etext = .;
  } >FLASH


  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)
    *(.rodata*)
    . = ALIGN(4);
  } >FLASH

  .ARM.extab : { *(.ARM.extab* .gnu.linkonce.armextab.*) } >FLASH
  .ARM : {
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
  } >FLASH

  .preinit_array :
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  } >FLASH
  .init_array :
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
  } >FLASH
  .fini_array :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array*))
    PROVIDE_HIDDEN (__fini_array_end = .);
  } >FLASH


  _sidata = LOADADDR(.data);


  .data :
  {
    . = ALIGN(4);
    _sdata = .;
    *(.data)
    *(.data*)

    . = ALIGN(4);
    _edata = .;
  } >RAM AT> FLASH


# 1 "arch/arm_m/boot/ld/../sys.ld.in" 1



  .init :
  {

      __start_initcallearly = .;
      KEEP(*(.initcallearly))
      __end_initcallearly = .;

      __start_pureinitcall = .;
      KEEP(*(.pureinitcall))
      __end_pureinitcall = .;

      __start_coreinitcall = .;
      KEEP(*(.coreinitcall))
      __end_coreinitcall = .;

      __start_core_initcall_sync = .;
      KEEP(*(.core_initcall_sync))
      __end_core_initcall_sync = .;

      __start_postcoreinitcall = .;
      KEEP(*(.postcoreinitcall))
      __end_postcoreinitcall = .;

      __start_postcore_initcall_sync = .;
      KEEP(*(.postcore_initcall_sync))
      __end_postcore_initcall_sync = .;

      __start_archinitcall = .;
      KEEP(*(.archinitcall))
      __end_archinitcall = .;

      __start_arch_initcall_sync = .;
      KEEP(*(.arch_initcall_sync))
      __end_arch_initcall_sync = .;

      __start_subsysinitcall = .;
      KEEP(*(.subsysinitcall))
      __end_subsysinitcall = .;

      __start_subsys_initcall_sync = .;
      KEEP(*(.subsys_initcall_sync))
      __end_subsys_initcall_sync = .;

      __start_fsinitcall = .;
      KEEP(*(.fsinitcall))
      __end_fsinitcall = .;

      __start_fs_initcall_sync = .;
      KEEP(*(.fs_initcall_sync))
      __end_fs_initcall_sync = .;

      __start_rootfsinitcall = .;
      KEEP(*(.rootfsinitcall))
      __end_rootfsinitcall = .;

      device_initcall_start__ = .;
      __start_deviceinitcall = .;
      KEEP(*(.deviceinitcall))
      __end_deviceinitcall = .;

      __start_device_initcall_sync = .;
      KEEP(*(.device_initcall_sync))
      __end_device_initcall_sync = .;

      __start_lateinitcall = .;
      KEEP(*(.lateinitcall))
      __end_lateinitcall = .;

      __start_late_initcall_sync = .;
      KEEP(*(.late_initcall_sync))
      __end_late_initcall_sync = .;

      . = ALIGN(4);
  } >FLASH

  . = ALIGN(8);
  .dtb :
  {
    . = ALIGN(8);
    __dtb_file_start_address = .;
     KEEP(*(.dtb))
    __dtb_file_end_address = .;
    . = ALIGN(8);
  } >FLASH

  . = ALIGN(4);
  .export_table :
  {
    __export_table_start = .;
    KEEP(*(.export_table))
    __export_table_end = .;
  } >FLASH
# 84 "arch/arm_m/boot/ld/../kernel.ld.in" 2


  . = ALIGN(4);
  .bss :
  {

    _sbss = .;
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)

    . = ALIGN(4);
    _ebss = .;
    __bss_end__ = _ebss;
  } >RAM


}
# 12 "arch/arm_m/boot/ld/stm32f4.ld.in" 2
