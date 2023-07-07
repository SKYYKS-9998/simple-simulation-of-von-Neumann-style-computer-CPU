#include <stdio.h>
#include <stdlib.h>
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

/*在多模块中，以下函数皆为外部函数*/
void numTransfer (int codeID);  	//数据传送函数 
void arith (int codeID, int op);  	//算术运算函数 
void logic (int codeID, int op);  	//逻辑运算函数 
void cmp (int codeID);  			//比较运算函数 
void jmp (int codeID);  			//跳转函数 
void input (int codeID);  			//输入指令序列函数 
void inINS (int codeID);  			//输入指令函数 
void outINS (int codeID);  			//输出指令函数 
void applyMutex (int codeID);  		//请求互斥对象函数 
void releaseMutex (int codeID);  	//释放互斥对象函数 
void sleep (int codeID);   			//休眠函数 
void outputStates (int codeID);  	//输出寄存器状态函数 
void outputCodeSegment (void);  	//输出代码段函数 
void outputDataSegment (void);  	//输出数据段函数 

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

/*main.c*/ 
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
	WaitForSingleObject(core1, INFINITE);		//等待1号核线程的信号
	CloseHandle(core1);
	WaitForSingleObject(core2, INFINITE);		//等待2号核线程的信号
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
	Data[0]=100; 				//将数据段内存16384初始化为票数 
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

/*
一些模块含有静态函数，为在单一.c文件中实现类似功能，将这些函数定义在所属模块里调用它们的外部函数中 
*/

/*transfer.c*/
void numTransfer (int codeID)	//数据传送函数 
{
	void numToreg(int codeID, int dest, int num) //将一个立即数传送至寄存器 
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

	void numToadr (int codeID, int src, int dest)	//将数据从数据寄存器传送到地址寄存器所指向的内存单元 
	{
		Data[(axAdr[codeID][dest-5]-DATA_BEG)/2]=axNum[codeID][src-1];   
	}

	void adrTonum (int codeID, int src, int dest)	//将数据从地址寄存器所指向的内存单元传送到数据寄存器 
	{
		axNum[codeID][dest-1]=Data[(axAdr[codeID][src-5]-DATA_BEG)/2];
	}
	
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

/*arithmetic.c*/
void arith (int codeID, int op)	//算术运算函数 
{
	int calculator (int num1, int num2, int op)				//算术运算函数 
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
	
	/*
	地址寄存器存储的是内存地址，而数据段内存从16384开始，也就是全局常量DATA_BEG
	两个字节一个单元，即每两个字节，表示数据段内存的数组Data的元素下标加1
	地址寄存器存储的内存地址减去DATA_BEG，再除以2，即为该内存地址在Data数组中所对应的元素下标 
	*/

	void regReg (int codeID, int reg1, int reg2, int op)		//寄存器与寄存器所存储的数据进行算术运算 
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

	void regNum (int codeID, int reg, int num, int op)		//寄存器所存储的数据与立即数进行算术运算 
	{
		/*根据寄存器编号判断寄存器类别，将所需运算数传递给calculator函数进行算术运算*/
		if(reg<5)  					//寄存器为数据寄存器 
			axNum[codeID][reg-1]=calculator(axNum[codeID][reg-1], num, op);
		else  						//寄存器为地址寄存器 
			Data[(axAdr[codeID][reg-5]-DATA_BEG)/2]=calculator(Data[(axAdr[codeID][reg-5]-DATA_BEG)/2], num, op);
	}
	
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

/*logic.c*/
void logic (int codeID, int op)	//逻辑运算函数 
{
	int numLogic (int num1, int num2, int op)			//逻辑运算函数 
	{
		/*op的值代表了逻辑运算的类别，1为与，2为或，3为非。返回运算结果*/
		switch(op)
		{
			case 1:
				return (num1&&num2);
			case 2:
			    return (num1||num2);
			case 3:
			    return !(num1);
		}
	}

	/*
	地址寄存器存储的是内存地址，而数据段内存从16384开始，也就是全局常量DATA_BEG
	两个字节一个单元，即每两个字节，表示数据段内存的数组Data的元素下标加1
	地址寄存器存储的内存地址减去DATA_BEG，再除以2，即为该内存地址在Data数组中所对应的元素下标 
	*/

	void regReg (int codeID, int reg1, int reg2, int op)	//寄存器与寄存器所存储的数据进行逻辑运算 
	{
		/*根据寄存器编号判断寄存器类别，将所需运算数传递给numLogic函数进行逻辑运算*/
		if(reg1<5 && reg2>=5)		//reg1为数据寄存器，reg2为地址寄存器 
			axNum[codeID][reg1-1]=numLogic(axNum[codeID][reg1-1], Data[(axAdr[codeID][reg2-5]-DATA_BEG)/2], op);
		else if(reg1>=5 && reg2<5)	//reg1为地址寄存器，reg2为数据寄存器 
			Data[(axAdr[codeID][reg1-5]-DATA_BEG)/2]=numLogic(Data[(axAdr[codeID][reg1-5]-DATA_BEG)/2], axNum[codeID][reg2-1], op);
		else if(reg1<5 && reg2<5)	//reg1和reg2都为数据寄存器 
			axNum[codeID][reg1-1]=numLogic(axNum[codeID][reg1-1], axNum[codeID][reg2-1], op);
		else						//reg1和reg2都为地址寄存器 
		    Data[(axAdr[codeID][reg1-5]-DATA_BEG)/2]=numLogic(Data[(axAdr[codeID][reg2-5]-DATA_BEG)/2], Data[(axAdr[codeID][reg2-5]-DATA_BEG)/2], op);
	}

	void regNum (int codeID, int reg, int num, int op)	//寄存器所存储的数据与立即数进行逻辑运算 
	{
		/*根据寄存器编号判断寄存器类别，将所需运算数传递给numLogic函数进行逻辑运算*/
		if(reg<5)	//寄存器为数据寄存器 
			axNum[codeID][reg-1]=numLogic(axNum[codeID][reg-1], num, op);
		else		//寄存器为地址寄存器 
		    Data[(axAdr[codeID][reg-5]-DATA_BEG)/2]=numLogic(Data[(axAdr[codeID][reg-5]-DATA_BEG)/2], num, op);
	}

	char reg1[OBJLEN/2+1], reg2[OBJLEN/2+1];
	strncpy(reg1, ir[codeID]+OPERATION, OBJLEN/2);				//将指令寄存器ir的第一个操作对象赋给reg1 
	strncpy(reg2, ir[codeID]+OPERATION+OBJLEN/2, OBJLEN/2);		//将指令寄存器ir的第二个操作对象赋给reg2 
	if(op==3)  							//op等于3，则代表逻辑非运算。只需要对单一寄存器所存储的数据进行逻辑非运算 
	{
		/*不需判断两个目标寄存器编号以及获取立即数，直接调用regNum，将十进制目标寄存器编号传递给函数。由于仍需要一个int num形参，所以传递0*/
		if(strcmp(reg1, "0000")!=0) 	//reg1为目标寄存器 
			regNum(codeID, binTodec(reg1, OBJLEN/2), 0, op);  
		else 							//reg2为目标寄存器 
		    regNum(codeID, binTodec(reg2, OBJLEN/2), 0, op);
	}
	else if(strcmp(reg2, "0000")==0)	//reg2为0，则代表是立即数与寄存器所存储的数值进行逻辑运算
	{
		char num[NUMLEN+1];//存储十六位二进制立即数 
		if(codeID==0)
			strncpy(num, (Instruction[ip[codeID]/4-1])+NUMSTR, NUMLEN); 		//取完指令后，ip增加了4，所以ip除以4的商减去1才是当前执行的指令的下标 
		else if(codeID==1)
			strncpy(num, (Instruction[64+ip[codeID]/4-1])+NUMSTR, NUMLEN); 		//2号核的指令从Instruction[64]开始存储
		regNum(codeID, binTodec(reg1, OBJLEN/2), binTodec(num, NUMLEN), op);	//调用regNum函数，将寄存器编号和立即数转换为十进制数，以形参方式传送的函数 
	}
	else
		regReg(codeID, binTodec(reg1, OBJLEN/2), binTodec(reg2, OBJLEN/2), op);	//将寄存器编号转换十进制数，传递给函数进行后续工作 
}

/*cmp.c*/
void cmp (int codeID)	//比较运算函数 
{
	int numCmp (int num1, int num2)	//比较运算函数 
	{
		/*如全局常量所示，numCmp函数返回比较运算结果对应的常量*/
		if(num1>num2)
	    	return LARGER;
		else if(num1<num2)
	    	return SMALLER;
		else if(num1==num2)
	    	return EQUAL;
	}
	
	/*
	地址寄存器存储的是内存地址，而数据段内存从16384开始，也就是全局常量DATA_BEG
	两个字节一个单元，即每两个字节，表示数据段内存的数组Data的元素下标加1
	地址寄存器存储的内存地址减去DATA_BEG，再除以2，即为该内存地址在Data数组中所对应的元素下标 

	numCmp函数的返回值将直接赋值给标志寄存器 
	*/

	void regReg (int codeID, int reg1, int reg2)	//寄存器与寄存器所存储的数据进行比较运算 
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

	void regNum (int codeID, int reg, int num)	//寄存器所存储的数据与立即数进行比较运算 
	{
		/*根据寄存器编号判断寄存器类别，将所需运算数传递给numCmp函数进行比较运算*/
		if(reg<5)	//寄存器为数据寄存器 
	    	flag[codeID]=numCmp(axNum[codeID][reg-1], num);
		else		//寄存器为地址寄存器 
	    	flag[codeID]=numCmp(Data[(axAdr[codeID][reg-5]-DATA_BEG)/2], num);
	}

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
/*jmp.c*/
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

/*IO.c*/
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
	ReleaseMutex(outMutex);						//释放outMutex互斥对象 	//字符串reg存储了八位二进制，如果目标寄存器的编号位于前四位，则no大于或等于2的四次幂（16），no需要除以16以得到正确的目标寄存器编号 
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

/*multicore.c*/ 
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
