/*
  +----------------------------------------------------------------------+
  | PHP Taint                                                            |
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
#ifndef php_taint_h
#define php_taint_h

extern zend_module_entry taint_module_entry;
#define phpext_taint_ptr &taint_module_entry

#ifdef php_win32
#define php_taint_api __declspec(dllexport)
#else
#define php_taint_api
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

#define php_taint_version "1.3.0-dev"
#define php_taint_magic_length sizeof(unsigned)
#define php_taint_magic_none   0x00000000
#define php_taint_magic_possible   0x6A8FCE84
#define php_taint_magic_untaint    0x2C5E7F2D


#if (php_major_version == 5) && (php_minor_version < 4)
# define taint_op1_type(n)					((n)->op1.op_type)
# define taint_op2_type(n)					((n)->op2.op_type)
# define taint_op1_node_ptr(n)				(&(n)->op1)
# define taint_op2_node_ptr(n)				(&(n)->op2)
# define taint_op1_var(n)					((n)->op1.u.var)
# define taint_op2_var(n)					((n)->op2.u.var)
# define taint_result_var(n)				((n)->result.u.var)
# define taint_op1_constant_ptr(n)			(&(n)->op1.u.constant)
# define taint_op2_constant_ptr(n)			(&(n)->op2.u.constant)
# define taint_get_zval_ptr_cv_2nd_arg(t)	(execute_data->Ts)
# define taint_return_value_used(n)			(!((&(n)->result)->u.EA.type & ext_type_used))
# define taint_op_linenum(n)				((n).u.opline_num)
# define taint_ai_set_ptr(ai, val)			(ai).ptr = (val); (ai).ptr_ptr = &((ai).ptr);

#else
# define taint_op1_type(n)					((n)->op1_type)
# define taint_op2_type(n)					((n)->op2_type)
# define taint_op1_node_ptr(n)				((n)->op1.var)
# define taint_op2_node_ptr(n)				((n)->op2.var)
# define taint_op1_var(n)					((n)->op1.var)
# define taint_op2_var(n)					((n)->op2.var)
# define taint_result_var(n)				((n)->result.var)
# define taint_op1_constant_ptr(n)			((n)->op1.zv)
# define taint_op2_constant_ptr(n)			((n)->op2.zv)
# define taint_get_zval_ptr_cv_2nd_arg(t)	(t)
# define taint_return_value_used(n)			(!((n)->result_type & ext_type_used))
# define taint_op_linenum(n)				((n).opline_num)
# define taint_ai_set_ptr(ai, val)			do{
		temp_variable *__t = (t);
		__t->var.ptr = (val);
		__t->ptr_ptr = &__t->var.ptr;
	} while (0)
#endif


#if (php_major_version == 5) && (php_minor_version < 3)
# define taint_arg_push(v)		zend_ptr_stack_push(&EG(argument_stack), v TSRMLS_CC)
#else 
# define taint_arg_push(v)		zend_vm_stack_push(v TSRMLS_CC)
#endif

#ifndef z_set_isref_pp
#define z_set_isref_pp(n)		((*n)->is_ref = 1)
#endif

#ifndef z_unset_isref_pp
#define z_unset_isref_pp(n)		((*n)->is_ref = 0)
#endif

#ifndef z_refcount_pp
#define z_refcount_pp(n)		((*n)->refcount)
#endif

#ifndef init_pzval_copy
#define init_pzval_copy(z, v) \
	(z)->value = (v)->value; \
	z_type_p(z)= z_type_p(v); \
	(z)->refcount = 1; \
	(z)->is_ref = 0;
#endif

#ifndef make_real_zval_ptr
#define make_real_zval_ptr(val) \
	do {	\
		zval *_tmp; \
		ALLOC_ZVAL(_tmp); \
		INIT_ZVAL_COPY(_tmp, (val)); \
		(val) = _tmp; \
	} while (0)
#endif

#define taint_t(offset)				(*(temp_variable *)((char *)execute_data->Ts + offset))
#define taint_ts(offset)			(*(temp_variable *)((char *)Ts + offset))
#define taint_cv(i)					(EG(current_execute_data)->CVs[i])
#define taint_pzval_lock(z, f)		taint_pzval_lock_func(z, f)
#define taint_pzval_unlock(z, f)	taint_pzval_unlock_func(z, f, 1)
#define taint_pzval_unlock_free(z)	taint_pzval_unlock_free_func(z)
#define taint_cv_of(i)				(EG(current_execute_data)->CVs[i])
#define taint_cv_def_of(i)			(EG(active_op_array)->vars[i])
#define taint_tmp_free(z)			(zval *)(((zend_uintptr_t)(z)) | 1L)
#define taint_ai_use_ptr(ai)	\
	if ((ai).ptr_ptr) { \
		(ai).ptr	=*((ai).ptr_ptr); \
		(ai).ptr_ptr=&((ai).ptr); \
	} else {	\
		(ai).ptr	= NULL;\
	}
#define taint_free_op(should_free)	\
	if (should_free.var) {   \
		if ((zend_uintptr_t)should_free.var & 1L) {  \
			zval_dtor((zval*)((zend_uintptr_t)should_free.var & ~1L)); \
		} else {	\
			zval_ptr_dtor(&should_free.var); \
		}\
	}

#define taint_free_op_var_ptr(should_free) \
	if (should_free.var) {  \
		zval_ptr_dtor(&should_free.var); \
	}

#define php_taint_mark(zv, mark)	*((unsigned *)(z_strval_p(zv) + z_strlen_p(zv) + 1)) = (mark)
#define php_taint_possible(zv)		(*(unsigned *)(z_strval_p(zv) + z_strlen_p(zv) + 1) == php_taint_magic_possible)
#define php_taint_untaint(zv)		(*(unsigned *)(z_strval_p(zv) + z_strlen_p(zv) + 1) == php_taint_magic_untaint)


#if (php_major_version == 5) && (php_minor_version < 3)
#define z_addref_p			zval_addref
#define z_refcount_p		zval_refcount
#define z_delref_p			zval_delref
#define z_set_refcount_p(pz, rc)		(pz)->refcount=rc
#define z_unset_isref_p(pz)		(pz)->is_ref = 0
#define z_isref_p(pz)			(pz)->is_ref
#endif

typedef struct _taint_free_op {
	zval *var;
	int  is_ref;
	int  type;
} taint_free_op;

PHP_MINIT_FUNCTION(taint);
PHP_MSHUTDOWN_FUNCTION(taint);
PHP_RINIT_FUNCTION(taint);
PHP_RSHUTOWN_FUNCTION(taint);
PHP_MINFO_FUNCTION(taint);

PHP_FUNCTION(taint);
PHP_FUNCTION(untaint);
PHP_FUNCTION(is_tainted);

PHP_FUNCTION(taint_strval);
PHP_FUNCTION(taint_sprintf);
PHP_FUNCTION(taint_vsprintf);
PHP_FUNCTION(taint_explode);
PHP_FUNCTION(taint_implode);
PHP_FUNCTION(taint_trim);
PHP_FUNCTION(taint_rtrim);
PHP_FUNCTION(taint_ltrim);
PHP_FUNCTION(taint_strstr);
PHP_FUNCTION(taint_substr);
PHP_FUNCTION(taint_str_replace);
PHP_FUNCTION(taint_str_pad);
PHP_FUNCTION(taint_strtolower);
PHP_FUNCTION(taint_strtoupper);

typedef void (*php_func)(INTERNAL_FUNCTION_PARAMETERS);

ZEND_BEGIN_MODULE_GLOBALS(taint)
	zend_bool	enable;
	int			error_level;
ZEND_END_MODULE_GLOBALS(taint)

#ifdef ZTS
#define TAINT_G(v)	TSRMG(taint_globals_id, zend_taint_globals *, v)
#else
#define TAINT_G(v)	(taint_globals.v)
#endif

#endif /*PHP_TAINT_H*/








/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
