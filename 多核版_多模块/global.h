extern char Instruction [128][MAXLEN];		//代码段内存，字符串二维数组存储指令，每个指令占一行
extern int Data[256];						//数据段内存，两字节为一个元素
extern char ir [2][OPERATION+OBJSTR+1];		//十六位指令寄存器，每个核一个
extern int ip [2];							//程序计数器，每个核一个
extern int flag [2];						//标志寄存器，每个核一个
extern int axNum [2][4];					//4个通用数据寄存器，每个核四个
extern int axAdr [2][4];					//4个通用地址寄存器，每个核四个
extern HANDLE mutex[256];					//执行指令时所需的互斥对象，分别对应256个数据段内存元素
extern HANDLE outMutex;						//进行输出工作（执行输出指令和输出内存状态）时所需的互斥对象
extern int binTodec(char*num, int len);		//二进制转十进制函数	
