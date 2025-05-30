
#ifndef  _INIT_H_
#define  _INIT_H_


#define __init __attribute__((__section__(".init.text")))
#define __weak __attribute__((weak))
#define __used __attribute__((__used__))
#define __cold __attribute__((cold))
#define __exit 

typedef int (*initcall_t)(void);
typedef void (*exitcall_t)(void);


extern initcall_t __start_initcallearly[];
extern initcall_t __end_initcallearly[];
extern initcall_t __start_pureinitcall[];
extern initcall_t __end_pureinitcall[];
extern initcall_t __start_coreinitcall[];
extern initcall_t __end_coreinitcall[];
extern initcall_t __start_core_initcall_sync[];
extern initcall_t __end_core_initcall_sync[];
extern initcall_t __start_postcoreinitcall[];
extern initcall_t __end_postcoreinitcall[];
extern initcall_t __start_postcore_initcall_sync[];
extern initcall_t __end_postcore_initcall_sync[];
extern initcall_t __start_archinitcall[];
extern initcall_t __end_archinitcall[];
extern initcall_t __start_arch_initcall_sync[];
extern initcall_t __end_arch_initcall_sync[];
extern initcall_t __start_subsysinitcall[];
extern initcall_t __end_subsysinitcall[];
extern initcall_t __start_subsys_initcall_sync[];
extern initcall_t __end_subsys_initcall_sync[];
extern initcall_t __start_fsinitcall[];
extern initcall_t __end_fsinitcall[];
extern initcall_t __start_fs_initcall_sync[];
extern initcall_t __end_fs_initcall_sync[];
extern initcall_t __start_rootfsinitcall[];
extern initcall_t __end_rootfsinitcall[];
extern initcall_t __start_deviceinitcall[];
extern initcall_t __end_deviceinitcall[];
extern initcall_t __start_device_initcall_sync[];
extern initcall_t __end_device_initcall_sync[];
extern initcall_t __start_lateinitcall[];
extern initcall_t __end_lateinitcall[];
extern initcall_t __start_late_initcall_sync[];
extern initcall_t __end_late_initcall_sync[];










#define __define_initcall(fn, prio) \
   static initcall_t __attribute__((__section__(prio))) \
    __used  _initcall_##fn  = fn              \
 
#define initcall_fn(name) \
static void do_##name(void) \
{                           \
    for (initcall_t *fn = __start_##name; fn != __end_##name; fn++)  \
        (*fn)();    \
}                  \




#define pure_initcall(fn)            __define_initcall(fn, ".pureinitcall")
#define core_initcall(fn)            __define_initcall(fn, ".coreinitcall")
#define core_initcall_sync(fn)       __define_initcall(fn, ".core_initcall_sync")
#define postcore_initcall(fn)        __define_initcall(fn, ".postcoreinitcall")
#define postcore_initcall_sync(fn)   __define_initcall(fn, ".postcore_initcall_sync")
#define arch_initcall(fn)            __define_initcall(fn, ".archinitcall")
#define arch_initcall_sync(fn)       __define_initcall(fn, ".arch_initcall_sync")
#define subsys_initcall(fn)          __define_initcall(fn, ".subsysinitcall")
#define subsys_initcall_sync(fn)     __define_initcall(fn, ".subsys_initcall_sync")
#define fs_initcall(fn)              __define_initcall(fn, ".fsinitcall")
#define fs_initcall_sync(fn)         __define_initcall(fn, ".fs_initcall_sync")
#define rootfs_initcall(fn)          __define_initcall(fn, ".rootfsinitcall")
#define device_initcall(fn)          __define_initcall(fn, ".deviceinitcall")
#define device_initcall_sync(fn)     __define_initcall(fn, ".device_initcall_sync")
#define late_initcall(fn)            __define_initcall(fn, ".lateinitcall")
#define late_initcall_sync(fn)       __define_initcall(fn, ".late_initcall_sync")


#define __initcall(fn)               device_initcall_sync(fn)
#define __exitcall(fn)		        

#define early_initcall(fn)           __define_initcall(fn, ".initcallearly")



#ifdef MODULE
extern struct module __this_module;
#define THIS_MODULE (&__this_module)
#else
#define THIS_MODULE (0)
#endif



#endif
