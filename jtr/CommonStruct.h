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
#define MaxFlowGroupNum 16    //每路口最大交通流组(相位)数量
#define MaxDatePlanNum 32     //每路口最大日期方案组数量
#define MaxStepTimePlanNum 32 //最大阶梯方案号

//消息格式定义
typedef enum {
	CM_MESSAGE,              //终端之间相互发消息
	CM_CHECKIN,              //各终端向调度台发标识名
    //下面是控制台向总控发的消息
	CM_SCHEDULECHANGED,      //控制台通知总控，人工干预计划表已改变
	CM_TIMETABLECHANGED,     //控制台通知总控，时间带方案已改变
	CM_PLANCHANGED,          //控制台通知总控，配时参数已改变
	CM_DATEPLANCHANGED,      //控制台通知总控，路口日期方案表已改变
	CM_CROSSCHANGED,         //控制台通知总控，路口、路段等特性参数已改变
    //下面是控制台向总控、总控再向通讯机发的请求发送参数的消息
	CM_STARTREALSTATUS,      //控制台通知总控，开始需要路口实时参数
	CM_STOPREALSTATUS,       //控制台通知总控，结束路口实时参数传送
	CM_STARTSLCOUNTDATA,     //控制台通知总控，开始需要路口的车辆计数参数
	CM_STOPSLCOUNTDATA,      //控制台通知总控，结束路口的交通参数
	CM_STARTSLFLOWDATA,      //控制台通知总控，开始需要路口的交通参数
	CM_STOPSLFLOWDATA,       //控制台通知总控，结束路口的交通参数
	CM_STARTSLAIRDATA,       //控制台通知总控，开始需要路口的大气参数
	CM_STOPSLAIRDATA,        //控制台通知总控，结束路口的大气参数
	CM_STARTSLCURSTATUS,     //控制台通知总控，开始需要路口的当前状态参数
	CM_STOPSLCURSTATUS,      //控制台通知总控，结束路口的当前状态参数
	CM_GETSLLAMPCONDITION,   //控制台通知总控，得到路口机的信号机好坏状态参数
    //下面是控制台向总控、总控再向通讯机发的请求执行具体操作的消息
	CM_ONLINE,               //在线指令
	CM_OFFLINE,              //离线指令
	CM_SETCONTROL,           //设置控制方式
	CM_YELLOWBLINK,          //路口黄闪指令
	CM_ENDYELLOWBLINK,       //结束黄闪指令
	CM_LAMPOFF,              //路口灯熄灭指令
	CM_ENDLAMPOFF,           //路口灯结束熄灭指令
	CM_TIMESYNC,             //时间同步指令
	CM_SETCOLOR,             //强制灯色指令
	CM_ENDCOLOR,             //解除强制灯色指令
	CM_SETSPLIT,             //设置路口相位方案
	CM_SETCYCLEPLAN,         //设置路口配时方案
	CM_SETTIMEBAND,          //设置路口时间段表
	CM_SETSCHEDULE,          //设置路口日期方案选择表
	CM_SETUSEPLAN,           //not used
	CM_SETUSEGROUPPLAN,		 //设置路口运行方案号
	CM_EXECUTEVIPPLAN,       //执行VIP路线
	CM_STOPEXECUTEVIPPLAN,   //停止执行VIP路线
	CM_SLSTEPNEXT,           //控制台控制三联信号机步进的控制命令
	CM_SLCOMTEST,            //控制台向信号机发送测试命令
    //下面是通讯机向总控上传数据的消息
	CM_FLOWDATA,             //由通讯机向总控台发送的交通参数累加数据
	CM_VEHICLESTATUS,        //由通讯机向总控台发送的实时车辆状态
	CM_LAMPSTATUS,           //由通讯机向总控台发送的实时灯色状态
	CM_SLLAMPCONDITION,      //由通讯机向总控台发送的信号灯好坏状态
	CM_SLCOUNTDATA,          //三联信号机上传检测器计数参数
	CM_SLFLOWDATA,           //三联信号机上传交通参数
	CM_SLAIRDATA,            //三联信号机上传路口大气参数
	CM_SLCURSTATUS,          //三联信号机返回当前的工作状态
    //下面是总控向通讯机发的命令(京三路口机)
	CM_FIRSTSTEPPARAMETER,   //第一次向通讯机下发信号阶梯控制参数
	CM_STEPPARAMETER,        //向通讯机下发信号阶梯控制参数,
		                     //通讯机在收到参数后从下一个周期开始运行该参数
	CM_CHANGECTRLTYPE,       //改变控制方式，E:D1-D2，标准／特1／特2／特3
	                         //收到本命令后，通讯机将下传指令中的E:D1-D2设为
							 //指定的数值，并从返回数据中确认改变已实现
	CM_SENDCOMMSTATUS,		
	CM_SETSTEPTABLE,		//阶梯表设置
	CM_GETSTEPLAMPCOLOR,	//请求上发阶梯表
	CM_SENDSTEPLAMPCOLORITEM,//上发阶梯灯色
	CM_SETSTEPTIMEPLAN,		//阶梯时长设置
	CM_GETSTEPTIMEPLAN,	//请求上发阶梯时长
	CM_SENDSTEPTIMEPLANITEM,	//上发阶梯时长
	CM_GETTIMEBANDPLAN,	//请求上发时间段方案
	CM_SENDTIMEBANDPLAN,	//上发时间段方案
	CM_GETDATAPLAN,	//请求上发日期方案
	CM_SENDDATAPLAN,	//上发日期方案
	CM_GETBENCHMARK,	//请求上发时间
	CM_SENDBENCHMARK,	//上发日期时间
	CM_LAMPCONDITION,
	CM_DETECTORCONDITION,
	CM_YELLOWGATESTATUS,
	CM_CONTROLSTATUS,

    //新增加的命令:
	CM_UNKNOWN
}CMsgType;         //消息类型

typedef enum 
{
 	LT_GREEN,    //绿色
	LT_YELLOW,   //黄色
	LT_RED,      //红色
	LT_GRBLINK,  //绿闪
	LT_YLBLINK,  //黄闪
	LT_REDBLINK, //红闪
	LT_UNKNOWN   //未知状态
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
	BYTE byFlag;	        //消息帧标志 == MsgFlag
    BYTE byCheckSum;	    //帧检查和
	CMsgType MsgType;	    //消息类型
	char SourceIP[16];		//消息源IP
	char SourceID[10];		//消息源ID
	char TargetIP[16];		//目标IP
	char TargetID[10];		//目标ID
	int iCrossNo;           //路口编号
	WORD iLength;           //后面所带字符长度
//added 09,21,2001
	WORD iReserved1;		//time duration
	WORD iReserved2;		//Number
//added end
}CMsgFrame;	//消息帧格式

//下发给通讯机,信号阶梯参数结构 
typedef struct 
{
	BYTE byStep;                            //路口阶梯数, <=32
	int iTime[MaxStepNum];                  //该阶梯的持续时间
	int iCoCrossNo;                         //协调路口编号
	int iOffset;                            //相位差，秒
	BYTE byDetectorNum;                     //该路口检测器数量
	BYTE byDetectorNo[MaxDetectorNum];      //各检测器的编号
//	BYTE byDetectorGreen[MaxStepNum][MaxDetectorNum];//该阶梯下处于绿灯状态的检测器编号
	BYTE byDetectorGreen1[4][MaxDetectorNum];  //各检测器的绿灯开始阶梯号
	BYTE byDetectorGreen2[4][MaxDetectorNum];  //各检测器的绿灯结束阶梯号
	BYTE MinStepTime[MaxStepNum];           //该阶梯的持续时间
	BYTE MaxStepTime[MaxStepNum];           //该阶梯的持续时间
}CStepsDown;                    //信号阶梯参数, 下发给通讯机
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

//0x03夜晚信号灯黄闪命令
/*typedef struct
{
	CMsgFrame Header;
	unsigned short  Flash_Data;
}CMD_YELLOWLAMP_FLASH;
*/
typedef CMsgFrame CMD_YELLOWLAMP_FLASH;

//#04发夜晚信号灯熄灭命令
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

//给路口发基准时间
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
	
//路口配时方案参数结构
typedef struct 
{
	BYTE byPlanNo;         //配时方案编号
	BYTE byFlowGroupNum;                //交通流组数
	int iGrTime[MaxFlowGroupNum];       //各交通流组的绿灯时间
	BYTE byGrBlinkTime[MaxFlowGroupNum];  //各交通流组的绿闪时间
	BYTE byYellowTime[MaxFlowGroupNum];   //各交通流组的黄灯时间
	BYTE byAllRedTime[MaxFlowGroupNum];   //各交通流组的全红时间
}CyclePlanDown;	//路口配时方案参数结构
typedef struct 
{
	CMsgFrame Header;
	CyclePlanDown CyclePlanData;
}Cmd_CyclePlanDown;

//路口时间带结构
typedef struct 
{
	BYTE byTimeBandPlanNo;                    //时间段方案组编号
	BYTE byTimeBandNum;                       //时间段数量
	float StartTime[MaxTimeBandNum];          //时间段开始时间munite.second
	float EndTime[MaxTimeBandNum];            //时间段结束时间
	BYTE byPlanNo[MaxTimeBandNum];            //配时方案编号
//	BYTE StartSecond[MaxTimeBandNum];         //时间段开始时间se
//	BYTE EndSecond[MaxTimeBandNum];           //时间段结束时间se
}TimeBandDown;               //路口时间带
typedef struct 
{
	CMsgFrame Header;
	TimeBandDown PlanDownData;
}Cmd_TimeBandDown;

//路口日期方案结构
typedef struct 
{
	BYTE byDatePlanNum;                     //日期方案数量
	BYTE byDateTypeNo[MaxDatePlanNum];      //日期方案编号(即日期类型)
	float StartDate[MaxDatePlanNum];        //起始日期
	float EndDate[MaxDatePlanNum];          //结束日期
	BYTE byPlanGroupNo[MaxDatePlanNum];     //方案组编号(不用)
}DatePlanDown;               //路口日期方案结构
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
//阶梯表设置
typedef struct 
{
	BYTE bStepNum;     //阶梯数<32
	BYTE LampColor[MaxStepNum][MaxLampNum];
}SetStepTable;               //路口阶梯表设置
typedef struct 
{
	CMsgFrame Header;
	SetStepTable StepTableData;
}Cmd_SetStepTable;
//请求上发阶梯表
typedef	CMsgFrame Cmd_GetStepTable;//iReserved1,iReserved2为阶梯范围
//上发阶梯灯色
typedef struct 
{
//	BYTE bStepNum;    //阶梯数<32
	BYTE bStepNo;     //灯色值对应的阶梯号<32
	BYTE LampColor[MaxLampNum];
}StepLampColorItem;               //路口阶梯表设置
typedef struct 
{
	CMsgFrame Header;
	StepLampColorItem StepLampColorItemData;
}Cmd_StepLampColorItem;

//阶梯时长设置
typedef struct 
{
	BYTE bStepTimePlanNum;     //阶梯时长方案号<32
	BYTE bStepNum/*[MaxStepTimePlanNum]*/;     //每个阶梯方案号对应额达阶梯数<32
	BYTE StepLen/*[MaxStepTimePlanNum]*/[MaxStepNum];
}SetStepTimePlan;               //路口阶梯时长设置
typedef struct 
{
	CMsgFrame Header;
	SetStepTimePlan StepTimePlanData;
}Cmd_SetStepTimePlan;
//请求上发阶梯时长
typedef	CMsgFrame Cmd_GetStepTimePlan;//iReserved1,iReserved2为阶梯方案号范围
//上发阶梯时长
typedef struct 
{
	BYTE bStepTimePlanNum;     //阶梯时长方案号<32
	BYTE bStepNum;				//阶梯数<32
	BYTE StepLen[MaxStepNum];
}StepTimePlanItem;               //路口阶梯时长
typedef struct 
{
	CMsgFrame Header;
	StepTimePlanItem StepTimePlanItemData;
}Cmd_StepTimePlanItem;

//timeband
typedef	CMsgFrame Cmd_GetTimeBand;//iReserved1为时间段方案组编号
typedef	Cmd_TimeBandDown Cmd_TimeBandUp;//上发时间段方案

//Cmd_DatePlanDown
typedef struct 
{
	BYTE byDatePlanNum;                     //日期方案数量
	BYTE byDateTypeNo[MaxDatePlanNum];      //日期方案编号(即日期类型)
}GetDatePlanItem;          
typedef struct 
{
	CMsgFrame Header;
	GetDatePlanItem GetDatePlanItemData;
}Cmd_GetDatePlanItem;
typedef struct 
{
	BYTE byDateTypeNo;      //日期方案编号(即日期类型)
	float StartDate;        //起始日期
	float EndDate;          //结束日期
	BYTE byPlanGroupNo;     //方案组编号
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
	BYTE FaultLampNum;//故障灯组总数
	BYTE FaultLampNo[MaxLampNum];//故障灯组编号
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
	BYTE FaultDetectorNum;//故障监测器总数
	BYTE FaultDetectorNo[MaxDetectorNum];//故障监测器编号
}DetectorStatus;            
typedef struct 
{
	CMsgFrame Header;
	DetectorStatus DetectorStatusData;
}Rep_DetectorStatus;

//report yellow and gate status
typedef	CMsgFrame Rep_YellowGateStatus;//Reserved1为门状态值, Reserved2为黄闪状态值

//上发
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

//sanlian检测器状态数据
typedef struct 
{
	BYTE byStatus[MaxDetectorNum];//路口各检测器状态 
}DetectorData;    //检测器参数
typedef struct
{
	CMsgFrame Header;
	DetectorData Data;
}Rep_DetectData;


//路口上发车辆检测器的5min累计数据
typedef struct 
{
	int iCycleTime;                      //累加得到的总周期时间, s
	int iTotalTime;                      //总检测时间, s
	int iCycleNo;				        //各检测器对应的5min内的完整周期数
	int iVehicleNo[MaxDetectorNum];      //累加得到的车辆计数数据, 辆
	int iOccupyTime[MaxDetectorNum];     //累加得到的高电平占有时间, s
	int iVehNoAtRed[MaxDetectorNum];     //红灯时间通过的车辆计数
	int iWaitTime[MaxDetectorNum];       //红灯时间所有车辆的总等待时间
}FlowData;                   //由通讯机上传的路口各车辆检测器状态计数值
typedef struct
{
	CMsgFrame Header;
	FlowData Data;
}Rep_FlowData;

typedef struct 
{
	BYTE D1:3;     //信号灯1灯色状态 (0-7)
	BYTE D2:3;     //信号灯2灯色状态 (0-7)
	BYTE D3:2;     //信号灯3灯色状态 (0-3)
}SLLTDataByte;	//信号灯状态数据字节的格式
typedef struct 
{
	SLLTDataByte LtData[10];    //信号灯状态参数，10字节
}SLLampStatusData;	//路口机信号灯损坏状况数据格式
typedef struct 
{
	CMsgFrame Header;
	SLLampStatusData Data;
}Rep_SlLampCondition;

typedef struct 
{
	BYTE byCount[16];    //车辆计数值，16个检测器
	BYTE byTime[16];    //车辆计数时间，秒
}SLVehicleCountData;	//路口机车辆计数数据格式
typedef struct 
{
	CMsgFrame Header;
	SLVehicleCountData Data;
}Rep_SLVehicleCount;

typedef struct 
{
	BYTE byNoise;    //路口噪音，dB
	BYTE byCO;       //路口CO浓度，ppm
	BYTE bySO2;      //路口SO2浓度，ppm
	BYTE byTemp;     //路口温度，C
}SLEnvData;  	//路口大气参数数据格式
typedef struct 
{
	CMsgFrame Header;
	SLEnvData Data;
}Rep_SLEnvData;

typedef struct 
{
	BYTE byFlow[16];    //车辆计数值，16个检测器
	BYTE byOccupy[16];    //车辆计数时间，秒
}SLVehicleFlowData;	//路口机车辆计数数据格式
typedef struct 
{
	CMsgFrame Header;
	SLVehicleFlowData Data;
}Rep_SLFlowData;

typedef struct 
{
	BYTE byCtrlMode;   //工作模式: 1多时段多相位，2半感应控制, 3全感应控制,
	                   //    4交通管制, 5指挥中心步进控制, 6指挥中心线控
	BYTE byFlowGroupNo;  // 现在执行的相位号
	BYTE byGreenTimeLeft;  //现在执行的相位绿灯剩余时间
	BYTE byGreenFlashTimeLeft;  //现在执行的相位绿闪剩余时间
	BYTE byYellowTimeLeft; //现在执行的相位黄灯剩余时间
	BYTE byAllRedTimeLeft;  //现在执行的相位全红剩余时间
	BYTE byTimePlanNo;      //现在执行的时段方案号
	BYTE byTimeBandNo;      //现在执行的时段号
	BYTE byCtrlPlanNo;      //现在执行的配时方案号
	BYTE IsYellowBlink;
	BYTE IsLampOff;
	BYTE Reserved1;
	BYTE Reserved2;
}SLUpStatusData;	//每次上传的当前工作状态数据格式
typedef struct 
{
	CMsgFrame Header;
	SLUpStatusData Data;
}Rep_UpStatus;

typedef CMsgFrame Rep_UpCommStatus;
typedef CMsgFrame Rep_CrossCtrStatus;

//#pragma pack(pop)

#endif
