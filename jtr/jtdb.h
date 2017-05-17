#ifndef _JTDB_H
#define _JTDB_H

#define IOCARD1_DRV_DLL
#define IOCARD1_DRV_SMAN
#ifndef IOCARD1_DRV_DLL
#undef	IOCARD1_DRV_SMAN
#endif

#ifdef cplusplus
extern "C"{
#endif

#include <windows.h>
//#include <string.h>
//#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <winsock.h>
#include <process.h>
#include <memory.h>
#include <sql.h>				
#include <sqlext.h>

#include "CommonStruct.h"
//#include "ctlschemep.h"
#ifndef __TEST
//#pragma option -b
#endif
#pragma pack(push,1)

#define PORT_360					((unsigned short)0x0360)
#define PORT_361					(PORT_360 + 1)
#define PORT_362					(PORT_360 + 2)

#define PORT_364					((unsigned short)0x0364)
#define	PORT_365					(PORT_364 + 1)
#define PORT_366					(PORT_364 + 2)

#define NO_PER_CARD					128

/* ODBC函数执行错误代码定义 */
#define LINKDISCONNECT      "08S01"   /* 在函数完成执行前，驱动程序及其
	                                     连接到数据源之间的通讯连接失败 */
#define CONTINUEFAILED      "40001"   /* 连续性失败 */
/*** 数据库操作结果标识 *******/

/**** 成功 *****/
#define DB_SQL_SUCCESS               (BYTE)0 

/**** Select操作无数据返回 *****/
#define DB_SQL_NODATAFOUND           (BYTE)1 

/**** Update,delect, insert操作无记录行受影响 ****/
#define DB_SQL_NOROWUPDATED          (BYTE)2 

/**** 与数据库Sever连接中断 *****/
#define DB_SQL_LINKDISCONNECT        (BYTE)3

/**** 一般性错误 ******/  
#define DB_SQL_GENERALLYERROR        (BYTE)4

/**** 非法的DBNo  ****/ 
#define DB_SQL_ILLEGALDBNO           (BYTE)5

/**** 所接收的消息中SQL语句为空  ****/ 
#define DB_SQL_RECVSQLSTRINGNULL     (BYTE)6

/**** Select操作的结果集超出所定义的最大字符数组大小 ****/  
#define DB_SQL_RESULTOVERFLOW        (BYTE)7  

/**** 调用 ODBC函数时ODBC系统出错 ****/
#define DB_SQL_SYSERROR              (BYTE)8

/**** 超出数组定义范围  ****/
#define DB_SQL_CONTINUE          (BYTE)51
#define CHECK_CONTINUE			 (BYTE)51	

#define MAX_RET_COLNUMBER       50
#define MAX_COL_LEN             400

#define MAX_SQLSTRING_LEN       1024    /* SQL语句最大长度 */
#define MAX_RETRESULT_LEN       1024    /* 返回的最大结果集长度 */
#define MAX_SQL_LEN				2048	/* SQL语句的最大长度 */
#define	MAX_DBNAME_LEN			20		/* 数据库中对象名的最大长度 */
#define MAX_DB_NUM				100		/* 一个DB上数据库最大个数 */

#define ZHAO_DATA_LEN			(UINT8)10
#define HU_DATA_LEN				(UINT8)4
#define ZHAO_ADD_LEN			(UINT8)3
#define EFFECT_YLF_HEAD_LEN		(UINT8)3

#define MAX_YLF_LEN				132
#define MAX_ENCRIPT_LEN			264


#define MAX_CROSS_NUM			(UINT32)255
#define MAX_SPOOL_NUM			(UINT32)256
#define	MAX_TASK_NUM			(UINT32)32
#define	MAX_LPT_LEN				(UINT32)1024  //最小也要比最大的数据长度大 7 
#define	MAX_SYSTASK_NUM			(UINT32)10

#define MAX_ERRRESEND_NUM		(UINT8)2
#define MAX_ERR_INCYCLE         (UINT8)6

#define MAX_RESEND_TIME			(UINT8)3
#define	SENDBUF_TIMEOUT			(UINT8)3
#define	SPOOL_TIMEOUT			(UINT8)30
#define MAX_OPR_TIME			(UINT8)30//????
#define	OPR_WAITTIME			(UINT8)15//????
#define CROSS_CONNECTTIME		(UINT8)20//10S为单位
#define CROSS_DOINGTIME			(UINT8)10//10S为单位
#define CROSS_TRANSTIME			(UINT8)60//10S为单位

#define MAX_XHD_NUM				(UINT8)24
#define	MAX_JCQ_NUM				(UINT8)32
#define MAX_STEP_NUM			(UINT8)32			//24
#define MAX_STREAM_NUM			(UINT8)8
#define MAX_PHASE_NUM			(UINT8)8
#define MAX_REALTIME_NUM		(UINT8)10

#define	LKJ_NAME_LEN			(UINT8)8
#define	XHD_NAME_LEN			(UINT8)8
#define	JCQ_NAME_LEN			(UINT8)2
#define	CD_NAME_LEN				(UINT8)2


#define IDLE_STATUS				(BYTE)0	
#define	INITIAL_STATUS			(BYTE)1
#define	TRANSIT_STATUS			(BYTE)2
#define	NORMAL_STATUS			(BYTE)3
#define	FORCED_STATUS			(BYTE)4
#define	OFFLINE_STATUS			(BYTE)5
#define	CONNECT_STATUS			(BYTE)6
#define	DISCONNECT_STATUS		(BYTE)7

#define OPR_DISCONNECT_STATUS	(BYTE)0	
#define	OPR_LOGIN_STATUS		(BYTE)1
#define	OPR_CONNECT_STATUS		(BYTE)2

#define BUF_IDLE_STATUS			(BYTE)0
#define BUF_QUE_STATUS			(BYTE)1
#define BUF_SEND_STATUS			(BYTE)2
#define BUF_RESEND_STATUS		(BYTE)3
#define BUF_CANCELED_STATUS		(BYTE)10

#define MAX_LIST_NUM			(BYTE)20
#define MIN_SAME_NUM			(BYTE)10

#define MAX_BUF_WAIT			30
#define MAX_HU_WAIT				10	//80以上的命令应立即返回值
#define MAX_YLF_WAIT			3
#define MAX_SYNERR_COUNT		30
#define MIN_SYN_LEN				2
#define MIN_GREEN_LEN			10

typedef BYTE IMPL_TYPE;

typedef struct
{
	UINT16 CmdNo;
	UINT8 Status;
	UINT8 SendNum;
	UINT16 Timeout;
	char *hMsgBuf;
	int MsgLen;
}DownMsgWaitNode;

typedef struct tag_CrossData_t
{
	UINT8 Type;
	UINT8 CardNo;
	UINT8 Status;
	UINT8 ErrorNo;
	UINT8 AulStatus;
	UINT8 LinkFlag;
	UINT8 NeedRealData;
	int NoSignalCount;
	UINT16 DeltaPhi;
	UINT8 BaseCross;
	UINT8 SynErrCount;
	UINT8 ForcedStep;//201 yellow,211 black
	int ForecedLeftTime;
	UINT16 lkbh;
	UINT8 jcqbh[MAX_JCQ_NUM];
	UINT8 jcqStartGreenStep[4][MAX_JCQ_NUM];
	UINT8 jcqEndGreenStep[4][MAX_JCQ_NUM];

//	UINT16 StreamGroup[MAX_STREAM_NUM][MAX_PHASE_NUM];
	UINT16 StepTable[MAX_STEP_NUM];
	UINT16 TransitStepTable[MAX_STEP_NUM];
	UINT16 MaxStepTable[MAX_STEP_NUM];
	UINT16 MinStepTable[MAX_STEP_NUM];
	UINT8 TotalStepNo;
	UINT8 SynFlag;

	UINT16 Left5Min;
//	UINT8 CycleCount5m;
	UINT16 RealTime5Min;
	int VeCountp5m[MAX_JCQ_NUM];
	int HiBitsp5m[MAX_JCQ_NUM];
	int RedVeCountp5m[MAX_JCQ_NUM];
	int RedVeWaitTime5m[MAX_JCQ_NUM];
//	int tempRedVeCountp5m[MAX_JCQ_NUM];
//	int tempRedVeWaitTime5m[MAX_JCQ_NUM];
	UINT8 RedCycleCount5m;
	UINT8 NeedStep;
	UINT8 StepErrCount;
	UINT8 InitalWait;

	UINT16 CycleTimeOffset;
	int CycleAulCount;
	UINT16 CycleVeCount[MAX_JCQ_NUM];
	UINT16 CycleHiBits[MAX_JCQ_NUM];

	UINT16 CycleRedVeCount[MAX_JCQ_NUM];
	UINT16 AsyCycRedVeCount[MAX_JCQ_NUM];
	UINT32 CycleRedVeWait[MAX_JCQ_NUM];
	
	int HiCount[MAX_JCQ_NUM];
	int LowCount[MAX_JCQ_NUM];
	UINT8 LastReceBit[MAX_JCQ_NUM];

	UINT8 CurCtrMode;
	UINT8 CurCValue;
	UINT8 CurStep;
	UINT8 ManCtrFlag;
//	UINT8 CurStepCount;

	int ReopenLeftCount;
	int DisconnectCount;
	int OpenTimes;
	SYSTEMTIME LastDisconnectTime;
	SYSTEMTIME LastConnectTime;
	UINT8 CtlModeReleaseNum;
	UINT8 LastStepCount;
	UINT8 CurStepPtr;
	UINT8 StepList[MAX_LIST_NUM];
	UINT8 WaitSendCount;			
	UINT8 WaitHead;
	UINT8 WaitEnd;
//	UINT8 BufStatus;
	DownMsgWaitNode DownMsgBuf[MAX_RESEND_NUM];
}CrossData_t;

typedef struct tag_RecBuf
{
	UINT8 CrossNo;
	UINT8 CmdNo;
	UINT8 Status;
	UINT8 CardNo;
	UINT8 ReSendTimes;
	UINT16 Timeout;
	UINT8 bResendFlag;

	UINT16 MsgLen;
	UINT8 MsgData[MAX_LPT_LEN];
}RecBuf;

typedef struct tag_OprData
{
	UINT8 OprStatus;
	UINT8 TestData;
	UINT8 Name[15];

	UINT8 NowCommand[MAX_CROSS_NUM];
	UINT8 IsTask[MAX_CROSS_NUM];
}COprData;

typedef struct tag_TaskData
{
	UINT8 Status;//0,未用;1,以发但等zhao ack;2,以发但等hu ack;3,将要被系统清除;ff,正常使用
	UINT8 SendParam;//发送参数 0,停止，1--fd周期， ff有信号发，fe只发一次;
					//如果路口机不支持此模式，此仅值为发送周期
	UINT8 ReservedStatus;
	UINT8 ReservedParam;
	UINT8 ReqTimes;
}TaskData;

typedef struct tag_SysAutoTask
{
	UINT8 CmdNo;
	UINT32 SendParam;
	UINT32 CastInterval;
	UINT32 LeftTime;
}SysAutoTaskData;

typedef struct tag_TransQ
{
	UINT8 Status;
	UINT8 OprIndex;
	UINT8 CmdNo;
	UINT8 MsgLen;
	UINT8 MsgData[MAX_LPT_LEN];
	UINT8 Next;
}TransQ;

#define RETCODE_IS_FAILURE(x) ((x) == SQL_ERROR || (x)==SQL_INVALID_HANDLE || (x)==SQL_STILL_EXECUTING)

//test function declared here
void LptExit(void);
BOOL LptInit(void);
void GetLptData(WPARAM CardPort);
void TestGetLptData(void);
void TestWrite2lcu(void);
void SetRealTimer(void);
void CALLBACK  RealTimerProc(UINT uID,UINT uMsg,DWORD dwUser,DWORD dw1,DWORD dw2);
void Send2Lcu(void);
BYTE GetDbInit(void);
BYTE GetJcqCode(void);
BYTE GetCrossCode(void);
BYTE SynClock(void);
BOOL DoConnect(char *);
void DoDisconnect(void);
BYTE DoUpdate(char *,UINT16);
BYTE DoQuery(char *,char *);
BYTE GetSQLError(HSTMT);
INT32 dbSQLExecDirect(HSTMT, char *);
void rtrim(char *Instr);
void CheckSignal(void);
BOOL Initdb(void);
void TsctlInit(void);
int InterpretClient(void * pMsgBuf,int RecLen,BYTE Index);
BOOL Write2Lpt(UINT8 *Buff,UINT16 Len,UINT8 CardNo);
int GetMsgLen(INT32 CommandNo,BYTE Type);
void SerialDataProc(int port,char *MsgPtr);
void DetectorProc(int port,char vdetctor);
void CurStepEventProc(int port,UINT8 D5Value);
void NewRoundPrepare(int port);
#ifdef __OLD__
void StartSynEx(int port,int AddLen);
#endif
void TransitPrepare(int port);
void InitialPrepare(int port);
void NormalPrepare(int port);
BOOL PeriodEqual(int port);
void SynPhase(int port,UINT16 DeltT);
void SynPeriod(int port);
BOOL Send2AllCross(void);
void SendCtr2Cross(int port,char *InPtr);
void SendParam2Sys(int port);
void NormalProc(int port,char *CtrCode);
void TransitProc(int port,char *CtrCode);
void InitialProc(int port,char *CtrCodePtr);
void RedLampProc(int port,UINT8 CurStep);
void InitialCross(int port);
void InitCrossComm(void);
void ServiceInit(void);
BYTE GetDefaultPlan(void);
void SetCrossInitStatus(int port);
void SendCheckin(void);
BYTE FormCheckSum(char *InPtr);
BOOL IsCheckSumOk(char *InPtr);
UINT32 FindCrossNo(int Crossing);
UINT8 StepParameterProc(UINT8 bCrossNo,CStepsDown *Data);
UINT8 FirstStepParameterProc(UINT8 bCrossNo,CStepsDown *Data);
void SendVehiclePassby(int Port,UINT8 jcqwlbh);
void SendVehiclePassing(int port,UINT8 jcqwlbh);
void IncRedlampWaitTime(int Port,UINT8 jcqwlbh);
void IncRedlampVehicleCount(int Port,UINT8 jcqwlbh);
void DecRedlampVehicleCount(int Port,UINT8 jcqwlbh);
void ReportFlowData(int port);
UINT8 StopRealProc(UINT8 bCrossNo);
UINT8 StartRealProc(UINT8 bCrossNo);
void ForcedProc(int port,char *CtrCodePtr);
BOOL CurStepLessNeedStep(int port);
UINT8 GetRealtimeNum(void);
UINT8 OnlineProc(UINT8 bCrossNo);
UINT8 OfflineProc(UINT8 bCrossNo);
UINT8 SetStepOnProc(UINT8 bCrossNo,CSetStepOn *Data);
UINT8 SetStepOffProc(UINT8 bCrossNo,CSetStepOff *Data);
void NewTransitAdjust(int port);
void SetStepoffAdjust(int port);
void ForcedAdjust(int port);
void SendOffline(int port,char *CtrCodePtr);
void SendCrossStatus2Wu(int port);
BOOL IsSanlian(UINT8 bCrossNo);
int SanlianProc(char * MsgIn,int RecLen,BYTE Index);
void SanlianSerialProc(int port,char *buf,int len);
BOOL YellowLampFlashReqProc(unsigned short Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex);
BOOL EndYellowBlinckReqProc(char *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex);
BOOL BlackOutReqProc(unsigned short Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex);
BOOL EndBlackReqProc(char *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex);
BOOL BenchMarkReqProc(DATA_BENCHMARK_TIME *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex);
BOOL SetColorReqProc(LightDataDown *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex);
BOOL EndColorReqProc(char *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex);
BOOL MonentPhaseReqProc(SplitPlanDown *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex);
BOOL PeriodTimeReqProc(CyclePlanDown *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex);
BOOL TimePlanReqProc(TimeBandDown *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex);
BOOL TimeScheduleReqProc(DatePlanDown *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex);
BOOL CurSchemeReqProc(WORD Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex);
BOOL QryAllStateReqProc(WORD Data,UINT32 CrossNo,UINT8 CommIndex);
BOOL QryLampStateReqProc(IMPL_TYPE *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex);
BOOL QryCheckNumReqProc(IMPL_TYPE *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex);
BOOL QryPollutingReqProc(IMPL_TYPE *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex);
BOOL OprTestReqProc(IMPL_TYPE *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex);
BOOL CanSendDirect(void);
UINT32 FindPos(void);
BOOL NowCanSend(UINT8 Port);
BOOL SanlianInQue(UINT8 Index,UINT8 CmdNo,char *hMsg,UINT16 hMsgLen);
void ClearSanlianSendQueue(BYTE Port);
void SetBuf2Ready(UINT8 Port,UINT8 Head);
void SetBuf2Busy(UINT8 Port,UINT8 Head);
void SetBuf2Que(UINT8 Port,UINT8 Head);
void DeleteDownBufHead(UINT8 port);
BOOL IsHuChecksumOk(char *InPtr,int Len);
BYTE FormHuChecksum(char *InPtr,int Len);
BOOL HuSerialPost(int Port,UINT8 CmdNo,char *MsgPtr,UINT16 MsgLen);
void CrossErrEventProc(int Port,UINT8 CmdNo);
void CrossAckEventProc(int Port,UINT8 CmdNo);
void CrossDataEventProc(int Port,UINT8 CmdNo);
void HuSerialTimeoutProc(void);
void CheckForecedTime(void);
void SendCurTime(void);
void SynCrossPhase(int port);
BOOL CycleIsEqual(int port,int BaseCross);
int GetCycleLen(int port);
int GetAdjustLen(int port,int BaseCross);
void StartSyn(int port,int Len);
BOOL IsStopSent(int Port,UINT8 CmdNo);
void GetLcuFlag(void);
void ReportForcedFlowData(int port);
UINT8 SetYellowOnProc(UINT8 bCrossNo,unsigned short Data);
UINT8 SetYellowOffProc(UINT8 bCrossNo);
UINT8 SetBlackOnProc(UINT8 bCrossNo,unsigned short Data);
UINT8 SetBlackOffProc(UINT8 bCrossNo);
void SendTestMsg(void);
UINT8 InnerBenchMarkProc(DATA_BENCHMARK_TIME *Data);
void GetLcuData(WPARAM CardNo,LPARAM CrossType);
void GetLptHuData(WPARAM CardNo);
void SanlianLcuProc(int Port,char *MsgPtr,int MsgLen);
void ZhaoErrEventProc(void);
UINT8 GetNextLcuHuItem(UINT8 CrossNo);
void ZhaoAckEventProc(int Port,UINT8 CmdNo);
void HuLcuTimeoutProc(void);
void HuLcuResend(void);
BOOL HuLcuPost(int Port,UINT8 CmdNo,char *MsgPtr,UINT16 MsgLen,UINT8 CardNo);
BOOL LcuCanSend(void);
void LcuBufResendProc(UINT8 bResend);
BOOL EndAllStateReqProc(UINT32 CrossNo,UINT8 CommIndex);
BOOL EndSlCountReqProc(UINT32 CrossNo,UINT8 CommIndex);
BOOL StartSlCountReqProc(WORD Data,UINT32 CrossNo,UINT8 CommIndex);
BOOL StartSlFlowReqProc(WORD Param,UINT32 CrossNo,UINT8 CommIndex);
BOOL EndSlFlowReqProc(UINT32 CrossNo,UINT8 CommIndex);
BOOL EndSlAirReqProc(UINT32 CrossNo,UINT8 CommIndex);
BOOL StartSlAirReqProc(WORD Param,UINT32 CrossNo,UINT8 CommIndex);
void AdjustData4Zhao(UINT8 *buff,int len);
void SendTest2Hu(void);
BOOL CurSchReqProc(WORD Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex);
void SendCommStatus2Wu(int port,int Status);
void CommStatusCheck(int CrossNo);
void IncOpenportTimes(int Port);
void GetComStatusChange(int port,int Status);
void ClearCommTxkh(void);
void IntHandler(PVOID pDrv);
void ClearSignal4Hu(void);
void AdjustTxkh(void);
void GetAppConfig(void);
BOOL GetSvrIp(char *InIpAddr);
UINT8 JSSetStepTableProc(SetStepTable *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex);
UINT32 Pack2YlfFormat(BYTE *Dest,BYTE *Src,UINT32 Length);
BYTE FormYlfChecksum(BYTE *InPtr,int Len);
UINT8 JSGetStepItemProc(Cmd_GetStepTable *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex);
UINT8 JSSetStepTimePlanProc(SetStepTimePlan *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex);
UINT8 JSGetStepTimePlanProc(Cmd_GetStepTimePlan *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex);
UINT8 JSTimePlanReqProc(TimeBandDown *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex);
UINT8 GetTimeScheduleProc(Cmd_GetTimeBand *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex);
UINT8 JSDatePlanProc(DatePlanDown *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex);
UINT8 GetDataPlanProc(GetDatePlanItem *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex);
UINT8 JSBenchMarkReqProc(DATA_BENCHMARK_TIME *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex);
UINT8 JSCrossTimeReqProc(char *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex);
BOOL IsJingSan(UINT8 bCrossNo);
UINT16 UnPackYlfShort(BYTE a,BYTE b);
BOOL IsYlfChecksumOk(char *InPtr,int Len);
UINT32 UnPackYlfForm(BYTE *Dest,BYTE *Src,UINT32 Length);
void JSSerialProc(int Port,UINT8 CmdNo,char *MsgPtr,int MsgLen);
void JSErrEventProc(int Port,UINT8 ErrNo);
void SendMsgInQue(int Port,int Info);
void JSAckEventProc(int Port,UINT8 CmdNo);
void JS11DataEventProc(int Port,UINT8 CmdNo,char *MsgPtr,int MsgLen);
void JS12DataEventProc(int Port,UINT8 CmdNo,char *MsgPtr,int MsgLen);
void JS22DataEventProc(int Port,UINT8 CmdNo,char *MsgPtr,int MsgLen);
void JS23DataEventProc(int Port,UINT8 CmdNo,char *MsgPtr,int MsgLen);
UINT32 GetMinLen(UINT32 ,UINT32 ,UINT32 MsgLen);
void JSSerialTimeoutProc(void);
BOOL EndCurStatusReqProc(UINT32 CrossNo,UINT8 CommIndex);
BOOL StartCurStatusReqProc(WORD Param,UINT32 CrossNo,UINT8 CommIndex);
void JS13DataEventProc(int Port,UINT8 CmdNo,char *MsgPtr,int MsgLen);
void JS14DataEventProc(int Port,UINT8 CmdNo,char *MsgPtr,int MsgLen);
void ClearOpenLeftTimes(int Port);
void JS16DataEventProc(int Port,UINT8 CmdNo,char *MsgPtr,int MsgLen);
void JS21DataEventProc(int Port,UINT8 CmdNo,char *MsgPtr,int MsgLen);
BOOL StartLampOkReqProc(WORD Param,UINT32 CrossNo,UINT8 CommIndex);
BOOL StartTestReqProc(WORD Param,UINT32 CrossNo,UINT8 CommIndex);
void JSCommStatusCheck(int CrossNo);
BOOL SlNextstepReqProc(WORD Param,UINT32 CrossNo,UINT8 CommIndex);
UINT8 JsNextstepReqProc(UINT8 bCrossNo,UINT8 Data);
BOOL IsDataAllNull(UINT8 *pIn,UINT16 MsgLen);
UINT8 JSGetTimeScheduleProc(Cmd_GetTimeBand *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex);
BOOL SlSetControlReqProc(WORD Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex);
int GetCycleLenEx(CStepsDown *pStepDown);
BOOL TimeCanDelay(int port);
BOOL TimeCanShort(int port);
void AckTypeCmdTimeoutProc(void);
BOOL CurStepGreatTheStep(int port,UINT8 TheStep);
BOOL JSCancelProc(int Port,UINT8 CmdNo);
void JSCancelLeftItems(int Port,UINT8 CmdNo,UINT8 ItemStart,UINT8 MaxItemNo);
void InSynCross(int port,int AddLen);
#ifdef cplusplus
}
#endif

#pragma pack(pop)
#endif
