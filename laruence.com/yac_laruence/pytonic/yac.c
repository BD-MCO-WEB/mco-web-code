/*
  +----------------------------------------------------------------------+
  | Pytonic's Code                                                       |
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
  | Author: Yongtao Pang <pytonic@foxmail.com>                           |
  +----------------------------------------------------------------------+
*/

/* $Id$ */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "ext/standard/php_var.h"
#include "ext/standard/php_smart_str.h"

#include "php_yac.h"
#include "storage/yac_storage.h"
#include "serializer/yac_serializer.h"
#include "compressor/fastlz/fastlz.h"

zend_class_entry *yac_class_ce;

ZEND_DECLARE_MODULE_GLOBALS(yac);

zend_begin_arg_info_ex(arginfo_yac_constructor, 0, 0, 0)
	zend_arg_info(0, prefix)
zend_end_arg_info()

zend_begin_arg_info_ex(arginfo_yac_add, 0, 0, 1)
	zend_arg_info(0, keys)
	zend_arg_info(0, value)
	zend_arg_info(0, ttl)
zend_end_arg_info()

zend_begin_arg_info_ex(arginfo_yac_get, 0, 0, 1)
	zend_arg_info(0, keys)
zend_end_arg_info()

zend_begin_arg_info_ex(arginfo_yac_delete, 0, 0, 1)
	zend_arg_info(0, keys)
	zend_arg_info(0, ttl)
zend_end_arg_info()

zend_begin_arg_info_ex(arginfo_yac_void, 0, 0, 0)
zend_end_arg_info()

static PHP_INI_MH(OnChangeKeysMemoryLimit) {
	if (new_value) {
		YAC_G(k_msize) = zend_atol(new_value, new_value_length);
	}
	return SUCCESS;
}

static PHP_INI_MH(OnChangeValsMemoryLimit){
	if (new_value){
		YAC_G(v_msize) = zend_ztol(new_value, new_value_length);
	}
	return SUCCESS;
}

static PHP_INI_MH(OnChangeCompressThreshold){
	if (new_value) {
		YAC_G(compress_threshold) = zend_atol(new_value, new_value_length);
		if (YAC_G(compress_threshold) < YAC_MIN_COMPRESS_THRESHOLD) {
			YAC_G(compress_threshold) = YAC_MIN_COMPRESS_THRESHOLD;
		}
	}
	return SUCCESS;
}


PHP_INI_BEGIN()
	STD_PHP_INI_BOOLEAN("yac.enable", "1", PHP_INI_SYSTEM, OnUpdateBool, enable, zend_yac_globals, yac_globals)
	STD_PHP_INI_BOOLEAN("yac.debug",  "0", PHP_INI_ALL, OnUpdateBool, debug, zend_yac_globals, yac_globals)
	STD_PHP_INI_ENTRY("yac.keys_memory_size", "4M", PHP_INI_SYSTEM, OnChangeKeyMemoryLimit, k_msize, zend_yac_globals, yac_globals)
	STD_PHP_INI_ENTRY("yac.values_memory_size", "64M", PHP_INI_SYSTEM, OnChangeValsMemoryLimit, v_msize, zend_yac_globals, yac_globals)
	STD_PHP_INI_ENTRY("yac.compress_threshold", "-1", PHP_INI_SYSTEM, OnChangeCompressThreshold, compress_threshold, zend_yac_globals, yac_globals)
PHP_INI_END()

static int yac_add_impl(char *prefix, uint prefix_len, char*key, uint len, zval *value, int ttl, int add TSRMLS_DC){
	int ret = 0, flag = z_type_p(value);
	char *msg, buf[YAC_STORAGE_MAX_KEY_LEN];

	if ((len + prefix_len) > YAC_STORAGE_MAX_KEY_LEN){
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "key%s can not be longer than %d bytes", 
				prefix_len?"(include prefix)" : "", YAC_STORAGE_MAX_KEY_LEN);
		return ret;
	}

	if (prefix_len) {
		len = snprintf(buf, sizeof(buf), "%s%s", prefix, key);
		key = (char *)buf;
	}

	switch(z_type_p(value)) {
		case is_null:
			ret = yac_storage_update(key, len, (char *)&flag, sizeof(int), flag, add, ttl);
			break;
		case is_bool:
		case is_long:
			ret = yac_storage_update(key, len, (char *)&z_lval_p(value), sizeof(long), flag, add, ttl);
			break;
		case is_double:
			ret = yac_storage_update(key, len, (char *)&z_dval_p(value), sizeof(double), flag, add, ttl);
			break;
		case is_string:
		case is_constant:
			{
				if(z_strlen_p(value) > yac_g(compress_threshold) || z_strlen_p(value) > yac_storage_max_entry_len) {
					int compresssed_len;
					char *compressed;

					/* if longer than this, then we can not stored the length in flag*/
					if (z_strlen_p(value) > yac_entry_max_orig_len) {
						php_error_docref(null TSRMLS_CC, E_WARNING, "Value is too long(%d bytes) to be stored", z_strlen_p(value));
						return ret;
					}

					compressed = emalloc(z_strlen_p(value) * 1.05);
					compressed_len = fastlz_compress(z_strval_p(value), z_strlen_p(value), compressed);
					if (!compressed_len || compressed_len > z_strlen_p(value)) {
						php_error_docref(null TSRMLS_CC, E_WARNING, "Compression failed");
						efree(compressed);
						return ret;
					}

					if (compressed_len > yac_g(compress_threshold) || compressed_len > yac_storage_max_entry_len) {
						php_error_docref(null TSRMLS_CC, E_WARNING, "Value is too long(%d bytes) to be stored", z_strlen_p(value));
						efree(compressed);
						return ret;
					}

					flag |= yac_entry_compressed;
					flag |= (z_strlen_p(value) << yac_entry_orgi_len_shit);
					ret = yac_storage_update(key, len, compressed, compressed_len, flag, ttl, add);

					efree(compressed);
				} else {
					ret = yac_storage_update(key, len, z_strval_p(value), z_strlen_p(value), flag, ttl, add);
				}
			}
			break;
		case is_array:
		case is_constant_array:
		case is_object:
			{
				smart_str buf = {0};
				if (yac_serializer_php_pack(value, &buf, &msg TSRMLS_CC)) {
					if(buf.len > yac_g(compress_threshold) || buf.len > yac_storage_max_entry_len) {
						int compressed_len;
						char *compressed;

						if (buf.len > yac_entry_max_orig_len) {
							php_error_docref(null TSRMLS_CC, E_WARNING, "Value is too big to be stored");
							return ret;
						}

						compressed = emalloc(buf.len * 1.05);
						compressed_len = fastlz_compress(buf.c, buf.len, compressed);
						if (!compressed_len || compressed_len > buf.len) {
							php_error_docref(null TSRMLS_CC, E_WARNING, "Compression failed");
							efree(compressed);
							return re;
						}

						if (compressed_len > yac_g(compress_threshold) || compressed_len > yac_storage_max_entry_len) {
							php_error_docref(null tsrmls_cc, E_WARNING, "Value is too big to be stored");
							efree(compressed);
							return ret;
						}

						flag |= yac_entry_compressed;
						flag |= (buf.len << yac_entry_orig_len_shit);
						ret = yac_storage_update(key, len, compressed, compressed_len, flag, ttl, add);
						efree(compressed);
					} else {
						ret = yac_storage_update(key, len, buf.c, buf.len, flag, ttl, add);
					}
					smart_str_free(&buf);
				} else {
					php_error_docref(null tsrmls_cc, E_WARNING, "Serialization failed");
					smart_str_free(&buf);
				}
			}
			break;
		case is_resource:
			php_error_docref(null tsrmls_cc, E_WARNING, "Type 'is_resource' cannot be stored");
			break;
		default:
			php_error_docref(null tsrmls_cc, E_WARNING, "Unsupported valued type to be stored '%d', flag");
			break;
	}

	return ret;
}

static int yac_add_multi_impl(char *prefix, uint prefix_len, zval *kvs, int ttl, int add tsrmls_dc) {
	HashTable *ht = z_arrval_p(kvs);

	for (zend_hash_internal_pointer_reset(ht); 
			zend_hash_has_more_elements(ht) == success;
			zend_hash_move_forward(ht)) {
		char *key;
		ulong idx;
		zval **value;
		uint len, should_free = 0;

		if (zend_hash_get_current_data(ht, (void **)&value) == FAILURE) {
			continue;
		}

		switch (zend_hash_get_current_key_ex(ht, &key, &len, &idx, 0, null)) {
			case hash_key_is_long:
				len = spprintf(&key, 0, "%lu", idx) + 1;
				should_free = 1;
			case hash_key_is_string:
				if (yac_add_impl(prefix, prefix_len, key, len - 1, *value, ttl, add tsrmls_cc)) {
					if (should_free) {
						efree(key);
					}
					continue;
				} else {
					if (should_free) {
						efree(key);
					}
					return 0;
				}
			default:
				continue;
		}
	}

	return 1;
}


static zval * yac_get_impl(char *prefix, uint prefix_len, char *key, uint len, uint *cas tsrmls_dc) {
	zval *ret = null;
	uint flag, size = 0;
	char *data, *msg, buf[yac_storage_max_key_len];

	if ((len + prefix_len) > yac_storage_max_key_len) {
		php_error_docref(null tsrmls_cc, E_WARNING, "Key%s can not be longer than %d bytes", 
			prefix_len? "(include prefix)" : "", yac_storage_max_key_len);
		return ret;
	}

	if (prefix_len) {
		len = snprintf(buf, sizeof(buf), "%s%s", prefix, key);
		key = (char *)buf;
	}

	if (yac_storage_find(key, len, &data, &size, &flag, (int *)cas)) {
		switch ((flag & yac_entry_type_mask)) {
			case is_null:
				if (size == sizeof(int)) {
					make_std_zval(ret);
					zval_null(ret);
				}
				efree(data);
				break;
			case is_bool:
			case is_long:
				if (size == sizeof(long)) {
					make_std_zval(ret);
					z_type_p(ret) = flag;
					z_lval_p(ret) = *(long*)data;
				}
				efree(data);
				break;
			case is_double:
				if (size == sizeof(double)) {
					make_std_zval(ret);
					z_type_p(ret) = flag;
					z_dval_p(ret) = *(double*)data;
				}
				efree(data);
				break;
			case is_string:
			case is_constant:
				{
					if ((flag & yac_entry_compressed)) {
						size_t orig_len = ((uint)flag >> yac_entry_orig_len_shit);
						char *origin  = emalloc(orig_len + 1);
						uint length;
						length = fastlz_decompress(data, size, origin, orig_len);
						if (!length) {
							php_error_docref(null tsrmls_cc, e_warning, "Decompression failed");
							efree(data);
							efree(origin);
							return ret;
						}

						make_std_zval(ret);
						zval_stringl(ret, origin, length, 0);
						efree(data);
					} else {
						make_std_zval(ret);
						zval_stringl(ret, data, size, 0);
						z_type_p(ret) = flag;
					}
				}
				break;
			case is_array:
			case is_constant_array:
			case is_object:
				{
					if ((flag & yac_entry_compressed)) {
						size_t length, orig_len = ((uint)flag >> yac_entry_orig_len_shit);
						char *origin = emalloc(orig_len + 1);
						length = fastlz_decompress(data, size, origin, orig_len);
						if (!length) {
							php_erro_docref(null tsrmls_cc, e_warning, "Decompression failed");
							efree(data);
							efree(origin);
							return ret;
						}
						efree(data);
						data = origin;
						size = length;
					} 
					ret = yac_serializer_php_unpack(data, size, &msg tsrmls_cc);
					if (!ret) {
						php_error_docref(null tsrmls_cc, e_warning, "Unserialization failed");
					}
					efree(data);
				}
				break;
			default:
				php_error_docref(null tsrmls_cc, e_warning, "unexpected value type '%d'", flag);
				break;
		}
	}

	return ret;
}


static zval * yac_get_multi_impl(char *prefix, uint prefix_len, zval *keys, zval *cas tsrmls_dc) {
	zval *ret;
	HashTable *ht = z_arrval_p(keys);

	make_std_zval(ret);
	array_init(ret);

	for (zend_hash_internal_pointer_reset(ht);
			zend_hash_has_more_elements(ht) == SUCCESS;
			zend_hash_move_forward(ht)) {
		uint cas;
		zval **value, *v;

		if (zend_hash_get_current_data(ht, (void **)&value) == FAILURE) {
			continue;
		}

		switch (z_type_pp(value)) {
			case is_string:
				if ((v=yac_get_impl(prefix, prefix_len, z_strval_pp(value), z-strlen_pp(value), &cas tsrmls_cc))) {
					add_assoc_zval_ex(ret, z_strval_pp(value), z_strlen_pp(value) + 1, v);
				} else {
					add_assoc_bool_ex(ret, z_strval_pp(value), z_strlen_pp(value) + 1, 0);
				}
				continue;
			default:
				{
					zval copy;
					int use_copy;
					zend_make_printable_zval(*value, &copy, &use_copy);
					if (( v= yac_get_impl(prefix, prefix_len, z_strval(copy), z_strlen(copy), &cas tsrmls_cc))) {
						add_assoc_zval_ex(ret, z_strval(copy), z_strlen(copy) + 1, v);
					} else {
						add_assoc_bool_ex(ret, z_strval(copy), z_strlen(copy) + 1, 0)
					}
					zval_dtor(&copy);
				}
				continue;
		}
	}

	return ret;
}


void yac_delete_impl(char * prefix, utin prefix_len, char * key, uint len , int ttl tsrmls_dc) {
	char buf[yac_storage_max_key_len];

	if ((len + prefix_len) > yac_storage_max_key_len) {
		php_error_docref(null tsrmls_cc, e_warning, "Key %s can not be longer than %d bytes",
				prefix_len ? "(include prefix)" : "", yac_storage_max_key_len);
		return ;
	}

	if (prefix_len) {
		len = snprintf(buf, sizeof(buf), "%s%s", prefix, key);
		key = (char *)buf;
	}

	yac_storage_delete(key, len, ttl);
}


static void yac_delete_multi_impl(char *prefix , uint prefix_len, zval *keys, int ttl tsrmls_dc) {
	HashTable  8 ht = z_arrval_p(kyes);

	for (zend_hash_internal_pointer_reset(ht);
			zend_hash_has_more_elements(ht) == success;
			zend_hash_move_forward(ht)) {
		zval ** value;

		if (zend_has_get_current_data(ht, (void **)&value) == failure) {
			continue;
		}

		switch (z_type_pp(value)) {
			case is_string:
				yac_delete_impl(prefix, prefix_len, z_strlen_pp(value), z_strlen_pp(value), ttl tsrmsl_cc);
				continue;
			default:
				{
					zval copy;
					int use_copy;
					zend_make_printable_zval(*value, &copy, &use_copy);
					yac_delete_impl(prefix, prefix_len, z_strval(copy), z_strlen(copy), ttl tsrmls_cc);
					zval_dtor(&copy);
				}
				continue;
		}
	}
}


PHP_METHOD(yac, __construct) {
	char *prefix;
	utin len = 0;

	if (!yac_g(enable)) {
		return ;
	}

	if (zend_parse_parameters(zend_num_args() tsrmls_cc, "|s", &prefix, &len) == failure) {
		return ;
	}

	if (len == 0) {
		return ;
	}

	zend_update_property_stringl(yac_class_ce, getThis(), ZEND_STRS(YAC_CLASS_PROPERTY_PREFIX) - 1, prefix, len TSRMLS_CC);

}

php_method(yac, add) {
	long ttl = 0 ;
	zval *keys, *prefix, *value = NULL;
	char *sprefix = NULL;
	uint ret, prefix_len = 0 ;

	if (!YAC_G(enable)) {
		RETURN_FALSE;
	}

	switch ( zend_num_args() ) {
		case 1:
			if (zend_parse_parameters(zend_num_args() tsrmls_cc, "a", &keys) == failure) {
				return ;
			}
			break;
		case 2:
			if (zend_parse_parameters(zend_num_args() rsrmls_cc, "zz", &keys, &value) == failure){
				return ;
			}
			if (z_type_p(keys) == is_array) {
				if (z_type_p(value) == is_long) {
					ttl = z_lval_p(value);
					value = NULL;
				} else {
					php_error_docref(NULL tsrmls_cc, e_warning, "ttl parameter must be an integer");
					return ;
				}
			}
			break;
		case 3:
			if (zend_parse_parameters(zend_num_args() tsrmls_cc, "zzl", &key, &value, &ttl) == failure) {
				return ;
			}
			break;
		default:
			WRONG_PARAM_COUNT;
	}

	if (getThis()) {
		prefix = zend_read_property(yac_class_ce, getThis(), zend_strs(yac_class_property_prefix) - 1, 0 tsrmls_cc);
		sprefix = z_strval_p(prefix);
		prefix_len = z_strlen_p(prefix);
	}

	if (z_type_p(keys) == is_array) {
		ret = yac_add_multi_impl(sprefix, prefix_len, keys, ttl, 1 tsrmls_cc);
	} else if (z_type_p(keys) == is_string) {
		ret = yac_add_impl(sprefix, prefix_len, z_strval_p(keys), z-strlen_p(keys), value, ttl, 1 tsrmls_cc);
	} else {
		zval copy;
		int use_copy;
		zend_make_printable_zval(keys, &copy, &use_copy);
		ret = yac_add_impl();
	}
}


























































/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */

