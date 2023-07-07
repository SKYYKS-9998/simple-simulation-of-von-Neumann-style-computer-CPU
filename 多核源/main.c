#include <stdio.h>
#include <stdlib.h>
#include "head.h"	//全局常量以及string.h和windows.h头文件 

/*外部函数声明*/
extern void numTransfer (int codeID);  		//数据传送函数 
extern void arith (int codeID, int op);  	//算术运算函数 
extern void logic (int codeID, int op);  	//逻辑运算函数 
extern void cmp (int codeID);  				//比较运算函数 
extern void jmp (int codeID);  				//跳转函数 
extern void input (int codeID);  			//输入指令序列函数 
extern void inINS (int codeID);  			//输入指令函数 
extern void outINS (int codeID);  			//输出指令函数 
extern void applyMutex (int codeID);  		//请求互斥对象函数 
extern void releaseMutex (int codeID);  	//释放互斥对象函数 
extern void sleep (int codeID);   			//休眠函数 
extern void outputStates (int codeID);  	//输出寄存器状态函数 
extern void outputCodeSegment (void);  		//输出代码段函数 
extern void outputDataSegment (void);  		//输出数据段函数 

/*全局变量*/
char Instruction [128][MAXLEN];		//代码段内存，字符串二维数组存储指令，每个指令占一行
int Data[256];  					//数据段内存，两字节为一个元素
char ir [2][OPERATION+OBJSTR+1];  	//十六位指令寄存器，每个核一个
int ip [2];  						//程序计数器，每个核一个
int flag [2];  						//标志寄存器，每个核一个
int axNum [2][4];  					//4个通用数据寄存器，每个核四个
int axAdr [2][4];  					//4个通用地址寄存器，每个核四个
HANDLE mutex[256];  				//执行指令时所需的互斥对象，分别对应256个数据段内存元素
HANDLE outMutex;  					//进行输出工作（执行输出指令和输出内存状态）时所需的互斥对象

static void initialize (void);						//初始化全局变量函数 
static void take(int codeID);						//取指令函数 
static int command(int codeID);						//判断指令操作类型函数 
int binTodec (char * num, int len); 				//二进制转十进制函数 
static DWORD WINAPI startCore (LPVOID lpParameter);	//线程开始函数 

int main(int argc, char *argv[])
{
	int codeID[2]={0,1};	//两个核的编号，为方便使用全局变量而在程序中使用0和1，而不是1和2。输出仍是1和2 
	int i;
	initialize();			//初始化全局变量 
	input(codeID[0]);		//输入1号核的指令序列 
	input(codeID[1]);		//输入2号核的指令序列 
	for(i=0; i<256; i++)
		mutex[i]=CreateMutex(NULL, FALSE, NULL);	//创建互斥量数组 
	outMutex=CreateMutex(NULL, FALSE, NULL);
	HANDLE core1=CreateThread(NULL, 0, startCore, &codeID[0], 0, NULL);		//创建1号核线程， 传递零号核的编号的地址LPVOID lpParameter参数，以便后续使用 
	HANDLE core2=CreateThread(NULL, 0, startCore, &codeID[1], 0, NULL);		//创建2号核线程 
	WaitForSingleObject(core1, INFINITE);
	CloseHandle(core1);
	WaitForSingleObject(core2, INFINITE);
	CloseHandle(core2); 
	outputCodeSegment();	//输出代码段内存 
	outputDataSegment();	//输出数据段内存 
	system("pause");
	return 0;
}

static void initialize (void)	//初始化全局变量函数 
{
	int i, j;
	for(i=0; i<128; i++)
	    Instruction[i][0]='\0';	//将代码段每一行第一个字符初始化为空字符，取指令只会取到空指令，输出代码段只会输出0 
	Data[0]=10; 				//将数据段内存16384初始化为票数 
	for(i=1; i<256; i++)
		Data[i]=0; 				//数据段内存其他部分初始化为0 
	for(j=0; j<2; j++)
	{
		ip[j]=0;				//程序计数器初始化为0 
		flag[j]=0;				//标志寄存器初始化为0 
		for(i=0; i<OPERATION+OBJSTR; i++)
	    	ir[j][i]='\0';		//指令寄存器初始化为0 
	    for(i=0; i<4; i++){
			axNum[j][i]=0;		//数据寄存器初始化为0 
			axAdr[j][i]=0;  	//地址寄存器初始化为0 
		}
	}
}

static void take(int codeID)	//取指令函数，int codeID为核的编号 
{
	/*根据codeID的值，判断该从代码段那个位置取指令，并决定该使用哪些寄存器*/ 
	if(codeID==0)
		strncpy(ir[codeID], Instruction[ip[codeID]/4], OPERATION+OBJLEN);
	else if(codeID==1)
		strncpy(ir[codeID], Instruction[64+ip[codeID]/4], OPERATION+OBJLEN); 	//2号核的指令从Instruction[64]开始存储 
	ip[codeID]+=4;				//程序计数器加4 
}

static int command(int codeID)				//判断指令操作类型函数，int codeID为核的编号 
{
	char num[OPERATION+1];
	strncpy(num, ir[codeID], OPERATION);	//将指令寄存器的前八位操作码传给num字符串 
	return binTodec(num, OPERATION);		//将num转换为十进制后返回，判断指令的操作类型 
}

int binTodec (char * num, int len)	//二进制转十进制函数 
{
	/*char * num 为二进制字符串的首字符地址，int len为字符串长度*/
	int dec=0;		//需返回的十进制数 
	int i=0;
	int fac=1;		//二进制数该位的位权 
	int j=0;
	
	if(num[0]=='\0')
	    return 0;	//空字符则直接返回0 
	
	if(num[0]=='1' && len==NUMLEN)	//第一位是1，则可能是负数。只有立即数部分才有负数，还需判断长度是不是16位的立即数 
	{
		/*将补码二进制负数转换为十进制*/
		/*先将二进制数减去1*/ 
		if(num[len-1]=='1')			//如果末位是1则直接减去，变为0 
		    num[len-1]='0';
		else						//如果末位不是1，则用循环找到最近的1，将其变为0，然后将之前的0都变为1 
		{
			for(i=len-2; i>=1 && num[i]!='1'; i--);
			num[i]='0';
			for(j=i+1; j<len; j++)
			    num[j]='1';
		}
		for(i=1; i<len; i++) 		//进行反码操作 
		{
			if(num[i]=='1')
				num[i]='0';
			else
			    num[i]='1';
		}
		for(i=len-1; i>=1; i--) 	//转换为十进制数 
		{
			if(num[i]=='0')			//位数为0则直接跳过 
			    continue;
			else
			{
				/*位数为1，则先透过循环乘法得出该位的位权，加到dec十进制数变量中*/
				for(j=len-1, fac=1; j>i; j--)  
				    fac*=2;
				dec+=fac;
			}
		}
		dec*=-1; 					//将十进制数变为负数 
	}
	else 							//不是二进制负数，直接转换为十进制 
	{
		for(i=len-1; i>=0; i--)
		{
			if(num[i]=='0')
			    continue;
			else
			{
				for(j=len-1, fac=1; j>i; j--)
				    fac*=2;
				dec+=fac;
			}
		}
	}
	return dec;
}

static DWORD WINAPI startCore (LPVOID lpParameter)	//线程开始函数 
{
	int codeID=(*(int*)lpParameter);				//从lpParameter得到核的编号 
	do{
		take(codeID); 								//取指令，改变指令寄存器ir和程序计数器ip的值 
		switch(command(codeID))  					//判断指令的操作类别 
		{
			case 1:
				numTransfer(codeID); break; 	 	//执行数据传送指令 
			case 2:
				arith(codeID, 1); break;  			//执行加法算术运算指令 
			case 3:
				arith(codeID, 2); break;  			//执行减法算术运算指令 
			case 4:
				arith(codeID, 3); break;  			//执行乘法算术运算指令 
			case 5:
				arith(codeID, 4); break;  			//执行除法算术运算指令 
			case 6:
				logic(codeID, 1); break;  			//执行逻辑与运算指令 
			case 7:
				logic(codeID, 2); break;  			//执行逻辑或运算指令 
			case 8:
				logic(codeID, 3); break;  			//执行逻辑非运算指令 
			case 9:
				cmp(codeID); break;  				//执行比较指令 
			case 10:
				jmp(codeID); break;  				//执行跳转指令 
			case 11:
				inINS(codeID); break;  				//执行输入指令 
			case 12:
				outINS(codeID); break;  			//执行输出指令 
			case 13:
				applyMutex(codeID); break;  		//执行请求互斥对象指令 
			case 14:
				releaseMutex(codeID); break;  		//执行释放互斥对象指令 
			case 15:
				sleep(codeID); break;  				//执行休眠指令 
		}
		outputStates(codeID);  						//输出寄存器状态 
	}while(strcmp(ir[codeID], "0000000000000000")!=0 && ir[codeID][0]!='\0'); //出口循环条件为指令寄存器ir不是停机指令，并且不为空字符串 
	return;
}
