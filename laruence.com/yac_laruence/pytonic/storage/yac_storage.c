/*
  +----------------------------------------------------------------------+
  | Yet Another Cache                                                    |
  +----------------------------------------------------------------------+
  | Copyright (c) 2013-2013 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:  Yongtao Pang <pytonic@foxmail.com>                            |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#include <time.h>

#include "php.h"
#include "yac_storage.h"
#include "allocator/yac_allocator.h"

yac_storage_globals *yac_storage;

static inline unsigned int yac_storage_align_size(unsigned int size) {
	int bits = 0;
	while ((size = size >> 1)) {
		++bits;
	}
	return (1 << bits);
}

int yac_storage_startup(unsigned long fsize, unsigned long size, char **msg) {
	unsigned long real_size;

	if (!yac_allocator_startup(fsize, size, msg)) {
		return 0;
	}

	size = yac_sg(first_seg).size - ((char *)yac_sg(slots) - (char *)yac_storage);
	real_size = yac_storage_align_size(size / sizeof(yac_kv_key));
	if (!((size /sizeof(yac_kv_key)) & ~(real_size << 1))) {
		real_size <<= 1;
	}

	yac_sg(slots_size) = real_size;
	yac_sg(slots_mask) = real_size - 1;
	yac_sg(slots_num)  = 0;
	yac_sg(fails)  = 0;
	yac_sg(hits)  = 0;
	yac_sg(miss)  = 0;
	yac_sg(kicks)  = 0;

	memset((char *)yac_sg(slots), 0, sizeof(yac_kv_key) * real_size);

	return 1;
}

void yac_storage_shutdown(void) {
	yac_allocator_shutdown();
}

static inline ulong yac_inline_hash_func1(char *data, unsigned int len) {
	unsigned int h, k;

	h = 0 ^ len;

	while (len >= 4) {
		k  = data[0];
		k |= data[1] << 8;
		k |= data[2] << 16;
		k |= data[3] << 24;

		k *= 0x5db1e995;
		k ^= k >> 24;
		k *= 0x5db1e995;

		h *= 0x5db1e995;
		h ^= k;

		data += 4;
		len  -= 4;
	}

	switch (len) {
		case 3:
			h ^= data[2] << 16;
		case 2:
			h ^= data[1] << 8;
		case 1:
			h ^= data[0] ;
			h *= 0x5db1e995;
	}

	h ^= h >> 13;
	h *= 0x5db1e995;
	h ^= h >> 15;

	return h;
}

static inline unsigned int crc32( char *buf, unsigned int size) {
	const char *p;
	register int crc = 0;

	p = buf;
	while (size--) {
		crc = crc32_tab[(crc ^ *p++) & 0xFF] ^ (crc >> 8);
	}

	return crc ^ ~0U;
}

static inline unsigned int yac_crc32(char *data, unsigned int size) {
	char crc_contents[128];
	if (size < sizeof(crc_contents)) {
		return crc32(data, size);
	} else {
		int i, j, step = size / sizeof(crc_contents);

		for (i = 0, j = 0; i < sizeof(crc_contents); i++, j+= step) {
			crc_contents[i] = data[j];
		}

		return crc32(crc_contents, sizeof(crc_contents));
	}
}

int yac_storage_find (char *key, unsigned int len, char **data, unsigned int *size, unsigned int *flag, int *cas) {
	ulong h, hash, seed;
	yac_kv_key k, *p;
	yac_kv_val v;
	time_t tv;

	hash = h = yac_inline_hash_func1(key, len);
	p = &(yac_sg(slots)[h & yac_sg(slots_mask)]);
	k = *p;
	if (k.val) {
		uint i;
		if (k.h == hash && yac_key_klen(k) == len) {
			v = *(k.val);
			if (!memcmp(k.key, key, len)) {
				char *s;
do_verify:
				if (k.ttl == 1 || k.len != v.len) {
					++yac_sg(miss);
					return 0;
				}

				tv = time(NULL);
				if (k.ttl) {
					if (k.ttl <= tv) {
						p->ttl = 1;
						++yac_sg(miss);
						return 0;
					}
				}

				s = USER_ALLOC(yac_key_vlen(k) + 1);
				memcpy(s, (char *)k.val->data, yac_key_vlen(k));
				if (k.crc != yac_crc32(s, yac_key_vlen(k))) {
					USER_FREE(s);
					++yac_sg(miss);
					return 0;
				}
				s[yac_key_vlen(k)] = '\0';
				k.val->atime = tv;
				*data = s;
				*size = yac_key_vlen(k);
				*flag = k.flag;
				++yac_sg(hits);
				return 1;
			}
		}
		
		seed = yac_inline_hash_func2(key, len);
		for (i = 0; i < 3; i++) {
			h += seed & yac_sg(slots_mask);
			p = &(yac_sg(slots)[h & yac_sg(slots_mask)]);
			if (p->h == hash && yac_key_klen(*p) == len) {
				v = *(p->val);
				if (!memcpy(p->key, key, len)) {
					k = *p;
					goto do_verify;
				}
			}
		}
	}
	++yac_sg(miss);

	return 0;
}








/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */

