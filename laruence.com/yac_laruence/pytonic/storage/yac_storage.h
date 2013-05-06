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

#ifndef YAC_STORAGE_H
#define yac_storage_h

/* 1 << 20  === 2^20 === 1024 * 1024 === 1M */
#define yac_storage_max_entry_len		( 1 << 20 )
#define yac_storage_max_key_len			( 48 )
#define yac_storage_factor				( 1.25 )
/* 255 === 2^8 - 1 === 0111 1111 === 低七位 */
#define yac_key_klen_mask				( 255 )
#define yac_key_vlen_bits				( 8 )
/* &意味着屏蔽也就是置0  |意味着打开也就是置1 */
#define yac_key_klen(k)					((k).len & yac_key_klen_mask)
/* 右移8位 意味着除以 2^8 === 256*/
#define yac_key_vlen(k)					((k).len >> yac_key_vlen_bits)
/*  */
#define yac_key_set_len(k, k1, v1)		((k).len = (v1 << yac_key_vlen_bits) | (k1 & yac_key_klen_mask))

#define USER_ALLOC		emalloc
#define USER_FREE		efree

typedef struct {
	unsigned long atime;
	unsigned int len;
	char data[1];
} yac_kv_val;

typedef struct {
	unsigned long h;
	unsigned long crc;
	unsigned int ttl;
	unsigned int len;
	unsigned int flag;
	unsigned int size;
	yac_kv_val *val;
	unsigned char key[yac_storage_max_key_len];
} yac_kv_key;

typedef struct {
	volatile unsigned int pos;
	unsigned int size;
	void *p;
} yac_shared_segment;

typedef struct {
	unsigned long k_msize;
	unsigned long v_msize;
	unsigned int  segment_num;
	unsigned int  segment_size;
	unsigned int  slots_num;
	unsigned int  slots_size;
	unsigned int  miss;
	unsigned int  fails;
	unsigned int  kicks;
	unsigned long hits;
} yac_storage_info;

typedef struct {
	yac_kv_key * slots;
	unsigned int slots_mask;
	unsigned int slots_num;
	unsigned int slots_size;
	unsigned int miss;
	unsigned int fails;
	unsigned int kicks;
	unsigned long hits;
	yac_shared_segment **segments;
	unsigned int segments_num;
	unsigned int segments_num_mask;
	yac_shared_segment first_seg;
} yac_storage_globals;

extern yac_storage_globals *yac_storage;

#define yac_sg(element)  (yac_storage->element)

int yac_storage_startup(unsigned long first_size, unsigned long size, char **err);
void yac_storage_shutdown(void);
int yac_storage_find(char *key, unsigned int len, char **data, unsigned int *size, unsigned int *flag, int *cas);
int yac_storage_update(char *key, unsigned int len, char *data, unsigned int size, unsigned int flag, int ttl, int add);
void yac_storage_delete(char *key, unsigned int len, int ttl);
void yac_storage_flush(void);
const char * yac_storage_shared_memory_name(void);
yac_storage_info * yac_storage_get_info(void);
void yac_storage_free_info(yac_storage_info *info);
#define yac_storage_exists(ht, key, len) yac_storage_find(ht, key, len, NULL)

#endif /*yac_storage_h*/



/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */

