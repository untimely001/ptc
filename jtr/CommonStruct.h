#ifndef COMMONSTRUCT_H
#define COMMONSTRUCT_H

#define MsgFlag 0xAA

//#ifndef __TEST
//#pragma option -b
//#endif
//#pragma pack(push,1)

#define MaxCrossNum 256
#define MaxStepNum 32
#define MaxFlowNum 32
#define MaxDetectorNum 32
#define MaxLampNum 32
#define MaxTimeBandNum 32	//16
#define MaxLaneNum 32
#define MaxRoadNum 1024
#define MaxFlowGroupNum 16    //ÿ·�����ͨ����(��λ)����
#define MaxDatePlanNum 32     //ÿ·��������ڷ���������
#define MaxStepTimePlanNum 32 //�����ݷ�����

//��Ϣ��ʽ����
typedef enum {
	CM_MESSAGE,              //�ն�֮���໥����Ϣ
	CM_CHECKIN,              //���ն������̨����ʶ��
    //�����ǿ���̨���ܿط�����Ϣ
	CM_SCHEDULECHANGED,      //����̨֪ͨ�ܿأ��˹���Ԥ�ƻ����Ѹı�
	CM_TIMETABLECHANGED,     //����̨֪ͨ�ܿأ�ʱ��������Ѹı�
	CM_PLANCHANGED,          //����̨֪ͨ�ܿأ���ʱ�����Ѹı�
	CM_DATEPLANCHANGED,      //����̨֪ͨ�ܿأ�·�����ڷ������Ѹı�
	CM_CROSSCHANGED,         //����̨֪ͨ�ܿأ�·�ڡ�·�ε����Բ����Ѹı�
    //�����ǿ���̨���ܿء��ܿ�����ͨѶ�����������Ͳ�������Ϣ
	CM_STARTREALSTATUS,      //����̨֪ͨ�ܿأ���ʼ��Ҫ·��ʵʱ����
	CM_STOPREALSTATUS,       //����̨֪ͨ�ܿأ�����·��ʵʱ��������
	CM_STARTSLCOUNTDATA,     //����̨֪ͨ�ܿأ���ʼ��Ҫ·�ڵĳ�����������
	CM_STOPSLCOUNTDATA,      //����̨֪ͨ�ܿأ�����·�ڵĽ�ͨ����
	CM_STARTSLFLOWDATA,      //����̨֪ͨ�ܿأ���ʼ��Ҫ·�ڵĽ�ͨ����
	CM_STOPSLFLOWDATA,       //����̨֪ͨ�ܿأ�����·�ڵĽ�ͨ����
	CM_STARTSLAIRDATA,       //����̨֪ͨ�ܿأ���ʼ��Ҫ·�ڵĴ�������
	CM_STOPSLAIRDATA,        //����̨֪ͨ�ܿأ�����·�ڵĴ�������
	CM_STARTSLCURSTATUS,     //����̨֪ͨ�ܿأ���ʼ��Ҫ·�ڵĵ�ǰ״̬����
	CM_STOPSLCURSTATUS,      //����̨֪ͨ�ܿأ�����·�ڵĵ�ǰ״̬����
	CM_GETSLLAMPCONDITION,   //����̨֪ͨ�ܿأ��õ�·�ڻ����źŻ��û�״̬����
    //�����ǿ���̨���ܿء��ܿ�����ͨѶ����������ִ�о����������Ϣ
	CM_ONLINE,               //����ָ��
	CM_OFFLINE,              //����ָ��
	CM_SETCONTROL,           //���ÿ��Ʒ�ʽ
	CM_YELLOWBLINK,          //·�ڻ���ָ��
	CM_ENDYELLOWBLINK,       //��������ָ��
	CM_LAMPOFF,              //·�ڵ�Ϩ��ָ��
	CM_ENDLAMPOFF,           //·�ڵƽ���Ϩ��ָ��
	CM_TIMESYNC,             //ʱ��ͬ��ָ��
	CM_SETCOLOR,             //ǿ�Ƶ�ɫָ��
	CM_ENDCOLOR,             //���ǿ�Ƶ�ɫָ��
	CM_SETSPLIT,             //����·����λ����
	CM_SETCYCLEPLAN,         //����·����ʱ����
	CM_SETTIMEBAND,          //����·��ʱ��α�
	CM_SETSCHEDULE,          //����·�����ڷ���ѡ���
	CM_SETUSEPLAN,           //not used
	CM_SETUSEGROUPPLAN,		 //����·�����з�����
	CM_EXECUTEVIPPLAN,       //ִ��VIP·��
	CM_STOPEXECUTEVIPPLAN,   //ִֹͣ��VIP·��
	CM_SLSTEPNEXT,           //����̨���������źŻ������Ŀ�������
	CM_SLCOMTEST,            //����̨���źŻ����Ͳ�������
    //������ͨѶ�����ܿ��ϴ����ݵ���Ϣ
	CM_FLOWDATA,             //��ͨѶ�����ܿ�̨���͵Ľ�ͨ�����ۼ�����
	CM_VEHICLESTATUS,        //��ͨѶ�����ܿ�̨���͵�ʵʱ����״̬
	CM_LAMPSTATUS,           //��ͨѶ�����ܿ�̨���͵�ʵʱ��ɫ״̬
	CM_SLLAMPCONDITION,      //��ͨѶ�����ܿ�̨���͵��źŵƺû�״̬
	CM_SLCOUNTDATA,          //�����źŻ��ϴ��������������
	CM_SLFLOWDATA,           //�����źŻ��ϴ���ͨ����
	CM_SLAIRDATA,            //�����źŻ��ϴ�·�ڴ�������
	CM_SLCURSTATUS,          //�����źŻ����ص�ǰ�Ĺ���״̬
    //�������ܿ���ͨѶ����������(����·�ڻ�)
	CM_FIRSTSTEPPARAMETER,   //��һ����ͨѶ���·��źŽ��ݿ��Ʋ���
	CM_STEPPARAMETER,        //��ͨѶ���·��źŽ��ݿ��Ʋ���,
		                     //ͨѶ�����յ����������һ�����ڿ�ʼ���иò���
	CM_CHANGECTRLTYPE,       //�ı���Ʒ�ʽ��E:D1-D2����׼����1����2����3
	                         //�յ��������ͨѶ�����´�ָ���е�E:D1-D2��Ϊ
							 //ָ������ֵ�����ӷ���������ȷ�ϸı���ʵ��
	CM_SENDCOMMSTATUS,		
	CM_SETSTEPTABLE,		//���ݱ�����
	CM_GETSTEPLAMPCOLOR,	//�����Ϸ����ݱ�
	CM_SENDSTEPLAMPCOLORITEM,//�Ϸ����ݵ�ɫ
	CM_SETSTEPTIMEPLAN,		//����ʱ������
	CM_GETSTEPTIMEPLAN,	//�����Ϸ�����ʱ��
	CM_SENDSTEPTIMEPLANITEM,	//�Ϸ�����ʱ��
	CM_GETTIMEBANDPLAN,	//�����Ϸ�ʱ��η���
	CM_SENDTIMEBANDPLAN,	//�Ϸ�ʱ��η���
	CM_GETDATAPLAN,	//�����Ϸ����ڷ���
	CM_SENDDATAPLAN,	//�Ϸ����ڷ���
	CM_GETBENCHMARK,	//�����Ϸ�ʱ��
	CM_SENDBENCHMARK,	//�Ϸ�����ʱ��
	CM_LAMPCONDITION,
	CM_DETECTORCONDITION,
	CM_YELLOWGATESTATUS,
	CM_CONTROLSTATUS,

    //�����ӵ�����:
	CM_UNKNOWN
}CMsgType;         //��Ϣ����

typedef enum 
{
 	LT_GREEN,    //��ɫ
	LT_YELLOW,   //��ɫ
	LT_RED,      //��ɫ
	LT_GRBLINK,  //����
	LT_YLBLINK,  //����
	LT_REDBLINK, //����
	LT_UNKNOWN   //δ֪״̬
}LT_COLOR;

typedef union _UNION_BYTE_BIT
{
	BYTE  U_Byte;
	struct{
		unsigned  D0:1;
		unsigned  D1:1;
		unsigned  D2:1;
		unsigned  D3:1;
		unsigned  D4:1;
		unsigned  D5:1;
		unsigned  D6:1;
		unsigned  D7:1;
		unsigned  reserved:24;
	}U_Bit;
}UNION_BYTE_BIT;

typedef struct 
{
	BYTE byFlag;	        //��Ϣ֡��־ == MsgFlag
    BYTE byCheckSum;	    //֡����
	CMsgType MsgType;	    //��Ϣ����
	char SourceIP[16];		//��ϢԴIP
	char SourceID[10];		//��ϢԴID
	char TargetIP[16];		//Ŀ��IP
	char TargetID[10];		//Ŀ��ID
	int iCrossNo;           //·�ڱ��
	WORD iLength;           //���������ַ�����
//added 09,21,2001
	WORD iReserved1;		//time duration
	WORD iReserved2;		//Number
//added end
}CMsgFrame;	//��Ϣ֡��ʽ

//�·���ͨѶ��,�źŽ��ݲ����ṹ 
typedef struct 
{
	BYTE byStep;                            //·�ڽ�����, <=32
	int iTime[MaxStepNum];                  //�ý��ݵĳ���ʱ��
	int iCoCrossNo;                         //Э��·�ڱ��
	int iOffset;                            //��λ���
	BYTE byDetectorNum;                     //��·�ڼ��������
	BYTE byDetectorNo[MaxDetectorNum];      //��������ı��
//	BYTE byDetectorGreen[MaxStepNum][MaxDetectorNum];//�ý����´����̵�״̬�ļ�������
	BYTE byDetectorGreen1[4][MaxDetectorNum];  //����������̵ƿ�ʼ���ݺ�
	BYTE byDetectorGreen2[4][MaxDetectorNum];  //����������̵ƽ������ݺ�
	BYTE MinStepTime[MaxStepNum];           //�ý��ݵĳ���ʱ��
	BYTE MaxStepTime[MaxStepNum];           //�ý��ݵĳ���ʱ��
}CStepsDown;                    //�źŽ��ݲ���, �·���ͨѶ��
typedef struct
{
	CMsgFrame Header;
	CStepsDown StepsDown;
}Cmd_StepsDown;

typedef Cmd_StepsDown Cmd_FirstStepsDown;

typedef CMsgFrame Cmd_OffLine;
typedef CMsgFrame Cmd_OnLine;

typedef struct 
{
	UINT16 MaxTime;
	UINT16 ForcedStep;
}CSetStepOn;

//following modified 09,21,2001

/*typedef struct
{
	CMsgFrame Header;
	CSetStepOn SetStepOn;
}Cmd_SetStepOn;
*/
typedef CMsgFrame Cmd_SetStepOn;

typedef struct 
{
	UINT16 Reserved;
	UINT16 ForcedStep;
}CSetStepOff;
/*typedef struct
{
	CMsgFrame Header;
	CSetStepOff SetStepOff;
}Cmd_SetStepOff;
*/
typedef CMsgFrame Cmd_SetStepOff;

//0x03ҹ���źŵƻ�������
/*typedef struct
{
	CMsgFrame Header;
	unsigned short  Flash_Data;
}CMD_YELLOWLAMP_FLASH;
*/
typedef CMsgFrame CMD_YELLOWLAMP_FLASH;

//#04��ҹ���źŵ�Ϩ������
/*typedef struct
{
	CMsgFrame Header;
	unsigned short Blackout_Data;
}CMD_BLACK_OUT;
*/
typedef CMsgFrame CMD_BLACK_OUT;
/*
typedef struct
{
	BYTE    D1;
	BYTE	D2;
	BYTE    D3;
	BYTE    D4;
	BYTE	D5;
}DATA_VMSTEXT;
typedef struct
{
	CMsgFrame Header;
	DATA_VMSTEXT Vms_Data;
}CMD_VMSTEXT;
*/

//��·�ڷ���׼ʱ��
typedef struct
{
	BYTE    Year;
	BYTE	Month;
	BYTE    Day;
	BYTE    Hour;
	BYTE	Minute;
	BYTE	Second;
	BYTE    Week;
}DATA_BENCHMARK_TIME;
typedef struct
{
	CMsgFrame Header;
	DATA_BENCHMARK_TIME  Time_Data;
}CMD_BENCHMARK_TIME;

typedef struct 
{
	BYTE byTime;
	BYTE byLampNum;
	LT_COLOR byStatus[MaxLampNum];
}LightDataDown;
typedef struct
{
	CMsgFrame Header;
	LightDataDown  LightDownData;
}CMD_LightDataDown;
	
typedef CMsgFrame CMD_EndSetColor;

typedef struct 
{
	BYTE byFlowGroupNum;
	BYTE byLampNum[MaxFlowGroupNum];//no use
	BYTE byLampNo[MaxFlowGroupNum][MaxLampNum];
}SplitPlanDown;
typedef struct 
{
	CMsgFrame Header;
	SplitPlanDown SplitPlanData;
}Cmd_SplitPlanDown;
	
//·����ʱ���������ṹ
typedef struct 
{
	BYTE byPlanNo;         //��ʱ�������
	BYTE byFlowGroupNum;                //��ͨ������
	int iGrTime[MaxFlowGroupNum];       //����ͨ������̵�ʱ��
	BYTE byGrBlinkTime[MaxFlowGroupNum];  //����ͨ���������ʱ��
	BYTE byYellowTime[MaxFlowGroupNum];   //����ͨ����ĻƵ�ʱ��
	BYTE byAllRedTime[MaxFlowGroupNum];   //����ͨ�����ȫ��ʱ��
}CyclePlanDown;	//·����ʱ���������ṹ
typedef struct 
{
	CMsgFrame Header;
	CyclePlanDown CyclePlanData;
}Cmd_CyclePlanDown;

//·��ʱ����ṹ
typedef struct 
{
	BYTE byTimeBandPlanNo;                    //ʱ��η�������
	BYTE byTimeBandNum;                       //ʱ�������
	float StartTime[MaxTimeBandNum];          //ʱ��ο�ʼʱ��munite.second
	float EndTime[MaxTimeBandNum];            //ʱ��ν���ʱ��
	BYTE byPlanNo[MaxTimeBandNum];            //��ʱ�������
//	BYTE StartSecond[MaxTimeBandNum];         //ʱ��ο�ʼʱ��se
//	BYTE EndSecond[MaxTimeBandNum];           //ʱ��ν���ʱ��se
}TimeBandDown;               //·��ʱ���
typedef struct 
{
	CMsgFrame Header;
	TimeBandDown PlanDownData;
}Cmd_TimeBandDown;

//·�����ڷ����ṹ
typedef struct 
{
	BYTE byDatePlanNum;                     //���ڷ�������
	BYTE byDateTypeNo[MaxDatePlanNum];      //���ڷ������(����������)
	float StartDate[MaxDatePlanNum];        //��ʼ����
	float EndDate[MaxDatePlanNum];          //��������
	BYTE byPlanGroupNo[MaxDatePlanNum];     //��������(����)
}DatePlanDown;               //·�����ڷ����ṹ
typedef struct 
{
	CMsgFrame Header;
	DatePlanDown DatePlanData;
}Cmd_DatePlanDown;

/*typedef struct 
{
	CMsgFrame Header;
	WORD PlanNo;
}Cmd_CurScheme;
*/
typedef CMsgFrame Cmd_CurScheme;

//02,18,2002
//���ݱ�����
typedef struct 
{
	BYTE bStepNum;     //������<32
	BYTE LampColor[MaxStepNum][MaxLampNum];
}SetStepTable;               //·�ڽ��ݱ�����
typedef struct 
{
	CMsgFrame Header;
	SetStepTable StepTableData;
}Cmd_SetStepTable;
//�����Ϸ����ݱ�
typedef	CMsgFrame Cmd_GetStepTable;//iReserved1,iReserved2Ϊ���ݷ�Χ
//�Ϸ����ݵ�ɫ
typedef struct 
{
//	BYTE bStepNum;    //������<32
	BYTE bStepNo;     //��ɫֵ��Ӧ�Ľ��ݺ�<32
	BYTE LampColor[MaxLampNum];
}StepLampColorItem;               //·�ڽ��ݱ�����
typedef struct 
{
	CMsgFrame Header;
	StepLampColorItem StepLampColorItemData;
}Cmd_StepLampColorItem;

//����ʱ������
typedef struct 
{
	BYTE bStepTimePlanNum;     //����ʱ��������<32
	BYTE bStepNum/*[MaxStepTimePlanNum]*/;     //ÿ�����ݷ����Ŷ�Ӧ��������<32
	BYTE StepLen/*[MaxStepTimePlanNum]*/[MaxStepNum];
}SetStepTimePlan;               //·�ڽ���ʱ������
typedef struct 
{
	CMsgFrame Header;
	SetStepTimePlan StepTimePlanData;
}Cmd_SetStepTimePlan;
//�����Ϸ�����ʱ��
typedef	CMsgFrame Cmd_GetStepTimePlan;//iReserved1,iReserved2Ϊ���ݷ����ŷ�Χ
//�Ϸ�����ʱ��
typedef struct 
{
	BYTE bStepTimePlanNum;     //����ʱ��������<32
	BYTE bStepNum;				//������<32
	BYTE StepLen[MaxStepNum];
}StepTimePlanItem;               //·�ڽ���ʱ��
typedef struct 
{
	CMsgFrame Header;
	StepTimePlanItem StepTimePlanItemData;
}Cmd_StepTimePlanItem;

//timeband
typedef	CMsgFrame Cmd_GetTimeBand;//iReserved1Ϊʱ��η�������
typedef	Cmd_TimeBandDown Cmd_TimeBandUp;//�Ϸ�ʱ��η���

//Cmd_DatePlanDown
typedef struct 
{
	BYTE byDatePlanNum;                     //���ڷ�������
	BYTE byDateTypeNo[MaxDatePlanNum];      //���ڷ������(����������)
}GetDatePlanItem;          
typedef struct 
{
	CMsgFrame Header;
	GetDatePlanItem GetDatePlanItemData;
}Cmd_GetDatePlanItem;
typedef struct 
{
	BYTE byDateTypeNo;      //���ڷ������(����������)
	float StartDate;        //��ʼ����
	float EndDate;          //��������
	BYTE byPlanGroupNo;     //��������
}DatePlanItemUp;            
typedef struct 
{
	CMsgFrame Header;
	DatePlanItemUp DatePlanItemUpData;
}Cmd_DatePlanItemUp;

//timebenchmark
typedef	CMsgFrame Cmd_GetBenchMark;
typedef	CMD_BENCHMARK_TIME Cmd_BenchMarkUp;

//report lamp
typedef struct 
{
	BYTE RedLamp;
	BYTE BlueLamp;
	BYTE YellowLamp;
}LampGroup;            
typedef struct 
{
	BYTE FaultLampNum;//���ϵ�������
	BYTE FaultLampNo[MaxLampNum];//���ϵ�����
	LampGroup LampGroupData[MaxLampNum];
}LampGroupStatus;            
typedef struct 
{
	CMsgFrame Header;
	LampGroupStatus LampGroupStatusData;
}Rep_LampGroupStatus;

//report detector
typedef struct 
{
	BYTE FaultDetectorNum;//���ϼ��������
	BYTE FaultDetectorNo[MaxDetectorNum];//���ϼ�������
}DetectorStatus;            
typedef struct 
{
	CMsgFrame Header;
	DetectorStatus DetectorStatusData;
}Rep_DetectorStatus;

//report yellow and gate status
typedef	CMsgFrame Rep_YellowGateStatus;//Reserved1Ϊ��״ֵ̬, Reserved2Ϊ����״ֵ̬

//�Ϸ�
typedef struct
{
	UINT16 DetectorBh;
	UINT16 Status;
}RealTimeData;
typedef CMsgFrame Rep_RealTimeData;

typedef struct 
{
	UINT16 NowCtrMode;
	UINT16 NowStep;
//	UINT8 Reserved;
}CrossNowStatus;
/*typedef struct
{
	CMsgFrame Header;
	CrossNowStatus Data;
}Rep_CrossNowStatus;
*/
typedef CMsgFrame Rep_CrossNowStatus;

typedef struct 
{
	LT_COLOR byStatus[MaxLampNum];
}LightData;		              
typedef struct
{
	CMsgFrame Header;
	LightData Data;
}Rep_LightData;

//sanlian�����״̬����
typedef struct 
{
	BYTE byStatus[MaxDetectorNum];//·�ڸ������״̬ 
}DetectorData;    //���������
typedef struct
{
	CMsgFrame Header;
	DetectorData Data;
}Rep_DetectData;


//·���Ϸ������������5min�ۼ�����
typedef struct 
{
	int iCycleTime;                      //�ۼӵõ���������ʱ��, s
	int iTotalTime;                      //�ܼ��ʱ��, s
	int iCycleNo;				        //���������Ӧ��5min�ڵ�����������
	int iVehicleNo[MaxDetectorNum];      //�ۼӵõ��ĳ�����������, ��
	int iOccupyTime[MaxDetectorNum];     //�ۼӵõ��ĸߵ�ƽռ��ʱ��, s
	int iVehNoAtRed[MaxDetectorNum];     //���ʱ��ͨ���ĳ�������
	int iWaitTime[MaxDetectorNum];       //���ʱ�����г������ܵȴ�ʱ��
}FlowData;                   //��ͨѶ���ϴ���·�ڸ����������״̬����ֵ
typedef struct
{
	CMsgFrame Header;
	FlowData Data;
}Rep_FlowData;

typedef struct 
{
	BYTE D1:3;     //�źŵ�1��ɫ״̬ (0-7)
	BYTE D2:3;     //�źŵ�2��ɫ״̬ (0-7)
	BYTE D3:2;     //�źŵ�3��ɫ״̬ (0-3)
}SLLTDataByte;	//�źŵ�״̬�����ֽڵĸ�ʽ
typedef struct 
{
	SLLTDataByte LtData[10];    //�źŵ�״̬������10�ֽ�
}SLLampStatusData;	//·�ڻ��źŵ���״�����ݸ�ʽ
typedef struct 
{
	CMsgFrame Header;
	SLLampStatusData Data;
}Rep_SlLampCondition;

typedef struct 
{
	BYTE byCount[16];    //��������ֵ��16�������
	BYTE byTime[16];    //��������ʱ�䣬��
}SLVehicleCountData;	//·�ڻ������������ݸ�ʽ
typedef struct 
{
	CMsgFrame Header;
	SLVehicleCountData Data;
}Rep_SLVehicleCount;

typedef struct 
{
	BYTE byNoise;    //·��������dB
	BYTE byCO;       //·��COŨ�ȣ�ppm
	BYTE bySO2;      //·��SO2Ũ�ȣ�ppm
	BYTE byTemp;     //·���¶ȣ�C
}SLEnvData;  	//·�ڴ����������ݸ�ʽ
typedef struct 
{
	CMsgFrame Header;
	SLEnvData Data;
}Rep_SLEnvData;

typedef struct 
{
	BYTE byFlow[16];    //��������ֵ��16�������
	BYTE byOccupy[16];    //��������ʱ�䣬��
}SLVehicleFlowData;	//·�ڻ������������ݸ�ʽ
typedef struct 
{
	CMsgFrame Header;
	SLVehicleFlowData Data;
}Rep_SLFlowData;

typedef struct 
{
	BYTE byCtrlMode;   //����ģʽ: 1��ʱ�ζ���λ��2���Ӧ����, 3ȫ��Ӧ����,
	                   //    4��ͨ����, 5ָ�����Ĳ�������, 6ָ�������߿�
	BYTE byFlowGroupNo;  // ����ִ�е���λ��
	BYTE byGreenTimeLeft;  //����ִ�е���λ�̵�ʣ��ʱ��
	BYTE byGreenFlashTimeLeft;  //����ִ�е���λ����ʣ��ʱ��
	BYTE byYellowTimeLeft; //����ִ�е���λ�Ƶ�ʣ��ʱ��
	BYTE byAllRedTimeLeft;  //����ִ�е���λȫ��ʣ��ʱ��
	BYTE byTimePlanNo;      //����ִ�е�ʱ�η�����
	BYTE byTimeBandNo;      //����ִ�е�ʱ�κ�
	BYTE byCtrlPlanNo;      //����ִ�е���ʱ������
	BYTE IsYellowBlink;
	BYTE IsLampOff;
	BYTE Reserved1;
	BYTE Reserved2;
}SLUpStatusData;	//ÿ���ϴ��ĵ�ǰ����״̬���ݸ�ʽ
typedef struct 
{
	CMsgFrame Header;
	SLUpStatusData Data;
}Rep_UpStatus;

typedef CMsgFrame Rep_UpCommStatus;
typedef CMsgFrame Rep_CrossCtrStatus;

//#pragma pack(pop)

#endif
