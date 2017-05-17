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
#define __CONFIG_FILE__					".\\jtcomm.ini"
#define APPLICATION_NAME				"SLJTTCP"
#define APPLICATION_TITLE				"JTControlWindow"

#define SLJK_TCP_PORT					5680
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

/*	Message	*/
//define event type
#define	SYSICONNOTIFY					(WM_USER + 998)
#define SERIALNOTIFY					(WM_USER + 1)
#define INITIALEVENT					(WM_USER + 2)
#define SERRIALERROR					(WM_USER + 3)
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
#define LPT_ZHAO_SUCC_ACK				(WM_USER + 15)
#define LPT_ZHAO_ERR_ACK				(WM_USER + 16)
#define WM_FACEUNLOAD					(WM_USER + 17)
#define MULTICARD_READ					(WM_USER + 18)
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
#define RECEIVE_BUFFER_LEN				2048
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


/*typedef struct
{    
	INT32		pHead;			
	BYTE		ErrCount;
}OprRetainP;
*/
// data structures
typedef struct tagSERIALINFO
{
#ifndef _WIN32
	BYTE		bToken;
#endif
	HANDLE		idComDev ;
	BYTE		bPort;
	BOOL		fConnected, fXonXoff, fUseCNReceive, fDisplayErrors;
	BYTE		bByteSize, bFlowCtrl, bParity, bStopBits ;
	DWORD		dwBaudRate ;
	HANDLE		hPostEvent, hWatchThread, hWatchEvent ;
	HWND		hTermWnd ;
	DWORD		dwThreadID ;
	OVERLAPPED	osWrite, osRead ;

} SERIALINFO, *NPSERIALINFO ;

typedef struct tag_SerialCtr
{
	HANDLE hThread;
	DWORD dwThreadID;
	UINT8 bUse;
}SerialCtr;

typedef struct tagCOMMPARAM
{
	int     Port;
	int     BaudRate,Parity,ByteSize,StopBits;
	BOOL    Hw;		/* RTS/CTS hardware flow control */
	BOOL	Sw;		/* XON/XOFF software flow control */
	BOOL    Dtr,Rts;
}COMMPARAM,*LPCOMMPARAM;

typedef struct tag_SerialBuff_t
{
	BYTE ReceErrorCount;
	BYTE LeftLen;
	BYTE CommBuff[BUFFERSIZE];

	long PostFlag;
	BYTE HighCount;
}SerialBuff_t;

typedef union tag_SerialBits_t
{
	char ByteWise;
	struct
	{
		unsigned StartFlag: 1;
		unsigned CharType:	3;
		unsigned D1:		1;
		unsigned D2:		1;
		unsigned D3:		1;
		unsigned D4:		1;
		unsigned Reserved:	24;
	}BitWise;
}SerialBits;

/*
typedef union tag_HuHead_t
{
	BYTE StartByte;
	BYTE CrossNo;
	BYTE CmdNo;
	BYTE Reserved;
	BYTE Length;
	BYTE Checksum;
}HuHead;
*/
/*	DEFINED DATA TYPE END	*/

// macros ( for easier readability )
#define IS_NT	(IS_WIN32 && (BOOL)(GetVersion() < 0x80000000))	

#define COMDEV( x ) (x -> idComDev)
#define PORT( x )   (x -> bPort)
#define CONNECTED( x ) (x -> fConnected)
#define XONXOFF( x ) (x -> fXonXoff)
#define BYTESIZE( x ) (x -> bByteSize)
#define FLOWCTRL( x ) (x -> bFlowCtrl)
#define PARITY( x ) (x -> bParity)
#define STOPBITS( x ) (x -> bStopBits)
#define BAUDRATE( x ) (x -> dwBaudRate)
#define USECNRECEIVE( x ) (x -> fUseCNReceive)
#define DISPLAYERRORS( x ) (x -> fDisplayErrors)
#define POSTEVENT( x ) (x -> hPostEvent)
#define TERMWND( x ) (x -> hTermWnd)
#define HTHREAD( x ) (x -> hWatchThread)
#define THREADID( x ) (x -> dwThreadID)
#define WRITE_OS( x ) (x -> osWrite)
#define READ_OS( x ) (x -> osRead)
#ifndef _WIN32
#define TOKEN( x ) (x-> bToken)
#endif

#define SET_PROP( x, y, z )  SetProp( x, MAKEINTATOM( y ), z )
#define GET_PROP( x, y )     GetProp( x, MAKEINTATOM( y ) )
#define REMOVE_PROP( x, y )  RemoveProp( x, MAKEINTATOM( y ) )
#define GETBYTEHI(x) ( (((char)(x))>>4) )


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

void	TcpResend(BYTE);
BOOL	InWaitSendQueue(BYTE, WORD, char *);
BOOL	TcpSend(BYTE, WORD, char *);
BOOL	ReceiveSendData(char * pBuff,WORD MsgLen);
void	ReceiveData(BYTE);
void	CloseSocketConnect(BYTE);
void	ClearWaitSendQueue(BYTE);
BYTE	SocketInsertQueue(DWORD);
void	CancelBlocking(void);
void	AcceptConnect(WPARAM, LPARAM);
LRESULT FAR PASCAL 	WndProc(HWND, UINT, WPARAM, LPARAM);
BOOL	InitTCPIP(void);
BOOL	InitUdp(void);
BOOL	InitUdpSvr(void);
BOOL	InitUdpClient(void);
BOOL	InitUdpData(void);
BOOL	GetLocalIp(void);
BOOL ConnectSvrProc(void);
void	InitSocketStatus(BYTE);//???????????
void InitSocketData(BYTE i);
BOOL ReConnectSvrProc(DWORD SvrIpAddr);
BYTE FindCommIndex(DWORD TargetIP,BYTE TargetTaskId);
WINAPI	WinMain(HINSTANCE, HINSTANCE, char *, int);
void	InitApp(void);
//BOOL GetSvrIp(char *InIpAddr);
BOOL InitSerialComm(void);
#ifdef SERVICE
HWND MakeWorkerWindow(void);

#endif

void DispatchClientReqProc(BYTE Index,void *pMsgIn,WORD MsgLen);
void ConnectPeer(WPARAM wParam, LPARAM lParam);
int UdpRecieve(char *buf);

BOOL InitIrq(int port);
void ClearIrq(int port);
VOID CALLBACK CntIrq(int port);
BOOL ClosePort(int port);
BOOL OpenPort(int port);
BOOL OpenPortEx(int port);
BOOL PortSet(int port,COMMPARAM *pCommParam);
int ReadMulSerialData(int wParam);
BOOL MultiSerialInit(void);
DWORD WINAPI SerialProc(LPSTR lpData);
BOOL Notify(int TaskNo,UINT16 Event, UINT8 *In, UINT16 InLen);
BOOL SerialPost(int Port,char *MsgPtr,UINT16 MsgLen);

// function prototypes (public)
BOOL SendData(char * pBuff,WORD MsgLen,SOCKET s);//socket
BOOL SendDataEx(char * pBuff,WORD MsgLen,unsigned char SockIndex);
BOOL WriteCommBlock(LPSTR, DWORD);   //serial port
void DebugWindow(char *);
void OsClearWindow(void);

int UdpSend(char *buf,int len);
void CheckSvrLink(void);

#ifdef cplusplus
}
#endif

#pragma pack(pop)

#endif