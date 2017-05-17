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

/* ODBC����ִ�д�����붨�� */
#define LINKDISCONNECT      "08S01"   /* �ں������ִ��ǰ������������
	                                     ���ӵ�����Դ֮���ͨѶ����ʧ�� */
#define CONTINUEFAILED      "40001"   /* ������ʧ�� */
/*** ���ݿ���������ʶ *******/

/**** �ɹ� *****/
#define DB_SQL_SUCCESS               (BYTE)0 

/**** Select���������ݷ��� *****/
#define DB_SQL_NODATAFOUND           (BYTE)1 

/**** Update,delect, insert�����޼�¼����Ӱ�� ****/
#define DB_SQL_NOROWUPDATED          (BYTE)2 

/**** �����ݿ�Sever�����ж� *****/
#define DB_SQL_LINKDISCONNECT        (BYTE)3

/**** һ���Դ��� ******/  
#define DB_SQL_GENERALLYERROR        (BYTE)4

/**** �Ƿ���DBNo  ****/ 
#define DB_SQL_ILLEGALDBNO           (BYTE)5

/**** �����յ���Ϣ��SQL���Ϊ��  ****/ 
#define DB_SQL_RECVSQLSTRINGNULL     (BYTE)6

/**** Select�����Ľ�������������������ַ������С ****/  
#define DB_SQL_RESULTOVERFLOW        (BYTE)7  

/**** ���� ODBC����ʱODBCϵͳ���� ****/
#define DB_SQL_SYSERROR              (BYTE)8

/**** �������鶨�巶Χ  ****/
#define DB_SQL_CONTINUE          (BYTE)51
#define CHECK_CONTINUE			 (BYTE)51	

#define MAX_RET_COLNUMBER       50
#define MAX_COL_LEN             400

#define MAX_SQLSTRING_LEN       1024    /* SQL�����󳤶� */
#define MAX_RETRESULT_LEN       1024    /* ���ص������������ */
#define MAX_SQL_LEN				2048	/* SQL������󳤶� */
#define	MAX_DBNAME_LEN			20		/* ���ݿ��ж���������󳤶� */
#define MAX_DB_NUM				100		/* һ��DB�����ݿ������� */

#define ZHAO_DATA_LEN			(UINT8)10
#define HU_DATA_LEN				(UINT8)4

#define MAX_CROSS_NUM			(UINT32)100
#define MAX_SPOOL_NUM			(UINT32)256
#define	MAX_TASK_NUM			(UINT32)32
#define	MAX_LPT_LEN				(UINT32)120  //��СҲҪ���������ݳ��ȴ� 7 
#define	MAX_SYSTASK_NUM			(UINT32)10

#define MAX_RESEND_TIME			(UINT8)3
#define	SENDBUF_TIMEOUT			(UINT8)3
#define	SPOOL_TIMEOUT			(UINT8)30
#define MAX_OPR_TIME			(UINT8)30//????
#define	OPR_WAITTIME			(UINT8)15//????
#define CROSS_CONNECTTIME		(UINT8)20//10SΪ��λ
#define CROSS_DOINGTIME			(UINT8)10//10SΪ��λ
#define CROSS_TRANSTIME			(UINT8)60//10SΪ��λ

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
	UINT8 DoTrans;//�Ƿ����ڽ����������
	UINT8 bTest;
}CrossStatus;
*/
/*typedef struct tag_CommandStatus
{
	UINT8 NowCommand;//now operating command no
	UINT8 Timeout;//now operating command timeout
}CCommandStatus;
*/

////ϵͳ�й����̱߳�ṹ
typedef struct tag_SlDBData
{
	HANDLE hDBThread;
	DWORD  dwThreadID ;
}SlDBData;

//ϵͳ�е��豸����·�ڻ�Ϊ���ģ�ӳ���ṹ
typedef struct tag_Mapping//ĳһ��Ϊ�գ���Ϊ���豸δ����
{
	UINT8 Status;//��·�ڵ�״̬ready,disconnect,doing
	UINT8 ConnectTest;//·��������Ӧ����ֵ
	UINT8 DoingTest;//������Ӧ������ʱ��
	UINT8 NowOprIndex;//���ڲ�����·�ڵĲ���Ա����
	UINT8 NowCommand;//��·������ִ�е������;
	UINT8 ErrorNo;

//	UINT8 DoTransOpr;//��·���Ƿ����ڽ����������(0 û��,����Ϊ�����ͻ���)
	UINT8 TransTest;//������������δ���ʱ��
	UINT8 TransHead;
	UINT8 TransTail;
	UINT8 AcceptTransItem;//�������������(0,������;����Ϊ�����ͻ���)
//	UINT8 TaskIndex;//  + 1
	UINT8 TurnOver;//added 04 02,2001

	UINT8 lkbh[LKJ_NAME_LEN + 1];//6];//·�ڱ��,��·�ڻ����ͬ

	UINT8 xhdbh[MAX_XHD_NUM][XHD_NAME_LEN + 1];//8];//�źŵƱ��
	UINT8 jcqbh[MAX_JCQ_NUM][JCQ_NAME_LEN + 1];//8];//��������
	UINT8 cdbh[MAX_JCQ_NUM][CD_NAME_LEN + 1];//2];//��·�ڵļ��������Ӧ�ĳ������
//	UINT8 lxb[MAX_JCQ_NUM];//������ǰ�����ű� per cent
}Mapping;

//ϵͳ�еĿͻ�ӳ���ṹ
typedef struct tag_OprData//��Ҫ���������Բ��Ժ����ʱʱ����ͻ�����
{
//	UINT8 CommIndex;
/*	UINT8 RtData;//if need real time data
	UINT8 RtCross;//real time data cross no
	UINT8 RtCommand;//real time data command no
*/	UINT8 OprStatus;//��ͻ��Ƿ���ͨ
	UINT8 TestData;//��ͻ���ͨ�ԵĲ���ֵ
	UINT8 Name[15];//�û���

	UINT8 NowCommand[MAX_CROSS_NUM];//�ͻ��ѳɹ����͸�·�ڻ�������δ�յ�·�ڻ�Ӧ��������
	UINT8 IsTask[MAX_CROSS_NUM];//03,07,2001(false,��������,true = task)

//deleted 02,22,2001 (��Mapping �е�DoingTest������,�ʿ�ʡȥ)
//	UINT8 Timeout[MAX_CROSS_NUM];//���϶�Ӧ���Ѿ�������δ�յ�·�ڻ��Ը������Ӧ����
}COprData;

//ϵͳ�еĽ���/���ͻ���ṹ
typedef struct tag_RecBuf
{
	UINT8 Status;//����״̬
	UINT8 OprIndex;//�û���
	UINT8 CrossNo;//·�ں�
	UINT8 CmdNo;//�����
	UINT8 ReSendTimes;//�ط�����
	UINT8 TestData;//��ʱ�ط��Ĳ���ֵ

	UINT8 StepNoInTrans;//�����в������裬��������˳��
/*	UINT8 TaskIndex;//  + 1(0,��������1--MAX_TASK_NUMָ�������������
					//MAX_TASK_NUM + 1 ������һ��δ���ͣ�����spool�У�������)
*/
	UINT8 IsTask;//(0,��������) 03,07,2001 
	UINT8 TurnOver;//added 04 02,2001
	UINT8 MsgLen;//��Ϣ����
	UINT8 MsgData[MAX_LPT_LEN];//��Ϣ��
}RecBuf;

//ϵͳ����ִ�е�����ṹ
typedef struct tag_TaskData//��Ҫ���������б�
{
	UINT8 Status;//0,δ��;1,�Է�����zhao ack;2,�Է�����hu ack;3,��Ҫ��ϵͳ���;ff,����ʹ��
	UINT8 OprIndex[MAX_SOCKET_NUM/* + 1*/];//Ҫ����û�����(added 03,07,2001)
//	UINT8 CrossNo;//·�ں�
//	UINT8 CmdNo;//�����
	UINT8 SendParam;//���Ͳ��� 0,ֹͣ��1--fd���ڣ� ff���źŷ���feֻ��һ��;
					//���·�ڻ���֧�ִ�ģʽ���˽�ֵΪ��������
	UINT8 ReservedStatus;//added 03,07,2001
	UINT8 ReservedParam;
	UINT8 ReqTimes;//������Ĵ��� added 03,06,2001
}TaskData;
//???�������������Ϣû���û���Ҫ����ֹͣ�����·�ڻ�
//���·�ڻ��յ�ֹͣ�����һ����Ϊ0�������
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
