#ifndef _STDINT_H
#define _STDINT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef signed char         int8_t;
typedef unsigned char       uint8_t;
typedef short               int16_t;
typedef unsigned short      uint16_t;
typedef int                 int32_t;
typedef unsigned int        uint32_t;

#if __SIZEOF_LONG_LONG__ >= 8
typedef long long           int64_t;
typedef unsigned long long  uint64_t;
#endif

typedef int8_t              int_least8_t;
typedef uint8_t             uint_least8_t;
typedef int16_t             int_least16_t;
typedef uint16_t            uint_least16_t;
typedef int32_t             int_least32_t;
typedef uint32_t            uint_least32_t;
#if __SIZEOF_LONG_LONG__ >= 8
typedef int64_t             int_least64_t;
typedef uint64_t            uint_least64_t;
#endif

typedef int8_t              int_fast8_t;
typedef uint8_t             uint_fast8_t;
typedef int16_t             int_fast16_t;
typedef uint16_t            uint_fast16_t;
typedef int32_t             int_fast32_t;
typedef uint32_t            uint_fast32_t;
#if __SIZEOF_LONG_LONG__ >= 8
typedef int64_t             int_fast64_t;
typedef uint64_t            uint_fast64_t;
#endif

#if __SIZEOF_POINTER__ == 8
typedef int64_t             intptr_t;
typedef uint64_t            uintptr_t;
#elif __SIZEOF_POINTER__ == 4
typedef int32_t             intptr_t;
typedef uint32_t            uintptr_t;
#endif

typedef long long           intmax_t;
typedef unsigned long long  uintmax_t;

#ifndef INT8_MIN
#define INT8_MIN            (-128)
#endif
#ifndef INT8_MAX
#define INT8_MAX            (127)
#endif
#ifndef UINT8_MAX
#define UINT8_MAX           (255U)
#endif

#ifndef INT16_MIN
#define INT16_MIN           (-32768)
#endif
#ifndef INT16_MAX
#define INT16_MAX           (32767)
#endif
#ifndef UINT16_MAX
#define UINT16_MAX          (65535U)
#endif

#ifndef INT32_MIN
#define INT32_MIN           (-2147483647L - 1L)
#endif
#ifndef INT32_MAX
#define INT32_MAX           (2147483647L)
#endif
#ifndef UINT32_MAX
#define UINT32_MAX          (4294967295UL)
#endif

#if __SIZEOF_LONG_LONG__ >= 8
#ifndef INT64_MIN
#define INT64_MIN           (-9223372036854775807LL - 1LL)
#endif
#ifndef INT64_MAX
#define INT64_MAX           (9223372036854775807LL)
#endif
#ifndef UINT64_MAX
#define UINT64_MAX          (18446744073709551615ULL)
#endif
#endif

#ifndef INT_LEAST8_MIN
#define INT_LEAST8_MIN      INT8_MIN
#endif
#ifndef INT_LEAST8_MAX
#define INT_LEAST8_MAX      INT8_MAX
#endif
#ifndef UINT_LEAST8_MAX
#define UINT_LEAST8_MAX     UINT8_MAX
#endif

#ifndef INT_LEAST16_MIN
#define INT_LEAST16_MIN     INT16_MIN
#endif
#ifndef INT_LEAST16_MAX
#define INT_LEAST16_MAX     INT16_MAX
#endif
#ifndef UINT_LEAST16_MAX
#define UINT_LEAST16_MAX    UINT16_MAX
#endif

#ifndef INT_LEAST32_MIN
#define INT_LEAST32_MIN     INT32_MIN
#endif
#ifndef INT_LEAST32_MAX
#define INT_LEAST32_MAX     INT32_MAX
#endif
#ifndef UINT_LEAST32_MAX
#define UINT_LEAST32_MAX    UINT32_MAX
#endif

#if __SIZEOF_LONG_LONG__ >= 8
#ifndef INT_LEAST64_MIN
#define INT_LEAST64_MIN     INT64_MIN
#endif
#ifndef INT_LEAST64_MAX
#define INT_LEAST64_MAX     INT64_MAX
#endif
#ifndef UINT_LEAST64_MAX
#define UINT_LEAST64_MAX    UINT64_MAX
#endif
#endif

#ifndef INT_FAST8_MIN
#define INT_FAST8_MIN       INT8_MIN
#endif
#ifndef INT_FAST8_MAX
#define INT_FAST8_MAX       INT8_MAX
#endif
#ifndef UINT_FAST8_MAX
#define UINT_FAST8_MAX      UINT8_MAX
#endif

#ifndef INT_FAST16_MIN
#define INT_FAST16_MIN      INT16_MIN
#endif
#ifndef INT_FAST16_MAX
#define INT_FAST16_MAX      INT16_MAX
#endif
#ifndef UINT_FAST16_MAX
#define UINT_FAST16_MAX     UINT16_MAX
#endif

#ifndef INT_FAST32_MIN
#define INT_FAST32_MIN      INT32_MIN
#endif
#ifndef INT_FAST32_MAX
#define INT_FAST32_MAX      INT32_MAX
#endif
#ifndef UINT_FAST32_MAX
#define UINT_FAST32_MAX     UINT32_MAX
#endif

#if __SIZEOF_LONG_LONG__ >= 8
#ifndef INT_FAST64_MIN
#define INT_FAST64_MIN      INT64_MIN
#endif
#ifndef INT_FAST64_MAX
#define INT_FAST64_MAX      INT64_MAX
#endif
#ifndef UINT_FAST64_MAX
#define UINT_FAST64_MAX     UINT64_MAX
#endif
#endif

#ifndef INTPTR_MIN
#define INTPTR_MIN          ((intptr_t)(1UL << (__SIZEOF_POINTER__ * 8 - 1)))
#endif
#ifndef INTPTR_MAX
#define INTPTR_MAX          ((intptr_t)~(1UL << (__SIZEOF_POINTER__ * 8 - 1)))
#endif
#ifndef UINTPTR_MAX
#define UINTPTR_MAX         (~(uintptr_t)0)
#endif

#ifndef INTMAX_MIN
#define INTMAX_MIN          (-9223372036854775807LL - 1LL)
#endif
#ifndef INTMAX_MAX
#define INTMAX_MAX          (9223372036854775807LL)
#endif
#ifndef UINTMAX_MAX
#define UINTMAX_MAX         (18446744073709551615ULL)
#endif

#define INT8_C(x)           (x)
#define INT16_C(x)          (x)
#define INT32_C(x)          (x ## L)
#define INT64_C(x)          (x ## LL)

#define UINT8_C(x)          (x ## U)
#define UINT16_C(x)         (x ## U)
#define UINT32_C(x)         (x ## UL)
#define UINT64_C(x)         (x ## ULL)

#define INTMAX_C(x)         (x ## LL)
#define UINTMAX_C(x)        (x ## ULL)

#ifdef __cplusplus
}
#endif

#endif // _STDINT_H

