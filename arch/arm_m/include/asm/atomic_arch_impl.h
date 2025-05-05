#ifndef ATOMIC_ARCH_IMPL_H
#define ATOMIC_ARCH_IMPL_H

#include <linux/types.h>
#include <linux/compiler.h>

#define arch_xchg(ptr, val)		__arch_xchg(ptr, val)
#define arch_xchg_relaxed(ptr, val)	__arch_xchg_relaxed(ptr, val)
#define arch_xchg_acquire(ptr, val)	__arch_xchg_acquire(ptr, val)
#define arch_xchg_release(ptr, val)	__arch_xchg_release(ptr, val)

#define arch_cmpxchg(ptr, old_val, new_val)	__arch_cmpxchg(ptr, old_val, new_val)
#define arch_cmpxchg_relaxed(ptr, old_val, new_val)	__arch_cmpxchg_relaxed(ptr, old_val, new_val)
#define arch_cmpxchg_acquire(ptr, old_val, new_val)	__arch_cmpxchg_acquire(ptr, old_val, new_val)
#define arch_cmpxchg_release(ptr, old_val, new_val)	__arch_cmpxchg_release(ptr, old_val, new_val)

static inline u32 __arch_xchg(volatile u32 *ptr, u32 new_val)
{
    u32 old_val;
    unsigned long tmp;
    u32 r_ptr; // Introduce a register variable for the pointer

    r_ptr = (u32)ptr; // Cast the pointer to u32 and store in a register variable

    __asm__ __volatile__("@ atomic_xchg\n"
"1: ldrex   %0, [%2]\n"
"   strex   %1, %3, [%2]\n"
"   teq %1, #0\n"
"   bne 1b"
    : "=&r" (old_val), "=&r" (tmp), "+r" (r_ptr) // Use "+r" to ensure r_ptr is in a register
    : "r" (new_val)
    : "cc", "memory"); // Add "memory" clobber as we are accessing memory
    (void)r_ptr; // Prevent unused variable warning

    return old_val;
}

static inline u32 __arch_xchg_relaxed(volatile u32 *ptr, u32 new_val)
{
    return arch_xchg(ptr, new_val);
}

static inline u32 __arch_xchg_acquire(volatile u32 *ptr, u32 new_val)
{
    /* Add memory barrier if needed for acquire semantics */
    return arch_xchg(ptr, new_val);
}

static inline u32 __arch_xchg_release(volatile u32 *ptr, u32 new_val)
{
    u32 old_val;
    unsigned long tmp;

    __asm__ __volatile__ (
        "1: ldrex %0, [%2]\n"        // 加载内存中的值到 old_val
        "strex %1, %3, [%2]\n"      // 尝试将 new_val 写入内存
        "teq %1, #0\n"              // 检查 strex 是否成功
        "bne 1b\n"                  // 如果失败，重试
        "dmb ish"                   // 释放语义：插入内存屏障
        : "=&r" (old_val), "=&r" (tmp)
        : "r" (ptr), "r" (new_val)
        : "cc", "memory"
    );
    return old_val;
}

/*
 * Atomic Compare and Exchange (32-bit)
 */

static inline u32 __arch_cmpxchg(volatile u32 *ptr, u32 old, u32 new)
{
    int prev;
    unsigned long tmp;

    __asm__ __volatile__ (
        "1: ldrex %0, [%2]\n"        // 加载内存中的值到 prev
        "cmp %0, %3\n"              // 比较 prev 和 old
        "bne 2f\n"                 // 如果不相等，跳转到 2
        "strex %1, %4, [%2]\n"     // 尝试将 new 写入内存
        "teq %1, #0\n"             // 检查 strex 是否成功
        "bne 1b\n"                 // 如果失败，重试
        "2:"
        : "=&r" (prev), "=&r" (tmp)
        : "r" (ptr), "r" (old), "r" (new)
        : "cc", "memory"
    );
    return prev;
}


static inline u32 __arch_cmpxchg_relaxed(volatile u32 *ptr, u32 old_val, u32 new_val)
{
    return arch_cmpxchg(ptr, old_val, new_val);
}

static inline u32 __arch_cmpxchg_acquire(volatile u32 *ptr, u32 old_val, u32 new_val)
{
    /* Add memory barrier if needed for acquire semantics */
    return arch_cmpxchg(ptr, old_val, new_val);
}

static inline u32 __arch_cmpxchg_release(volatile u32 *ptr, u32 old_val, u32 new_val)
{
    u32 current_val;
    unsigned long tmp;
    u32 result;

//     __asm__ __volatile__("@ atomic_cmpxchg_release\n"
// "1: ldrex   %0, [%4]\n"
// "   teq %0, %1\n"
// "   it eq\n"
// "   strexeq %2, %3, [%4]\n"
// "   teq %2, #0\n"
// "   bne 1b"
//     : "=&r" (current_val), "+r" (old_val), "=&r" (result), "=&r" (tmp), "+Qo" (ptr)
//     : "r" (new_val)
//     : "cc", "memory"); /* Memory clobber for release */

    return current_val;
}

/*
 * Atomic Compare and Exchange (64-bit)
 */
#ifndef CONFIG_GENERIC_ATOMIC64
#ifdef CONFIG_ARCH_64BIT

/* 64-bit atomic compare and exchange (for 64-bit ARM) */
static inline u64 __arch_cmpxchg64(volatile u64 *ptr, u64 old_val, u64 new_val)
{
    u64 current_val;
    unsigned long tmp;
    u32 result; /* strexd returns 0 on success */

    __asm__ __volatile__("@ atomic_cmpxchg64\n"
"1: ldrexd   %0, %H0, [%4]\n"
"   cmp %0, %1\n"
"   cmpeq %H0, %H1\n"
"   strexdeq %2, %3, %H3, [%4]\n"
"   teq %2, #0\n"
"   bne 1b"
    : "=&r" (current_val), "+r" (old_val), "=&r" (result), "=&r" (tmp), "+Qo" (ptr)
    : "r" (new_val)
    : "cc");

    return current_val;
}

static inline u64 __arch_cmpxchg64_relaxed(volatile u64 *ptr, u64 old_val, u64 new_val)
{
    return arch_cmpxchg64(ptr, old_val, new_val);
}

static inline u64 __arch_cmpxchg64_acquire(volatile u64 *ptr, u64 old_val, u64 new_val)
{
    /* Add memory barrier if needed for acquire semantics */
    return arch_cmpxchg64(ptr, old_val, new_val);
}

static inline u64 __arch_cmpxchg64_release(volatile u64 *ptr, u64 old_val, u64 new_val)
{
    u64 current_val;
    unsigned long tmp;
    u32 result; /* strexd returns 0 on success */

    __asm__ __volatile__("@ atomic_cmpxchg64_release\n"
"1: ldrexd   %0, %H0, [%4]\n"
"   cmp %0, %1\n"
"   cmpeq %H0, %H1\n"
"   strexdeq %2, %3, %H3, [%4]\n"
"   teq %2, #0\n"
"   bne 1b"
    : "=&r" (current_val), "+r" (old_val), "=&r" (result), "=&r" (tmp), "+Qo" (ptr)
    : "r" (new_val)
    : "cc", "memory"); /* Memory clobber for release */

    return current_val;
}

#else /* If 64-bit operations are not supported, add empty functions */

static inline u64 __arch_cmpxchg64(volatile u64 *ptr, u64 old_val, u64 new_val)
{
    return old_val;  /* No-op, 64-bit operations not supported on this architecture */
}

static inline u64 __arch_cmpxchg64_relaxed(volatile u64 *ptr, u64 old_val, u64 new_val)
{
    return old_val;  /* No-op */
}

static inline u64 __arch_cmpxchg64_acquire(volatile u64 *ptr, u64 old_val, u64 new_val)
{
    return old_val;  /* No-op */
}

static inline u64 __arch_cmpxchg64_release(volatile u64 *ptr, u64 old_val, u64 new_val)
{
    return old_val;  /* No-op */
}

#endif /* CONFIG_ARCH_64BIT */

#endif /* !CONFIG_GENERIC_ATOMIC64 */

#endif /* __KERNEL__ */
