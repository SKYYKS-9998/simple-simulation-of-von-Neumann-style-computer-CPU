#include "head.h"
#include "global.h"   	//全局变量以及binTodec二进制转十进制函数 

void jmp (int codeID)	//跳转函数 
{
	char con[OBJLEN+1], num[NUMLEN+1];
	strncpy(con, ir[codeID]+OPERATION, OBJLEN); 	//将指令寄存器的操作对象码，在这里也就是跳转条件，赋给con字符串 
	/*获取立即数*/ 
	if(codeID==0)
		strncpy(num, (Instruction[ip[codeID]/4-1])+NUMSTR, NUMLEN); 	//取完指令后，ip增加了4，所以ip除以4的商减去1才是当前执行的指令的下标 	
	else if(codeID==1)
		strncpy(num, (Instruction[64+ip[codeID]/4-1])+NUMSTR, NUMLEN); 	//2号核的指令从Instruction[64]开始存储
	switch(binTodec(con, OBJLEN))		//将con字符串转换为十进制数，得到跳转条件，展开条件语句 
	{
		/*0为无条件跳转，1则当标志寄存器flag为0时跳转，2则当flag为1时跳转，3则当flag为2时跳转*/ 
		/*程序计数器ip加上十进制立即数再减去4则完成跳转*/
		/*取指令时，ip加上了4，所以跳转时还需要减去这个4*/
		case 0:
			ip[codeID]+=binTodec(num, NUMLEN)-4;
			break;
		case 1:
			if(flag[codeID]==EQUAL)
				ip[codeID]+=binTodec(num, NUMLEN)-4;
			break;
		case 2:
			if(flag[codeID]==LARGER)
			    ip[codeID]+=binTodec(num, NUMLEN)-4;
			break;
		case 3:
			if(flag[codeID]==SMALLER)
			    ip[codeID]+=binTodec(num, NUMLEN)-4;
			break;
	}
}
