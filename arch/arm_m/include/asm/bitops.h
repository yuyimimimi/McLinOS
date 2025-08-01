/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _ALPHA_BITOPS_H
#define _ALPHA_BITOPS_H

#ifndef _LINUX_BITOPS_H
 #error only <linux/bitops.h> can be included directly
#endif

#include <asm/compiler.h>
#include <asm/barrier.h>

/*
 * Copyright 1994, Linus Torvalds.
 */

/*
 * These have to be done with inline assembly: that way the bit-setting
 * is guaranteed to be atomic. All bit operations return 0 if the bit
 * was cleared before the operation and != 0 if it was not.
 *
 * To get proper branch prediction for the main line, we must branch
 * forward to code at the end of this object's .text section, then
 * branch back to restart the operation.
 *
 * bit 0 is the LSB of addr; bit 64 is the LSB of (addr+1).
 */

static inline void
set_bit(unsigned long nr, volatile void * addr)
{
	unsigned long temp;
	int *m = ((int *) addr) + (nr >> 5);

	__asm__ __volatile__(
	"1:	ldl_l %0,%3\n"
	"	bis %0,%2,%0\n"
	"	stl_c %0,%1\n"
	"	beq %0,2f\n"
	".subsection 2\n"
	"2:	br 1b\n"
	".previous"
	:"=&r" (temp), "=m" (*m)
	:"Ir" (1UL << (nr & 31)), "m" (*m));
}

/*
 * WARNING: non atomic version.
 */
static __always_inline void
arch___set_bit(unsigned long nr, volatile unsigned long *addr)
{
	int *m = ((int *) addr) + (nr >> 5);

	*m |= 1 << (nr & 31);
}

static inline void
clear_bit(unsigned long nr, volatile void * addr)
{
	unsigned long temp;
	int *m = ((int *) addr) + (nr >> 5);

	__asm__ __volatile__(
	"1:	ldl_l %0,%3\n"
	"	bic %0,%2,%0\n"
	"	stl_c %0,%1\n"
	"	beq %0,2f\n"
	".subsection 2\n"
	"2:	br 1b\n"
	".previous"
	:"=&r" (temp), "=m" (*m)
	:"Ir" (1UL << (nr & 31)), "m" (*m));
}

static inline void
clear_bit_unlock(unsigned long nr, volatile void * addr)
{
	smp_mb();
	clear_bit(nr, addr);
}

/*
 * WARNING: non atomic version.
 */
static __always_inline void
arch___clear_bit(unsigned long nr, volatile unsigned long *addr)
{
	int *m = ((int *) addr) + (nr >> 5);

	*m &= ~(1 << (nr & 31));
}

static inline void
__clear_bit_unlock(unsigned long nr, volatile void * addr)
{
	smp_mb();
	arch___clear_bit(nr, (unsigned long *)addr);
}

static inline void
change_bit(unsigned long nr, volatile void * addr)
{
	volatile unsigned int *m = ((unsigned int *)addr) + (nr >> 5);
    unsigned int mask = 1U << (nr & 31);
    (void)__sync_fetch_and_xor(m, mask); // 原子异或
}


/*
 * fls = Find Last Set in word
 * @result: [1-32]
 * fls(1) = 1, fls(0x80000000) = 32, fls(0) = 0
 */
static inline int fls(unsigned int x)
{
    int n = 0;
    if (x == 0)
        return 0;
    while (x >>= 1)
        n++;
    return n + 1;
}


/*
 * WARNING: non atomic version.
 */
static __always_inline void
arch___change_bit(unsigned long nr, volatile unsigned long *addr)
{
	int *m = ((int *) addr) + (nr >> 5);

	*m ^= 1 << (nr & 31);
}

static inline int
test_and_set_bit(unsigned long nr, volatile void *addr)
{

    volatile unsigned long *p = (unsigned long *)addr + (nr >> 5);
    unsigned long mask = 1UL << (nr & 31);
    return __sync_fetch_and_or(p, mask) & mask ? 1 : 0;
}

static inline int
test_and_set_bit_lock(unsigned long nr, volatile void *addr)
{
    volatile unsigned long *p = (unsigned long *)addr + (nr >> 5);
    unsigned long mask = 1UL << (nr & 31);
    return __sync_fetch_and_or(p, mask) & mask ? 1 : 0;
}

/*
 * WARNING: non atomic version.
 */
static __always_inline bool
arch___test_and_set_bit(unsigned long nr, volatile unsigned long *addr)
{
	unsigned long mask = 1 << (nr & 0x1f);
	int *m = ((int *) addr) + (nr >> 5);
	int old = *m;

	*m = old | mask;
	return (old & mask) != 0;
}

static inline int
test_and_clear_bit(unsigned long nr, volatile void * addr)
{
	volatile unsigned long *p = (unsigned long *)addr + (nr >> 5);
    unsigned long mask = 1UL << (nr & 31);
    return (__sync_fetch_and_and(p, ~mask) & mask) ? 1 : 0;
}

/*
 * WARNING: non atomic version.
 */
static __always_inline bool
arch___test_and_clear_bit(unsigned long nr, volatile unsigned long *addr)
{
	unsigned long mask = 1 << (nr & 0x1f);
	int *m = ((int *) addr) + (nr >> 5);
	int old = *m;

	*m = old & ~mask;
	return (old & mask) != 0;
}

static inline int
test_and_change_bit(unsigned long nr, volatile void * addr)
{
    volatile unsigned long *p = (unsigned long *)addr + (nr >> 5);
    unsigned long mask = 1UL << (nr & 31);
    return (__sync_fetch_and_xor(p, mask) & mask) ? 1 : 0;
}

/*
 * WARNING: non atomic version.
 */
static __always_inline bool
arch___test_and_change_bit(unsigned long nr, volatile unsigned long *addr)
{
	unsigned long mask = 1 << (nr & 0x1f);
	int *m = ((int *) addr) + (nr >> 5);
	int old = *m;

	*m = old ^ mask;
	return (old & mask) != 0;
}

#define arch_test_bit generic_test_bit
#define arch_test_bit_acquire generic_test_bit_acquire

static inline bool xor_unlock_is_negative_byte(unsigned long mask,
		volatile unsigned long *p)
{
    unsigned long old = __sync_fetch_and_xor(p, mask);
    return (old & BIT(7)) != 0; 
}

/*
 * ffz = Find First Zero in word. Undefined if no zero exists,
 * so code should check against ~0UL first..
 *
 * Do a binary search on the bits.  Due to the nature of large
 * constants on the alpha, it is worthwhile to split the search.
 */
static inline unsigned long ffz_b(unsigned long x)
{
	unsigned long sum, x1, x2, x4;

	x = ~x & -~x;		/* set first 0 bit, clear others */
	x1 = x & 0xAA;
	x2 = x & 0xCC;
	x4 = x & 0xF0;
	sum = x2 ? 2 : 0;
	sum += (x4 != 0) * 4;
	sum += (x1 != 0);

	return sum;
}

static inline unsigned long ffz(unsigned long word)
{
#if defined(CONFIG_ALPHA_EV6) && defined(CONFIG_ALPHA_EV67)
	/* Whee.  EV67 can calculate it directly.  */
	return __kernel_cttz(~word);
#else
	unsigned long bits, qofs, bofs;

	bits = __kernel_cmpbge(word, ~0UL);
	qofs = ffz_b(bits);
	bits = __kernel_extbl(word, qofs);
	bofs = ffz_b(bits);

	return qofs*8 + bofs;
#endif
}

/*
 * __ffs = Find First set bit in word.  Undefined if no set bit exists.
 */
static inline unsigned long __ffs(unsigned long word)
{
#if defined(CONFIG_ALPHA_EV6) && defined(CONFIG_ALPHA_EV67)
	/* Whee.  EV67 can calculate it directly.  */
	return __kernel_cttz(word);
#else
	unsigned long bits, qofs, bofs;

	bits = __kernel_cmpbge(0, word);
	qofs = ffz_b(bits);
	bits = __kernel_extbl(word, qofs);
	bofs = ffz_b(~bits);

	return qofs*8 + bofs;
#endif
}

#ifdef __KERNEL__

/*
 * ffs: find first bit set. This is defined the same way as
 * the libc and compiler builtin ffs routines, therefore
 * differs in spirit from the above __ffs.
 */

static inline int ffs(int word)
{
	int result = __ffs(word) + 1;
	return word ? result : 0;
}

/*
 * fls: find last bit set.
 */
#if defined(CONFIG_ALPHA_EV6) && defined(CONFIG_ALPHA_EV67)
static inline int fls64(unsigned long word)
{
	return 64 - __kernel_ctlz(word);
}
#else
extern const unsigned char __flsm1_tab[256];

static inline int fls64(unsigned long x)
{
	unsigned long t, a, r;

	t = __kernel_cmpbge (x, 0x0101010101010101UL);
	a = __flsm1_tab[t];
	t = __kernel_extbl (x, a);
	r = a*8 + __flsm1_tab[t] + (x != 0);

	return r;
}
#endif

static inline unsigned long __fls(unsigned long x)
{
	return fls64(x) - 1;
}

static inline int fls(unsigned int x)
{
	return fls64(x);
}

/*
 * hweightN: returns the hamming weight (i.e. the number
 * of bits set) of a N-bit word
 */

#if defined(CONFIG_ALPHA_EV6) && defined(CONFIG_ALPHA_EV67)
/* Whee.  EV67 can calculate it directly.  */
static inline unsigned long __arch_hweight64(unsigned long w)
{
	return __kernel_ctpop(w);
}

static inline unsigned int __arch_hweight32(unsigned int w)
{
	return __arch_hweight64(w);
}

static inline unsigned int __arch_hweight16(unsigned int w)
{
	return __arch_hweight64(w & 0xffff);
}

static inline unsigned int __arch_hweight8(unsigned int w)
{
	return __arch_hweight64(w & 0xff);
}
#else
#include <asm-generic/bitops/arch_hweight.h>
#endif

#include <asm-generic/bitops/const_hweight.h>

#endif /* __KERNEL__ */

#ifdef __KERNEL__

/*
 * Every architecture must define this function. It's the fastest
 * way of searching a 100-bit bitmap.  It's guaranteed that at least
 * one of the 100 bits is cleared.
 */
static inline unsigned long
sched_find_first_bit(const unsigned long b[2])
{
	unsigned long b0, b1, ofs, tmp;

	b0 = b[0];
	b1 = b[1];
	ofs = (b0 ? 0 : 64);
	tmp = (b0 ? b0 : b1);

	return __ffs(tmp) + ofs;
}

#include <asm-generic/bitops/non-instrumented-non-atomic.h>

#include <asm-generic/bitops/le.h>

#include <asm-generic/bitops/ext2-atomic-setbit.h>

#endif /* __KERNEL__ */

#endif /* _ALPHA_BITOPS_H */
