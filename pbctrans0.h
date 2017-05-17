
typedef struct tag_Test
{
	BYTE cmdNo;        //CM_TEST����
	BYTE CustType;     //�ͻ�����
	UINT16 EquipID;    //�豸���
}TestMsg;
typedef struct tag_StationStatusReq
{
	BYTE cmdNo;				//CM_STATIONSTATUSReq����վ̨״̬��Ϣ
	UINT16 StationNo;
}StationStatusReq;
typedef struct tag_StationStatus
{
	BYTE cmdNo;				//CM_STATIONSTATUS վ̨״̬��Ϣ
	UINT16 StationNo;
	BYTE BusPits;			//ͣ��λ��
	BYTE BusWaiting;		//�ȴ�������
	UINT16 BusId1;			//��λ1�ϵĹ�����ID 
	UINT16 BusId2;			//��λ2�ϵĹ�����ID
	UINT16 BusId3;			//��λ3�ϵĹ�����ID
	UINT16 BusId4;			//��λ4�ϵĹ�����ID
	UINT16 BusId5;			//��λ5�ϵĹ�����ID
}StationStatus;
typedef struct tag_BusStatusReq
{
	BYTE cmdNo;				//CM_BUSSTATUSREQ,			//���󹫽���״̬��Ϣ
	UINT16 BusNo;
}BusStatusReq;
typedef struct tag_BusStatus
{
	BYTE cmdNo;				//CM_BUSSTATUS,			//������״̬��Ϣ
	UINT16 BusNo;
	double Longitude;		//����
	double Latitude;		//γ��
	BYTE Direction;			//��������/���У�
	float Velocity;			//��ʱ��ʻ�ٶ�
	UINT32 UTC;				//ʱ��
	UINT16 Passengers;		//�˿�����
}BusStatus;
typedef struct tag_StationInitReq
{
	BYTE cmdNo;				//CM_STATIONINITREQ		//վ̨��ʼ������
	UINT16 StationNo;
}StationInitReq;
typedef struct tag_StationInit
{
	BYTE cmdNo;				//CM_STATIONINIT			//վ̨��ʼ����Ϣ
	UINT16 StationNo;
	BYTE BusPits;			//ͣ��λ��
	BYTE BusLines;			//ͣ���Ĺ�����·��
	double Longitude;		//����
	double Latitude;		//γ��
	UINT16 BusLineIDs[20];	//ͣ���Ĺ�����·
	char Name[40];			//��վ��
	int  Reserved;			//����
}StationInit;
typedef struct tag_BusInitReq
{
	BYTE cmdNo;				//CM_BUSINITREQ,				//��������ʼ������
	BYTE Auxiliary;       //�������
	UINT16 BusNo;
}BusInitReq;
typedef struct tag_BusInit
{
	BYTE cmdNo;				//CM_BUSINITREQ,				//��������ʼ��
	UINT16 BusNo;
	char NumberPlate[10];     //���ƺ�
	UINT16 BusLineID;		//�����ߺ�
	UINT16 CarryCap;		//���������
	char BusLineName[10];	//������·��
	int  Reserved;			//����
}BusInit;
typedef struct tag_BusComing
{
	BYTE cmdNo;			//CM_BUSCOMING,				//��������վͨ��
	UINT16 StationNo;
	UINT16 BusID;		//������
	BYTE Direction;		//��������/���У�
	UINT32 UTC;			//ʱ��
	UINT16 Passengers;	//�˿�����
}BusComing;
typedef struct tag_BusStopping
{
	BYTE cmdNo;			//CM_BUSSTOPPing,			���������ﳵλͨ��
	UINT16 StationNo;
	UINT16 BusID;		//������
	BYTE Direction;		//��������/���У�
	BYTE BusPit;		//��λ��
	UINT32 UTC;			//ʱ��
	UINT16 Passengers;	//�˿�����
}BusStopping;
typedef struct tag_BusLeaving
{
	BYTE cmdNo;			//CM_BUSLEAVING,				//��������վͨ��
	UINT16 StationNo;
	UINT16 BusID;		//������
	BYTE Direction;		//��������/���У�
	BYTE BusPit;		//��λ��
	UINT32 UTC;			//ʱ��
	UINT16 Passengers;	//�˿�����
}BusLeaving;
typedef struct tag_BusRunning
{
	BYTE cmdNo;			//CM_BUSRUNNING,		//������վ��ͨ��
	UINT16 BusID;		//������
	BYTE Direction;		//��������/���У�
	double Longitude;	//����
	double Latitude;	//γ��
	float Velocity;		//��ʱ��ʻ�ٶ�
	UINT32 UTC;			//ʱ��
	UINT16 Passengers;	//�˿�����
}BusRunning;
typedef struct tag_Login
{
	BYTE cmdNo;			//CM_LOGON,					//��¼
	BYTE CustType;		//�ͻ�����	0=վ̨, 1=BUS, 2=����
	UINT16 EquipID;		//�豸���
	char UserID[20];
	char Password[20];
}Login;
typedef struct tag_Logout
{
	BYTE cmdNo;			//CM_LOGOUT,					//�ǳ�
	BYTE CustType;		//�ͻ�����	0=վ̨, 1=BUS, 2=����
	UINT16 EquipID;		//�豸���
	char UserID[20];
	char Password[20];
}Logout;
typedef struct tag_ChangePassword
{
	BYTE cmdNo;			//CM_CHANGEPASSWORD,			//�޸ĵ�¼����
	BYTE CustType;		//�ͻ�����	0=վ̨, 1=BUS, 2=����
	UINT16 EquipID;		//�豸���
	char UserID[20];
	char NewPassword[20];
}ChangePassword;
typedef struct tag_AlarmMsg
{
	BYTE cmdNo;         //CM_ALARMMSG,		//�쳣������Ϣ
	BYTE CustType;		//�ͻ�����	0=վ̨, 1=BUS, 2=����
	UINT16 EquipID;		//�豸���
	UINT32 aux;			//�����ֽڣ����������
	UINT32 Status;		//�쳣״̬�֣����ء���Դ��ͨѶ�ڵȸ����豸״̬�����������
}AlarmMsg;
typedef struct tag_SysTimeReq
{
	BYTE cmdNo;			//CM_SYSTIMEREQ    ����ϵͳʱ��
	BYTE CustType;		//�ͻ�����	0=վ̨, 1=BUS, 2=����
	UINT16 EquipID;		//�豸���
	UINT32 aux;			//�����ֽڣ����������
}SysTimeReq;
typedef struct tag_SysTime
{
	BYTE cmdNo;				//CM_SYSTIME    ϵͳʱ��
	BYTE CustType;			//�ͻ�����	0=վ̨, 1=BUS, 2=����
	UINT16 EquipID;			//�豸���
	UINT32 aux;				//�����ֽڣ����������
	UINT32 UTC;				//ʱ��
}SysTime;
typedef struct tag_BusLineReq
{
	BYTE cmdNo;				//CM_BUSLINEREQ    ���󹫽���·
	BYTE CustType;			//�ͻ�����	0=վ̨, 1=BUS, 2=����
	UINT16 EquipID;			//�豸���
	UINT16 BusLineID;		//�����ߺ�
}BusLineReq;
typedef struct tag_BusLine
{
	BYTE cmdNo;				//CM_BUSLINE    ������·��Ϣ
	UINT16 BusLineID;		//�����ߺ�
	char BusLineName[10];	//������·��
	UINT16 Stations[50];	//������վ̨�� 
	UINT32 Reserved;
}BusLine;

