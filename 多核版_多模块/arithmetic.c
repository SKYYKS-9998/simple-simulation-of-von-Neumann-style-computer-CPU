#include "head.h"
#include "global.h"   //全局变量以及binTodec二进制转十进制函数 

static void regReg (int codeID, int reg1, int reg2, int op); 	//寄存器与寄存器所存储的数据进行算术运算 
static void regNum (int codeID, int reg, int num, int op);  	//寄存器所存储的数据与立即数进行算术运算 
static int calculator (int num1, int num2, int op);  			//算术运算函数 

void arith (int codeID, int op)	//算术运算函数 
{
	char reg1[OBJLEN/2+1], reg2[OBJLEN/2+1];
	strncpy(reg1, ir[codeID]+OPERATION, OBJLEN/2);				//将指令寄存器ir的第一个操作对象赋给reg1 
	strncpy(reg2, ir[codeID]+OPERATION+OBJLEN/2, OBJLEN/2);		//将指令寄存器ir的第二个操作对象赋给reg2 
	if(strcmp(reg2, "0000")==0)	//reg2为0，则代表是立即数与寄存器所存储的数值进行算术运算 
	{
		char num[NUMLEN+1];		//存储十六位二进制立即数 
		if(codeID==0)
			strncpy(num, (Instruction[ip[codeID]/4-1])+NUMSTR, NUMLEN); 		//取完指令后，ip增加了4，所以ip除以4的商减去1才是当前执行的指令的下标 
		else if(codeID==1)
			strncpy(num, (Instruction[64+ip[codeID]/4-1])+NUMSTR, NUMLEN); 		//2号核的指令从Instruction[64]开始存储
		regNum(codeID, binTodec(reg1, OBJLEN/2), binTodec(num, NUMLEN), op);  	//调用regNum函数，将寄存器编号和立即数转换为十进制数，以形参方式传送的函数 
	}
	else
	    regReg(codeID, binTodec(reg1, OBJLEN/2), binTodec(reg2, OBJLEN/2), op); //将寄存器编号转换十进制数，传递给函数进行后续工作 
}

/*
地址寄存器存储的是内存地址，而数据段内存从16384开始，也就是全局常量DATA_BEG
两个字节一个单元，即每两个字节，表示数据段内存的数组Data的元素下标加1
地址寄存器存储的内存地址减去DATA_BEG，再除以2，即为该内存地址在Data数组中所对应的元素下标 
*/

static void regReg (int codeID, int reg1, int reg2, int op)		//寄存器与寄存器所存储的数据进行算术运算 
{
	/*根据寄存器编号判断寄存器类别，将所需运算数传递给calculator函数进行算术运算*/
	if(reg1<5 && reg2>=5)  		//reg1为数据寄存器，reg2为地址寄存器 
		axNum[codeID][reg1-1]=calculator(axNum[codeID][reg1-1], Data[(axAdr[codeID][reg2-5]-DATA_BEG)/2], op);
	else if(reg1>=5 && reg2<5)  //reg1为地址寄存器，reg2为数据寄存器 
	    Data[(axAdr[codeID][reg1-5]-DATA_BEG)/2]=calculator(Data[(axAdr[codeID][reg1-5]-DATA_BEG)/2], axNum[codeID][reg2-1], op);
	else if(reg1<5 && reg2<5)  	//reg1和reg2都为数据寄存器 
	    axNum[codeID][reg1-1]=calculator(axNum[codeID][reg1-1], axNum[codeID][reg2-1], op);
	else  						//reg1和reg2都为地址寄存器 
	    Data[(axAdr[codeID][reg1-5]-DATA_BEG)/2]=calculator(Data[(axAdr[codeID][reg1-5]-DATA_BEG)/2], Data[(axAdr[codeID][reg2-5]-DATA_BEG)/2], op);
}

static void regNum (int codeID, int reg, int num, int op)		//寄存器所存储的数据与立即数进行算术运算 
{
	/*根据寄存器编号判断寄存器类别，将所需运算数传递给calculator函数进行算术运算*/
	if(reg<5)  					//寄存器为数据寄存器 
		axNum[codeID][reg-1]=calculator(axNum[codeID][reg-1], num, op);
	else  						//寄存器为地址寄存器 
		Data[(axAdr[codeID][reg-5]-DATA_BEG)/2]=calculator(Data[(axAdr[codeID][reg-5]-DATA_BEG)/2], num, op);
}

static int calculator (int num1, int num2, int op)				//算术运算函数 
{
	/*op的值代表了算术运算的类别，1为加法，2为减法，3为乘法，4为除法。返回运算结果*/
	switch(op)
	{
		case 1:
			return num1+num2;
		case 2:
		    return num1-num2;
		case 3:
		    return num1*num2;
		case 4:
		    return num1/num2; 
	}
}
