#ifndef _STDDEF_H
#define _STDDEF_H

#ifdef __cplusplus
extern "C" {
#endif

#define __SIZEOF_POINTER__ 4


#if __SIZEOF_POINTER__ == 8
typedef unsigned long long  size_t;
typedef long long           ptrdiff_t;
#elif __SIZEOF_POINTER__ == 4
typedef unsigned int        size_t;
typedef int                 ptrdiff_t;
#endif

#ifndef NULL
#define NULL ((void*)0)
#endif

#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((size_t)&((TYPE *)0)->MEMBER)
#endif

#ifdef __cplusplus
}
#endif

#endif // _STDDEF_H