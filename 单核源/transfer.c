#include "head.h"
#include "global.h"

static void numToreg(int dest, int num);
static void numToadr (int src, int dest);
static void adrTonum (int src, int dest);

void numTransfer (void)
{
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

static void numToreg(int dest, int num)
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
