/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef _LINUX_EXPORT_H
#define _LINUX_EXPORT_H

#include <linux/compiler.h>
#include <linux/linkage.h>
#include <linux/stringify.h>
#include <linux/init.h>
#include <generated/autoconf.h>
/*
 * This comment block is used by fixdep. Please do not remove.
 *
 * When CONFIG_MODVERSIONS is changed from n to y, all source files having
 * EXPORT_SYMBOL variants must be re-compiled because genksyms is run as a
 * side effect of the *.o build rule.
 */



#ifdef CONFIG_64BIT
#define __EXPORT_SYMBOL_REF(sym)			\
	.balign 8				ASM_NL	\
	.quad sym
#else
#define __EXPORT_SYMBOL_REF(sym)			\
	.balign 4				ASM_NL	\
	.long sym
#endif

#define ___EXPORT_SYMBOL(sym, license, ns)		\
	.section ".export_symbol","a"		ASM_NL	\
	__export_symbol_##sym:			ASM_NL	\
		.asciz license			ASM_NL	\
		.asciz ns			ASM_NL	\
		__EXPORT_SYMBOL_REF(sym)	ASM_NL	\
	.previous

#if defined(__DISABLE_EXPORTS)

/*
 * Allow symbol exports to be disabled completely so that C code may
 * be reused in other execution contexts such as the UEFI stub or the
 * decompressor.
 */
#define __EXPORT_SYMBOL(sym, license, ns)

#elif defined(__GENKSYMS__)

#define __EXPORT_SYMBOL(sym, license, ns)	__GENKSYMS_EXPORT_SYMBOL(sym)

#elif defined(__ASSEMBLY__)

#define __EXPORT_SYMBOL(sym, license, ns) \
	___EXPORT_SYMBOL(sym, license, ns)
#else


struct export_node_struct{
	char *export_node_name;
	char *export_node_license;
	void *export_node_fn_address;
};
 
#ifdef CONFIG_USE_EXPORT_SYMBOL_TABLE
#define __EXPORT_SYMBOL(sym, license, ns)        \
	static struct export_node_struct             \
	__attribute__((__section__(".export_table")))\
	__attribute__((__used__))                    \
	sym##_export_struct = { 	                 \
		.export_node_name = #sym,			     \
		.export_node_license = license,			 \
		.export_node_fn_address = sym 			 \
	}
#else 
#define __EXPORT_SYMBOL(sym, license, ns)
#endif

#endif

#ifdef DEFAULT_SYMBOL_NAMESPACE
#define _EXPORT_SYMBOL(sym, license)	__EXPORT_SYMBOL(sym, license, __stringify(DEFAULT_SYMBOL_NAMESPACE))
#else
#define _EXPORT_SYMBOL(sym, license)	__EXPORT_SYMBOL(sym, license, "")
#endif

#define EXPORT_SYMBOL(sym)		_EXPORT_SYMBOL(sym, "")
#define EXPORT_SYMBOL_GPL(sym)		_EXPORT_SYMBOL(sym, "GPL")
#define EXPORT_SYMBOL_NS(sym, ns)	__EXPORT_SYMBOL(sym, "", __stringify(ns))
#define EXPORT_SYMBOL_NS_GPL(sym, ns)	__EXPORT_SYMBOL(sym, "GPL", __stringify(ns))

#endif /* _LINUX_EXPORT_H */
