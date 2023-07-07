#include "head.h"
#include "global.h"   //全局变量以及binTodec二进制转十进制函数 

static void regReg (int codeID, int reg1, int reg2);	//寄存器与寄存器所存储的数据进行比较运算 
static void regNum (int codeID, int reg, int num);		//寄存器所存储的数据与立即数进行比较运算 
static int numCmp (int num1, int num2);					//比较运算函数 

void cmp (int codeID)	//比较运算函数 
{
	char reg1[OBJLEN/2+1], reg2[OBJLEN/2+1];
	strncpy(reg1, ir[codeID]+OPERATION, OBJLEN/2);			//将指令寄存器ir的第一个操作对象赋给reg1 
	strncpy(reg2, ir[codeID]+OPERATION+OBJLEN/2, OBJLEN/2);	//将指令寄存器ir的第二个操作对象赋给reg2 
	if(strcmp(reg2, "0000")==0)			//reg2为0，则代表是立即数与寄存器所存储的数值进行比较运算
	{
		char num[NUMLEN+1];				//存储十六位二进制立即数 
		if(codeID==0)
			strncpy(num, (Instruction[ip[codeID]/4-1])+NUMSTR, NUMLEN); 	//取完指令后，ip增加了4，所以ip除以4的商减去1才是当前执行的指令的下标 
		else if(codeID==1)
			strncpy(num, (Instruction[64+ip[codeID]/4-1])+NUMSTR, NUMLEN); 	//2号核的指令从Instruction[64]开始存储
		regNum(codeID, binTodec(reg1, OBJLEN/2), binTodec(num, NUMLEN));	//调用regNum函数，将寄存器编号和立即数转换为十进制数，以形参方式传送的函数 
	}
	else
	    regReg(codeID, binTodec(reg1, OBJLEN/2), binTodec(reg2, OBJLEN/2));	//将寄存器编号转换十进制数，传递给函数进行后续工作 
}

/*
地址寄存器存储的是内存地址，而数据段内存从16384开始，也就是全局常量DATA_BEG
两个字节一个单元，即每两个字节，表示数据段内存的数组Data的元素下标加1
地址寄存器存储的内存地址减去DATA_BEG，再除以2，即为该内存地址在Data数组中所对应的元素下标 

numCmp函数的返回值将直接赋值给标志寄存器 
*/

static void regReg (int codeID, int reg1, int reg2)	//寄存器与寄存器所存储的数据进行比较运算 
{
	/*根据寄存器编号判断寄存器类别，将所需运算数传递给numCmp函数进行比较运算*/
	if(reg1<5 && reg2>=5)		//reg1为数据寄存器，reg2为地址寄存器 
		flag[codeID]=numCmp(axNum[codeID][reg1-1], Data[(axAdr[codeID][reg2-5]-DATA_BEG)/2]);
	else if(reg1>=5 && reg2<5)	//reg1为地址寄存器，reg2为数据寄存器 
		flag[codeID]=numCmp(Data[(axAdr[codeID][reg1-5]-DATA_BEG)/2], axNum[codeID][reg2-1]);
	else if(reg1<5 && reg2<5)	//reg1和reg2都为数据寄存器 
		flag[codeID]=numCmp(axNum[codeID][reg1-1], axNum[codeID][reg2-1]);
	else						//reg1和reg2都为地址寄存器 
		flag[codeID]=numCmp(Data[(axAdr[codeID][reg1-5]-DATA_BEG)/2], Data[(axAdr[codeID][reg2-5]-DATA_BEG)/2]);
}

static void regNum (int codeID, int reg, int num)	//寄存器所存储的数据与立即数进行比较运算 
{
	/*根据寄存器编号判断寄存器类别，将所需运算数传递给numCmp函数进行比较运算*/
	if(reg<5)	//寄存器为数据寄存器 
	    flag[codeID]=numCmp(axNum[codeID][reg-1], num);
	else		//寄存器为地址寄存器 
	    flag[codeID]=numCmp(Data[(axAdr[codeID][reg-5]-DATA_BEG)/2], num);
}

static int numCmp (int num1, int num2)	//比较运算函数 
{
	/*如全局常量所示，numCmp函数返回比较运算结果对应的常量*/
	if(num1>num2)
	    return LARGER;
	else if(num1<num2)
	    return SMALLER;
	else if(num1==num2)
	    return EQUAL;
}
