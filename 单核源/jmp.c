#include "head.h"
#include "global.h"

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
