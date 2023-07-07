#include <stdio.h>
#include <stdlib.h> 
#include "head.h"

extern void numTransfer (void);
extern void arith (int op);
extern void logic (int op);
extern void cmp (void);
extern void jmp (void);
extern void input (void);
extern void inINS (void);
extern void outINS (void);
extern void outputStates (void);
extern void outputCodeSegment (void);
extern void outputDataSegment (void);

char Instruction [128][MAXLEN];
int Data[256];
char ir [OPERATION+OBJSTR+1];
int ip;
int flag;
int axNum[4];
int axAdr[4];

static void take(void);
static int command(void);
int binTodec (char * num, int len);

int main(int argc, char *argv[])
{
	int i;
	for(i=0; i<128; i++)
	    Instruction[i][0]='\0';
	for(i=0; i<256; i++)
		Data[i]=0;
	for(i=0; i<OPERATION+OBJSTR; i++)
	    ir[i]='\0';
	ip=0;
	flag=0;
	for(i=0; i<4; i++){
		axNum[i]=0;
		axAdr[i]=0;
	}
	
	input();
	do{
		take();
		switch(command())
		{
			case 1:
				numTransfer(); break;
			case 2:
				arith(1); break;
			case 3:
				arith(2); break;
			case 4:
				arith(3); break;
			case 5:
				arith(4); break;
			case 6:
				logic(1); break;
			case 7:
				logic(2); break;
			case 8:
				logic(3); break;
			case 9:
				cmp(); break;
			case 10:
				jmp(); break;
			case 11:
				inINS(); break;
			case 12:
				outINS(); break;	
		}
		outputStates();
	}while(strcmp(ir, "0000000000000000")!=0 && ir[0]!='\0');
	outputCodeSegment();
	outputDataSegment();
	return 0;
}

static void take(void)
{
	strncpy(ir, Instruction[ip/4], OPERATION+OBJLEN);
	ip+=4;
}

static int command(void)
{
	char num[OPERATION+1];
	strncpy(num, ir, OPERATION);
	return binTodec(num, OPERATION);
}

int binTodec (char * num, int len)
{
	int dec=0;
	int i=0;
	int fac=1;
	int j=0;
	
	if(num[0]=='\0')
	    return 0;
	
	if(num[0]=='1' && len==NUMLEN) 
	{
		if(num[len-1]=='1')
		    num[len-1]='0';
		else
		{
			for(i=len-2; i>=1 && num[i]!='1'; i--);
			num[i]='0';
			for(j=i+1; j<len; j++)
			    num[j]='1';
		}
		for(i=1; i<len; i++)
		{
			if(num[i]=='1')
				num[i]='0';
			else
			    num[i]='1';
		}
		for(i=len-1; i>=1; i--)
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
		dec*=-1;
	}
	else
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

