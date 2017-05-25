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
#define MAX_COL_LEN             255

#define MAX_SQLSTRING_LEN       1024    /* SQL�����󳤶� */
#define MAX_RETRESULT_LEN       1024    /* ���ص������������ */
#define MAX_SQL_LEN				2048	/* SQL������󳤶� */
#define	MAX_DBNAME_LEN			20		/* ���ݿ��ж���������󳤶� */
#define MAX_DB_NUM				100		/* һ��DB�����ݿ������� */


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
	BYTE Type;//0����,1Client����,2serverӦ��,3server֪ͨ,>10����serverת��
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
	unsigned char SockIndex;//�׽��ִ洢��λ��
	int           DevId;
}DevMap;
typedef struct tag_Test
{
	BYTE h1;	
	BYTE h2;
	UINT16 MsgLen;
	BYTE cmdNo;        //CM_TEST����
	BYTE CustType;     //�ͻ�����
	UINT16 EquipID;    //�豸���
}TestMsg;
typedef struct tag_StationStatusReq
{
	BYTE h1;	
	BYTE h2;
	UINT16 MsgLen;
	BYTE cmdNo;				//CM_STATIONSTATUSReq����վ̨״̬��Ϣ
	UINT16 StationNo;
}StationStatusReq;
typedef struct tag_StationStatus
{
	BYTE h1;	
	BYTE h2;
	UINT16 MsgLen;
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
	BYTE h1;	
	BYTE h2;
	UINT16 MsgLen;
	BYTE cmdNo;				//CM_BUSSTATUSREQ,			//���󹫽���״̬��Ϣ
	BYTE Aux;
	UINT16 BusNo;
}BusStatusReq;
typedef struct tag_BusStatus
{
	BYTE h1;	
	BYTE h2;
	UINT16 MsgLen;
	BYTE cmdNo;				//CM_BUSSTATUS,			//������״̬��Ϣ
	UINT16 BusNo;
	double Longitude;		//����
	double Latitude;		//γ��
	BYTE Direction;			//��������/���У�
	float Velocity;			//��ʱ��ʻ�ٶ�
	BYTE Year;				//ʱ��
	BYTE Month;
	BYTE Day;
	BYTE Hour;
	BYTE Minute;
	BYTE Second;
	UINT16 Passengers;		//�˿�����
}BusStatus;
typedef struct tag_StationInitReq
{
	BYTE h1;	
	BYTE h2;
	UINT16 MsgLen;
	BYTE cmdNo;				//CM_STATIONINITREQ		//վ̨��ʼ������
	UINT16 StationNo;
}StationInitReq;
typedef struct tag_StationInit
{
	BYTE h1;	
	BYTE h2;
	UINT16 MsgLen;
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
	BYTE h1;	
	BYTE h2;
	UINT16 MsgLen;
	BYTE cmdNo;				//CM_BUSINITREQ,				//��������ʼ������
	BYTE Auxiliary;       //�������
	UINT16 BusNo;
}BusInitReq;
typedef struct tag_BusInit
{
	BYTE h1;	
	BYTE h2;
	UINT16 MsgLen;
	BYTE cmdNo;				//CM_BUSINITREQ,				//��������ʼ��
	UINT16 BusNo;
	char NumberPlate[10];     //���ƺ�
	UINT16 BusLineID;		//�����ߺ�
	UINT16 BusType;			//����������
	float DepRate;			//�۾���
	float Price;			//�۸�
	int  Reserved;			//����
}BusInit;
typedef struct tag_BusComing
{
	BYTE h1;	
	BYTE h2;
	UINT16 MsgLen;
	BYTE cmdNo;			//CM_BUSCOMING,				//��������վͨ��
	UINT16 StationNo;
	UINT16 BusID;		//������
	BYTE Direction;		//��������/���У�
	BYTE Year;			//ʱ��
	BYTE Month;
	BYTE Day;
	BYTE Hour;
	BYTE Minute;
	BYTE Second;
	UINT16 Passengers;	//�˿�����
}BusComing;
typedef struct tag_BusComingAck
{
	BYTE h1;	
	BYTE h2;
	UINT16 MsgLen;
	BYTE cmdNo;			//CM_BUSCOMINGACK,				//��������վͨ��Ӧ��
	UINT16 StationNo;
	UINT16 BusID;		//������
}BusComingAck;
typedef struct tag_BusStopping
{
	BYTE h1;	
	BYTE h2;
	UINT16 MsgLen;
	BYTE cmdNo;			//CM_BUSSTOPPing,			���������ﳵλͨ��
	UINT16 StationNo;
	UINT16 BusID;		//������
	BYTE Direction;		//��������/���У�
	BYTE BusPit;		//��λ��
	BYTE Year;				//ʱ��
	BYTE Month;
	BYTE Day;
	BYTE Hour;
	BYTE Minute;
	BYTE Second;
	UINT16 Passengers;	//�˿�����
}BusStopping;
typedef struct tag_BusStoppingAck
{
	BYTE h1;	
	BYTE h2;
	UINT16 MsgLen;
	BYTE cmdNo;			//CM_BUSSTOPPingAck,			���������ﳵλͨ��Ӧ��
	UINT16 StationNo;
	UINT16 BusID;		//������
}BusStoppingAck;
typedef struct tag_BusLeaving
{
	BYTE h1;	
	BYTE h2;
	UINT16 MsgLen;
	BYTE cmdNo;			//CM_BUSLEAVING,				//��������վͨ��
	UINT16 StationNo;
	UINT16 BusID;		//������
	BYTE Direction;		//��������/���У�
	BYTE BusPit;		//��λ��
	BYTE Year;				//ʱ��
	BYTE Month;
	BYTE Day;
	BYTE Hour;
	BYTE Minute;
	BYTE Second;
	UINT16 Passengers;	//�˿�����
}BusLeaving;
typedef struct tag_BusLeavingAck
{
	BYTE h1;	
	BYTE h2;
	UINT16 MsgLen;
	BYTE cmdNo;			//CM_BUSLEAVING,				//��������վͨ��
	UINT16 StationNo;
	UINT16 BusID;		//������
}BusLeavingAck;
typedef struct tag_BusRunning
{
	BYTE h1;	
	BYTE h2;
	UINT16 MsgLen;
	BYTE cmdNo;			//CM_BUSRUNNING,		//������վ��ͨ��
	UINT16 BusID;		//������
	BYTE Direction;		//��������/���У�
	double Longitude;	//����
	double Latitude;	//γ��
	float Velocity;		//��ʱ��ʻ�ٶ�
	BYTE Year;				//ʱ��
	BYTE Month;
	BYTE Day;
	BYTE Hour;
	BYTE Minute;
	BYTE Second;
	UINT16 Passengers;	//�˿�����
}BusRunning;
typedef struct tag_BusRunningAck
{
	BYTE h1;	
	BYTE h2;
	UINT16 MsgLen;
	BYTE cmdNo;			//CM_BUSRUNNING,		//������վ��ͨ��
	UINT16 BusID;		//������
}BusRunningAck;
typedef struct tag_Logon
{
	BYTE h1;	
	BYTE h2;
	UINT16 MsgLen;
	BYTE cmdNo;			//CM_LOGON,					//��¼
	BYTE CustType;		//�ͻ�����	0=վ̨, 1=BUS, 2=����
	UINT16 EquipID;		//�豸���
	char UserID[20];
	char Password[20];
}Logon;
typedef struct tag_LogonAck
{
	BYTE h1;	
	BYTE h2;
	UINT16 MsgLen;
	BYTE cmdNo;			//CM_LOGON,					//��¼
	BYTE CustType;		//�ͻ�����	0=վ̨, 1=BUS, 2=����
	UINT16 EquipID;		//�豸���
	char UserID[20];
	char Password[20];
}LogonAck;
typedef struct tag_Logout
{
	BYTE h1;	
	BYTE h2;
	UINT16 MsgLen;
	BYTE cmdNo;			//CM_LOGOUT,					//�ǳ�
	BYTE CustType;		//�ͻ�����	0=վ̨, 1=BUS, 2=����
	UINT16 EquipID;		//�豸���
	char UserID[20];
//	char Password[20];
}Logout;
typedef struct tag_LogoutAck
{
	BYTE h1;	
	BYTE h2;
	UINT16 MsgLen;
	BYTE cmdNo;			//CM_LOGOUT,					//�ǳ�
	BYTE CustType;		//�ͻ�����	0=վ̨, 1=BUS, 2=����
	UINT16 EquipID;		//�豸���
	char UserID[20];
	//	char Password[20];
}LogoutAck;
typedef struct tag_ChangePassword
{
	BYTE h1;	
	BYTE h2;
	UINT16 MsgLen;
	BYTE cmdNo;			//CM_CHANGEPASSWORD,			//�޸ĵ�¼����
	BYTE CustType;		//�ͻ�����	0=վ̨, 1=BUS, 2=����
	UINT16 EquipID;		//�豸���
	char UserID[20];
	char NewPassword[20];
}ChangePassword;
typedef struct tag_ChangePasswordAck
{
	BYTE h1;	
	BYTE h2;
	UINT16 MsgLen;
	BYTE cmdNo;			//CM_CHANGEPASSWORD,			//�޸ĵ�¼����
	BYTE CustType;		//�ͻ�����	0=վ̨, 1=BUS, 2=����
	UINT16 EquipID;		//�豸���
	char UserID[20];
	char NewPassword[20];
}ChangePasswordAck;

typedef struct tag_AlarmMsg
{
	BYTE h1;	
	BYTE h2;
	UINT16 MsgLen;
	BYTE cmdNo;         //CM_ALARMMSG,		//�쳣������Ϣ
	BYTE CustType;		//�ͻ�����	0=վ̨, 1=BUS, 2=����
	UINT16 EquipID;		//�豸���
	UINT32 aux;			//�����ֽڣ����������
	UINT32 Status;		//�쳣״̬�֣����ء���Դ��ͨѶ�ڵȸ����豸״̬�����������
}AlarmMsg;
typedef struct tag_SysTimeReq
{
	BYTE h1;	
	BYTE h2;
	UINT16 MsgLen;
	BYTE cmdNo;			//CM_SYSTIMEREQ    ����ϵͳʱ��
	BYTE CustType;		//�ͻ�����	0=վ̨, 1=BUS, 2=����
	UINT16 EquipID;		//�豸���
	UINT32 aux;			//�����ֽڣ����������
}SysTimeReq;
typedef struct tag_SysTime
{
	BYTE h1;	
	BYTE h2;
	UINT16 MsgLen;
	BYTE cmdNo;				//CM_SYSTIME    ϵͳʱ��
	BYTE CustType;			//�ͻ�����	0=վ̨, 1=BUS, 2=����
	UINT16 EquipID;			//�豸���
	UINT32 aux;				//�����ֽڣ����������
	BYTE Year;				//ʱ��
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
	BYTE cmdNo;				//CM_BUSLINEREQ    ���󹫽���·
	BYTE CustType;			//�ͻ�����	0=վ̨, 1=BUS, 2=����
	UINT16 EquipID;			//�豸���
	UINT16 BusLineID;		//�����ߺ�
}BusLineReq;
typedef struct tag_BusLine
{
	BYTE h1;	
	BYTE h2;
	UINT16 MsgLen;
	BYTE cmdNo;				//CM_BUSLINE    ������·��Ϣ
	BYTE CustType;			//�ͻ�����	0=վ̨, 1=BUS, 2=����
	UINT16 EquipID;			//�豸���
	UINT16 BusLineID;		//�����ߺ�
	char BusLineName[20];	//������·��
	UINT16 StationNum;		//վ̨��	
	UINT16 Stations[50];	//������վ̨�� 
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