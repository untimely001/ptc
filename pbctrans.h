#define TEST

#ifndef _WINCOMM_H
#define _WINCOMM_H

#ifdef cplusplus
extern "C"{
#endif

#define USECOMM      // yes, we need the COMM API

#include <windows.h>
//#include <string.h>
//#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <winsock.h>
#include <process.h>
#include <memory.h>
//#include <mmsystem.h>
#include <sql.h>				
#include <sqlext.h>		

#pragma pack(push,1)
//#pragma pack(1)

#ifdef TEST
	void CommOutputDebugString(const char *);
#else
	void __DebugWindowUINT32(const char * DisplayMessage);
#endif


/*	DEFINED CONST BEGIN	*/
#define __CONFIG_FILE__					".\\pbctrans.ini"
#define APPLICATION_NAME				"PBCTRANSTCP"
#define APPLICATION_TITLE				"PTRANSControlWindow"

#define PBCTRANS_TCP_PORT				8126
#define NO_FLAG_SET						0
#define REPEAT_SEND_PACKET_TIME			1000
#define	TIMEOUT_CHECK_TIME				100
#define	WAIT_ACK_TIME					10000
#define WAIT_SYN_TIME					1000
#define MAX_CHECK_NUM					3
#define	CHECKINTERVAL					10

// hard coded maximum number of ports for device under Win32
#define MAXPORTS        4

// terminal size

#define MAXBLOCK        250

#define MAXLEN_TEMPSTR  81

#define RXQUEUE         4096
#define TXQUEUE         4096
// Flow control flags

#define FC_DTRDSR       0x01
#define FC_RTSCTS       0x02
#define FC_XONXOFF      0x04

// ascii definitions

#define ASCII_BEL       0x07
#define ASCII_BS        0x08
#define ASCII_LF        0x0A
#define ASCII_CR        0x0D
#define ASCII_XON       0x11
#define ASCII_XOFF      0x13

#define A_CHAR			(UINT8)0x01
#define B_CHAR			(UINT8)0x02
#define C_CHAR			(UINT8)0x03
#define D_CHAR			(UINT8)0x04
#define E_CHAR			(UINT8)0x05
#define F_CHAR			(UINT8)0x06
#define G_CHAR			(UINT8)0x07
#define MAX_CHAR_VALUE	(UINT8)0x07

#define HU_HEAD_LEN		(UINT8)5
#define ZHAO_HEAD_LEN	(UINT8)3
#define MIN_YLF_LEN		(UINT8)9
#define MAX_REC_DATA	1400
#define MAX_QRSTR_LEN	512

/*	Message	*/
//define event type
#define	SYSICONNOTIFY					(WM_USER + 998)
#define INITIALEVENT					(WM_USER + 2)
#define WSA_ACCEPT						(WM_USER + 4)
#define WSA_READ						(WM_USER + 5)
#define WSA_CONNECT						(WM_USER + 6)
#define REPEAT_SEND_WAIT_PACKET			(WM_USER + 7)
#define TIMEOUT_CHECK					(WM_USER + 8)
#define IS_SENT_CHECK					(WM_USER + 9)
#define LPT_READ						(WM_USER + 10)
#define UDP_READ						(WM_USER + 11)
#define SERIAL_READ						(WM_USER + 12)
#define SERIAL_WRITE					(WM_USER + 13)
#define DB_INITOK_EVENT					(WM_USER + 14)
#define WM_FACEUNLOAD					(WM_USER + 17)
#define SENDCTR_EVENT					(WM_USER + 19)
#define SVR_RECONNECT					(WM_USER + 20)
#define LCUBUF_RESEND					(WM_USER + 21)
#define EXTENDMSGSEND					(WM_USER + 22)

/*	RANGE	*/
#define MAX_MSG_LEN						(WORD)1400
#define MAX_SOCKET_NUM					(BYTE)100
#define MAX_RESEND_NUM					(BYTE)100
#define MAX_POST_NUM					(BYTE)5
#define MAX_PENDING_CONNECTS			(BYTE)8
#define MAX_RECEIVE_ERROR_NUM			(BYTE)20
#define RECEIVE_BUFFER_LEN				512
#define NAME_LEN						20
#define MAX_APPLICATION_WAIT_QUEUE_LEN	100

#define TCP_CHECK_TYPE		0
#define REQUEST_TYPE		1
#define CONFIRM_TYPE		2
#define NOTIFY_TYPE			3
#define TRANSFER_TYPE		4

//define access token for nonWin32
#ifndef _WIN32
#define SM_IDLESTATUS	0x00
#define SM_BUSYSTATUS   0xFF
#endif

#define BUFFERSIZE		511
#define MAX_DISPLAY_LEN 400
//#define MAX_REC_DATA	1024
/*	DEFINED CONST END	*/

typedef unsigned char	BYTE8;
/*typedef unsigned long	INT32;*/
typedef unsigned char	BOOLBYTE;
typedef unsigned long	WORD32;

typedef unsigned char	UINT8;
typedef unsigned short	UINT16;
//typedef unsigned long	UINT32;
typedef unsigned char	BOOLEAN;


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
#define MAX_COL_LEN             255

#define MAX_SQLSTRING_LEN       1024    /* SQL语句最大长度 */
#define MAX_RETRESULT_LEN       1024    /* 返回的最大结果集长度 */
#define MAX_SQL_LEN				2048	/* SQL语句的最大长度 */
#define	MAX_DBNAME_LEN			20		/* 数据库中对象名的最大长度 */
#define MAX_DB_NUM				100		/* 一个DB上数据库最大个数 */


/*	DEFINED DATA TYPE BEGIN	*/
typedef struct
{
	char * hMsgBuf;
	int MsgLen;
} MsgWaitNode;
typedef struct
{    
	SOCKET Socket;
	DWORD PeerIPAddr;//network order
	BYTE PeerTaskId;//client must register
   
	BYTE WaitSendCount;			
	BYTE WaitHead;
	BYTE WaitEnd;

	BYTE ReceErrorCount;
    int LeftLen;
    char ReceBuff[RECEIVE_BUFFER_LEN];

	BOOL LinkStatus;
    BYTE CheckLinkCount;
}SocketDataNode;

typedef struct tag_MsgHead
{
	BYTE Packet1;//~MsgLen
	BYTE Packet2;//~Event
	WORD MsgLen; //header length not inluded
	BYTE Type;//0测试,1Client请求,2server应答,3server通知,>10请求server转发
	DWORD SelfIP;
	BYTE SelfTaskId;
	DWORD TargetIP;
	BYTE TargetTaskId;
	WORD Event;
	WORD Reserved;
//	BYTE ErrCode;//0=success,other=errorcode
//	BYTE TurnOver;
}MsgHead;

typedef struct tag_DevMap
{
	unsigned char SockIndex;//套接字存储的位置
	int           DevId;
}DevMap;
typedef struct tag_Test
{
	BYTE h1;	
	BYTE h2;
	UINT16 MsgLen;
	BYTE cmdNo;        //CM_TEST心跳
	BYTE CustType;     //客户类型
	UINT16 EquipID;    //设备编号
}TestMsg;
typedef struct tag_StationStatusReq
{
	BYTE h1;	
	BYTE h2;
	UINT16 MsgLen;
	BYTE cmdNo;				//CM_STATIONSTATUSReq请求站台状态信息
	UINT16 StationNo;
}StationStatusReq;
typedef struct tag_StationStatus
{
	BYTE h1;	
	BYTE h2;
	UINT16 MsgLen;
	BYTE cmdNo;				//CM_STATIONSTATUS 站台状态信息
	UINT16 StationNo;
	BYTE BusPits;			//停车位数
	BYTE BusWaiting;		//等待车辆数
	UINT16 BusId1;			//车位1上的公交车ID 
	UINT16 BusId2;			//车位2上的公交车ID
	UINT16 BusId3;			//车位3上的公交车ID
	UINT16 BusId4;			//车位4上的公交车ID
	UINT16 BusId5;			//车位5上的公交车ID
}StationStatus;
typedef struct tag_BusStatusReq
{
	BYTE h1;	
	BYTE h2;
	UINT16 MsgLen;
	BYTE cmdNo;				//CM_BUSSTATUSREQ,			//请求公交车状态信息
	BYTE Aux;
	UINT16 BusNo;
}BusStatusReq;
typedef struct tag_BusStatus
{
	BYTE h1;	
	BYTE h2;
	UINT16 MsgLen;
	BYTE cmdNo;				//CM_BUSSTATUS,			//公交车状态信息
	UINT16 BusNo;
	double Longitude;		//经度
	double Latitude;		//纬度
	BYTE Direction;			//方向（上行/下行）
	float Velocity;			//即时行驶速度
	BYTE Year;				//时间
	BYTE Month;
	BYTE Day;
	BYTE Hour;
	BYTE Minute;
	BYTE Second;
	UINT16 Passengers;		//乘客人数
}BusStatus;
typedef struct tag_StationInitReq
{
	BYTE h1;	
	BYTE h2;
	UINT16 MsgLen;
	BYTE cmdNo;				//CM_STATIONINITREQ		//站台初始化请求
	UINT16 StationNo;
}StationInitReq;
typedef struct tag_StationInit
{
	BYTE h1;	
	BYTE h2;
	UINT16 MsgLen;
	BYTE cmdNo;				//CM_STATIONINIT			//站台初始化信息
	UINT16 StationNo;
	BYTE BusPits;			//停车位数
	BYTE BusLines;			//停靠的公交线路数
	double Longitude;		//经度
	double Latitude;		//纬度
	UINT16 BusLineIDs[20];	//停靠的公交线路
	char Name[40];			//车站名
	int  Reserved;			//保留
}StationInit;
typedef struct tag_BusInitReq
{
	BYTE h1;	
	BYTE h2;
	UINT16 MsgLen;
	BYTE cmdNo;				//CM_BUSINITREQ,				//公交车初始化请求
	BYTE Auxiliary;       //意义待定
	UINT16 BusNo;
}BusInitReq;
typedef struct tag_BusInit
{
	BYTE h1;	
	BYTE h2;
	UINT16 MsgLen;
	BYTE cmdNo;				//CM_BUSINITREQ,				//公交车初始化
	UINT16 BusNo;
	char NumberPlate[10];     //车牌号
	UINT16 BusLineID;		//公交线号
	UINT16 BusType;			//公交车类型
	float DepRate;			//折旧率
	float Price;			//价格
	int  Reserved;			//保留
}BusInit;
typedef struct tag_BusComing
{
	BYTE h1;	
	BYTE h2;
	UINT16 MsgLen;
	BYTE cmdNo;			//CM_BUSCOMING,				//公交车到站通告
	UINT16 StationNo;
	UINT16 BusID;		//车辆号
	BYTE Direction;		//方向（上行/下行）
	BYTE Year;			//时间
	BYTE Month;
	BYTE Day;
	BYTE Hour;
	BYTE Minute;
	BYTE Second;
	UINT16 Passengers;	//乘客人数
}BusComing;
typedef struct tag_BusComingAck
{
	BYTE h1;	
	BYTE h2;
	UINT16 MsgLen;
	BYTE cmdNo;			//CM_BUSCOMINGACK,				//公交车到站通告应答
	UINT16 StationNo;
	UINT16 BusID;		//车辆号
}BusComingAck;
typedef struct tag_BusStopping
{
	BYTE h1;	
	BYTE h2;
	UINT16 MsgLen;
	BYTE cmdNo;			//CM_BUSSTOPPing,			公交车到达车位通告
	UINT16 StationNo;
	UINT16 BusID;		//车辆号
	BYTE Direction;		//方向（上行/下行）
	BYTE BusPit;		//车位号
	BYTE Year;				//时间
	BYTE Month;
	BYTE Day;
	BYTE Hour;
	BYTE Minute;
	BYTE Second;
	UINT16 Passengers;	//乘客人数
}BusStopping;
typedef struct tag_BusStoppingAck
{
	BYTE h1;	
	BYTE h2;
	UINT16 MsgLen;
	BYTE cmdNo;			//CM_BUSSTOPPingAck,			公交车到达车位通告应答
	UINT16 StationNo;
	UINT16 BusID;		//车辆号
}BusStoppingAck;
typedef struct tag_BusLeaving
{
	BYTE h1;	
	BYTE h2;
	UINT16 MsgLen;
	BYTE cmdNo;			//CM_BUSLEAVING,				//公交车离站通告
	UINT16 StationNo;
	UINT16 BusID;		//车辆号
	BYTE Direction;		//方向（上行/下行）
	BYTE BusPit;		//车位号
	BYTE Year;				//时间
	BYTE Month;
	BYTE Day;
	BYTE Hour;
	BYTE Minute;
	BYTE Second;
	UINT16 Passengers;	//乘客人数
}BusLeaving;
typedef struct tag_BusLeavingAck
{
	BYTE h1;	
	BYTE h2;
	UINT16 MsgLen;
	BYTE cmdNo;			//CM_BUSLEAVING,				//公交车离站通告
	UINT16 StationNo;
	UINT16 BusID;		//车辆号
}BusLeavingAck;
typedef struct tag_BusRunning
{
	BYTE h1;	
	BYTE h2;
	UINT16 MsgLen;
	BYTE cmdNo;			//CM_BUSRUNNING,		//公交车站间通告
	UINT16 BusID;		//车辆号
	BYTE Direction;		//方向（上行/下行）
	double Longitude;	//经度
	double Latitude;	//纬度
	float Velocity;		//即时行驶速度
	BYTE Year;				//时间
	BYTE Month;
	BYTE Day;
	BYTE Hour;
	BYTE Minute;
	BYTE Second;
	UINT16 Passengers;	//乘客人数
}BusRunning;
typedef struct tag_BusRunningAck
{
	BYTE h1;	
	BYTE h2;
	UINT16 MsgLen;
	BYTE cmdNo;			//CM_BUSRUNNING,		//公交车站间通告
	UINT16 BusID;		//车辆号
}BusRunningAck;
typedef struct tag_Logon
{
	BYTE h1;	
	BYTE h2;
	UINT16 MsgLen;
	BYTE cmdNo;			//CM_LOGON,					//登录
	BYTE CustType;		//客户类型	0=站台, 1=BUS, 2=其它
	UINT16 EquipID;		//设备编号
	char UserID[20];
	char Password[20];
}Logon;
typedef struct tag_LogonAck
{
	BYTE h1;	
	BYTE h2;
	UINT16 MsgLen;
	BYTE cmdNo;			//CM_LOGON,					//登录
	BYTE CustType;		//客户类型	0=站台, 1=BUS, 2=其它
	UINT16 EquipID;		//设备编号
	char UserID[20];
	char Password[20];
}LogonAck;
typedef struct tag_Logout
{
	BYTE h1;	
	BYTE h2;
	UINT16 MsgLen;
	BYTE cmdNo;			//CM_LOGOUT,					//登出
	BYTE CustType;		//客户类型	0=站台, 1=BUS, 2=其它
	UINT16 EquipID;		//设备编号
	char UserID[20];
//	char Password[20];
}Logout;
typedef struct tag_LogoutAck
{
	BYTE h1;	
	BYTE h2;
	UINT16 MsgLen;
	BYTE cmdNo;			//CM_LOGOUT,					//登出
	BYTE CustType;		//客户类型	0=站台, 1=BUS, 2=其它
	UINT16 EquipID;		//设备编号
	char UserID[20];
	//	char Password[20];
}LogoutAck;
typedef struct tag_ChangePassword
{
	BYTE h1;	
	BYTE h2;
	UINT16 MsgLen;
	BYTE cmdNo;			//CM_CHANGEPASSWORD,			//修改登录密码
	BYTE CustType;		//客户类型	0=站台, 1=BUS, 2=其它
	UINT16 EquipID;		//设备编号
	char UserID[20];
	char NewPassword[20];
}ChangePassword;
typedef struct tag_ChangePasswordAck
{
	BYTE h1;	
	BYTE h2;
	UINT16 MsgLen;
	BYTE cmdNo;			//CM_CHANGEPASSWORD,			//修改登录密码
	BYTE CustType;		//客户类型	0=站台, 1=BUS, 2=其它
	UINT16 EquipID;		//设备编号
	char UserID[20];
	char NewPassword[20];
}ChangePasswordAck;

typedef struct tag_AlarmMsg
{
	BYTE h1;	
	BYTE h2;
	UINT16 MsgLen;
	BYTE cmdNo;         //CM_ALARMMSG,		//异常报警信息
	BYTE CustType;		//客户类型	0=站台, 1=BUS, 2=其它
	UINT16 EquipID;		//设备编号
	UINT32 aux;			//辅助字节，具体待定义
	UINT32 Status;		//异常状态字，如电池、电源、通讯口等各种设备状态，具体待定义
}AlarmMsg;
typedef struct tag_SysTimeReq
{
	BYTE h1;	
	BYTE h2;
	UINT16 MsgLen;
	BYTE cmdNo;			//CM_SYSTIMEREQ    请求系统时间
	BYTE CustType;		//客户类型	0=站台, 1=BUS, 2=其它
	UINT16 EquipID;		//设备编号
	UINT32 aux;			//辅助字节，具体待定义
}SysTimeReq;
typedef struct tag_SysTime
{
	BYTE h1;	
	BYTE h2;
	UINT16 MsgLen;
	BYTE cmdNo;				//CM_SYSTIME    系统时间
	BYTE CustType;			//客户类型	0=站台, 1=BUS, 2=其它
	UINT16 EquipID;			//设备编号
	UINT32 aux;				//辅助字节，具体待定义
	BYTE Year;				//时间
	BYTE Month;
	BYTE Day;
	BYTE Hour;
	BYTE Minute;
	BYTE Second;
}SysTime;
typedef struct tag_BusLineReq
{
	BYTE h1;	
	BYTE h2;
	UINT16 MsgLen;
	BYTE cmdNo;				//CM_BUSLINEREQ    请求公交线路
	BYTE CustType;			//客户类型	0=站台, 1=BUS, 2=其它
	UINT16 EquipID;			//设备编号
	UINT16 BusLineID;		//公交线号
}BusLineReq;
typedef struct tag_BusLine
{
	BYTE h1;	
	BYTE h2;
	UINT16 MsgLen;
	BYTE cmdNo;				//CM_BUSLINE    公交线路信息
	BYTE CustType;			//客户类型	0=站台, 1=BUS, 2=其它
	UINT16 EquipID;			//设备编号
	UINT16 BusLineID;		//公交线号
	char BusLineName[20];	//公交线路名
	UINT16 StationNum;		//站台数	
	UINT16 Stations[50];	//经过的站台号 
	UINT32 Reserved;
}BusLine;
typedef struct tag_SerialCtr
{
	HANDLE hThread;
	DWORD dwThreadID;
	UINT8 bUse;
}SerialCtr;

// macros ( for easier readability )
#define IS_NT	(IS_WIN32 && (BOOL)(GetVersion() < 0x80000000))	
#define RETCODE_IS_FAILURE(x) ((x) == SQL_ERROR || (x)==SQL_INVALID_HANDLE || (x)==SQL_STILL_EXECUTING)

// function prototypes (private)
LONG MyUnhandledExceptionFilter(struct _EXCEPTION_POINTERS *ExceptionInfo);
BOOL InitApplication( HINSTANCE ) ;
BOOL InitInstance( HINSTANCE, int ) ;
BOOL CreateSerialInfo(void) ;
BOOL DestroySerialInfo(void) ;
BOOL ProcessSerialCharacter(BYTE ) ;
int ReadCommBlock(LPSTR, int ) ;
BOOL OpenConnection(void) ;
BOOL SetupConnection(void) ;
BOOL CloseConnection(void) ;
void OnNotify(DWORD dwLength);
void __debugWindows(const char* fmt,...);
LRESULT FAR PASCAL WndProc( HWND, UINT, WPARAM, LPARAM ) ;
DWORD FAR PASCAL CommWatchProc( LPSTR ) ;

void TcpResend(BYTE);
BOOL InWaitSendQueue(BYTE, WORD, char *);
BOOL TcpSend(BYTE, WORD, char *);
BOOL ReceiveSendData(char * pBuff,WORD MsgLen);
void ReceiveData(BYTE);
void CloseSocketConnect(BYTE);
void ClearWaitSendQueue(BYTE);
BYTE SocketInsertQueue(DWORD);
void CancelBlocking(void);
void AcceptConnect(WPARAM, LPARAM);
LRESULT FAR PASCAL 	WndProc(HWND, UINT, WPARAM, LPARAM);
BOOL InitTCPIP(void);
BOOL InitUdp(void);
BOOL InitUdpSvr(void);
BOOL InitUdpClient(void);
BOOL InitUdpData(void);
BOOL GetLocalIp(void);
BOOL ConnectSvrProc(void);
void InitSocketStatus(BYTE);//???????????
void InitSocketData(BYTE i);
BOOL ReConnectSvrProc(DWORD SvrIpAddr);
BYTE FindCommIndex(DWORD TargetIP,BYTE TargetTaskId);
WINAPI WinMain(HINSTANCE, HINSTANCE, char *, int);
void InitApp(void);

void DispatchClientReqProc(BYTE Index,void *pMsgIn,WORD MsgLen);
void ConnectPeer(WPARAM wParam, LPARAM lParam);

BYTE DoUpdate(char *SQLString,UINT16 WaitTime);
void rtrim(char *Instr);
INT32  dbSQLExecDirect(HSTMT hstmt, char *SQLString);
void MapDeviceSock(BYTE index,int DeviceId);
BYTE GetSockfromDeviceId(int DeviceId);
int GetDevIdfromIndex(BYTE Index);
void DeviceLostProc(BYTE Index);
void PackNotifyMsg(int DevId,UINT8 CmdNo,char *params,BYTE Index);

// function prototypes (public)
BOOL SendData(char * pBuff,WORD MsgLen,SOCKET s);//socket
BOOL SendDataEx(char * pBuff,WORD MsgLen,unsigned char SockIndex);
void DebugWindow(char *);
void OsClearWindow(void);

void CheckSvrLink(void);
int InterpretClient(void * pMsgBuf,int RecLen,BYTE Index);
void GetAppConfig(void);
BOOL Initdb(void);
BOOL GetSvrIp(char *SvrIpSting);
BYTE FormCheckSum(char *InPtr,int Length);
BOOL IsCheckSumOk(char *InPtr,int Length);
int ConnectProc(int CmdNo,BYTE Index);

BYTE  GetSQLError(HSTMT hstmt);
BYTE GetNotify(void);

int GetMsgLen(int cmd);
void StationStatusProc(StationStatus *pMsg);
void BusStatusProc(BusStatus *pMsg);
BYTE GetStationBaseInfo(StationInit *msg,StationInitReq *inmsg);
BYTE GetStationBusLines(StationInit *msg,StationInitReq *inmsg);
BYTE GetBusInfo(BusInit *msg,BusInitReq *inmsg);
void BusComingProc(BusComing *pMsg);
void BusStoppingProc(BusStopping *pMsg);
void BusLeavingProc(BusLeaving *pMsg);
void BusRunningProc(BusRunning *pMsg);
BYTE LogonProc(Logon *pMsg);
BYTE LogoutProc(Logout *pMsg);
BYTE GetBusLineName(BusLine *msg,BusLineReq *inmsg);
BYTE GetBusLineStations(BusLine *msg,BusLineReq *inmsg);

void MsgTestProc(void);
void Utf2Gb2312(BYTE *putf, int Len,char *pOut,int OutLen);
void Gb2312Utf(BYTE *p2312, int Len,char *pOut,int OutLen);

#ifdef cplusplus
}
#endif

#pragma pack(pop)

#endif