#include "head.h"
#include "global.h"   //ȫ�ֱ����Լ�binTodec������תʮ���ƺ��� 

static void regReg (int codeID, int reg1, int reg2, int op); 	//�Ĵ�����Ĵ������洢�����ݽ����������� 
static void regNum (int codeID, int reg, int num, int op);  	//�Ĵ������洢�������������������������� 
static int calculator (int num1, int num2, int op);  			//�������㺯�� 

void arith (int codeID, int op)	//�������㺯�� 
{
	char reg1[OBJLEN/2+1], reg2[OBJLEN/2+1];
	strncpy(reg1, ir[codeID]+OPERATION, OBJLEN/2);				//��ָ��Ĵ���ir�ĵ�һ���������󸳸�reg1 
	strncpy(reg2, ir[codeID]+OPERATION+OBJLEN/2, OBJLEN/2);		//��ָ��Ĵ���ir�ĵڶ����������󸳸�reg2 
	if(strcmp(reg2, "0000")==0)	//reg2Ϊ0�����������������Ĵ������洢����ֵ������������ 
	{
		char num[NUMLEN+1];		//�洢ʮ��λ������������ 
		if(codeID==0)
			strncpy(num, (Instruction[ip[codeID]/4-1])+NUMSTR, NUMLEN); 		//ȡ��ָ���ip������4������ip����4���̼�ȥ1���ǵ�ǰִ�е�ָ����±� 
		else if(codeID==1)
			strncpy(num, (Instruction[64+ip[codeID]/4-1])+NUMSTR, NUMLEN); 		//2�ź˵�ָ���Instruction[64]��ʼ�洢
		regNum(codeID, binTodec(reg1, OBJLEN/2), binTodec(num, NUMLEN), op);  	//����regNum���������Ĵ�����ź�������ת��Ϊʮ�����������βη�ʽ���͵ĺ��� 
	}
	else
	    regReg(codeID, binTodec(reg1, OBJLEN/2), binTodec(reg2, OBJLEN/2), op); //���Ĵ������ת��ʮ�����������ݸ��������к������� 
}

/*
��ַ�Ĵ����洢�����ڴ��ַ�������ݶ��ڴ��16384��ʼ��Ҳ����ȫ�ֳ���DATA_BEG
�����ֽ�һ����Ԫ����ÿ�����ֽڣ���ʾ���ݶ��ڴ������Data��Ԫ���±��1
��ַ�Ĵ����洢���ڴ��ַ��ȥDATA_BEG���ٳ���2����Ϊ���ڴ��ַ��Data����������Ӧ��Ԫ���±� 
*/

static void regReg (int codeID, int reg1, int reg2, int op)		//�Ĵ�����Ĵ������洢�����ݽ����������� 
{
	/*���ݼĴ�������жϼĴ�����𣬽��������������ݸ�calculator����������������*/
	if(reg1<5 && reg2>=5)  		//reg1Ϊ���ݼĴ�����reg2Ϊ��ַ�Ĵ��� 
		axNum[codeID][reg1-1]=calculator(axNum[codeID][reg1-1], Data[(axAdr[codeID][reg2-5]-DATA_BEG)/2], op);
	else if(reg1>=5 && reg2<5)  //reg1Ϊ��ַ�Ĵ�����reg2Ϊ���ݼĴ��� 
	    Data[(axAdr[codeID][reg1-5]-DATA_BEG)/2]=calculator(Data[(axAdr[codeID][reg1-5]-DATA_BEG)/2], axNum[codeID][reg2-1], op);
	else if(reg1<5 && reg2<5)  	//reg1��reg2��Ϊ���ݼĴ��� 
	    axNum[codeID][reg1-1]=calculator(axNum[codeID][reg1-1], axNum[codeID][reg2-1], op);
	else  						//reg1��reg2��Ϊ��ַ�Ĵ��� 
	    Data[(axAdr[codeID][reg1-5]-DATA_BEG)/2]=calculator(Data[(axAdr[codeID][reg1-5]-DATA_BEG)/2], Data[(axAdr[codeID][reg2-5]-DATA_BEG)/2], op);
}

static void regNum (int codeID, int reg, int num, int op)		//�Ĵ������洢�������������������������� 
{
	/*���ݼĴ�������жϼĴ�����𣬽��������������ݸ�calculator����������������*/
	if(reg<5)  					//�Ĵ���Ϊ���ݼĴ��� 
		axNum[codeID][reg-1]=calculator(axNum[codeID][reg-1], num, op);
	else  						//�Ĵ���Ϊ��ַ�Ĵ��� 
		Data[(axAdr[codeID][reg-5]-DATA_BEG)/2]=calculator(Data[(axAdr[codeID][reg-5]-DATA_BEG)/2], num, op);
}

static int calculator (int num1, int num2, int op)				//�������㺯�� 
{
	/*op��ֵ������������������1Ϊ�ӷ���2Ϊ������3Ϊ�˷���4Ϊ����������������*/
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
