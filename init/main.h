
static void do_initcallearly(void){
    for (initcall_t *fn = __start_initcallearly; fn != __end_initcallearly; fn++) 
        (*fn)();
}
static void do_pureinitcall(void){
    for (initcall_t *fn = __start_pureinitcall; fn != __end_pureinitcall; fn++) 
        (*fn)();
}
static void do_coreinitcall(void){
    for (initcall_t *fn = __start_coreinitcall; fn != __end_coreinitcall; fn++) 
        (*fn)();
}
static void do_core_initcall_sync(void){
    for (initcall_t *fn = __start_core_initcall_sync; fn != __end_core_initcall_sync; fn++) 
        (*fn)();
}
static void do_postcoreinitcall(void){
    for (initcall_t *fn = __start_postcoreinitcall; fn != __end_postcoreinitcall; fn++) 
        (*fn)();
}
static void do_postcore_initcall_sync(void){
    for (initcall_t *fn = __start_postcore_initcall_sync; fn != __end_postcore_initcall_sync; fn++) 
        (*fn)();
}
static void do_archinitcall(void){
    for (initcall_t *fn = __start_archinitcall; fn != __end_archinitcall; fn++) 
        (*fn)();
}
static void do_arch_initcall_sync(void){
    for (initcall_t *fn = __start_arch_initcall_sync; fn != __end_arch_initcall_sync; fn++) 
        (*fn)();
}
static void do_subsysinitcall(void){
    for (initcall_t *fn = __start_subsysinitcall; fn != __end_subsysinitcall; fn++)
        (*fn)();
}
static void do_subsys_initcall_sync(void){
    for (initcall_t *fn = __start_subsys_initcall_sync; fn != __end_subsys_initcall_sync; fn++) 
        (*fn)();
}
static void do_fsinitcall(void){
    for (initcall_t *fn = __start_fsinitcall; fn != __end_fsinitcall; fn++) 
        (*fn)();
}
static void do_fs_initcall_sync(void){
    for (initcall_t *fn = __start_fs_initcall_sync; fn != __end_fs_initcall_sync; fn++) 
        (*fn)();
}
static void rootfs_init(void){
    for (initcall_t *fn = __start_rootfsinitcall; fn != __end_rootfsinitcall; fn++) 
        (*fn)();
}
initcall_fn(deviceinitcall);
static void do_device_initcall_sync(void){
    for (initcall_t *fn = __start_device_initcall_sync; fn != __end_device_initcall_sync; fn++) 
        (*fn)();
}
static void do_lateinitcall(void){
    for (initcall_t *fn = __start_lateinitcall; fn != __end_lateinitcall; fn++){
        (*fn)();
    }
}

static void do_late_initcall_sync(void){
    for (initcall_t *fn = __start_late_initcall_sync; fn != __end_late_initcall_sync; fn++) 
        (*fn)();
}
