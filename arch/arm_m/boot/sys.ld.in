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


