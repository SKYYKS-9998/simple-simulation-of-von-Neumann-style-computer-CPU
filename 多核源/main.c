#include <stdio.h>
#include <stdlib.h>
#include "head.h"	//ȫ�ֳ����Լ�string.h��windows.hͷ�ļ� 

/*�ⲿ��������*/
extern void numTransfer (int codeID);  		//���ݴ��ͺ��� 
extern void arith (int codeID, int op);  	//�������㺯�� 
extern void logic (int codeID, int op);  	//�߼����㺯�� 
extern void cmp (int codeID);  				//�Ƚ����㺯�� 
extern void jmp (int codeID);  				//��ת���� 
extern void input (int codeID);  			//����ָ�����к��� 
extern void inINS (int codeID);  			//����ָ��� 
extern void outINS (int codeID);  			//���ָ��� 
extern void applyMutex (int codeID);  		//���󻥳������ 
extern void releaseMutex (int codeID);  	//�ͷŻ�������� 
extern void sleep (int codeID);   			//���ߺ��� 
extern void outputStates (int codeID);  	//����Ĵ���״̬���� 
extern void outputCodeSegment (void);  		//�������κ��� 
extern void outputDataSegment (void);  		//������ݶκ��� 

/*ȫ�ֱ���*/
char Instruction [128][MAXLEN];		//������ڴ棬�ַ�����ά����洢ָ�ÿ��ָ��ռһ��
int Data[256];  					//���ݶ��ڴ棬���ֽ�Ϊһ��Ԫ��
char ir [2][OPERATION+OBJSTR+1];  	//ʮ��λָ��Ĵ�����ÿ����һ��
int ip [2];  						//�����������ÿ����һ��
int flag [2];  						//��־�Ĵ�����ÿ����һ��
int axNum [2][4];  					//4��ͨ�����ݼĴ�����ÿ�����ĸ�
int axAdr [2][4];  					//4��ͨ�õ�ַ�Ĵ�����ÿ�����ĸ�
HANDLE mutex[256];  				//ִ��ָ��ʱ����Ļ�����󣬷ֱ��Ӧ256�����ݶ��ڴ�Ԫ��
HANDLE outMutex;  					//�������������ִ�����ָ�������ڴ�״̬��ʱ����Ļ������

static void initialize (void);						//��ʼ��ȫ�ֱ������� 
static void take(int codeID);						//ȡָ��� 
static int command(int codeID);						//�ж�ָ��������ͺ��� 
int binTodec (char * num, int len); 				//������תʮ���ƺ��� 
static DWORD WINAPI startCore (LPVOID lpParameter);	//�߳̿�ʼ���� 

int main(int argc, char *argv[])
{
	int codeID[2]={0,1};	//�����˵ı�ţ�Ϊ����ʹ��ȫ�ֱ������ڳ�����ʹ��0��1��������1��2���������1��2 
	int i;
	initialize();			//��ʼ��ȫ�ֱ��� 
	input(codeID[0]);		//����1�ź˵�ָ������ 
	input(codeID[1]);		//����2�ź˵�ָ������ 
	for(i=0; i<256; i++)
		mutex[i]=CreateMutex(NULL, FALSE, NULL);	//�������������� 
	outMutex=CreateMutex(NULL, FALSE, NULL);
	HANDLE core1=CreateThread(NULL, 0, startCore, &codeID[0], 0, NULL);		//����1�ź��̣߳� ������ź˵ı�ŵĵ�ַLPVOID lpParameter�������Ա����ʹ�� 
	HANDLE core2=CreateThread(NULL, 0, startCore, &codeID[1], 0, NULL);		//����2�ź��߳� 
	WaitForSingleObject(core1, INFINITE);
	CloseHandle(core1);
	WaitForSingleObject(core2, INFINITE);
	CloseHandle(core2); 
	outputCodeSegment();	//���������ڴ� 
	outputDataSegment();	//������ݶ��ڴ� 
	system("pause");
	return 0;
}

static void initialize (void)	//��ʼ��ȫ�ֱ������� 
{
	int i, j;
	for(i=0; i<128; i++)
	    Instruction[i][0]='\0';	//�������ÿһ�е�һ���ַ���ʼ��Ϊ���ַ���ȡָ��ֻ��ȡ����ָ���������ֻ�����0 
	Data[0]=10; 				//�����ݶ��ڴ�16384��ʼ��ΪƱ�� 
	for(i=1; i<256; i++)
		Data[i]=0; 				//���ݶ��ڴ��������ֳ�ʼ��Ϊ0 
	for(j=0; j<2; j++)
	{
		ip[j]=0;				//�����������ʼ��Ϊ0 
		flag[j]=0;				//��־�Ĵ�����ʼ��Ϊ0 
		for(i=0; i<OPERATION+OBJSTR; i++)
	    	ir[j][i]='\0';		//ָ��Ĵ�����ʼ��Ϊ0 
	    for(i=0; i<4; i++){
			axNum[j][i]=0;		//���ݼĴ�����ʼ��Ϊ0 
			axAdr[j][i]=0;  	//��ַ�Ĵ�����ʼ��Ϊ0 
		}
	}
}

static void take(int codeID)	//ȡָ�����int codeIDΪ�˵ı�� 
{
	/*����codeID��ֵ���жϸôӴ�����Ǹ�λ��ȡָ���������ʹ����Щ�Ĵ���*/ 
	if(codeID==0)
		strncpy(ir[codeID], Instruction[ip[codeID]/4], OPERATION+OBJLEN);
	else if(codeID==1)
		strncpy(ir[codeID], Instruction[64+ip[codeID]/4], OPERATION+OBJLEN); 	//2�ź˵�ָ���Instruction[64]��ʼ�洢 
	ip[codeID]+=4;				//�����������4 
}

static int command(int codeID)				//�ж�ָ��������ͺ�����int codeIDΪ�˵ı�� 
{
	char num[OPERATION+1];
	strncpy(num, ir[codeID], OPERATION);	//��ָ��Ĵ�����ǰ��λ�����봫��num�ַ��� 
	return binTodec(num, OPERATION);		//��numת��Ϊʮ���ƺ󷵻أ��ж�ָ��Ĳ������� 
}

int binTodec (char * num, int len)	//������תʮ���ƺ��� 
{
	/*char * num Ϊ�������ַ��������ַ���ַ��int lenΪ�ַ�������*/
	int dec=0;		//�践�ص�ʮ������ 
	int i=0;
	int fac=1;		//����������λ��λȨ 
	int j=0;
	
	if(num[0]=='\0')
	    return 0;	//���ַ���ֱ�ӷ���0 
	
	if(num[0]=='1' && len==NUMLEN)	//��һλ��1��������Ǹ�����ֻ�����������ֲ��и����������жϳ����ǲ���16λ�������� 
	{
		/*����������Ƹ���ת��Ϊʮ����*/
		/*�Ƚ�����������ȥ1*/ 
		if(num[len-1]=='1')			//���ĩλ��1��ֱ�Ӽ�ȥ����Ϊ0 
		    num[len-1]='0';
		else						//���ĩλ����1������ѭ���ҵ������1�������Ϊ0��Ȼ��֮ǰ��0����Ϊ1 
		{
			for(i=len-2; i>=1 && num[i]!='1'; i--);
			num[i]='0';
			for(j=i+1; j<len; j++)
			    num[j]='1';
		}
		for(i=1; i<len; i++) 		//���з������ 
		{
			if(num[i]=='1')
				num[i]='0';
			else
			    num[i]='1';
		}
		for(i=len-1; i>=1; i--) 	//ת��Ϊʮ������ 
		{
			if(num[i]=='0')			//λ��Ϊ0��ֱ������ 
			    continue;
			else
			{
				/*λ��Ϊ1������͸��ѭ���˷��ó���λ��λȨ���ӵ�decʮ������������*/
				for(j=len-1, fac=1; j>i; j--)  
				    fac*=2;
				dec+=fac;
			}
		}
		dec*=-1; 					//��ʮ��������Ϊ���� 
	}
	else 							//���Ƕ����Ƹ�����ֱ��ת��Ϊʮ���� 
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

static DWORD WINAPI startCore (LPVOID lpParameter)	//�߳̿�ʼ���� 
{
	int codeID=(*(int*)lpParameter);				//��lpParameter�õ��˵ı�� 
	do{
		take(codeID); 								//ȡָ��ı�ָ��Ĵ���ir�ͳ��������ip��ֵ 
		switch(command(codeID))  					//�ж�ָ��Ĳ������ 
		{
			case 1:
				numTransfer(codeID); break; 	 	//ִ�����ݴ���ָ�� 
			case 2:
				arith(codeID, 1); break;  			//ִ�мӷ���������ָ�� 
			case 3:
				arith(codeID, 2); break;  			//ִ�м�����������ָ�� 
			case 4:
				arith(codeID, 3); break;  			//ִ�г˷���������ָ�� 
			case 5:
				arith(codeID, 4); break;  			//ִ�г�����������ָ�� 
			case 6:
				logic(codeID, 1); break;  			//ִ���߼�������ָ�� 
			case 7:
				logic(codeID, 2); break;  			//ִ���߼�������ָ�� 
			case 8:
				logic(codeID, 3); break;  			//ִ���߼�������ָ�� 
			case 9:
				cmp(codeID); break;  				//ִ�бȽ�ָ�� 
			case 10:
				jmp(codeID); break;  				//ִ����תָ�� 
			case 11:
				inINS(codeID); break;  				//ִ������ָ�� 
			case 12:
				outINS(codeID); break;  			//ִ�����ָ�� 
			case 13:
				applyMutex(codeID); break;  		//ִ�����󻥳����ָ�� 
			case 14:
				releaseMutex(codeID); break;  		//ִ���ͷŻ������ָ�� 
			case 15:
				sleep(codeID); break;  				//ִ������ָ�� 
		}
		outputStates(codeID);  						//����Ĵ���״̬ 
	}while(strcmp(ir[codeID], "0000000000000000")!=0 && ir[codeID][0]!='\0'); //����ѭ������Ϊָ��Ĵ���ir����ͣ��ָ����Ҳ�Ϊ���ַ��� 
	return;
}
