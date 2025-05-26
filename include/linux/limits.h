/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_LIMITS_H
#define _LINUX_LIMITS_H

#include <uapi/linux/limits.h>
#include <linux/types.h>
#include <vdso/limits.h>

#define SIZE_MAX	(~(size_t)0)
#define SSIZE_MAX	((ssize_t)(SIZE_MAX >> 1))
#define PHYS_ADDR_MAX	(~(phys_addr_t)0)
 
#define RESOURCE_SIZE_MAX	((resource_size_t)~0)

#define U8_MAX		((u8)~0U)
#define S8_MAX		((s8)(U8_MAX >> 1))
#define S8_MIN		((s8)(-S8_MAX - 1))
#define U16_MAX		((u16)~0U)
#define S16_MAX		((s16)(U16_MAX >> 1))
#define S16_MIN		((s16)(-S16_MAX - 1))
#define U32_MAX		((u32)~0U)
#define U32_MIN		((u32)0)
#define S32_MAX		((s32)(U32_MAX >> 1))
#define S32_MIN		((s32)(-S32_MAX - 1))
#define U64_MAX		((u64)~0ULL)
#define S64_MAX		((s64)(U64_MAX >> 1))
#define S64_MIN		((s64)(-S64_MAX - 1))

#define INT8_MAX    127
#define INT16_MAX   32767
#define INT32_MAX   2147483647
#define INT64_MAX   9223372036854775807LL

#define INT8_MIN    (-128)
#define INT16_MIN   (-32768)
#define INT32_MIN   (-2147483647 - 1)
#define INT64_MIN   (-9223372036854775807LL - 1)

#define UINT8_MAX   255U
#define UINT16_MAX  65535U
#define UINT32_MAX  4294967295U
#define UINT64_MAX  18446744073709551615ULL

#endif /* _LINUX_LIMITS_H */
