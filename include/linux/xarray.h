#ifndef  __XARRAY_H__
#define  __XARRAY_H__

#include <linux/types.h>


struct xarray {
    /* private: The rest of the data structure is not to be used directly. */
       gfp_t		xa_flags;
       void __rcu *	xa_head;
   };




#endif // !