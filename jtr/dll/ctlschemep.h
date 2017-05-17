//This File Created By Liz and modified by Yincy

#ifndef _CTLSCHEME_PROTOCOL_
#define _CTLSCHEME_PROTOCOL_

#pragma pack(push,1)

typedef BYTE YEAR;
typedef BYTE MONTH;
typedef BYTE DAY;
typedef BYTE HOUR;
typedef BYTE MINUTE;
typedef BYTE SECOND;
typedef BYTE WEEK;

typedef struct _Header
{
	//below is requested by liz 04,06,2001
	BYTE Packet1;//0xbb
	BYTE Packet2;//0xbb
	unsigned short MsgLen; //whole packet length
	//added end
	BYTE CrossingCode[7]; //modify 03,01,2001;BYTE CrossingCode 
	BYTE Command;//0x01
	BYTE bSucc;//1,Operation success;0,operation failed
	BYTE TurnOver;
}Header,*PHeader;

//modify 03,14,2001
typedef union _UNION_BYTE_BIT
{
	BYTE  U_Byte;
	struct{
		BYTE  D0:1;
		BYTE  D1:1;
		BYTE  D2:1;
		BYTE  D3:1;
		BYTE  D4:1;
		BYTE  D5:1;
		BYTE  D6:1;
		BYTE  D7:1;
	}U_Bit;
}UNION_BYTE_BIT,PUNION_BYTE_BIT;

//#01发路口机要显示的VSM内容
typedef struct _DATA_VSM_CONTENT
{
	UNION_BYTE_BIT VsmContent[4];
	BYTE MaxSpeed;
}DATA_VSM_CONTENT,*PDATA_VSM_CONTENT;
/*
typedef struct _DATA_VSM_CONTENT
{
   union
   {
      struct
      {
         BYTE B1;
         BYTE B2;
         BYTE B3;
         BYTE B4;
      }vsm_b;
      struct
      {
         BYTE B1D0:1;
         BYTE B1D1:1;
         BYTE B1D2:1;
         BYTE B1D3:1;
         BYTE B1D4:1;
         BYTE B1D5:1;
         BYTE B1D6:1;
         BYTE B1D7:1;

         BYTE B2D0:1;
         BYTE B2D1:1;
         BYTE B2D2:1;
         BYTE B2D3:1;
         BYTE B2D4:1;
         BYTE B2D5:1;
         BYTE B2D6:1;
         BYTE B2D7:1;

         BYTE B3D0:1;
         BYTE B3D1:1;
         BYTE B3D2:1;
         BYTE B3D3:1;
         BYTE B3D4:1;
         BYTE B3D5:1;
         BYTE B3D6:1;
         BYTE B3D7:1;

         BYTE B4D0:1;
         BYTE B4D1:1;
         BYTE B4D2:1;
         BYTE B4D3:1;
         BYTE B4D4:1;
         BYTE B4D5:1;
         BYTE B4D6:1;
         BYTE B4D7:1;
      }vsm_d;
   }vsm;
   BYTE  MaxSpeed;
}DATA_VSM_CONTENT,*PDATA_VSM_CONTENT;
*/

typedef struct _CMD_VSM_CONTENT
{
	Header cmdHeader;
	DATA_VSM_CONTENT  Vsm_Data;
}CMD_VSM_CONTENT,*PCMD_VSM_CONTENT;


//#02给路口发基准时间
typedef struct _DATA_BENCHMARK_TIME
{
	YEAR    Year;
	MONTH   Month;
	DAY     Day;
	HOUR    Hour;
	MINUTE  Minute;
	SECOND  Second;
	WEEK    Week;
}DATA_BENCHMARK_TIME,*PDATA_BENCHMARK_TIME;

typedef struct _CMD_BENCHMARK_TIME
{
	Header cmdHeader;
	DATA_BENCHMARK_TIME  Time_Data;
}CMD_BENCHMARK_TIME,*PCMD_BENCHMARK_TIME;

//0x03夜晚信号灯黄闪命令
typedef struct _DATA_YELLOWLAMP_FLASH
{
	HOUR        BeginHour;
	MINUTE      BeginMinute;
	HOUR        EndHour;
	MINUTE      EndMinute;
	BYTE        Enable;
}DATA_YELLOWLAMP_FLASH,*PDATA_YELLOWLAMP_FLASH;
typedef struct _CMD_YELLOWLAMP_FLASH
{
	Header cmdHeader;
	DATA_YELLOWLAMP_FLASH   Flash_Data;
}CMD_YELLOWLAMP_FLASH,*PCMD_YELLOWLAMP_FLASH;

//#04发夜晚信号灯熄灭命令
typedef DATA_YELLOWLAMP_FLASH  DATA_BLACK_OUT;
typedef PDATA_YELLOWLAMP_FLASH PDATA_BLACK_OUT;
typedef struct _CMD_BLACK_OUT
{
	Header cmdHeader;
	DATA_BLACK_OUT          Blackout_Data;
}CMD_BLACK_OUT,*PCMD_BLACK_OUT;

//#05给路口机设置各相位
typedef struct _DATA_MOMENT_PHASE_A
{
	BYTE        LampNo[11];
}DATA_MOMENT_PHASE_A,*PDATA_MOMENT_PHASE_A;

typedef struct _DATA_MOMENT_PAHSE
{
	BYTE        PlanNo;//must be 0xff otherwise reject 03,01,2001
	BYTE        MomentPhaseCount;   //相位数(B1)
	DATA_MOMENT_PHASE_A MomentPhases[8];
}DATA_MOMENT_PAHSE,*PDATA_MOMENT_PAHSE;

typedef struct _CMD_MOMENT_PAHSE
{
	Header cmdHeader;
	DATA_MOMENT_PAHSE       MomentPhase_Data;
}CMD_MOMENT_PAHSE,*PCMD_MOMENT_PAHSE;

//#06设置平时各时段(revised 02,27,2001)
typedef struct _DATA_PERIOD_TIME_A
{
	HOUR     BeginHour;
	MINUTE   BeginMinute;
	HOUR     EndHour;
	MINUTE   EndMinute;
	BYTE     TimePlanNo;//所选的配时方案号
//	SECOND   GreenTimes[8];
}DATA_PERIOD_TIME_A,*PDATA_PERIOD_TIME_A;

typedef struct _DATA_PERIOD_TIME
{
	BYTE     PlanNo;             //方案号
	BYTE     PeriodCount;               //时段总数(B1)
	DATA_PERIOD_TIME_A PeriodTimes[10];
}DATA_PERIOD_TIME,*PDATA_PERIOD_TIME;

typedef struct _CMD_PERIOD_TIME
{
	Header cmdHeader;
	DATA_PERIOD_TIME        PeriodTime_Data;
}CMD_PERIOD_TIME,*PCMD_PERIOD_TIME;

//#07设置配时各时段(revised 02,27,2001)
typedef struct _TIME_PLAN_A
{
	BYTE GreenTime;//八个相位的绿灯时间
	BYTE GreenFlashTime;
	BYTE YellowTime;
	BYTE ReadTime;
}TIME_PLAN_A,*PTIME_PLAN_A;

typedef struct _TIME_PLAN
{
	BYTE     TimePlanNo;//配时方案号
	TIME_PLAN_A PlanTimes[8];
}TIME_PLAN,*PTIME_PLAN;

typedef struct _CMD_TIME_PLAN
{
	Header cmdHeader;
	TIME_PLAN  PlanTimes_Data;
}CMD_TIME_PLAN,*PCMD_TIME_PLAN;
//appendix to liz
//#08给路口机设置时段方案运行时间表(revised 02,27,2001)
typedef struct _TIME_SCHEDULE
{
	BYTE DataType;//日期类型：1－7：星期一－－星期日；其它：特殊日期
	BYTE StartMonth;//起始日期－月
	BYTE StartDay;//起始日期－日
	BYTE EndMonth;
	BYTE EndDay;
	BYTE PeriodPlanNo;//时段方案号
}TIME_SCHEDULE,*PTIME_SCHEDULE;
typedef struct _CMD_TIME_SCHEDULE
{
	Header cmdHeader;
	TIME_SCHEDULE  TimeSchedule_Data;
}CMD_TIME_SCHEDULE,*PCMD_TIME_SCHEDULE;

//#17         (revised 02,27,2001)
typedef struct _DATA_FORCEDLAMPCOLOR_A
{
	union 
	{
		BYTE Color_Byte;//revised 03,21,2001
		struct{
			BYTE     YelloOn:1;        //bit0,黄色开关
			BYTE     RedOn:1;          //bit0,红开关
			BYTE     GreenOn:1;        //bit0,绿色开关
		}Color_bit;
	}lamp_color;
}DATA_FORCEDLAMPCOLOR_A,*PDATA_FORCEDLAMPCOLOR_A;

typedef struct _DATA_FORCEDLAMPCOLOR
{
	DATA_FORCEDLAMPCOLOR_A  LampColors[24];
}DATA_FORCEDLAMPCOLOR,*PDATA_FORCEDLAMPCOLOR;

typedef struct _CMD_FORCEDLAMPCOLOR
{
	Header cmdHeader;
	DATA_FORCEDLAMPCOLOR    ForcedLampColor_Data;
}CMD_FORCEDLAMPCOLOR,*PCMD_FORCEDLAMPCOLOR;

//#18给路口机发现在要执行的时段方案号(revised 02,27,2001)
typedef BYTE   DATA_CUR_SCHEME;
typedef struct _CMD_CUR_SCHEME
{
	Header cmdHeader;
	DATA_CUR_SCHEME  CurScheme_Data;   //<127为要执行的方案号，实际1-16
                                             //>128按约定的方式进行方案选择
}CMD_CUR_SCHEME,*PCMD_CUR_SCHEME;

//#29结束(revised 02,27,2001)
typedef BYTE   DATA_END_LAMPCTL;
typedef struct _CMD_END_LAMPCTL
{
	Header cmdHeader;
//	DATA_END_LAMPCTL EndLampCtl_Data;
}CMD_END_LAMPCTL,*PCMD_END_LAMPCTL;


//以下为路口机返回信息

//#80
//向路口机发送查询命令0x80(revised 02,27,2001)
typedef struct _QRY_ALL_STATE
{
	Header cmdHeader;
	IMPL_TYPE Param;//B1＝0：只发一次数据,B1＝1－FD：发送间隔秒数；
					//B1＝FE：停止发送, B1＝FF：参数（状态改变）时发送
}QRY_ALL_STATE,*PQRY_ALL_STATE;

//对0x80命令的响应，发路口特征，车辆检测器的电平状态与灯组亮灭情况
typedef struct _DATA_All_STATE
{
	BYTE           CrossingID;
	UNION_BYTE_BIT State[10];
}DATA_All_STATE,*PDATA_All_STATE;
typedef struct _RESP_ALL_STATE
{
	Header cmdHeader;
	DATA_All_STATE          AllState_Data;            
}RESP_ALL_STATE,*PRESP_ALL_STATE;

//向路口机发送查询命令0x81
typedef QRY_ALL_STATE QRY_LAMP_STATUS;
//对0x81命令的响应，发信号灯损坏情况
typedef struct _DATA_LAMP_STATUS
{
	UNION_BYTE_BIT LampStatus[8];
}DATA_LAMP_STATUS,*PDATA_LAMP_STATUS;
typedef struct _RESP_LAMP_STATUS
{
	Header cmdHeader;
	DATA_LAMP_STATUS LampStatus_Data;          
}RESP_LAMP_STATUS,*PRESP_LAMP_STATUS;


//向路口机发送查询命令0x82
typedef QRY_ALL_STATE QRY_CHECKERS_NUM;
//对0x82命令的响应，发16个车辆检测器的数值
typedef struct _DATA_CHECKERS_NUM
{
	BYTE  Checkers[16];
	BYTE  TimeLen[16];
	BYTE  Occupancy[16];  //占有率
}DATA_CHECKERS_NUM,*PDATA_CHECKERS_NUM;

typedef struct _RESP_CHECKERS_NUM
{
	Header cmdHeader;
	DATA_CHECKERS_NUM       CheckersNum_Data;            
}RESP_CHECKERS_NUM,*PRESP_CHECKERS_NUM;

//向路口机发送查询命令0x83
typedef QRY_ALL_STATE QRY_POLLUTING_VAL;
//对0x83命令的响应，发噪音、CO、SO2、温度的数值
typedef struct _DATA_POLLUTING_VAL
{
	BYTE  Noise;
	BYTE  CO;
	BYTE  SO2;
	BYTE  Temperature;
}DATA_POLLUTING_VAL,*PDATA_POLLUTING_VAL;
typedef struct _RESP_POLLUTING_VAL
{
	Header cmdHeader;
	DATA_POLLUTING_VAL      PollutingVal_Data;            
}RESP_POLLUTING_VAL,*PRESP_POLLUTING_VAL;

//向路口机发送查询命令0x84(revised 02,27,2001)
typedef QRY_ALL_STATE QRY_OTHER_VAL;
//对0x84命令的响应，发交通流量、占有率等
/*typedef struct _DATA_OTHER_VAL
{
	BYTE     Flux[16];       //交通流量(??)
	BYTE     Occupancy[16];  //占有率(??)
}DATA_OTHER_VAL,*PDATA_OTHER_VAL;
typedef struct _RESP_OTHER_VAL
{
	Header cmdHeader;
	DATA_OTHER_VAL          Other_Data;            
}RESP_OTHER_VAL,*PRESP_OTHER_VAL;
*/
//0x85发TEST event,客户应间隔一段时间主动发(for later use,revised 02,27,2001)
typedef QRY_ALL_STATE OPR_TEST_MSG;


//0xf0 Client login(revised 02,27,2001)
typedef struct _DATA_LOGIN_VAL
{
	BYTE  bLogin;//true login,false logout
	BYTE  Name[15];
	BYTE  PassWord[15];
}DATA_LOGIN_VAL,*PDATA_LOGIN_VAL;
typedef struct _DATA_LOGIN
{
	Header cmdHeader;
	DATA_LOGIN_VAL  Login_Data;            
}DATA_LOGIN,*PDATA_LOGIN;

//0xf1 trans perform,for later use(not used so far)
typedef QRY_ALL_STATE TRANS_MSG;
//0 trans start,1 trans commit,2 trans rollback

#pragma pack(pop)
#endif

