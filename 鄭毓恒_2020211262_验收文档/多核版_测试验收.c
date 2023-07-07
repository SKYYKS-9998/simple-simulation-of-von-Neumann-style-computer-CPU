#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define MAXLEN 33		//�洢ָ������������Ԫ����
#define OPERATION 8		//�����볤��
#define OBJSTR 8		//���������뿪ʼ
#define OBJLEN 8		//���������볤��
#define NUMSTR 16		//��������ʼ
#define NUMLEN 16		//����������
#define DATA_BEG 16384	//���ݶε�ַ��ʼ
#define LARGER 1		//��־�Ĵ���ǰ���ں�
#define EQUAL 0			//����
#define SMALLER -1		//С��

/*�ڶ�ģ���У����º�����Ϊ�ⲿ����*/
void numTransfer (int codeID);  	//���ݴ��ͺ��� 
void arith (int codeID, int op);  	//�������㺯�� 
void logic (int codeID, int op);  	//�߼����㺯�� 
void cmp (int codeID);  			//�Ƚ����㺯�� 
void jmp (int codeID);  			//��ת���� 
void input (int codeID);  			//����ָ�����к��� 
void inINS (int codeID);  			//����ָ��� 
void outINS (int codeID);  			//���ָ��� 
void applyMutex (int codeID);  		//���󻥳������ 
void releaseMutex (int codeID);  	//�ͷŻ�������� 
void sleep (int codeID);   			//���ߺ��� 
void outputStates (int codeID);  	//����Ĵ���״̬���� 
void outputCodeSegment (void);  	//�������κ��� 
void outputDataSegment (void);  	//������ݶκ��� 

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

/*main.c*/ 
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
	WaitForSingleObject(core1, INFINITE);		//�ȴ�1�ź��̵߳��ź�
	CloseHandle(core1);
	WaitForSingleObject(core2, INFINITE);		//�ȴ�2�ź��̵߳��ź�
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
	Data[0]=100; 				//�����ݶ��ڴ�16384��ʼ��ΪƱ�� 
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

/*
һЩģ�麬�о�̬������Ϊ�ڵ�һ.c�ļ���ʵ�����ƹ��ܣ�����Щ��������������ģ����������ǵ��ⲿ������ 
*/

/*transfer.c*/
void numTransfer (int codeID)	//���ݴ��ͺ��� 
{
	void numToreg(int codeID, int dest, int num) //��һ���������������Ĵ��� 
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

	void numToadr (int codeID, int src, int dest)	//�����ݴ����ݼĴ������͵���ַ�Ĵ�����ָ����ڴ浥Ԫ 
	{
		Data[(axAdr[codeID][dest-5]-DATA_BEG)/2]=axNum[codeID][src-1];   
	}

	void adrTonum (int codeID, int src, int dest)	//�����ݴӵ�ַ�Ĵ�����ָ����ڴ浥Ԫ���͵����ݼĴ��� 
	{
		axNum[codeID][dest-1]=Data[(axAdr[codeID][src-5]-DATA_BEG)/2];
	}
	
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

/*arithmetic.c*/
void arith (int codeID, int op)	//�������㺯�� 
{
	int calculator (int num1, int num2, int op)				//�������㺯�� 
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
	
	/*
	��ַ�Ĵ����洢�����ڴ��ַ�������ݶ��ڴ��16384��ʼ��Ҳ����ȫ�ֳ���DATA_BEG
	�����ֽ�һ����Ԫ����ÿ�����ֽڣ���ʾ���ݶ��ڴ������Data��Ԫ���±��1
	��ַ�Ĵ����洢���ڴ��ַ��ȥDATA_BEG���ٳ���2����Ϊ���ڴ��ַ��Data����������Ӧ��Ԫ���±� 
	*/

	void regReg (int codeID, int reg1, int reg2, int op)		//�Ĵ�����Ĵ������洢�����ݽ����������� 
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

	void regNum (int codeID, int reg, int num, int op)		//�Ĵ������洢�������������������������� 
	{
		/*���ݼĴ�������жϼĴ�����𣬽��������������ݸ�calculator����������������*/
		if(reg<5)  					//�Ĵ���Ϊ���ݼĴ��� 
			axNum[codeID][reg-1]=calculator(axNum[codeID][reg-1], num, op);
		else  						//�Ĵ���Ϊ��ַ�Ĵ��� 
			Data[(axAdr[codeID][reg-5]-DATA_BEG)/2]=calculator(Data[(axAdr[codeID][reg-5]-DATA_BEG)/2], num, op);
	}
	
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

/*logic.c*/
void logic (int codeID, int op)	//�߼����㺯�� 
{
	int numLogic (int num1, int num2, int op)			//�߼����㺯�� 
	{
		/*op��ֵ�������߼���������1Ϊ�룬2Ϊ��3Ϊ�ǡ�����������*/
		switch(op)
		{
			case 1:
				return (num1&&num2);
			case 2:
			    return (num1||num2);
			case 3:
			    return !(num1);
		}
	}

	/*
	��ַ�Ĵ����洢�����ڴ��ַ�������ݶ��ڴ��16384��ʼ��Ҳ����ȫ�ֳ���DATA_BEG
	�����ֽ�һ����Ԫ����ÿ�����ֽڣ���ʾ���ݶ��ڴ������Data��Ԫ���±��1
	��ַ�Ĵ����洢���ڴ��ַ��ȥDATA_BEG���ٳ���2����Ϊ���ڴ��ַ��Data����������Ӧ��Ԫ���±� 
	*/

	void regReg (int codeID, int reg1, int reg2, int op)	//�Ĵ�����Ĵ������洢�����ݽ����߼����� 
	{
		/*���ݼĴ�������жϼĴ�����𣬽��������������ݸ�numLogic���������߼�����*/
		if(reg1<5 && reg2>=5)		//reg1Ϊ���ݼĴ�����reg2Ϊ��ַ�Ĵ��� 
			axNum[codeID][reg1-1]=numLogic(axNum[codeID][reg1-1], Data[(axAdr[codeID][reg2-5]-DATA_BEG)/2], op);
		else if(reg1>=5 && reg2<5)	//reg1Ϊ��ַ�Ĵ�����reg2Ϊ���ݼĴ��� 
			Data[(axAdr[codeID][reg1-5]-DATA_BEG)/2]=numLogic(Data[(axAdr[codeID][reg1-5]-DATA_BEG)/2], axNum[codeID][reg2-1], op);
		else if(reg1<5 && reg2<5)	//reg1��reg2��Ϊ���ݼĴ��� 
			axNum[codeID][reg1-1]=numLogic(axNum[codeID][reg1-1], axNum[codeID][reg2-1], op);
		else						//reg1��reg2��Ϊ��ַ�Ĵ��� 
		    Data[(axAdr[codeID][reg1-5]-DATA_BEG)/2]=numLogic(Data[(axAdr[codeID][reg2-5]-DATA_BEG)/2], Data[(axAdr[codeID][reg2-5]-DATA_BEG)/2], op);
	}

	void regNum (int codeID, int reg, int num, int op)	//�Ĵ������洢�������������������߼����� 
	{
		/*���ݼĴ�������жϼĴ�����𣬽��������������ݸ�numLogic���������߼�����*/
		if(reg<5)	//�Ĵ���Ϊ���ݼĴ��� 
			axNum[codeID][reg-1]=numLogic(axNum[codeID][reg-1], num, op);
		else		//�Ĵ���Ϊ��ַ�Ĵ��� 
		    Data[(axAdr[codeID][reg-5]-DATA_BEG)/2]=numLogic(Data[(axAdr[codeID][reg-5]-DATA_BEG)/2], num, op);
	}

	char reg1[OBJLEN/2+1], reg2[OBJLEN/2+1];
	strncpy(reg1, ir[codeID]+OPERATION, OBJLEN/2);				//��ָ��Ĵ���ir�ĵ�һ���������󸳸�reg1 
	strncpy(reg2, ir[codeID]+OPERATION+OBJLEN/2, OBJLEN/2);		//��ָ��Ĵ���ir�ĵڶ����������󸳸�reg2 
	if(op==3)  							//op����3��������߼������㡣ֻ��Ҫ�Ե�һ�Ĵ������洢�����ݽ����߼������� 
	{
		/*�����ж�����Ŀ��Ĵ�������Լ���ȡ��������ֱ�ӵ���regNum����ʮ����Ŀ��Ĵ�����Ŵ��ݸ���������������Ҫһ��int num�βΣ����Դ���0*/
		if(strcmp(reg1, "0000")!=0) 	//reg1ΪĿ��Ĵ��� 
			regNum(codeID, binTodec(reg1, OBJLEN/2), 0, op);  
		else 							//reg2ΪĿ��Ĵ��� 
		    regNum(codeID, binTodec(reg2, OBJLEN/2), 0, op);
	}
	else if(strcmp(reg2, "0000")==0)	//reg2Ϊ0�����������������Ĵ������洢����ֵ�����߼�����
	{
		char num[NUMLEN+1];//�洢ʮ��λ������������ 
		if(codeID==0)
			strncpy(num, (Instruction[ip[codeID]/4-1])+NUMSTR, NUMLEN); 		//ȡ��ָ���ip������4������ip����4���̼�ȥ1���ǵ�ǰִ�е�ָ����±� 
		else if(codeID==1)
			strncpy(num, (Instruction[64+ip[codeID]/4-1])+NUMSTR, NUMLEN); 		//2�ź˵�ָ���Instruction[64]��ʼ�洢
		regNum(codeID, binTodec(reg1, OBJLEN/2), binTodec(num, NUMLEN), op);	//����regNum���������Ĵ�����ź�������ת��Ϊʮ�����������βη�ʽ���͵ĺ��� 
	}
	else
		regReg(codeID, binTodec(reg1, OBJLEN/2), binTodec(reg2, OBJLEN/2), op);	//���Ĵ������ת��ʮ�����������ݸ��������к������� 
}

/*cmp.c*/
void cmp (int codeID)	//�Ƚ����㺯�� 
{
	int numCmp (int num1, int num2)	//�Ƚ����㺯�� 
	{
		/*��ȫ�ֳ�����ʾ��numCmp�������رȽ���������Ӧ�ĳ���*/
		if(num1>num2)
	    	return LARGER;
		else if(num1<num2)
	    	return SMALLER;
		else if(num1==num2)
	    	return EQUAL;
	}
	
	/*
	��ַ�Ĵ����洢�����ڴ��ַ�������ݶ��ڴ��16384��ʼ��Ҳ����ȫ�ֳ���DATA_BEG
	�����ֽ�һ����Ԫ����ÿ�����ֽڣ���ʾ���ݶ��ڴ������Data��Ԫ���±��1
	��ַ�Ĵ����洢���ڴ��ַ��ȥDATA_BEG���ٳ���2����Ϊ���ڴ��ַ��Data����������Ӧ��Ԫ���±� 

	numCmp�����ķ���ֵ��ֱ�Ӹ�ֵ����־�Ĵ��� 
	*/

	void regReg (int codeID, int reg1, int reg2)	//�Ĵ�����Ĵ������洢�����ݽ��бȽ����� 
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

	void regNum (int codeID, int reg, int num)	//�Ĵ������洢�����������������бȽ����� 
	{
		/*���ݼĴ�������жϼĴ�����𣬽��������������ݸ�numCmp�������бȽ�����*/
		if(reg<5)	//�Ĵ���Ϊ���ݼĴ��� 
	    	flag[codeID]=numCmp(axNum[codeID][reg-1], num);
		else		//�Ĵ���Ϊ��ַ�Ĵ��� 
	    	flag[codeID]=numCmp(Data[(axAdr[codeID][reg-5]-DATA_BEG)/2], num);
	}

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
/*jmp.c*/
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

/*IO.c*/
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
	ReleaseMutex(outMutex);						//�ͷ�outMutex������� 	//�ַ���reg�洢�˰�λ�����ƣ����Ŀ��Ĵ����ı��λ��ǰ��λ����no���ڻ����2���Ĵ��ݣ�16����no��Ҫ����16�Եõ���ȷ��Ŀ��Ĵ������ 
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

/*multicore.c*/ 
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
