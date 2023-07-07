#include <stdio.h>
#include "head.h"

void numTransfer (void);
void arith (int op);
void logic (int op);
void cmp (void);
void jmp (void);
void input (void);
void inINS (void);
void outINS (void);
void outputStates (void);
void outputCodeSegment (void);
void outputDataSegment (void);

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

int main(void)
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



void numTransfer (void)
{
	void numToreg(int dest, int num)
	{
		if(dest>=5)
			axAdr[dest-5]=num;
		else if(dest<=4)
		    axNum[dest-1]=num;
	}

	void numToadr (int src, int dest)
	{
		Data[(axAdr[dest-5]-DATA_BEG)/2]=axNum[src-1];
	}

	void adrTonum (int src, int dest)
	{
		axNum[dest-1]=Data[(axAdr[src-5]-DATA_BEG)/2];
	}
	
	char dest[OBJLEN/2+1], src[OBJLEN/2+1];
	strncpy(dest, ir+OPERATION, OBJLEN/2);
	strncpy(src, ir+OPERATION+OBJLEN/2, OBJLEN/2);
	if(strcmp(src, "0000")==0)
	{
		char num[NUMLEN+1];
		strncpy(num, (Instruction[ip/4-1])+NUMSTR, NUMLEN); 
		numToreg(binTodec(dest, OBJLEN/2), binTodec(num, NUMLEN));
	}
	else if(strcmp(dest, src)>0)
		numToadr(binTodec(src, OBJLEN/2), binTodec(dest, OBJLEN/2));
	else 
	    adrTonum(binTodec(src, OBJLEN/2), binTodec(dest, OBJLEN/2));
}



void arith (int op)
{
	int calculator (int num1, int num2, int op)
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
	
	void regReg (int reg1, int reg2, int op)
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

	void regNum (int reg, int num, int op)
	{
		if(reg<5)
			axNum[reg-1]=calculator(axNum[reg-1], num, op);
		else
			Data[(axAdr[reg-5]-DATA_BEG)/2]=calculator(Data[(axAdr[reg-5]-DATA_BEG)/2], num, op);
	}
	
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



void logic (int op)
{
	int numLogic (int num1, int num2, int op)
	{
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
	
	void regReg (int reg1, int reg2, int op)
	{
		if(reg1<5 && reg2>=5)
			axNum[reg1-1]=numLogic(axNum[reg1-1], Data[(axAdr[reg2-5]-DATA_BEG)/2], op);
		else if(reg1>=5 && reg2<5)
		    Data[(axAdr[reg1-5]-DATA_BEG)/2]=numLogic(Data[(axAdr[reg1-5]-DATA_BEG)/2], axNum[reg2-1], op);
		else if(reg1<5 && reg2<5)
		    axNum[reg1-1]=numLogic(axNum[reg1-1], axNum[reg2-1], op);
		else
		    Data[(axAdr[reg1-5]-DATA_BEG)/2]=numLogic(Data[(axAdr[reg2-5]-DATA_BEG)/2], Data[(axAdr[reg2-5]-DATA_BEG)/2], op);
	}

	void regNum (int reg, int num, int op)
	{
		if(reg<5)
			axNum[reg-1]=numLogic(axNum[reg-1], num, op);
		else
	    	Data[(axAdr[reg-5]-DATA_BEG)/2]=numLogic(Data[(axAdr[reg-5]-DATA_BEG)/2], num, op);
	}

	char reg1[OBJLEN/2+1], reg2[OBJLEN/2+1];
	strncpy(reg1, ir+OPERATION, OBJLEN/2);
	strncpy(reg2, ir+OPERATION+OBJLEN/2, OBJLEN/2);
	if(op==3)
	{
		if(strcmp(reg1, "0000")!=0)
			regNum(binTodec(reg1, OBJLEN/2), 0, op);
		else
		    regNum(binTodec(reg2, OBJLEN/2), 0, op);
	}
	else if(strcmp(reg2, "0000")==0)
	{
		char num[NUMLEN+1];
		strncpy(num, (Instruction[ip/4-1])+NUMSTR, NUMLEN); 
		regNum(binTodec(reg1, OBJLEN/2), binTodec(num, NUMLEN), op);
	}
	else
		regReg(binTodec(reg1, OBJLEN/2), binTodec(reg2, OBJLEN/2), op);
}



void cmp (void)
{
	int numCmp (int num1, int num2)
	{
		if(num1>num2)
		    return LARGER;
		else if(num1<num2)
		    return SMALLER;
		else if(num1==num2)
		    return EQUAL;
	}
	
	void regReg (int reg1, int reg2)
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

	void regNum (int reg, int num)
	{
		if(reg<5)
	    	flag=numCmp(axNum[reg-1], num);
		else
	    	flag=numCmp(Data[(axAdr[reg-5]-DATA_BEG)/2], num);
	}
	
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



void jmp (void)
{
	char con[OBJLEN+1], num[NUMLEN+1];
	strncpy(con, ir+OPERATION, OBJLEN);
	strncpy(num, (Instruction[ip/4-1])+NUMSTR, NUMLEN);
	switch(binTodec(con, OBJLEN))
	{
		case 0:
			ip+=binTodec(num, NUMLEN)-4;
			break;
		case 1:
			if(flag==EQUAL)
				ip+=binTodec(num, NUMLEN)-4;
			break;
		case 2:
			if(flag==LARGER)
			    ip+=binTodec(num, NUMLEN)-4;
			break;
		case 3:
			if(flag==SMALLER)
			    ip+=binTodec(num, NUMLEN)-4;
			break;
	}
}



void input (void)
{
	FILE * fp=fopen("dict.txt", "r");
	int i=-1;
	do{
		i++;
		fgets(Instruction[i], MAXLEN, fp);
		fgetc(fp);
	}while(strncmp(Instruction[i], "00000000", 8)!=0); 
	fclose(fp);
}

void inINS (void)
{
	char reg[OBJLEN+1];
	strncpy(reg, ir+OPERATION, OBJLEN);
	int no=binTodec(reg, OBJLEN);
	printf("in:\n");
	if(no>=16)
		no/=16;
	if(no<5)
		scanf("%d", &axNum[no-1]);
	else
	    scanf("%d", &Data[(axAdr[no-5]-DATA_BEG)/2]);
}

void outINS (void)
{
	char reg[OBJLEN+1];
	strncpy(reg, ir+OPERATION, OBJLEN);
	int no=binTodec(reg, OBJLEN);
	printf("out: ");
	if(no>=16)
		no/=16;
	if(no<5)
	    printf("%d\n", axNum[no-1]);
	else
	    printf("%d\n", Data[(axAdr[no-5]-DATA_BEG)/2]);
}

void outputStates (void)
{
	int i;
	printf("ip = %d\n", ip);
	printf("flag = %d\n", flag);
	printf("ir = %d\n", binTodec(ir, OPERATION+OBJLEN));
	for(i=0; i<4; i++)
	    printf((i==3)?"ax%d = %d\n":"ax%d = %d ", i+1, axNum[i]);
	for(i=0; i<4; i++)
	    printf((i==3)?"ax%d = %d\n":"ax%d = %d ", i+5, axAdr[i]);
}

void outputCodeSegment (void)
{
	int i; 
	printf("\ncodeSegment :\n");
	for(i=0; i<128; i++)
	    printf(((i+1)%8==0)?"%d\n":"%d ", binTodec(Instruction[i], MAXLEN-1));
}

void outputDataSegment (void)
{
	int i;
	printf("\ndataSegment :\n");
	for(i=0; i<256; i++)
	    printf(((i+1)%16==0)?"%d\n":"%d ", Data[i]);
}
