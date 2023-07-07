#include "head.h"
#include "global.h"   //ȫ�ֱ����Լ�binTodec������תʮ���ƺ��� 

void applyMutex (int codeID)	//���󻥳������ 
{ 
	char num[NUMLEN+1];		//�洢ʮ��λ������������ 
	if(codeID==0)
		strncpy(num, (Instruction[ip[codeID]/4-1])+NUMSTR, NUMLEN);		//ȡ��ָ���ip������4������ip����4���̼�ȥ1���ǵ�ǰִ�е�ָ����±� 
	else if(codeID==1)
		strncpy(num, (Instruction[64+ip[codeID]/4-1])+NUMSTR, NUMLEN);	//2�ź˵�ָ���Instruction[64]��ʼ�洢
	/*
	���������ַ���numת��Ϊʮ�����������ڴ��ַ
	�ټ�ȥDATA_BEG���ٳ���2����Ϊ���ڴ��ַ��Data����������Ӧ��Ԫ���±꣬Ҳ�Ƕ�Ӧ�Ļ��������mutex�е��±�
	*/ 
	WaitForSingleObject(mutex[(binTodec(num, NUMLEN)-DATA_BEG)/2], INFINITE);	//���󻥳���� 
}

void releaseMutex (int codeID)	//�ͷŻ�������� 
{
	char num[NUMLEN+1];			//�洢ʮ��λ������������ 
	if(codeID==0)
		strncpy(num, (Instruction[ip[codeID]/4-1])+NUMSTR, NUMLEN);		//ȡ��ָ���ip������4������ip����4���̼�ȥ1���ǵ�ǰִ�е�ָ����±� 
	else if(codeID==1)
		strncpy(num, (Instruction[64+ip[codeID]/4-1])+NUMSTR, NUMLEN);	//2�ź˵�ָ���Instruction[64]��ʼ�洢
	/*
	���������ַ���numת��Ϊʮ�����������ڴ��ַ
	�ټ�ȥDATA_BEG���ٳ���2����Ϊ���ڴ��ַ��Data����������Ӧ��Ԫ���±꣬Ҳ�Ƕ�Ӧ�Ļ��������mutex�е��±�
	*/ 
	ReleaseMutex(mutex[(binTodec(num, NUMLEN)-DATA_BEG)/2]); 			//�ͷŻ������ 
}

void sleep (int codeID)		//���ߺ��� 
{
	char num[NUMLEN+1];		//�洢ʮ��λ������������ 
	if(codeID==0)
		strncpy(num, (Instruction[ip[codeID]/4-1])+NUMSTR, NUMLEN); 	//ȡ��ָ���ip������4������ip����4���̼�ȥ1���ǵ�ǰִ�е�ָ����±� 
	else if(codeID==1)
		strncpy(num, (Instruction[64+ip[codeID]/4-1])+NUMSTR, NUMLEN); 	//2�ź˵�ָ���Instruction[64]��ʼ�洢
	Sleep(binTodec(num, NUMLEN));	//�����������ַ���numת��Ϊʮ����������ΪӦ���ߵĺ����� 
}
