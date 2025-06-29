/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __ASM_ARM_DIV64
#define __ASM_ARM_DIV64

#include <linux/types.h>
#include <asm/compiler.h>

/*
 * The semantics of __div64_32() are:
 *
 * uint32_t __div64_32(uint64_t *n, uint32_t base)
 * {
 * 	uint32_t remainder = *n % base;
 * 	*n = *n / base;
 * 	return remainder;
 * }
 *
 * In other words, a 64-bit dividend with a 32-bit divisor producing
 * a 64-bit result and a 32-bit remainder.  To accomplish this optimally
 * we override the generic version in lib/div64.c to call our __do_div64
 * assembly implementation with completely non standard calling convention
 * for arguments and results (beware).
 */

static inline uint32_t __div64_32(uint64_t *n, uint32_t base)
{
    uint64_t dividend = *n; // 被除数
    uint32_t remainder = 0;  // 余数
    uint64_t quotient = 0;   // 商

    // 如果被除数是 64 位的, 我们需要对其做除法处理
    if (base == 0) {
        // 除数为 0 时，处理错误。这里简单返回 0，实际情况应根据具体需求处理。
        return 0;
    }

    // 64 位除以 32 位，先将高32位和低32位拆开处理
    if (dividend >= (uint64_t)base) {
        quotient = dividend / base;
        remainder = dividend % base;
    }

    // 更新结果
    *n = quotient; // 商保存到 *n 中
    return (uint32_t)remainder; // 返回余数
}

// static inline uint32_t __div64_32(uint64_t *n, uint32_t base)
// {
// 	register unsigned int __base      asm("r4") = base;
// 	register unsigned long long __n   asm("r0") = *n;
// 	register unsigned long long __res asm("r2");
// 	unsigned int __rem;
// 	asm(	__asmeq("%0", "r0")
// 		__asmeq("%1", "r2")
// 		__asmeq("%2", "r4")
// 		"bl	__do_div64"
// 		: "+r" (__n), "=r" (__res)
// 		: "r" (__base)
// 		: "ip", "lr", "cc");
// 	__rem = __n >> 32;
// 	*n = __res;
// 	return __rem;
// }

#define __div64_32 __div64_32

#if !defined(CONFIG_AEABI)

/*
 * In OABI configurations, some uses of the do_div function
 * cause gcc to run out of registers. To work around that,
 * we can force the use of the out-of-line version for
 * configurations that build a OABI kernel.
 */
#define do_div(n, base) __div64_32(&(n), base)

#else

#ifdef CONFIG_CC_OPTIMIZE_FOR_PERFORMANCE
static __always_inline
#else
static inline
#endif
uint64_t __arch_xprod_64(uint64_t m, uint64_t n, bool bias)
{
	unsigned long long res;
	register unsigned int tmp asm("ip") = 0;
	bool no_ovf = __builtin_constant_p(m) &&
		      ((m >> 32) + (m & 0xffffffff) < 0x100000000);

	if (!bias) {
		asm (	"umull	%Q0, %R0, %Q1, %Q2\n\t"
			"mov	%Q0, #0"
			: "=&r" (res)
			: "r" (m), "r" (n)
			: "cc");
	} else if (no_ovf) {
		res = m;
		asm (	"umlal	%Q0, %R0, %Q1, %Q2\n\t"
			"mov	%Q0, #0"
			: "+&r" (res)
			: "r" (m), "r" (n)
			: "cc");
	} else {
		asm (	"umull	%Q0, %R0, %Q2, %Q3\n\t"
			"cmn	%Q0, %Q2\n\t"
			"adcs	%R0, %R0, %R2\n\t"
			"adc	%Q0, %1, #0"
			: "=&r" (res), "+&r" (tmp)
			: "r" (m), "r" (n)
			: "cc");
	}

	if (no_ovf) {
		asm (	"umlal	%R0, %Q0, %R1, %Q2\n\t"
			"umlal	%R0, %Q0, %Q1, %R2\n\t"
			"mov	%R0, #0\n\t"
			"umlal	%Q0, %R0, %R1, %R2"
			: "+&r" (res)
			: "r" (m), "r" (n)
			: "cc");
	} else {
		asm (	"umlal	%R0, %Q0, %R2, %Q3\n\t"
			"umlal	%R0, %1, %Q2, %R3\n\t"
			"mov	%R0, #0\n\t"
			"adds	%Q0, %1, %Q0\n\t"
			"adc	%R0, %R0, #0\n\t"
			"umlal	%Q0, %R0, %R2, %R3"
			: "+&r" (res), "+&r" (tmp)
			: "r" (m), "r" (n)
			: "cc");
	}

	return res;
}
#define __arch_xprod_64 __arch_xprod_64

#include <asm-generic/div64.h>

#endif

#endif
