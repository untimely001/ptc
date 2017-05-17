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

#pragma option -b

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

/*	Message	*/
//define event type
#define SYSICONNOTIFY					(WM_USER + 998)
#define SERIALNOTIFY					(WM_USER + 1)
#define INITIALEVENT					(WM_USER + 2)
#define SERRIALERROR					(WM_USER + 3)
#define WSA_ACCEPT						(WM_USER + 4)
#define WSA_READ						(WM_USER + 5)
#define WSA_CONNECT						(WM_USER + 6)
#define REPEAT_SEND_WAIT_PACKET			(WM_USER + 7)
#define TIMEOUT_CHECK					(WM_USER + 8)
#define IS_SENT_CHECK					(WM_USER + 9)
#define SLDB_EVENT						(WM_USER + 10)
#define LPT_READ						(WM_USER + 11)
#define UDP_READ						(WM_USER + 12)
#define SLDB_INIT_EVENT					(WM_USER + 13)
#define DB_INITOK_EVENT					(WM_USER + 14)
#define LPT_ZHAO_SUCC_ACK				(WM_USER + 15)
#define LPT_ZHAO_ERR_ACK				(WM_USER + 16)
#define WM_FACEUNLOAD					(WM_USER + 17)

/*	RANGE	*/
#define MAX_MSG_LEN						(WORD)1024
#define MAX_SOCKET_NUM					(BYTE)100
#define MAX_RESEND_NUM					(BYTE)100
#define MAX_POST_NUM					(BYTE)5
#define MAX_PENDING_CONNECTS			(BYTE)8
#define MAX_RECEIVE_ERROR_NUM			(BYTE)20
#define RECEIVE_BUFFER_LEN				1024
#define NAME_LEN						20
#define MAX_APPLICATION_WAIT_QUEUE_LEN	100

//define access token for nonWin32
#ifndef _WIN32
#define SM_IDLESTATUS	0x00
#define SM_BUSYSTATUS   0xFF
#endif

#define BUFFERSIZE		250
#define MAX_DISPLAY_LEN 400
#define MAX_REC_DATA	1024
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
	SOCKET		Socket;
	DWORD		PeerIPAddr;	//network order
   
	BYTE		WaitSendCount;			
	BYTE		WaitHead;
	BYTE		WaitEnd;
	BYTE		ReceErrorCount;
} SocketStatusNode;

typedef struct
{    
	INT32		pHead;			
	BYTE		ErrCount;
}OprRetainP;

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
void	InitSocketStatus(BYTE);
WINAPI	WinMain(HINSTANCE, HINSTANCE, char *, int);
void	InitApp(void);
BOOL InitSerialComm(void);
#ifdef SERVICE
HWND MakeWorkerWindow(void);
#endif


int UdpRecieve(char *buf);

// function prototypes (public)
BOOL SendData(char * pBuff,WORD MsgLen,SOCKET s);//socket
BOOL SendDataEx(char * pBuff,WORD MsgLen,unsigned char SockIndex);
BOOL WriteCommBlock(LPSTR, DWORD);   //serial port
void DebugWindow(char *);
void OsClearWindow(void);

int UdpSend(char *buf,int len);

#ifdef cplusplus
}
#endif

#pragma pack(pop)

#endif