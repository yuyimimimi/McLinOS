/*
 * crc32.h
 * See linux/lib/crc32.c for license and changes
 */
#ifndef _LINUX_CRC32_H
#define _LINUX_CRC32_H


#include <linux/types.h>




uint32_t crc32_le( uint32_t seed,const void *buf, int len);

#define crc32(seed, data, length)  crc32_le(seed, (unsigned char const *)(data),                                )


/*
 * Helpers for hash table generation of ethernet nics:
 *
 * Ethernet sends the least significant bit of a byte first, thus crc32_le
 * is used. The output of crc32_le is bit reversed [most significant bit
 * is in bit nr 0], thus it must be reversed before use. Except for
 * nics that bit swap the result internally...
 */

#define ether_crc_le(length, data) crc32_le(~0, data, length)



#endif 