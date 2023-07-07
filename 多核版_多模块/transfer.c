#include "head.h"
#include "global.h"   //全局变量以及binTodec二进制转十进制函数 

static void numToreg(int codeID, int dest, int num);  	//将一个立即数传送至寄存器 
static void numToadr (int codeID, int src, int dest);  	//将数据从数据寄存器传送到地址寄存器所指向的内存单元 
static void adrTonum (int codeID, int src, int dest);  	//将数据从地址寄存器所指向的内存单元传送到数据寄存器 

void numTransfer (int codeID)	//数据传送函数 
{
	char dest[OBJLEN/2+1], src[OBJLEN/2+1];  				//dest为传送目标，src为传送起点 
	strncpy(dest, ir[codeID]+OPERATION, OBJLEN/2);  		//将指令寄存器ir的第一个操作对象赋给dest 
	strncpy(src, ir[codeID]+OPERATION+OBJLEN/2, OBJLEN/2);  //将指令寄存器ir的第二个操作对象赋给src 
	if(strcmp(src, "0000")==0)  //src为0，则代表传送起点不是一个寄存器，是立即数 
	{
		char num[NUMLEN+1];		//存储十六位二进制立即数 
		if(codeID==0)
			strncpy(num, (Instruction[ip[codeID]/4-1])+NUMSTR, NUMLEN);  		//取完指令后，ip增加了4，所以ip除以4的商减去1才是当前执行的指令的下标 
		else if(codeID==1)
			strncpy(num, (Instruction[64+ip[codeID]/4-1])+NUMSTR, NUMLEN); 		//2号核的指令从Instruction[64]开始存储
		numToreg(codeID, binTodec(dest, OBJLEN/2), binTodec(num, NUMLEN));  	//调用numToreg函数，将寄存器编号和立即数转换为十进制数，以形参方式传送给函数 
	}
	else if(strcmp(dest, src)>0)//dest比src大，证明是数据寄存器传送到地址寄存器所指向的内存单元 
		numToadr(codeID, binTodec(src, OBJLEN/2), binTodec(dest, OBJLEN/2));  	//将寄存器编号转换十进制数，传递给函数进行后续工作 
	else 
	    adrTonum(codeID, binTodec(src, OBJLEN/2), binTodec(dest, OBJLEN/2));
}

static void numToreg(int codeID, int dest, int num) //将一个立即数传送至寄存器 
{
	if(dest>=5)  									//dest大于或等于5，则传送目标寄存器为地址寄存器 
		axAdr[codeID][dest-5]=num;
	else if(dest<=4)  								//传送目标寄存器为数据寄存器 
	    axNum[codeID][dest-1]=num;
}

/*
地址寄存器存储的是内存地址，而数据段内存从16384开始，也就是全局常量DATA_BEG
两个字节一个单元，即每两个字节，表示数据段内存的数组Data的元素下标加1
地址寄存器存储的内存地址减去DATA_BEG，再除以2，即为该内存地址在Data数组中所对应的元素下标 
*/

static void numToadr (int codeID, int src, int dest)	//将数据从数据寄存器传送到地址寄存器所指向的内存单元 
{
	Data[(axAdr[codeID][dest-5]-DATA_BEG)/2]=axNum[codeID][src-1];   
}

static void adrTonum (int codeID, int src, int dest)	//将数据从地址寄存器所指向的内存单元传送到数据寄存器 
{
	axNum[codeID][dest-1]=Data[(axAdr[codeID][src-5]-DATA_BEG)/2];
}
