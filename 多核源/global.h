extern char Instruction [128][MAXLEN];		//������ڴ棬�ַ�����ά����洢ָ�ÿ��ָ��ռһ��
extern int Data[256];						//���ݶ��ڴ棬���ֽ�Ϊһ��Ԫ��
extern char ir [2][OPERATION+OBJSTR+1];		//ʮ��λָ��Ĵ�����ÿ����һ��
extern int ip [2];							//�����������ÿ����һ��
extern int flag [2];						//��־�Ĵ�����ÿ����һ��
extern int axNum [2][4];					//4��ͨ�����ݼĴ�����ÿ�����ĸ�
extern int axAdr [2][4];					//4��ͨ�õ�ַ�Ĵ�����ÿ�����ĸ�
extern HANDLE mutex[256];					//ִ��ָ��ʱ����Ļ�����󣬷ֱ��Ӧ256�����ݶ��ڴ�Ԫ��
extern HANDLE outMutex;						//�������������ִ�����ָ�������ڴ�״̬��ʱ����Ļ������
extern int binTodec(char*num, int len);		//������תʮ���ƺ���	
