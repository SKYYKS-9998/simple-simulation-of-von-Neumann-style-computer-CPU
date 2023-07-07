#include "head.h"
#include "global.h"

static void regReg (int reg1, int reg2, int op);
static void regNum (int reg, int num, int op);
static int calculator (int num1, int num2, int op);

void arith (int op)
{
	char reg1[OBJLEN/2+1], reg2[OBJLEN/2+1];
	strncpy(reg1, ir+OPERATION, OBJLEN/2);
	strncpy(reg2, ir+OPERATION+OBJLEN/2, OBJLEN/2);
	if(strcmp(reg2, "0000")==0)
	{
		char num[NUMLEN+1];
		strncpy(num, (Instruction[ip/4-1])+NUMSTR, NUMLEN); 
		regNum(binTodec(reg1, OBJLEN/2), binTodec(num, NUMLEN), op);
	}
	else
	    regReg(binTodec(reg1, OBJLEN/2), binTodec(reg2, OBJLEN/2), op);
}

static void regReg (int reg1, int reg2, int op)
{
	if(reg1<5 && reg2>=5)
		axNum[reg1-1]=calculator(axNum[reg1-1], Data[(axAdr[reg2-5]-DATA_BEG)/2], op);
	else if(reg1>=5 && reg2<5)
	    Data[(axAdr[reg1-5]-DATA_BEG)/2]=calculator(Data[(axAdr[reg1-5]-DATA_BEG)/2], axNum[reg2-1], op);
	else if(reg1<5 && reg2<5)
	    axNum[reg1-1]=calculator(axNum[reg1-1], axNum[reg2-1], op);
	else
	    Data[(axAdr[reg1-5]-DATA_BEG)/2]=calculator(Data[(axAdr[reg1-5]-DATA_BEG)/2], Data[(axAdr[reg2-5]-DATA_BEG)/2], op);
}

static void regNum (int reg, int num, int op)
{
	if(reg<5)
		axNum[reg-1]=calculator(axNum[reg-1], num, op);
	else
		Data[(axAdr[reg-5]-DATA_BEG)/2]=calculator(Data[(axAdr[reg-5]-DATA_BEG)/2], num, op);
}

static int calculator (int num1, int num2, int op)
{
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
