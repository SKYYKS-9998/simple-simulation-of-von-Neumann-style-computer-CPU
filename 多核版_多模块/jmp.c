#include "head.h"
#include "global.h"   	//ȫ�ֱ����Լ�binTodec������תʮ���ƺ��� 

void jmp (int codeID)	//��ת���� 
{
	char con[OBJLEN+1], num[NUMLEN+1];
	strncpy(con, ir[codeID]+OPERATION, OBJLEN); 	//��ָ��Ĵ����Ĳ��������룬������Ҳ������ת����������con�ַ��� 
	/*��ȡ������*/ 
	if(codeID==0)
		strncpy(num, (Instruction[ip[codeID]/4-1])+NUMSTR, NUMLEN); 	//ȡ��ָ���ip������4������ip����4���̼�ȥ1���ǵ�ǰִ�е�ָ����±� 	
	else if(codeID==1)
		strncpy(num, (Instruction[64+ip[codeID]/4-1])+NUMSTR, NUMLEN); 	//2�ź˵�ָ���Instruction[64]��ʼ�洢
	switch(binTodec(con, OBJLEN))		//��con�ַ���ת��Ϊʮ���������õ���ת������չ��������� 
	{
		/*0Ϊ��������ת��1�򵱱�־�Ĵ���flagΪ0ʱ��ת��2��flagΪ1ʱ��ת��3��flagΪ2ʱ��ת*/ 
		/*���������ip����ʮ�����������ټ�ȥ4�������ת*/
		/*ȡָ��ʱ��ip������4��������תʱ����Ҫ��ȥ���4*/
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
