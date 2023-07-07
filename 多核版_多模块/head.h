#include <string.h>
#include <windows.h>
#define MAXLEN 33		//存储指令的最大数组行元素数
#define OPERATION 8		//操作码长度
#define OBJSTR 8		//操作对象码开始
#define OBJLEN 8		//操作对象码长度
#define NUMSTR 16		//立即数开始
#define NUMLEN 16		//立即数长度
#define DATA_BEG 16384	//数据段地址开始
#define LARGER 1		//标志寄存器前大于后
#define EQUAL 0			//等于
#define SMALLER -1		//小于
