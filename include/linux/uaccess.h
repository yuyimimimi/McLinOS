#ifndef _UACCESS_H
#define _UACCESS_H

#include <linux/compiler.h>
#include <linux/types.h>
#include <linux/string.h>

static inline int copy_from_user(void *to, const void __user *from, size_t n) //用于裸机环境中的代替品
{
    memcpy(to, (void *)from, n);
    return 0;
}
static inline int copy_to_user(void __user *to, const void *from, size_t n) //用于裸机环境中的代替品
{
    return  memcpy((void *)to, from, n);
    return 0;
}

#define __get_user(x, ptr)		({ x = *(ptr); 0; })
#define __put_user(x, ptr)		({ *(ptr) = x; 0; })
#define __get_user_bad(x, ptr)	({ x = 0; 1; })
#define __put_user_bad(x, ptr)	({ 1; })

#define __copy_to_user(to, from, n)	copy_to_user(to, from, n)
#define __copy_from_user(to, from, n)	copy_from_user(to, from, n)

#define __copy_to_user_inatomic(to, from, n)	__copy_to_user(to, from, n)
#define __copy_from_user_inatomic(to, from, n)	__copy_from_user(to, from, n)

#define __copy_to_user_inatomic_nocache(to, from, n)	__copy_to_user(to, from, n)
#define __copy_from_user_inatomic_nocache(to, from, n)	__copy_from_user(to, from, n)

#define __copy_to_user_inatomic_cacheline(to, from, n)	__copy_to_user(to, from, n)
#define __copy_from_user_inatomic_cacheline(to, from, n)	__copy_from_user(to, from, n)

#define __copy_to_user_inatomic_nocacheline(to, from, n)	__copy_to_user(to, from, n)
#define __copy_from_user_inatomic_nocacheline(to, from, n)	__copy_from_user(to, from, n)

#define __copy_to_user_inatomic_cacheline_nocache(to, from, n)	__copy_to_user(to, from, n)
#define __copy_from_user_inatomic_cacheline_nocache(to, from, n)	__copy_from_user(to, from, n)





#endif /* _UACCESS_H */