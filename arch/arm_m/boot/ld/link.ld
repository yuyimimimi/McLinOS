# 0 "/mnt/c/Users/31740/Documents/codes/newcore-rp2350/arch/arm_m/boot/ld/main.ld"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "/mnt/c/Users/31740/Documents/codes/newcore-rp2350/arch/arm_m/boot/ld/main.ld"
# 1 "/mnt/c/Users/31740/Documents/codes/newcore-rp2350/include/generated/autoconf.h" 1
# 2 "/mnt/c/Users/31740/Documents/codes/newcore-rp2350/arch/arm_m/boot/ld/main.ld" 2

ENTRY(Reset_Handler)
_estack = ORIGIN(RAM) + LENGTH(RAM);

MEMORY
{
    RAM (xrw) : ORIGIN = 0x20008000 , LENGTH = 480 * 1k
    FLASH (rx) : ORIGIN = 0x10100000 , LENGTH = 3072 * 1k
}



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


# 1 "/mnt/c/Users/31740/Documents/codes/newcore-rp2350/arch/arm_m/boot/ld/sys.ld" 1
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

  .initramfs :
  {
    __export_file_start = .;
    KEEP(*(.init_ramfs_files))
    __export_file_end = .;
  } > FLASH
# 67 "/mnt/c/Users/31740/Documents/codes/newcore-rp2350/arch/arm_m/boot/ld/main.ld" 2


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
