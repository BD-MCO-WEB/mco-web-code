/*
  +----------------------------------------------------------------------+
  | TAINT                                                                |
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "SAPI.h"
#include "zend_compile.h"
#include "zend_execute.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_taint.h"

ZEND_DECLARE_MODULE_GLOBALS(taint)

/* {{{ TAINT_ARG_INFO
 */
ZEND_BEGIN_ARG_INFO_EX(taint_arginfo, 0, 0, 1)
        ZEND_ARG_INFO(1, string)
        ZEND_ARG_INFO(1, ...) 
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(untaint_arginfo, 0, 0, 1)
        ZEND_ARG_INFO(1, string)
        ZEND_ARG_INFO(1, ...) 
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(is_tainted_arginfo, 0, 0, 1)
        ZEND_ARG_INFO(0, string)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ taint_functions[]
 */
zend_function_entry taint_functions[] = {
	PHP_FE(taint, taint_arginfo)
	PHP_FE(untaint, untaint_arginfo)
	PHP_FE(is_tainted, is_tainted_arginfo)
	{NULL, NULL, NULL}
};
/* }}} */

/* {{{ module depends 
 */
#if ZEND_MODULE_API_NO >= 20050922
zend_module_dep taint_deps[] = {
	ZEND_MOD_CONFLICTS("xdebug")
	{NULL, NULL, NULL}
};
#endif
/* }}} */

/* {{{ zend_module_entry taint_module_entry
 */
zend_module_entry taint_module_entry = {
#if ZEND_MODULE_API_NO >= 200050922
	STANDARD_MODULE_HEADER_EX, NULL, taint_deps,
#else
	STANDARD_MODULE_HEADER,
#endif
	"taint",
	taint_functions,
	PHP_MINIT(taint),
	PHP_MSHUTDOWN(taint),
	PHP_RINIT(taint),
	PHP_RSHUTDOWN(taint),
	PHP_MINFO(taint),
#if ZEND_MODULE_API_NO >= 20010901
	PHP_TAINT_VERSION, 
#endif
	PHP_MODULE_GLOBALS(taint),
	NULL,
	NULL,
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};
/* }}} */

static struct taint_overridden_fucs /* {{{ */ {
	php_func strval,
	php_func sprintf,
	php_func vsprintf,
	php_func explode,
	php_func implode,
	php_func trim,
	php_func ltrim,
	php_func rtrim,
	php_func strstr,
	php_func str_pad,
	php_func str_replace,
	php_func substr,
	php_func strtolower,
	php_func strtoupper,
} taint_origin_funcs;

#define TAINT_O_FUNC(m)	(taint_origin_funcs.m)
/* }}} */

static void php_taint_mark_strings(zval *symbol_table TSRMLS_DC) /* {{{ */ {
	zval **ppzval;
	HashTable * ht = Z_ARRVAL_P(symbol_table);
	HashPosition pos = {0};

	for (zend_hash_internal_pointer_reset_ex(ht, &pos);
			zend_hash_has_more_elements_ex(ht, &pos) == SUCCESS;
			zend_hash_move_forward_ex(ht, &pos)) {
		if (zend_hash_get_current_data_ex(ht, (void **)&ppzval, &pos) == FAILURE) {
			continue;
		}

		if (z_type_pp(ppzval) == IS_ARRAY) {
			php_taint_mark_strings(*ppzval TSRMLS_CC);
		} else if (IS_STRING == Z_TYPE_P(ppzval)) {
			Z_STRVAL_P(ppzval) = erealloc(Z_STRVAL_P(ppzval), Z_STRLEN_PP(ppzval) + 1 + PHP_TAINT_MAGIC_LENGTH);
			PHP_TAINT_MARK(*ppzval, PHP_TAINT_MAGIC_POSSIBLE);
		}
	}
}

/*}}}*/

static void taint_pzval_unlock_func (zval *z, taint_free_op *should_free, int unref) /* {{{ */ {
	if (!Z_DELREF_P(z)) {
		Z_SET_REFCOUNT_P(z, 1);
		Z_UNSET_ISREF_P(z);
		should_free->var = z;
	} else {
		should_free->var = 0;
		if (unref && Z_ISREF_P(z) && Z_REFCOUNT_P(z) == 1) {
			should_ref->is_ref = 1;
			Z_UNSET_ISREF_P(z);
		}
	}
}
/*}}}*/

static void taint_pzval_unlock_free_func(zval *z) /* {{{ */{
	if (!) {}
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */

