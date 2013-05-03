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

#ifndef PHP_YAC_H
#define PHP_YAC_H

extern zend_module_entry yac_module_entry;
//extern : 用在函数和变量的声明前面，表示此变量/函数是在别处定义的，要在此处引用。

#define phpext_yac_ptr &yac_module_entry

#ifdef PHP_WIN32
#define PHP_YAC_API __declspec(dllexport)
#else
#define PHP_YAC_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

#define YAC_VERSION "0.1.0"

#define YAC_CLASS_PROPERTY_PREFIX	"_prefix"
 YAC_ENTRY_COMPRESSED		0x0020
#define YAC_ENTRY_TYPE_MASK 		0x1f
#define YAC_ENTRY_ORIG_LEN_SHIT		6
#define YAC_ENTRY_MAX_ORIG_LEN		((1U << ((sizeof(int)*8 - YAC_ENTRY_ORIG_LEN_SHIT))) - 1)
#define YAC_MIN_COMPRESS_THREADHOLD	1024


ZEND_BEGIN_MODULE_GLOBALS(yac)
	zend_bool	enable;
	zend_bool	debug;
	size_t		k_msize;
	size_t		v_msize;
	ulong		compress_threadhold;
ZEND_END_MODULE_GLOBALS(yac)

PHP_MINIT_FUNCTION(yac);
PHP_MSHUTDOWN_FUNCTION(yac);
PHP_RINIT_FUNCTION(yac);
PHP_RSHUTDOWN_FUNCTION(yac);
PHP_MINFO_FUNCTION(yac);

#ifdef ZTS
#define YAC_G(v) TSRMG(yac_global_id, zend_yac_globals *, v)
#else
#define YAC_G(v) (yac_globals.v)
#endif

#endif /*PHP_YAC_H*/
