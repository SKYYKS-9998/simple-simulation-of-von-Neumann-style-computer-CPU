#include "head.h"
#include "global.h"   //ȫ�ֱ����Լ�binTodec������תʮ���ƺ��� 

static void regReg (int codeID, int reg1, int reg2);	//�Ĵ�����Ĵ������洢�����ݽ��бȽ����� 
static void regNum (int codeID, int reg, int num);		//�Ĵ������洢�����������������бȽ����� 
static int numCmp (int num1, int num2);					//�Ƚ����㺯�� 

void cmp (int codeID)	//�Ƚ����㺯�� 
{
	char reg1[OBJLEN/2+1], reg2[OBJLEN/2+1];
	strncpy(reg1, ir[codeID]+OPERATION, OBJLEN/2);			//��ָ��Ĵ���ir�ĵ�һ���������󸳸�reg1 
	strncpy(reg2, ir[codeID]+OPERATION+OBJLEN/2, OBJLEN/2);	//��ָ��Ĵ���ir�ĵڶ����������󸳸�reg2 
	if(strcmp(reg2, "0000")==0)			//reg2Ϊ0�����������������Ĵ������洢����ֵ���бȽ�����
	{
		char num[NUMLEN+1];				//�洢ʮ��λ������������ 
		if(codeID==0)
			strncpy(num, (Instruction[ip[codeID]/4-1])+NUMSTR, NUMLEN); 	//ȡ��ָ���ip������4������ip����4���̼�ȥ1���ǵ�ǰִ�е�ָ����±� 
		else if(codeID==1)
			strncpy(num, (Instruction[64+ip[codeID]/4-1])+NUMSTR, NUMLEN); 	//2�ź˵�ָ���Instruction[64]��ʼ�洢
		regNum(codeID, binTodec(reg1, OBJLEN/2), binTodec(num, NUMLEN));	//����regNum���������Ĵ�����ź�������ת��Ϊʮ�����������βη�ʽ���͵ĺ��� 
	}
	else
	    regReg(codeID, binTodec(reg1, OBJLEN/2), binTodec(reg2, OBJLEN/2));	//���Ĵ������ת��ʮ�����������ݸ��������к������� 
}

/*
��ַ�Ĵ����洢�����ڴ��ַ�������ݶ��ڴ��16384��ʼ��Ҳ����ȫ�ֳ���DATA_BEG
�����ֽ�һ����Ԫ����ÿ�����ֽڣ���ʾ���ݶ��ڴ������Data��Ԫ���±��1
��ַ�Ĵ����洢���ڴ��ַ��ȥDATA_BEG���ٳ���2����Ϊ���ڴ��ַ��Data����������Ӧ��Ԫ���±� 

numCmp�����ķ���ֵ��ֱ�Ӹ�ֵ����־�Ĵ��� 
*/

static void regReg (int codeID, int reg1, int reg2)	//�Ĵ�����Ĵ������洢�����ݽ��бȽ����� 
{
	/*���ݼĴ�������жϼĴ�����𣬽��������������ݸ�numCmp�������бȽ�����*/
	if(reg1<5 && reg2>=5)		//reg1Ϊ���ݼĴ�����reg2Ϊ��ַ�Ĵ��� 
		flag[codeID]=numCmp(axNum[codeID][reg1-1], Data[(axAdr[codeID][reg2-5]-DATA_BEG)/2]);
	else if(reg1>=5 && reg2<5)	//reg1Ϊ��ַ�Ĵ�����reg2Ϊ���ݼĴ��� 
		flag[codeID]=numCmp(Data[(axAdr[codeID][reg1-5]-DATA_BEG)/2], axNum[codeID][reg2-1]);
	else if(reg1<5 && reg2<5)	//reg1��reg2��Ϊ���ݼĴ��� 
		flag[codeID]=numCmp(axNum[codeID][reg1-1], axNum[codeID][reg2-1]);
	else						//reg1��reg2��Ϊ��ַ�Ĵ��� 
		flag[codeID]=numCmp(Data[(axAdr[codeID][reg1-5]-DATA_BEG)/2], Data[(axAdr[codeID][reg2-5]-DATA_BEG)/2]);
}

static void regNum (int codeID, int reg, int num)	//�Ĵ������洢�����������������бȽ����� 
{
	/*���ݼĴ�������жϼĴ�����𣬽��������������ݸ�numCmp�������бȽ�����*/
	if(reg<5)	//�Ĵ���Ϊ���ݼĴ��� 
	    flag[codeID]=numCmp(axNum[codeID][reg-1], num);
	else		//�Ĵ���Ϊ��ַ�Ĵ��� 
	    flag[codeID]=numCmp(Data[(axAdr[codeID][reg-5]-DATA_BEG)/2], num);
}

static int numCmp (int num1, int num2)	//�Ƚ����㺯�� 
{
	/*��ȫ�ֳ�����ʾ��numCmp�������رȽ���������Ӧ�ĳ���*/
	if(num1>num2)
	    return LARGER;
	else if(num1<num2)
	    return SMALLER;
	else if(num1==num2)
	    return EQUAL;
}
