#include "head.h"
#include "global.h"   //ȫ�ֱ����Լ�binTodec������תʮ���ƺ��� 

static void numToreg(int codeID, int dest, int num);  	//��һ���������������Ĵ��� 
static void numToadr (int codeID, int src, int dest);  	//�����ݴ����ݼĴ������͵���ַ�Ĵ�����ָ����ڴ浥Ԫ 
static void adrTonum (int codeID, int src, int dest);  	//�����ݴӵ�ַ�Ĵ�����ָ����ڴ浥Ԫ���͵����ݼĴ��� 

void numTransfer (int codeID)	//���ݴ��ͺ��� 
{
	char dest[OBJLEN/2+1], src[OBJLEN/2+1];  				//destΪ����Ŀ�꣬srcΪ������� 
	strncpy(dest, ir[codeID]+OPERATION, OBJLEN/2);  		//��ָ��Ĵ���ir�ĵ�һ���������󸳸�dest 
	strncpy(src, ir[codeID]+OPERATION+OBJLEN/2, OBJLEN/2);  //��ָ��Ĵ���ir�ĵڶ����������󸳸�src 
	if(strcmp(src, "0000")==0)  //srcΪ0�����������㲻��һ���Ĵ������������� 
	{
		char num[NUMLEN+1];		//�洢ʮ��λ������������ 
		if(codeID==0)
			strncpy(num, (Instruction[ip[codeID]/4-1])+NUMSTR, NUMLEN);  		//ȡ��ָ���ip������4������ip����4���̼�ȥ1���ǵ�ǰִ�е�ָ����±� 
		else if(codeID==1)
			strncpy(num, (Instruction[64+ip[codeID]/4-1])+NUMSTR, NUMLEN); 		//2�ź˵�ָ���Instruction[64]��ʼ�洢
		numToreg(codeID, binTodec(dest, OBJLEN/2), binTodec(num, NUMLEN));  	//����numToreg���������Ĵ�����ź�������ת��Ϊʮ�����������βη�ʽ���͸����� 
	}
	else if(strcmp(dest, src)>0)//dest��src��֤�������ݼĴ������͵���ַ�Ĵ�����ָ����ڴ浥Ԫ 
		numToadr(codeID, binTodec(src, OBJLEN/2), binTodec(dest, OBJLEN/2));  	//���Ĵ������ת��ʮ�����������ݸ��������к������� 
	else 
	    adrTonum(codeID, binTodec(src, OBJLEN/2), binTodec(dest, OBJLEN/2));
}

static void numToreg(int codeID, int dest, int num) //��һ���������������Ĵ��� 
{
	if(dest>=5)  									//dest���ڻ����5������Ŀ��Ĵ���Ϊ��ַ�Ĵ��� 
		axAdr[codeID][dest-5]=num;
	else if(dest<=4)  								//����Ŀ��Ĵ���Ϊ���ݼĴ��� 
	    axNum[codeID][dest-1]=num;
}

/*
��ַ�Ĵ����洢�����ڴ��ַ�������ݶ��ڴ��16384��ʼ��Ҳ����ȫ�ֳ���DATA_BEG
�����ֽ�һ����Ԫ����ÿ�����ֽڣ���ʾ���ݶ��ڴ������Data��Ԫ���±��1
��ַ�Ĵ����洢���ڴ��ַ��ȥDATA_BEG���ٳ���2����Ϊ���ڴ��ַ��Data����������Ӧ��Ԫ���±� 
*/

static void numToadr (int codeID, int src, int dest)	//�����ݴ����ݼĴ������͵���ַ�Ĵ�����ָ����ڴ浥Ԫ 
{
	Data[(axAdr[codeID][dest-5]-DATA_BEG)/2]=axNum[codeID][src-1];   
}

static void adrTonum (int codeID, int src, int dest)	//�����ݴӵ�ַ�Ĵ�����ָ����ڴ浥Ԫ���͵����ݼĴ��� 
{
	axNum[codeID][dest-1]=Data[(axAdr[codeID][src-5]-DATA_BEG)/2];
}
