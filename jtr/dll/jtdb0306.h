#ifndef _JTDB_H
#define _JTDB_H

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

#include "ctlschemep.h"

#pragma option -b

#pragma pack(push,1)


#define	MAX_CONNECT_NO			5

#define DATA_PORT				((unsigned short)0x0360)
#define	ZHAO_INT_PORT			((unsigned short)0x0361)
#define STATUS_PORT				((unsigned short)0x0362)

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

#define MAX_CROSS_NUM			(UINT32)100
#define MAX_SPOOL_NUM			(UINT32)256
#define	MAX_TASK_NUM			(UINT32)32
#define	MAX_LPT_LEN				(UINT32)120  //最小也要比最大的数据长度大 7 
#define	MAX_SYSTASK_NUM			(UINT32)10

#define MAX_RESEND_TIME			(UINT8)3
#define	SENDBUF_TIMEOUT			(UINT8)3
#define	SPOOL_TIMEOUT			(UINT8)30
#define MAX_OPR_TIME			(UINT8)30//????
#define	OPR_WAITTIME			(UINT8)15//????
#define CROSS_CONNECTTIME		(UINT8)20//10S为单位
#define CROSS_DOINGTIME			(UINT8)10//10S为单位
#define CROSS_TRANSTIME			(UINT8)60//10S为单位

#define MAX_XHD_NUM				(UINT8)24
#define	MAX_JCQ_NUM				(UINT8)16

#define	LKJ_NAME_LEN			(UINT8)6
#define	XHD_NAME_LEN			(UINT8)8
#define	JCQ_NAME_LEN			(UINT8)8
#define	CD_NAME_LEN				(UINT8)2


#define CROSS_DISCONNECT_STATUS	(BYTE)0	
#define	CROSS_READY_STATUS		(BYTE)1
#define	CROSS_DOING_STATUS		(BYTE)2

#define OPR_DISCONNECT_STATUS	(BYTE)0	
#define	OPR_LOGIN_STATUS		(BYTE)1
#define	OPR_CONNECT_STATUS		(BYTE)2

#define SENDBUF_READY_STATUS	(BYTE)0
#define SENDBUF_SEND_STATUS		(BYTE)1

#define TASK_NOTUSE				(BYTE)0
#define TASK_WAIT_ZHAO			(BYTE)1
#define TASK_WAIT_HU			(BYTE)2
#define TASK_SYS_CLEAR			(BYTE)3
#define TASK_WORKING			(BYTE)0xff

#define REQ_ERROR_CLASS			(BYTE)0
#define REQ_COMMON_CLASS		(BYTE)1
#define REQ_TRANS_CLASS			(BYTE)2
#define REQ_TASKSTART_CLASS		(BYTE)3
#define REQ_TASKEND_CLASS		(BYTE)4
#define REQ_AUTO_CLASS			(BYTE)5

#define NOTUSE					(BYTE)0
#define SYSTASK2SEND			(BYTE)1
#define TASKSENDING				(BYTE)2

/*typedef struct tag_CrossStatus
{
	UINT8 Status;//cross status;ready,disconnect,unknown
	UINT8 NowOprIndex;//now operating client index
	UINT8 NowCommand;//now operating command no;
	UINT8 DoTrans;//是否正在进行事务操作
	UINT8 bTest;
}CrossStatus;
*/
/*typedef struct tag_CommandStatus
{
	UINT8 NowCommand;//now operating command no
	UINT8 Timeout;//now operating command timeout
}CCommandStatus;
*/

////系统中工作线程表结构
typedef struct tag_SlDBData
{
	HANDLE hDBThread;
	DWORD  dwThreadID ;
}SlDBData;

//系统中的设备（以路口机为中心）映射表结构
typedef struct tag_Mapping//某一项为空，则为该设备未配置
{
	UINT8 Status;//该路口的状态ready,disconnect,doing
	UINT8 ConnectTest;//路口连接性应答检测值
	UINT8 DoingTest;//操作无应答的最大时间
	UINT8 NowOprIndex;//正在操作该路口的操作员索引
	UINT8 NowCommand;//该路口正在执行的命令号;
	UINT8 ErrorNo;

//	UINT8 DoTransOpr;//该路口是否正在进行事务操作(0 没有,其他为操作客户号)
	UINT8 TransTest;//事务操作的最大未完成时间
	UINT8 TransHead;
	UINT8 TransTail;
	UINT8 AcceptTransItem;//接收事务结束项(0,不接收;其他为操作客户号)
//	UINT8 TaskIndex;//  + 1
	UINT8 TurnOver;//added 04 02,2001

	UINT8 lkbh[LKJ_NAME_LEN + 1];//6];//路口编号,与路口机编号同

	UINT8 xhdbh[MAX_XHD_NUM][XHD_NAME_LEN + 1];//8];//信号灯编号
	UINT8 jcqbh[MAX_JCQ_NUM][JCQ_NAME_LEN + 1];//8];//检测器编号
	UINT8 cdbh[MAX_JCQ_NUM][CD_NAME_LEN + 1];//2];//该路口的检测器所对应的车道编号
//	UINT8 lxb[MAX_JCQ_NUM];//车道当前的绿信比 per cent
}Mapping;

//系统中的客户映射表结构
typedef struct tag_OprData//主要用于连接性测试和命令超时时处理客户数据
{
//	UINT8 CommIndex;
/*	UINT8 RtData;//if need real time data
	UINT8 RtCross;//real time data cross no
	UINT8 RtCommand;//real time data command no
*/	UINT8 OprStatus;//与客户是否连通
	UINT8 TestData;//与客户连通性的测试值
	UINT8 Name[15];//用户名

	UINT8 NowCommand[MAX_CROSS_NUM];//客户已成功发送给路口机，但尚未收到路口机应答的命令号
	UINT8 IsTask[MAX_CROSS_NUM];//03,07,2001(false,不是任务,true = task)

//deleted 02,22,2001 (与Mapping 中的DoingTest不独立,故可省去)
//	UINT8 Timeout[MAX_CROSS_NUM];//与上对应；已经多少秒未收到路口机对该请求的应答了
}COprData;

//系统中的接收/发送缓冲结构
typedef struct tag_RecBuf
{
	UINT8 Status;//缓冲状态
	UINT8 OprIndex;//用户号
	UINT8 CrossNo;//路口号
	UINT8 CmdNo;//命令号
	UINT8 ReSendTimes;//重发次数
	UINT8 TestData;//超时重发的测试值

	UINT8 StepNoInTrans;//事务中操作步骤，表明发送顺序
/*	UINT8 TaskIndex;//  + 1(0,不是任务，1--MAX_TASK_NUM指向任务的索引，
					//MAX_TASK_NUM + 1 表明是一个未发送（仅进spool中）的任务)
*/
	UINT8 IsTask;//(0,不是任务) 03,07,2001 
	UINT8 TurnOver;//added 04 02,2001
	UINT8 MsgLen;//消息长度
	UINT8 MsgData[MAX_LPT_LEN];//消息体
}RecBuf;

//系统中需执行的任务结构
typedef struct tag_TaskData//需要发的任务列表
{
	UINT8 Status;//0,未用;1,以发但等zhao ack;2,以发但等hu ack;3,将要被系统清除;ff,正常使用
	UINT8 OprIndex[MAX_SOCKET_NUM/* + 1*/];//要求的用户序列(added 03,07,2001)
//	UINT8 CrossNo;//路口号
//	UINT8 CmdNo;//命令号
	UINT8 SendParam;//发送参数 0,停止，1--fd周期， ff有信号发，fe只发一次;
					//如果路口机不支持此模式，此仅值为发送周期
	UINT8 ReservedStatus;//added 03,07,2001
	UINT8 ReservedParam;
	UINT8 ReqTimes;//被请求的次数 added 03,06,2001
}TaskData;
//???如果发上来的消息没有用户需要，则发停止命令给路口机
//如果路口机收到停止命令，则发一长度为0的命令包
typedef struct tag_SysAutoTask
{
	UINT8 CmdNo;
	UINT32 SendParam;
	UINT32 CastInterval;
	UINT32 LeftTime;
//	UINT32 CrossNo;
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


#ifdef cplusplus
}
#endif

#pragma pack(pop)
#endif
