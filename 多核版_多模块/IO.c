#include <stdio.h>
#include "head.h"
#include "global.h"   	//ȫ�ֱ����Լ�binTodec������תʮ���ƺ��� 

void input (int codeID)	//����ָ�����к��� 
{
	FILE * fp;
	int i;
	/*��ָ����ļ����������뵽Instruction��ά�ַ�����*/
	if(codeID==0)  
	{
		fp=fopen("dict1.dic", "r");	//1�ź˵�ָ��������dict1.dic 
		/*��������ѭ���Ľ���������Ҫ�жϵ�ǰ��ȡ��ָ�����ѭ��ָ��iֻ��������һ��ָ��ǰ�����������뿪ʼʱiӦΪ0�������ȸ�ֵ-1*/ 
		i=-1;  
		do{
			i++;
			fgets(Instruction[i], MAXLEN, fp);
			fgetc(fp);	//�������з� 
		}while(strncmp(Instruction[i], "00000000", 8)!=0);	//��ǰ�����ָ��ǰ��λ�����벻ȫΪ0��������ͣ��ָ�� 
	}
	else if(codeID==1)
	{
		fp=fopen("dict2.dic", "r");	//2�ź˵�ָ��������dict2.dic
		/*2�ź˵�ָ��Ӵ���ε�ַ256��ʼ�洢��Ҳ����Instruction[64]������i�ȸ�ֵ63*/ 
		i=63; 
		do{
			i++;
			fgets(Instruction[i], MAXLEN, fp);
			fgetc(fp);	//�������з� 
		}while(strncmp(Instruction[i], "00000000", 8)!=0);	//��ǰ�����ָ��ǰ��λ�����벻ȫΪ0��������ͣ��ָ��
	}
	fclose(fp);
}

void inINS (int codeID)	//����ָ��� 
{
	char reg[OBJLEN+1];
	strncpy(reg, ir[codeID]+OPERATION, OBJLEN);	//��ָ��Ĵ����Ĳ������󸳸�reg 
	int no=binTodec(reg, OBJLEN);	//regת��Ϊʮ���ƼĴ�����ţ���ֵ��no 
	/*
	Ϊ��ֹ�������ϻ򱾴����������������ʹ�����ר�õĻ������outMutex
	���ǰ���������󻥳��������ѱ�ռ����һֱ�ȴ��������ɺ��ͷŻ������ 
	*/
	WaitForSingleObject(outMutex, INFINITE);	//����outMutex������� 
	printf("in:\n");
	ReleaseMutex(outMutex);						//�ͷ�outMutex������� 
	//�ַ���reg�洢�˰�λ�����ƣ����Ŀ��Ĵ����ı��λ��ǰ��λ����no���ڻ����2���Ĵ��ݣ�16����no��Ҫ����16�Եõ���ȷ��Ŀ��Ĵ������ 
	if(no>=16)  
		no/=16;
	if(no<5)	//Ŀ��Ĵ���Ϊ���ݼĴ��� 
		scanf("%d", &axNum[codeID][no-1]);
	else		//Ŀ��Ĵ���Ϊ��ַ�Ĵ���
		/*
		��ַ�Ĵ����洢�����ڴ��ַ�������ݶ��ڴ��16384��ʼ��Ҳ����ȫ�ֳ���DATA_BEG
		�����ֽ�һ����Ԫ����ÿ�����ֽڣ���ʾ���ݶ��ڴ������Data��Ԫ���±��1
		��ַ�Ĵ����洢���ڴ��ַ��ȥDATA_BEG���ٳ���2����Ϊ���ڴ��ַ��Data����������Ӧ��Ԫ���±� 
		*/ 
	    scanf("%d", &Data[(axAdr[codeID][no-5]-DATA_BEG)/2]);
}

void outINS (int codeID)	//���ָ��� 
{
	/*
	Ϊ��ֹ�������ϻ򱾴����������������ʹ�����ר�õĻ������outMutex
	���ǰ���������󻥳��������ѱ�ռ����һֱ�ȴ��������ɺ��ͷŻ������ 
	*/
	WaitForSingleObject(outMutex, INFINITE);	//����outMutex������� 
	char reg[OBJLEN+1];
	strncpy(reg, ir[codeID]+OPERATION, OBJLEN);	//��ָ��Ĵ����Ĳ������󸳸�reg 
	int no=binTodec(reg, OBJLEN);				//regת��Ϊʮ���ƼĴ�����ţ���ֵ��no 
	printf("id = %d", codeID+1);				//������˱�� 
	printf("    out: ");
	//�ַ���reg�洢�˰�λ�����ƣ����Ŀ��Ĵ����ı��λ��ǰ��λ����no���ڻ����2���Ĵ��ݣ�16����no��Ҫ����16�Եõ���ȷ��Ŀ��Ĵ������ 
	if(no>=16)
		no/=16;
	if(no<5)	//Ŀ��Ĵ���Ϊ���ݼĴ��� 
	    printf("%d\n", axNum[codeID][no-1]);
	else		//Ŀ��Ĵ���Ϊ��ַ�Ĵ���
		/*
		��ַ�Ĵ����洢�����ڴ��ַ�������ݶ��ڴ��16384��ʼ��Ҳ����ȫ�ֳ���DATA_BEG
		�����ֽ�һ����Ԫ����ÿ�����ֽڣ���ʾ���ݶ��ڴ������Data��Ԫ���±��1
		��ַ�Ĵ����洢���ڴ��ַ��ȥDATA_BEG���ٳ���2����Ϊ���ڴ��ַ��Data����������Ӧ��Ԫ���±� 
		*/ 
	    printf("%d\n", Data[(axAdr[codeID][no-5]-DATA_BEG)/2]);
	ReleaseMutex(outMutex);	//�ͷ�outMutex������� 
}

void outputStates (int codeID)	//����Ĵ���״̬���� 
{
	/*
	Ϊ��ֹ�������ϻ򱾴����������������ʹ�����ר�õĻ������outMutex
	���ǰ���������󻥳��������ѱ�ռ����һֱ�ȴ��������ɺ��ͷŻ������ 
	*/
	WaitForSingleObject(outMutex, INFINITE);	//����outMutex������� 
	int i;
	printf("id = %d\n", codeID+1); 				//����˱�� 
	printf("ip = %d\n", (codeID==0)?ip[codeID]:64*4+ip[codeID]);	//������������ip�������2�ź�����Ҫ����64*4����Ϊ2�ź˵Ĵ���Ӵ����64*4�ֽڿ�ʼ�洢 
	printf("flag = %d\n", flag[codeID]);		//�����־�Ĵ��� 
	printf("ir = %d\n", binTodec(ir[codeID], OPERATION+OBJLEN));	//���ָ��Ĵ��� 
	for(i=0; i<4; i++)
	    printf((i==3)?"ax%d = %d\n":"ax%d = %d ", i+1, axNum[codeID][i]);	//������ݼĴ��� 
	for(i=0; i<4; i++) 
	    printf((i==3)?"ax%d = %d\n":"ax%d = %d ", i+5, axAdr[codeID][i]);  	//�����ַ�Ĵ��� 
	ReleaseMutex(outMutex);		//�ͷ�outMutex������� 
}

void outputCodeSegment (void)	//�������κ��� 
{
	int i; 
	printf("\ncodeSegment :\n");
	for(i=0; i<128; i++)
	    printf(((i+1)%8==0)?"%d\n":"%d ", binTodec(Instruction[i], MAXLEN-1));
}

void outputDataSegment (void)	//������ݶκ��� 
{
	int i;
	printf("\ndataSegment :\n");
	for(i=0; i<256; i++)
	    printf(((i+1)%16==0)?"%d\n":"%d ", Data[i]);
}
