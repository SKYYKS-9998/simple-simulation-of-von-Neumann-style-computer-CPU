#include <stdio.h>
#include "head.h"
#include "global.h"   	//全局变量以及binTodec二进制转十进制函数 

void input (int codeID)	//输入指令序列函数 
{
	FILE * fp;
	int i;
	/*将指令从文件中逐行输入到Instruction二维字符串中*/
	if(codeID==0)  
	{
		fp=fopen("dict1.dic", "r");	//1号核的指令序列在dict1.dic 
		/*出口条件循环的结束条件需要判断当前读取的指令，所以循环指数i只能在输入一行指令前递增，且输入开始时i应为0，所以先赋值-1*/ 
		i=-1;  
		do{
			i++;
			fgets(Instruction[i], MAXLEN, fp);
			fgetc(fp);	//跳过换行符 
		}while(strncmp(Instruction[i], "00000000", 8)!=0);	//当前读入的指令前八位操作码不全为0，即不是停机指令 
	}
	else if(codeID==1)
	{
		fp=fopen("dict2.dic", "r");	//2号核的指令序列在dict2.dic
		/*2号核的指令从代码段地址256开始存储，也就是Instruction[64]，所以i先赋值63*/ 
		i=63; 
		do{
			i++;
			fgets(Instruction[i], MAXLEN, fp);
			fgetc(fp);	//跳过换行符 
		}while(strncmp(Instruction[i], "00000000", 8)!=0);	//当前读入的指令前八位操作码不全为0，即不是停机指令
	}
	fclose(fp);
}

void inINS (int codeID)	//输入指令函数 
{
	char reg[OBJLEN+1];
	strncpy(reg, ir[codeID]+OPERATION, OBJLEN);	//将指令寄存器的操作对象赋给reg 
	int no=binTodec(reg, OBJLEN);	//reg转换为十进制寄存器编号，赋值给no 
	/*
	为防止输出被打断或本次输出打断其他输出，使用输出专用的互斥对象outMutex
	输出前必须先请求互斥对象，如果已被占用则一直等待，输出完成后释放互斥对象 
	*/
	WaitForSingleObject(outMutex, INFINITE);	//请求outMutex互斥对象 
	printf("in:\n");
	ReleaseMutex(outMutex);						//释放outMutex互斥对象 
	//字符串reg存储了八位二进制，如果目标寄存器的编号位于前四位，则no大于或等于2的四次幂（16），no需要除以16以得到正确的目标寄存器编号 
	if(no>=16)  
		no/=16;
	if(no<5)	//目标寄存器为数据寄存器 
		scanf("%d", &axNum[codeID][no-1]);
	else		//目标寄存器为地址寄存器
		/*
		地址寄存器存储的是内存地址，而数据段内存从16384开始，也就是全局常量DATA_BEG
		两个字节一个单元，即每两个字节，表示数据段内存的数组Data的元素下标加1
		地址寄存器存储的内存地址减去DATA_BEG，再除以2，即为该内存地址在Data数组中所对应的元素下标 
		*/ 
	    scanf("%d", &Data[(axAdr[codeID][no-5]-DATA_BEG)/2]);
}

void outINS (int codeID)	//输出指令函数 
{
	/*
	为防止输出被打断或本次输出打断其他输出，使用输出专用的互斥对象outMutex
	输出前必须先请求互斥对象，如果已被占用则一直等待，输出完成后释放互斥对象 
	*/
	WaitForSingleObject(outMutex, INFINITE);	//请求outMutex互斥对象 
	char reg[OBJLEN+1];
	strncpy(reg, ir[codeID]+OPERATION, OBJLEN);	//将指令寄存器的操作对象赋给reg 
	int no=binTodec(reg, OBJLEN);				//reg转换为十进制寄存器编号，赋值给no 
	printf("id = %d", codeID+1);				//先输出核编号 
	printf("    out: ");
	//字符串reg存储了八位二进制，如果目标寄存器的编号位于前四位，则no大于或等于2的四次幂（16），no需要除以16以得到正确的目标寄存器编号 
	if(no>=16)
		no/=16;
	if(no<5)	//目标寄存器为数据寄存器 
	    printf("%d\n", axNum[codeID][no-1]);
	else		//目标寄存器为地址寄存器
		/*
		地址寄存器存储的是内存地址，而数据段内存从16384开始，也就是全局常量DATA_BEG
		两个字节一个单元，即每两个字节，表示数据段内存的数组Data的元素下标加1
		地址寄存器存储的内存地址减去DATA_BEG，再除以2，即为该内存地址在Data数组中所对应的元素下标 
		*/ 
	    printf("%d\n", Data[(axAdr[codeID][no-5]-DATA_BEG)/2]);
	ReleaseMutex(outMutex);	//释放outMutex互斥对象 
}

void outputStates (int codeID)	//输出寄存器状态函数 
{
	/*
	为防止输出被打断或本次输出打断其他输出，使用输出专用的互斥对象outMutex
	输出前必须先请求互斥对象，如果已被占用则一直等待，输出完成后释放互斥对象 
	*/
	WaitForSingleObject(outMutex, INFINITE);	//请求outMutex互斥对象 
	int i;
	printf("id = %d\n", codeID+1); 				//输出核编号 
	printf("ip = %d\n", (codeID==0)?ip[codeID]:64*4+ip[codeID]);	//输出程序计数器ip。如果是2号核则需要加上64*4，因为2号核的代码从代码段64*4字节开始存储 
	printf("flag = %d\n", flag[codeID]);		//输出标志寄存器 
	printf("ir = %d\n", binTodec(ir[codeID], OPERATION+OBJLEN));	//输出指令寄存器 
	for(i=0; i<4; i++)
	    printf((i==3)?"ax%d = %d\n":"ax%d = %d ", i+1, axNum[codeID][i]);	//输出数据寄存器 
	for(i=0; i<4; i++) 
	    printf((i==3)?"ax%d = %d\n":"ax%d = %d ", i+5, axAdr[codeID][i]);  	//输出地址寄存器 
	ReleaseMutex(outMutex);		//释放outMutex互斥对象 
}

void outputCodeSegment (void)	//输出代码段函数 
{
	int i; 
	printf("\ncodeSegment :\n");
	for(i=0; i<128; i++)
	    printf(((i+1)%8==0)?"%d\n":"%d ", binTodec(Instruction[i], MAXLEN-1));
}

void outputDataSegment (void)	//输出数据段函数 
{
	int i;
	printf("\ndataSegment :\n");
	for(i=0; i<256; i++)
	    printf(((i+1)%16==0)?"%d\n":"%d ", Data[i]);
}
