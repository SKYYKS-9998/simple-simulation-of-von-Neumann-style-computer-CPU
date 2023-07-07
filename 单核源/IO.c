#include <stdio.h>
#include "head.h"
#include "global.h"

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
