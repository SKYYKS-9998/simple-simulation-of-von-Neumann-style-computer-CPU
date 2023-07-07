#include "head.h"
#include "global.h"

static void regReg (int reg1, int reg2);
static void regNum (int reg, int num);
static int numCmp (int num1, int num2);

void cmp (void)
{
	char reg1[OBJLEN/2+1], reg2[OBJLEN/2+1];
	strncpy(reg1, ir+OPERATION, OBJLEN/2);
	strncpy(reg2, ir+OPERATION+OBJLEN/2, OBJLEN/2);
	if(strcmp(reg2, "0000")==0)
	{
		char num[NUMLEN+1];
		strncpy(num, (Instruction[ip/4-1])+NUMSTR, NUMLEN); 
		regNum(binTodec(reg1, OBJLEN/2), binTodec(num, NUMLEN));
	}
	else
	    regReg(binTodec(reg1, OBJLEN/2), binTodec(reg2, OBJLEN/2));
}

static void regReg (int reg1, int reg2)
{
	if(reg1<5 && reg2>=5)
		flag=numCmp(axNum[reg1-1], Data[(axAdr[reg2-5]-DATA_BEG)/2]);
	else if(reg1>=5 && reg2<5)
		flag=numCmp(Data[(axAdr[reg1-5]-DATA_BEG)/2], axNum[reg2-1]);
	else if(reg1<5 && reg2<5)
		flag=numCmp(axNum[reg1-1], axNum[reg2-1]);
	else
		flag=numCmp(Data[(axAdr[reg1-5]-DATA_BEG)/2], Data[(axAdr[reg2-5]-DATA_BEG)/2]);
}

static void regNum (int reg, int num)
{
	if(reg<5)
	    flag=numCmp(axNum[reg-1], num);
	else
	    flag=numCmp(Data[(axAdr[reg-5]-DATA_BEG)/2], num);
}

static int numCmp (int num1, int num2)
{
	if(num1>num2)
	    return LARGER;
	else if(num1<num2)
	    return SMALLER;
	else if(num1==num2)
	    return EQUAL;
}
