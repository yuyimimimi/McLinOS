// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * libfdt - Flat Device Tree manipulation
 * Copyright (C) 2006 David Gibson, IBM Corporation.
 */
#include <linux/libfdt_env.h>

#include <linux/fdt.h>
#include <linux/libfdt.h>


#include "libfdt_internal.h"

int fdt_check_full(const void *fdt, size_t bufsize)
{
	int err;
	int num_memrsv;
	int offset, nextoffset = 0;
	uint32_t tag;
	unsigned int depth = 0;
	const void *prop;
	const char *propname;
	bool expect_end = false;

	if (bufsize < FDT_V1_SIZE)
		return -FDT_ERR_TRUNCATED;
	if (bufsize < fdt_header_size(fdt))
		return -FDT_ERR_TRUNCATED;
	err = fdt_check_header(fdt);
	if (err != 0)
		return err;
	if (bufsize < fdt_totalsize(fdt))
		return -FDT_ERR_TRUNCATED;

	num_memrsv = fdt_num_mem_rsv(fdt);
	if (num_memrsv < 0)
		return num_memrsv;

	while (1) {
		offset = nextoffset;
		tag = fdt_next_tag(fdt, offset, &nextoffset);

		if (nextoffset < 0)
			return nextoffset;

		/* If we see two root nodes, something is wrong */
		if (expect_end && tag != FDT_END)
			return -FDT_ERR_BADSTRUCTURE;

		switch (tag) {
		case FDT_NOP:
			break;

		case FDT_END:
			if (depth != 0)
				return -FDT_ERR_BADSTRUCTURE;
			return 0;

		case FDT_BEGIN_NODE:
			depth++;
			if (depth > INT_MAX)
				return -FDT_ERR_BADSTRUCTURE;

			/* The root node must have an empty name */
			if (depth == 1) {
				const char *name;
				int len;

				name = fdt_get_name(fdt, offset, &len);
				if (!name)
					return len;

				if (*name || len)
					return -FDT_ERR_BADSTRUCTURE;
			}
			break;

		case FDT_END_NODE:
			if (depth == 0)
				return -FDT_ERR_BADSTRUCTURE;
			depth--;
			if (depth == 0)
				expect_end = true;
			break;

		case FDT_PROP:
			prop = fdt_getprop_by_offset(fdt, offset, &propname,
						     &err);
			if (!prop)
				return err;
			break;

		default:
			return -FDT_ERR_INTERNAL;
		}
	}
}
