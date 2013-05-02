#include <stdio.h>
#include <math.h>


void print_logarithm( double x)
{
	if ( x < 0.0 ){
		printf(" positive numbers only, please \n");
		return ;
	}
	printf("The log of  x 	is  %f", log(x));
}


void newfunc();
void print_time(int hour, int minure);


int main(void)
{
	print_logarithm(8);
	char first='1';
	printf("%c", first);	
newfunc();
	printf("Hello World .\n" );
print_time(8, 30);

//字符串 数组 字符指针 
	char * str = "this is my string";
	printf("%s.\n", str );

	char day[15]="abcdefghijklmn";
	char * str_tmp="opqrstuvwxyz";
	printf("&day is %x \n", &day);
	printf("&day[0] is %x \n", &day[0]);
	printf("&day is %x \n", &day);


	return 0;
}

void newfunc(){
	printf("\n");
}

void print_time(int hour, int minite){
	printf("%d:%d", hour, minite);
}



/* ================================================================

//语法规则是由关于符号（token）和结构（structure）的规则组成。
//关于token的规则称之为词法规则；而关于语句规则的称之为语法规则；

请记住: 理解一个概念不是把定义背下来就行了,一定要理解它的外延和内涵,也就是什么情 况属于这个概念,什么情况不属于这个概念,什么情况虽然属于这个概念但作为一种最佳实践 (Best Practice)应该避免这种情况,这才算是真正理解了。

如果一个声明，同时也要求分配存储空间，则称为定义。

定义一个变量，就是分配一块存储空间并给它命名。
给一个变量赋值，就是把一个值存到了这块存储空间中。
变量的定义和赋值也可以一步完成，这称为变量的初始化(Initialization)。

只有带函数体的声明才叫定义。


我还是比较自负的一个人；

函数原型：函数返回值类型、函数名、参数类型和个数；
函数声明；函数定义；
先声明后使用；

函数的隐式声明；返回值类型总是int；

最少例外原则；

设计者这么规定的原因：rationable . 

全局变量只能使用常量来初始化；

函数返回一个值，相当于定义一个和函数返回值类型相同的临时变量并用return后面的表达式进行初始化；

左值 右值 表达式  initialitor   scaffold脚手架 增量式开发比较适合新手 尽量复用代码 不要重复；封装就是为了复用；
stack & stack frame;  Leap  of  Faith ; Base Case对于递归的意义;

循环（loop），指的是在满足条件的情况下，重复执行同一段代码。比如，while语句。
迭代（iterate），指的是按照某种顺序逐个访问列表中的每一项。比如，for语句。
遍历（traversal），指的是按照一定的规则访问树形结构中的每个节点，而且每个节点都只访问一次。
递归（recursion），指的是一个函数不断调用自身的行为。比如，以编程方式输出著名的斐波纳契数列。

计算机硬件能做的所有事情就是数据存取、运算、测试和分支、循环(或递归);

跟循环最相关的就是算法效率了：
1、在多重循环中如果有可能，应该将最长循环放在最里层，将最短的循环放在最外层，这样可以最大程度减少CPU的跨切循环的次数。
2、如果循环体内存在逻辑判断，并且循环次数很大，宜将逻辑判断移到循环体的外面。

这门语言提供了哪些：
1、primitive：例如基本数据类型、基本运算符、表达式、语句等。
2、组合规则：表达式和语句的组合规则、复合数据结构。
3、抽象机制：数据抽象(其结果是数据类型，面向对象程序设计的基本手段)、过程抽象（其结果是函数，面向过程程序设计的基本手段）等。

数据类型 把数据和其上的操作紧密、逻辑地联系在一起。



函数式编程： Functional Programming
命令式编程： Imperative Programming
数学函数是没有Side Effect的,而C语言的函数可以有Side Effect。


*/
