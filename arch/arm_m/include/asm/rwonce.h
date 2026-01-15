/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
#ifndef _ALPHA_RWONCE_H
#define _ALPHA_RWONCE_H


static inline void sync(void)
{
	asm volatile("sync" : : : "memory");
}

static inline void eieio(void)
{
	asm volatile("eieio" : : : "memory");
}

static inline void barrier(void)
{
	asm volatile("" : : : "memory");
}
#define mb()    asm volatile("dsb sy" ::: "memory")
#define wmb()   asm volatile("dsb st" ::: "memory")
#define rmb()   asm volatile("dsb ld" ::: "memory")


#endif