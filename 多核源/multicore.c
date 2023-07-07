#include "head.h"
#include "global.h"   //全局变量以及binTodec二进制转十进制函数 

void applyMutex (int codeID)	//请求互斥对象函数 
{ 
	char num[NUMLEN+1];		//存储十六位二进制立即数 
	if(codeID==0)
		strncpy(num, (Instruction[ip[codeID]/4-1])+NUMSTR, NUMLEN);		//取完指令后，ip增加了4，所以ip除以4的商减去1才是当前执行的指令的下标 
	else if(codeID==1)
		strncpy(num, (Instruction[64+ip[codeID]/4-1])+NUMSTR, NUMLEN);	//2号核的指令从Instruction[64]开始存储
	/*
	将二进制字符串num转换为十进制数，即内存地址
	再减去DATA_BEG，再除以2，即为该内存地址在Data数组中所对应的元素下标，也是对应的互斥对象在mutex中的下标
	*/ 
	WaitForSingleObject(mutex[(binTodec(num, NUMLEN)-DATA_BEG)/2], INFINITE);	//请求互斥对象 
}

void releaseMutex (int codeID)	//释放互斥对象函数 
{
	char num[NUMLEN+1];			//存储十六位二进制立即数 
	if(codeID==0)
		strncpy(num, (Instruction[ip[codeID]/4-1])+NUMSTR, NUMLEN);		//取完指令后，ip增加了4，所以ip除以4的商减去1才是当前执行的指令的下标 
	else if(codeID==1)
		strncpy(num, (Instruction[64+ip[codeID]/4-1])+NUMSTR, NUMLEN);	//2号核的指令从Instruction[64]开始存储
	/*
	将二进制字符串num转换为十进制数，即内存地址
	再减去DATA_BEG，再除以2，即为该内存地址在Data数组中所对应的元素下标，也是对应的互斥对象在mutex中的下标
	*/ 
	ReleaseMutex(mutex[(binTodec(num, NUMLEN)-DATA_BEG)/2]); 			//释放互斥对象 
}

void sleep (int codeID)		//休眠函数 
{
	char num[NUMLEN+1];		//存储十六位二进制立即数 
	if(codeID==0)
		strncpy(num, (Instruction[ip[codeID]/4-1])+NUMSTR, NUMLEN); 	//取完指令后，ip增加了4，所以ip除以4的商减去1才是当前执行的指令的下标 
	else if(codeID==1)
		strncpy(num, (Instruction[64+ip[codeID]/4-1])+NUMSTR, NUMLEN); 	//2号核的指令从Instruction[64]开始存储
	Sleep(binTodec(num, NUMLEN));	//将二进制数字符串num转换为十进制数，即为应休眠的毫秒数 
}
