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

#include <stdio.h>

#include "pytonic.h"
//extern int e_pytonic;

int p; // 这是声明 还是 定义？

int function1();
extern int ccc;
int main(){
	char a[10]="abcdefghij";
	char *p = &a[1];
	char *q = &a[2];
	int num = q - p + 1;
	printf("p:%s \n", p);
	printf("q:%s \n", q);
	printf("num:%d \n", num);

	int b = function1();
	printf("function1:%d \n", b);
	b = function1();
	printf("function1:%d \n", b);
	b = function1();
	printf("function1:%d \n", b);

	printf("ccc:%d \n", ccc);

	printf("pytonic%d \n", pytonic);
	printf("e_pytonic%d \n", e_pytonic);
//	printf("s_pytonic%d \n", s_pytonic);
	return 0;
}

int ccc=90;

int function1(){
	static int a=100;
	if(1) {
		a++;
	}
	return a;
}



/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
