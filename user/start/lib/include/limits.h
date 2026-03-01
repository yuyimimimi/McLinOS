#ifndef _LIMITS_H
#define _LIMITS_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CHAR_BIT
#define CHAR_BIT    __CHAR_BIT__
#endif

#ifndef SCHAR_MIN
#define SCHAR_MIN   (-__SCHAR_MAX__ - 1)
#endif
#ifndef SCHAR_MAX
#define SCHAR_MAX   __SCHAR_MAX__
#endif

#ifndef UCHAR_MAX
#define UCHAR_MAX   __UCHAR_MAX__
#endif

#ifndef SHRT_MIN
#define SHRT_MIN    (-__SHRT_MAX__ - 1)
#endif
#ifndef SHRT_MAX
#define SHRT_MAX    __SHRT_MAX__
#endif

#ifndef USHRT_MAX
#define USHRT_MAX   __USHRT_MAX__
#endif

#ifndef INT_MIN
#define INT_MIN     (-__INT_MAX__ - 1)
#endif
#ifndef INT_MAX
#define INT_MAX     __INT_MAX__
#endif

#ifndef UINT_MAX
#define UINT_MAX    __UINT_MAX__
#endif

#ifndef LONG_MIN
#define LONG_MIN    (-__LONG_MAX__ - 1L)
#endif
#ifndef LONG_MAX
#define LONG_MAX    __LONG_MAX__
#endif

#ifndef ULONG_MAX
#define ULONG_MAX   __ULONG_MAX__
#endif

#ifndef LLONG_MIN
#define LLONG_MIN   (-__LLONG_MAX__ - 1LL)
#endif
#ifndef LLONG_MAX
#define LLONG_MAX   __LLONG_MAX__
#endif

#ifndef ULLONG_MAX
#define ULLONG_MAX  __ULLONG_MAX__
#endif

#ifdef __cplusplus
}
#endif

#endif // _LIMITS_H
