#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <winsock.h>
#include <process.h>
#include <memory.h>
#include <sql.h>				
#include <sqlext.h>
#include "pbctrans.h"
#include "commonstruct.h"

#pragma comment(lib,"ODBC32.lib")
#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"ws2_32.lib")

#define TEST

static HDC gHDC;
static int gY;
DWORD WinOsVerId = 4;
HWND hMainWnd;
FILE *fp = NULL;
char HostIp[16];
DWORD LocalIPAddr = 0;
char SvrIp[16];
DWORD SvrIPAddr = 0;
int TcpPort = 0;
HENV henv = NULL;
HDBC hdbc = NULL;

char			szAppName[NAME_LEN] = APPLICATION_NAME;
char			szTitle[NAME_LEN] = APPLICATION_TITLE;
HANDLE			MyProcessHeapHandle = NULL;
SocketDataNode	SocketData[MAX_SOCKET_NUM];
MsgWaitNode		TcpWaitSendQueue[MAX_SOCKET_NUM][MAX_RESEND_NUM];
BYTE RLOGON[]={0x68,0x32,0x00,0x32,0x00,0x68,0xC9,0x06,0x32,0x01,0x00,0x01,0x02,0x70,0x00,0x00,0x01,0x00,0x76,0x16};
BYTE SLOGON[]={0x68,0x32,0x00,0x32,0x00,0x68,0x09,0x06,0x32,0x01,0x00,0x01,0x00,0x60,0x00,0x00,0x01,0x00,0xA4,0x16};

SOCKET ClientSock,SvrSock;
struct sockaddr_in	remote,
					from;   
BYTE clientOk=0;
unsigned int Auxiliary[] = { 0,				//站台
							0xf0000000,		//车辆
							0x80000000,		//行人
							0x40000000 };	//操作员
							
#define __DEBUG_FILE__     ".\\gdebug.inf"

WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, char * lpCmdLine, int nCmdShow)
{ 
	MSG msg;

	if (!InitApplication(hInstance))		
	{
		MessageBox(hMainWnd, "Init App Failed!", "Warning", MB_ICONSTOP | MB_OK);
		return (FALSE);

	}

	if (!InitInstance( hInstance, nCmdShow ))
	{
		MessageBox(hMainWnd, "Init Instance Failed!", "Warning", MB_ICONSTOP | MB_OK);
		return (FALSE);
	}

	GetAppConfig();

/*	if (!Initdb())
	{
		MessageBox(hMainWnd, "Connect to DB Failed!", "Warning", MB_ICONSTOP | MB_OK);
		return (FALSE);
	}	
	else
		PostMessage(hMainWnd,INITIALEVENT,0x10,0x20);
*/
	if (!InitTCPIP())	
	{
		MessageBox(hMainWnd, "Init Tcp/Ip Failed!", "Warning", MB_ICONSTOP | MB_OK);
		return (FALSE);
	}

	//initilize app uten   
	InitApp();

    {
    NOTIFYICONDATA NotifyData;

    NotifyData.cbSize = sizeof(NOTIFYICONDATA);
	NotifyData.hWnd = hMainWnd;
	NotifyData.uID = (UINT)100;
	NotifyData.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
    NotifyData.uCallbackMessage = SYSICONNOTIFY;
	NotifyData.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	strcpy(NotifyData.szTip, "PUBLICTRANS");

	Shell_NotifyIcon(NIM_ADD, &NotifyData);
	#ifndef FORDEBUG
//	ShowWindow(hMainWnd, SW_HIDE);
	#endif
    }

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

    {
    NOTIFYICONDATA NotifyData;

    NotifyData.cbSize = sizeof(NOTIFYICONDATA);
    NotifyData.hWnd = hMainWnd;
    NotifyData.uID = (UINT)100;
    NotifyData.uFlags = NIF_ICON | NIF_TIP;
    NotifyData.hIcon = LoadIcon(NULL, IDI_WINLOGO);
    strcpy(NotifyData.szTip, "PUBLICTRANS");

    Shell_NotifyIcon(NIM_DELETE, &NotifyData);
    }

	return msg.wParam;
}

LRESULT FAR PASCAL WndProc(HWND hMainWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
//	int len;
	switch (message)
	{
		case WM_CREATE:

			RegisterHotKey(hMainWnd, 0x01, MOD_CONTROL | MOD_ALT, VK_F12);
			PostMessage(hMainWnd,INITIALEVENT,0,0);
			break;

		case WM_HOTKEY: //reInitialize	
			{
				BYTE i;
				
				DebugWindow("System ReInitialize Now.....");
				CancelBlocking();

				for (i = 0; i < MAX_SOCKET_NUM; i++)	
					CloseSocketConnect(i);
					
				CancelBlocking();
				WSACleanup();

				if (!InitTCPIP())	
					return (FALSE);

				DebugWindow("System ReInitialize Finished!");
			}
			break;

		case EXTENDMSGSEND:
			
//			if(wParam > 0  && wParam <= MAX_CROSS_NUM)
//				SendMsgInQue((int)wParam,(int)lParam);

			break;

		case WSA_ACCEPT:		

			DebugWindow("To Accept Client!"); 
			AcceptConnect(wParam, lParam);
			clientOk = 1;
			break;

		case SVR_RECONNECT:

//			ReConnectSvrProc((DWORD)wParam);
			break;

        case WSA_CONNECT:

            //DebugWindow("Have Connected to Server!"); 
            ConnectPeer(wParam, lParam);
            break;
            
		case WSA_READ:
			{
				BYTE i;
				int Status;

				for (i = 1; i < MAX_SOCKET_NUM; i++)
				{	
					if (SocketData[i].Socket == wParam)
						break;
				} 	 
				if (i >= MAX_SOCKET_NUM)
					return -1;
				
				Status = WSAGETSELECTERROR(lParam);
				if (Status != 0)
				{
					CloseSocketConnect(i);

					return -1;
				}

				switch (WSAGETSELECTEVENT(lParam))
				{
					case FD_READ:
						DebugWindow("Read Data From Socket!");
						ReceiveData(i);
						break;

					case FD_WRITE:
						DebugWindow("FD_WRITE,Can Send Data !");
						TcpResend(i);
						break;

					case FD_CLOSE:
						DebugWindow("A Socket Will Be Closed!");
						CloseSocketConnect(i);
						break;
				}
			}
			break;

		case INITIALEVENT:
			break;

		case WM_TIMER:

			switch (wParam)
			{
				BYTE i;
				case REPEAT_SEND_WAIT_PACKET://1000

/*					memset(disp,0,300);
					for(i=0;i<14;i++)
						sprintf(disp+i*5,"0x%02x ",pstr[i]);
					DebugWindow(disp);

					memset(&pQrFunc,0,sizeof(QRFunc));
					pQrFunc.gateNo=12;
					Utf2Gb2312(pstr, 20,pQrFunc.QRString,MAX_QRSTR_LEN);
					pQrFunc.Len=strlen(pQrFunc.QRString);
					QRFuncProc(&pQrFunc);
	
					memset(putf,0,300);
					Gb2312Utf(pgb2312, 16,putf,300);
					len=strlen(putf);

					InterpretClient(0,0,0);//
*/					for (i = 1; i < MAX_SOCKET_NUM; i++)	
					{
						if ((SocketData[i].Socket == INVALID_SOCKET) \
							|| (SocketData[i].WaitSendCount <= 0))
						continue;

						TcpResend(i);
					}

					if(clientOk == 1)
						GetNotify();

					break;

				case IS_SENT_CHECK://10000
					break;

				default:
					break;
			}

			break;

		case WM_DESTROY:
			{
				BYTE i;

				UnregisterHotKey(NULL, 0x01);

				//close socket
				for (i = 1/*0*/; i < MAX_SOCKET_NUM; i++)
					CloseSocketConnect(i);

				CancelBlocking();
				WSACleanup();

				{
					NOTIFYICONDATA NotifyData;

					NotifyData.cbSize = sizeof(NOTIFYICONDATA);
					NotifyData.hWnd = hMainWnd;
					NotifyData.uID = (UINT)100;
					NotifyData.uFlags = NIF_ICON | NIF_TIP;
					NotifyData.hIcon = LoadIcon(NULL, IDI_WINLOGO);
					strcpy(NotifyData.szTip, "SLJTControl");

					Shell_NotifyIcon(NIM_DELETE, &NotifyData);
				}
				
				PostQuitMessage(0);
			}
			break;

		 case WM_PAINT:

			OsClearWindow();
			break;

		case WM_SYSCOMMAND:
 
			if(wParam == SC_SCREENSAVE)
				return 0;

			return (DefWindowProc(hMainWnd, message, wParam, lParam));

			break;

 		default:

			return (DefWindowProc(hMainWnd, message, wParam, lParam));
	}

	return 0;
}

void Utf2Gb2312(BYTE *putf, int Len,char *pOut,int OutLen)
{
	unsigned short wStr2[300];

	//utf8 to unicode
	memset(wStr2,0,300*sizeof(short));
	MultiByteToWideChar(CP_UTF8, 0, putf, -1, wStr2, 300);

	//unicode to gb2312
	WideCharToMultiByte(CP_ACP, 0, wStr2, -1, pOut, OutLen, NULL, NULL);

	return;
}

void Gb2312Utf(BYTE *p2312, int Len,char *pOut,int OutLen)
{
	unsigned short wStr[300];

	//gb2312 to unicode
	memset(wStr,0,300*sizeof(short));
	MultiByteToWideChar(CP_ACP, 0, p2312, -1, wStr, 300);

	//unicode to utf8
	WideCharToMultiByte(CP_UTF8, 0, wStr, -1, pOut, OutLen, NULL, NULL);

	return;
}

BOOL InitApplication(HINSTANCE hInstance)
{
	WNDCLASS wc;
	OSVERSIONINFO osVer;

	memset(&osVer,0,sizeof(OSVERSIONINFO));
	osVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osVer);

	WinOsVerId = osVer.dwMajorVersion;

	if (FindWindow(szAppName, szTitle))
	{
       MessageBox(NULL,"Already Running,Quit and Retry.", "Warning!!", MB_ICONSTOP | MB_OK);
       return FALSE;
	}


	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = (WNDPROC)WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIcon         = LoadIcon(NULL,IDI_APPLICATION);
	wc.hCursor       = LoadCursor(NULL,IDC_ARROW);
	wc.hbrBackground = (struct HBRUSH__ *)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = szAppName;

	return RegisterClass(&wc);
}

int UnhandledExceptionHappen = 0;
LONG MyUnhandledExceptionFilter(struct _EXCEPTION_POINTERS *ExceptionInfo)
{
	UnhandledExceptionHappen = 1;
	SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX);

	return EXCEPTION_EXECUTE_HANDLER;
	ExceptionInfo;
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd = NULL;			//窗口句柄
	hWnd = CreateWindow(szAppName,
						szTitle, 
						WS_MINIMIZE | WS_OVERLAPPEDWINDOW,
						CW_USEDEFAULT,
						CW_USEDEFAULT, 
						CW_USEDEFAULT, 
						CW_USEDEFAULT,
						NULL, 
						NULL,
						hInstance, 
						NULL);

	if (!hWnd)	
		return (FALSE);

	hMainWnd = hWnd;

	ShowWindow(hMainWnd, SW_SHOW);
	UpdateWindow(hMainWnd);
	
	//设置当前进程的高优先权

	SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);

	//设置异常过滤处理函数
	SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)MyUnhandledExceptionFilter);

	MyProcessHeapHandle = GetProcessHeap();

	return (TRUE);
	nCmdShow;
}

BOOL InitTCPIP(void)
{
	WSADATA WSAData;
	SOCKET TempSocket;
	SOCKADDR_IN local_sin;
    BOOL TempBool = TRUE;
#ifdef _DEBUG
	int errNo;
#endif

	if (WSAStartup(MAKEWORD(1,1), &WSAData) != 0) 
	{
#ifdef _DEBUG
		errNo = WSAGetLastError();
#endif
		MessageBeep(MB_OK);
		MessageBox(hMainWnd, "TCP/IP 协议初始化出错!", "警告", MB_ICONSTOP | MB_OK);
		return FALSE;
	}

	if ((LOBYTE( WSAData.wVersion ) != 1) || (HIBYTE( WSAData.wVersion ) != 1))
	{
		MessageBeep(MB_OK);
		MessageBox(hMainWnd, "套接字版本不匹配!", "警告", MB_ICONSTOP | MB_OK);
 		WSACleanup();
		return FALSE; 
	}

	if (!GetLocalIp()) //may not use
	{
		MessageBeep(MB_OK);
		MessageBox(hMainWnd, "取得本地IP出错!", "警告", MB_ICONSTOP | MB_OK);
		return FALSE;
	}

	CancelBlocking();

	TempSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (TempSocket == INVALID_SOCKET)
	{
#ifdef _DEBUG
		errNo = WSAGetLastError();
#endif
		MessageBeep(MB_OK);
		MessageBox(hMainWnd, "建立数据流套接字失败!", "警告", MB_ICONSTOP | MB_OK);
		WSACleanup();
		return FALSE;
	}

	local_sin.sin_family = AF_INET;

	TcpPort = GetPrivateProfileInt("GENERAL","SockPort",PBCTRANS_TCP_PORT, __CONFIG_FILE__);
	local_sin.sin_port = htons((u_short)TcpPort);

	local_sin.sin_addr.s_addr = INADDR_ANY;

	if (bind(TempSocket, (PSOCKADDR)&local_sin, sizeof(local_sin)) == SOCKET_ERROR)
	{
#ifdef _DEBUG
		errNo = WSAGetLastError();
#endif
		MessageBeep(MB_OK);
		MessageBox(hMainWnd, "捆扎数据流套接字失败!", "警告", MB_ICONSTOP | MB_OK);
		WSACleanup();
		return FALSE;
	}

	if(listen(TempSocket, MAX_PENDING_CONNECTS) == SOCKET_ERROR) 
	{
#ifdef _DEBUG
		errNo = WSAGetLastError();
#endif
		MessageBeep(MB_OK);
		MessageBox(hMainWnd, "监听数据流套接字失败!", "警告", MB_ICONSTOP | MB_OK);
		WSACleanup();
		return FALSE;
	}
    
	if (WSAAsyncSelect(TempSocket, hMainWnd, WSA_ACCEPT, FD_ACCEPT) == SOCKET_ERROR) 
	{
#ifdef _DEBUG
		errNo = WSAGetLastError();
#endif
		MessageBeep(MB_OK);
		MessageBox(hMainWnd, "设置数据流套接字模式失败!", "警告", MB_ICONSTOP | MB_OK);
		WSACleanup();
		return FALSE;
	}
	SocketData[0].Socket = TempSocket;

	//as a client to connect server
/*	if(!ConnectSvrProc())
	{
		MessageBox(hMainWnd, "连接服务器失败!", "警告", MB_ICONSTOP | MB_OK);
		return FALSE;
	}
*/
	SetTimer(hMainWnd, REPEAT_SEND_WAIT_PACKET, REPEAT_SEND_PACKET_TIME, NULL);

	return TRUE;
}

BOOL ConnectSvrProc(void)
{
	BOOL Ret;
	char lSvrIp[20];
    int status;
    BOOL TempBool = TRUE;
	SOCKET TempSocket;
	SOCKADDR_IN remote_sin, local_sin;
	BYTE i;

	memset(lSvrIp,0,20);
	Ret = GetSvrIp(lSvrIp);
	if(!Ret)
	{
		MessageBox(hMainWnd, "获得服务器地址失败!", "警告", MB_ICONSTOP | MB_OK);
		return FALSE;
	}

	local_sin.sin_family = AF_INET;
	local_sin.sin_port = 0;
	local_sin.sin_addr.s_addr = LocalIPAddr;

	remote_sin.sin_family = AF_INET;
	remote_sin.sin_port = htons((u_short)TcpPort);
/*
	SocketData[1].PeerIPAddr = inet_addr(lSvrIp);//??????????
	remote_sin.sin_addr.s_addr = SocketData[1].PeerIPAddr;
*/
	remote_sin.sin_addr.s_addr = inet_addr(lSvrIp);

    TempSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (TempSocket == INVALID_SOCKET)
    {
        MessageBox(hMainWnd, "建立数据流套接字失败!", "警告", MB_ICONSTOP | MB_OK);
        WSACleanup();
        return FALSE;
    }
    
    if (setsockopt(TempSocket, SOL_SOCKET, SO_REUSEADDR, 
					(char *)&TempBool, sizeof(BOOL)) == SOCKET_ERROR)
    {
        MessageBox(hMainWnd, "设置数据流套接字失败!", "警告", MB_ICONSTOP | MB_OK);
        WSACleanup();
        return FALSE;
    }
    
    status = bind(TempSocket, (PSOCKADDR)&local_sin, sizeof(local_sin));
    if (status == SOCKET_ERROR)
    {
        MessageBox(hMainWnd, "捆扎数据流套接字失败!", "警告", MB_ICONSTOP | MB_OK);
        WSACleanup();
        return FALSE;
    }
    
    if (WSAAsyncSelect(TempSocket, hMainWnd, WSA_CONNECT, FD_CONNECT))
    {
        MessageBox(hMainWnd, "设置数据流套接字模式失败!", "警告", MB_ICONSTOP | MB_OK);
        WSACleanup();
        return FALSE;
    }
    
	i = SocketInsertQueue(inet_addr(lSvrIp));
	if (i <= 0 || i > MAX_SOCKET_NUM)
	{
		DebugWindow("ConnectSvrProc(),Can Not Insert Socket Data!");
		closesocket(TempSocket);
		return FALSE;
	}

	if(SocketData[i].Socket != INVALID_SOCKET)
	{
		closesocket(TempSocket);
	}

	InitSocketData(i);
	SocketData[i].Socket = TempSocket;
	SocketData[i].PeerIPAddr = inet_addr(lSvrIp);

    status = connect(TempSocket, (PSOCKADDR)&remote_sin, sizeof(remote_sin));
    if (status == SOCKET_ERROR)    
    {
        status = WSAGetLastError();
        if (status != WSAEWOULDBLOCK) 
        {
            MessageBox(hMainWnd, "数据流套接字连接失败!", "警告", MB_ICONSTOP | MB_OK);
            WSACleanup();
            return FALSE;
        }
    }

	return TRUE;
}

BOOL ReConnectSvrProc(DWORD SvrIpAddr)
{
    int status;
    BOOL TempBool = TRUE;
	SOCKET TempSocket;
	SOCKADDR_IN remote_sin, local_sin;

	DebugWindow("Invoking ReConnectSvrProc()!");
	if(SvrIpAddr == INADDR_NONE)
	{
		MessageBox(hMainWnd,"服务器地址为空!", "警告", MB_ICONSTOP | MB_OK);
		return FALSE;
	}

	if(SocketData[1].Socket != INVALID_SOCKET)
		CloseSocketConnect(1);

	local_sin.sin_family = AF_INET;
	local_sin.sin_port = 0;
	local_sin.sin_addr.s_addr = LocalIPAddr;

	remote_sin.sin_family = AF_INET;
	remote_sin.sin_port = htons((u_short)TcpPort);
	remote_sin.sin_addr.s_addr = SvrIpAddr;//SocketData[1].PeerIPAddr;

	SocketData[1].PeerIPAddr = SvrIpAddr;

    TempSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (TempSocket == INVALID_SOCKET)
    {
        MessageBox(hMainWnd, "建立数据流套接字失败!", "警告", MB_ICONSTOP | MB_OK);
        WSACleanup();
        return FALSE;
    }
    
    if (setsockopt(TempSocket, SOL_SOCKET, SO_REUSEADDR, 
					(char *)&TempBool, sizeof(BOOL)) == SOCKET_ERROR)
    {
        MessageBox(hMainWnd, "设置数据流套接字失败!", "警告", MB_ICONSTOP | MB_OK);
        WSACleanup();
        return FALSE;
    }
    
    status = bind(TempSocket, (PSOCKADDR)&local_sin, sizeof(local_sin));
    if (status == SOCKET_ERROR)
    {
        MessageBox(hMainWnd, "捆扎数据流套接字失败!", "警告", MB_ICONSTOP | MB_OK);
        WSACleanup();
        return FALSE;
    }
    
    if (WSAAsyncSelect(TempSocket, hMainWnd, WSA_CONNECT, FD_CONNECT))
    {
        MessageBox(hMainWnd, "设置数据流套接字模式失败!", "警告", MB_ICONSTOP | MB_OK);
        WSACleanup();
        return FALSE;
    }
    
    SocketData[1].Socket = TempSocket;
    
    status = connect(TempSocket, (PSOCKADDR)&remote_sin, sizeof(remote_sin));
    if (status == SOCKET_ERROR)    
    {
        status = WSAGetLastError();
        if (status != WSAEWOULDBLOCK) 
        {
            MessageBox(hMainWnd, "数据流套接字连接失败!", "警告", MB_ICONSTOP | MB_OK);
            WSACleanup();
            return FALSE;
        }
    }

	return TRUE;
}

BOOL GetLocalIp(void)
{
	BYTE i;
	int status;

	char HostName[255]={0};
	struct hostent * pHostent;
	struct	sockaddr_in dest;
//#ifdef TEST
	char * hIP;
//	char HostIp[16];
//#endif

	for (i = 0; i < MAX_SOCKET_NUM; i++)	
		InitSocketData(i);

	if(gethostname(HostName,sizeof(HostName)) == SOCKET_ERROR)
	{
		status = WSAGetLastError();
		return FALSE;
	}

	pHostent = gethostbyname(HostName);
	if(!pHostent)
		return FALSE;

	memset(&dest,0,sizeof(struct sockaddr_in));
	memcpy(&dest.sin_addr,pHostent->h_addr,pHostent->h_length); 

//#ifdef TEST
	hIP = inet_ntoa(dest.sin_addr);
	memset(HostIp,0,16);
	strcpy(HostIp,hIP);
//#endif
	LocalIPAddr = dest.sin_addr.s_addr;

	return TRUE;
}


void InitSocketData(BYTE i)
{
	if ((i < 0) || (i >= MAX_SOCKET_NUM))	
#ifdef TEST
	{
		DebugWindow("InitSocketData(),Socket Index Error!");
		return;
	}
#else
		return;
#endif
		
	SocketData[i].Socket = INVALID_SOCKET;
	SocketData[i].PeerIPAddr = INADDR_NONE;
	SocketData[i].PeerTaskId = 0;

	SocketData[i].WaitSendCount = 0;			
	SocketData[i].WaitHead = 0;
	SocketData[i].WaitEnd = 0;
	SocketData[i].ReceErrorCount = 0;

	SocketData[i].LeftLen = 0;
	memset(SocketData[i].ReceBuff,0,RECEIVE_BUFFER_LEN);

	SocketData[i].LinkStatus = FALSE;
    SocketData[i].CheckLinkCount = 0;

	return;
}


void AcceptConnect(WPARAM wParam, LPARAM lParam)
{
	int status, acc_sin_len;
	BYTE i;
	SOCKADDR_IN acc_sin;
	SOCKET TempSocket;

	if (WSAGETSELECTERROR(lParam) != 0) 
	{
		DebugWindow("AcceptConnect(),Accept Error!");
		return;
	}

	if (wParam != SocketData[0].Socket)
	{
		DebugWindow("AcceptConnect(),Listen Socket Mismatch!");
		closesocket(wParam);
		return;
	}

	acc_sin_len = sizeof(acc_sin);
	TempSocket = accept(SocketData[0].Socket, (PSOCKADDR)&acc_sin, (int *)&acc_sin_len );
	
	if (TempSocket == INVALID_SOCKET)	
	{
		DebugWindow("AcceptConnect(),Accept Failed!");
		return; 
	}

	status = WSAAsyncSelect(TempSocket, hMainWnd, WSA_READ, FD_READ | FD_CLOSE | FD_WRITE);
	if (status == SOCKET_ERROR)
	{
		DebugWindow("AcceptConnect(),Select Failed!");
		closesocket(TempSocket);
		return;
	}

	{
		struct linger
		{ 
			int    l_onoff; 
			int    l_linger; 
		} TempLinger;
		
		TempLinger.l_onoff = 1;
		TempLinger.l_linger = 0;

		status = setsockopt(TempSocket, SOL_SOCKET, SO_LINGER, 
			(char *)&TempLinger, sizeof(TempLinger));
		if (status == SOCKET_ERROR)
		{
			DebugWindow("Set Option Failed!");
			closesocket(TempSocket);
			return;
		}
	}
	
	i = SocketInsertQueue(acc_sin.sin_addr.s_addr);//further consideration
	if (i <= 0 || i > MAX_SOCKET_NUM)
	{
		DebugWindow("AcceptConnect(),Can Not Insert Socket Data!");
		closesocket(TempSocket);
		return;
	}

	InitSocketData(i);

	SocketData[i].Socket = TempSocket;
	SocketData[i].PeerIPAddr = acc_sin.sin_addr.s_addr;
	SocketData[i].LinkStatus = TRUE;

	return;
}

void ConnectPeer(WPARAM wParam, LPARAM lParam)
{
	int status;
	BYTE i;
	
	status = WSAGETSELECTERROR(lParam) ;
	if (status != 0) 
	{
		DebugWindow("ConnectPeer(),WSAGETSELECTERROR!");
		return;
	}

	for (i = 1; i < MAX_SOCKET_NUM; i++)
	{	
		if (SocketData[i].Socket == wParam)
			break;
	} 	 
					
	if (i >= MAX_SOCKET_NUM)
	{
		DebugWindow("ConnectPeer(),Index Out of Range!");
		return;
	}
	
	status = WSAAsyncSelect(SocketData[i].Socket, hMainWnd, WSA_READ, FD_READ | FD_CLOSE | FD_WRITE);
	if (status == SOCKET_ERROR)
	{	
#ifdef _DEBUG
		int errNo;
		errNo = WSAGetLastError();
#endif
		DebugWindow("ConnectPeer(),Select Error!");
		CloseSocketConnect(i);
		return;
	}
	
	{
		struct linger
		{ 
			int    l_onoff; 
			int    l_linger; 
		} TempLinger;
		
		TempLinger.l_onoff = 1;
		TempLinger.l_linger = 0;

		status = setsockopt(SocketData[i].Socket, SOL_SOCKET, SO_LINGER, 
			(char *)&TempLinger, sizeof(TempLinger));
		if (status == SOCKET_ERROR)
		{
#ifdef _DEBUG
		int errNo;
		errNo = WSAGetLastError();
#endif
			DebugWindow("ConnectPeer(),Set Option Error!");
			closesocket(SocketData[i].Socket);
			return;
		}
	}

	//set connect status
	SocketData[i].LinkStatus = TRUE;
	SocketData[i].CheckLinkCount = 0;

	//send my id to dispatch
//	SendCheckin();

	return;
}

void CancelBlocking(void)
{
	if (WSAIsBlocking() == TRUE)
		WSACancelBlockingCall();
}

BYTE SocketInsertQueue(DWORD nIPAddr)
{
	BYTE i, nFree = 0;    //total items and index

	if(nIPAddr == INADDR_NONE)
	{
		DebugWindow("SocketInsertQueue(),Ip Address Error!");
		return 0;
	}

//to find empty index
	for (i = 1; i < MAX_SOCKET_NUM; i++)
	{
		if (SocketData[i].Socket == INVALID_SOCKET && SocketData[i].PeerIPAddr == INADDR_NONE)//????
		{	
			nFree = i;
			break;
		}
	}

	if(nFree > 0 && nFree < MAX_SOCKET_NUM)
		return nFree;
	else
	{
		DebugWindow("SocketInsertQueue(),Out of Buffer!");
		return 0;
	}

	return 0; //to silent complier
}

void ClearWaitSendQueue(BYTE Index)
{	
	BYTE i = Index, j;

	if (SocketData[i].WaitSendCount <= 0)
	{
		SocketData[i].WaitSendCount = 0;
		SocketData[i].WaitHead = 0;
		SocketData[i].WaitEnd = 0;
		return;
	}

	for (j = SocketData[i].WaitSendCount; j > 0; j--)
	{
		HeapFree(MyProcessHeapHandle, 0, TcpWaitSendQueue[i][SocketData[i].WaitHead].hMsgBuf);
		TcpWaitSendQueue[i][SocketData[i].WaitHead].hMsgBuf = NULL;
		TcpWaitSendQueue[i][SocketData[i].WaitHead].MsgLen = 0;

		SocketData[i].WaitHead = (++SocketData[i].WaitHead) % MAX_RESEND_NUM;
	}

	SocketData[i].WaitSendCount = 0;
	SocketData[i].WaitHead = 0;
	SocketData[i].WaitEnd = 0;

}

void CloseSocketConnect(BYTE Index)
{
	BYTE i = Index;
	
	if(i <= 0 || i >MAX_SOCKET_NUM)
	{
		DebugWindow("CloseSocketConnect(),Socket Index Error aaa!");
		return;
	}

	closesocket(SocketData[i].Socket);
	ClearWaitSendQueue(i);

	InitSocketData(i);

/*	OprIndex = Comm2Opr(Index);	
	if(OprIndex == 0 || OprIndex >=MAX_OPR_NUM)
	{
//		DebugWindow("Process Function Global Data Exception!");//delete 11/09/2000
		return;
	}
*/
//	LogoutProc(OprIndex);
	
	clientOk = 0;

	return;
}

void ReceiveData(BYTE Index)
{
	BYTE i = Index;
	int status, ReceLen;
    char MsgBuff[RECEIVE_BUFFER_LEN];
	BYTE MsgLen,CommNo,Msg;
	char *pMsgBuff;
	int pos = 0;
/*	
#ifdef TEST
	char disp[200];
#endif
*/
    SocketData[i].CheckLinkCount = 0;

    ReceLen = recv(SocketData[i].Socket, MsgBuff, RECEIVE_BUFFER_LEN, NO_FLAG_SET);
	if(ReceLen == SOCKET_ERROR)	
	{
		status = WSAGetLastError();
		if ((status != WSAEWOULDBLOCK) && (status != WSAEINPROGRESS))
		{
			DebugWindow("ReceiveData(),Receive Data Error!");

			CloseSocketConnect(i);
			return;
		}
		else
		{
			DebugWindow("ReceiveData(),Receive Data Blocking!");
			memcpy(SocketData[i].ReceBuff, MsgBuff,SocketData[i].LeftLen);
			return;
		}
	}
	
	SendDataEx((char*)MsgBuff,(WORD)ReceLen,Index);
/*
#ifdef TEST
	MsgBuff[ReceLen] = 0;
	DebugWindow(MsgBuff);
#endif
*/
	pos = 0;
//	while(*(MsgBuff+pos) != 0x68 && (BYTE)(*(MsgBuff+pos)) != (BYTE)(0xaa))
//		pos++;

	pMsgBuff = MsgBuff+pos;
	if((*pMsgBuff == 0xEB) && (*(pMsgBuff+1) == 0x32))
	{
		clientOk = 1;
		CommNo = *(pMsgBuff + 16) + 5;

		SocketData[i].ReceErrorCount = 0;
		ConnectProc(CommNo,Index);

        return;
	}
	else if(((BYTE)(*pMsgBuff) == (BYTE)(0xEB)) && (*(pMsgBuff + 1) == 0x55))
	{
		Msg = (unsigned char)(*pMsgBuff);
		MsgLen = (unsigned char)GetMsgLen(Msg);

/*		if(!IsCheckSumOk((char *)Msg,MsgLen+2))
		{
			DebugWindow("ReceiveData(),Checksum Error!");
			SocketData[i].LeftLen = 0;
			memset(SocketData[i].ReceBuff,0, RECEIVE_BUFFER_LEN);
			SocketData[i].ReceErrorCount++;
			return;
		}
*/
		SocketData[i].ReceErrorCount = 0;
		InterpretClient((void *)pMsgBuff,MsgLen,Index);//传入长度只包括头和消息体，不包括校验和

        return;
	}
	else//出错
	{
		DebugWindow("ReceiveData(), Data Format Error!");
        SocketData[i].LeftLen = 0;
        memset(SocketData[i].ReceBuff,0, RECEIVE_BUFFER_LEN);
        SocketData[i].ReceErrorCount++;
        return;
	}


	return;
}
int GetMsgLen(BYTE cmd)
{
	int len = 0;

	switch(cmd)
	{
	case CM_TEST:
		len = sizeof(TestMsg);
		break;
	case CM_STATIONSTATUSREQ:
		len = sizeof(StationStatusReq);
		break;
	case CM_STATIONSTATUS:
		len = sizeof(StationStatus);
		break;
	case CM_BUSSTATUSREQ:
		len = sizeof(BusStatusReq);
		break;
	case CM_BUSSTATUS:
		len = sizeof(BusStatus);
		break;
	case CM_STATIONINITREQ:
		len = sizeof(StationInitReq);
		break;
	case CM_STATIONINIT:
		len = sizeof(StationInit);
		break;
	case CM_BUSINITREQ:
		len = sizeof(BusInitReq);
		break;
	case CM_BUSINIT:
		len = sizeof(BusInit);
		break;
	case CM_BUSCOMING:
		len = sizeof(BusComing);
		break;
	case CM_CM_BUSSTOPPING:
		len = sizeof(BusStopping);
		break;
	case CM_BUSLEAVING:
		len = sizeof(BusLeaving);
		break;
	case CM_BUSRUNNING:
		len = sizeof(BusRunning);
		break;
	case CM_LOGON:
		len = sizeof(Login);
		break;
	case CM_LOGOUT:
		len = sizeof(Logout);
		break;
	case CM_CHANGEPASSWORD:
		len = sizeof(ChangePassword);
		break;
	case CM_ALARMMSG:
		len = sizeof(AlarmMsg);
		break;
	case CM_SYSTIMEREQ:
		len = sizeof(SysTimeReq);
		break;
	case CM_SYSTIME:
		len = sizeof(SysTime);
		break;
	case CM_BUSLINEREQ:
		len = sizeof(BusLineReq);
		break;
	case CM_BUSLINE:
		len = sizeof(BusLine);
		break;
	default:
		break;
	}

	return len;
}

int InterpretClient(void * pMsgBuf,int HeadandMsgLen,BYTE Index)//HeadandMsgLen只包括头和消息体，不包括校验和
{
	BYTE cmdNo;
	BYTE RecieveBuf[MAX_MSG_LEN + 1];
	BYTE bSucc = 0;//default is success

	memset((void *)RecieveBuf,0x00,MAX_MSG_LEN);
	memcpy((void *)RecieveBuf,(void *)pMsgBuf,HeadandMsgLen);
	cmdNo = *RecieveBuf;

	switch(cmdNo)
	{
	case CM_TEST:
#ifdef TEST
		DebugWindow("--Received Command 0x01--");
#endif
		break;

	case CM_STATIONSTATUSREQ:
#ifdef TEST
		DebugWindow("--Received Command 0x02--");
#endif
		break;

	case CM_STATIONSTATUS:
#ifdef TEST
		DebugWindow("--Received Command 0x03--");
#endif		
		break;

	case CM_BUSSTATUSREQ:
#ifdef TEST
		DebugWindow("--Received Command 0x04--");
#endif
		break;

	case CM_BUSSTATUS:
#ifdef TEST
		DebugWindow("--Received Command 0x05--");
#endif

		break;

	case CM_STATIONINITREQ:
#ifdef TEST
		DebugWindow("--Received Command 0x06--");
#endif		
		break;

	case CM_STATIONINIT:
#ifdef TEST
		DebugWindow("--Received Command 0x07--");
#endif		
		break;

	case CM_BUSINITREQ:
#ifdef TEST
		DebugWindow("--Received Command 0x08--");
#endif		
		break;

	case CM_BUSINIT:
#ifdef TEST
		DebugWindow("--Received Command 0x09--");
#endif		
		break;

	case CM_BUSCOMING:
#ifdef TEST
		DebugWindow("--Received Command 0x0a--");
#endif	
		break;

	case CM_CM_BUSSTOPPING:
#ifdef TEST
		DebugWindow("--Received Command 0x0b--");
#endif	
		break;

	case CM_BUSLEAVING:
#ifdef TEST
		DebugWindow("--Received Command 0x0c--");
#endif	
		break;

	case CM_BUSRUNNING:
#ifdef TEST
		DebugWindow("--Received Command 0x0d--");
#endif		
		break;

	case CM_LOGON:
#ifdef TEST
		DebugWindow("--Received Command 0x0e--");
#endif	
		break;

	case CM_LOGOUT:
#ifdef TEST
		DebugWindow("--Received Command 0x0f--");
#endif	
		break;

	case CM_CHANGEPASSWORD:
#ifdef TEST
		DebugWindow("--Received Command 0x10--");
#endif	
		break;

	case CM_ALARMMSG:
#ifdef TEST
		DebugWindow("--Received Command 0x11--");
#endif	
		{
/*			AlarmMsg alarmMsg;
			
			alarmMsg.gateNo = *(RecieveBuf+4);
			alarmMsg.AlarmId = *(RecieveBuf+5);
			
			RecieveBuf[2] = ~RecieveBuf[2];
			RecieveBuf[HeadandMsgLen] = FormCheckSum((char *)RecieveBuf,HeadandMsgLen);
			bSucc = SendDataEx((char*)RecieveBuf,(WORD)(HeadandMsgLen+1),Index);//应答,加上校验和一个字节
			
			AlarmMsgProc(&alarmMsg);
*/		}		break;

	case CM_SYSTIMEREQ:
#ifdef TEST
		DebugWindow("--Received Command 0x12--");
#endif		
		{
			struct tm time1;
			time_t t;
			
			time1.tm_year = *(RecieveBuf+4);
			time1.tm_mon = *(RecieveBuf+5);
			time1.tm_mday = *(RecieveBuf+6);
			time1.tm_hour = *(RecieveBuf+7);
			time1.tm_min = *(RecieveBuf+8);
			time1.tm_sec = *(RecieveBuf+9);
			t = mktime(&time1);//转换成time_t类型
			
			RecieveBuf[2] = ~RecieveBuf[2];
			RecieveBuf[HeadandMsgLen] = FormCheckSum((char *)RecieveBuf,HeadandMsgLen);
			bSucc = SendDataEx((char*)RecieveBuf,(WORD)(HeadandMsgLen+1),Index);//应答,加上校验和一个字节
			
			//			SetLocalTime(&t);//设置本地时间
		}
		break;

	case CM_SYSTIME:
#ifdef TEST
		DebugWindow("--Received Command 0x13--");
#endif		
		break;

	case CM_BUSLINEREQ:
#ifdef TEST
		DebugWindow("--Received Command 0x14--");
#endif	
		break;

	case CM_BUSLINE:
#ifdef TEST
		DebugWindow("--Received Command 0x15--");
#endif
		break;
		
	default:

		DebugWindow("----Received Unknown Command!----");
		bSucc = 43;//unknown message type
		break;
	}

	return bSucc;
}
/*
void AlarmMsgProc(AlarmMsg *pAlarmMsg)
{
	char SqlString[1024];
	time_t curtime;
	struct tm *pclock;
	char timestr[20];

 	memset(SqlString,0,1024);
	curtime =time(NULL);
	pclock = localtime(&curtime);
	memset(timestr,0,20);
	sprintf(timestr,"%02d%02d%02d%02d%02d%02d",pclock->tm_year-100,pclock->tm_mon+1,pclock->tm_mday,pclock->tm_hour,pclock->tm_min,pclock->tm_sec);

 	sprintf(SqlString,"insert into aspdb.dbo.报警表(GateNo,AlarmNo,Time) values(%d,%d,\'%s\')",pAlarmMsg->gateNo,pAlarmMsg->AlarmId,timestr);

	DoUpdate(SqlString,30);

	return;
}
void QRFuncProc(QRFunc *pQrFunc)
{
	time_t t;
	char filename[300];
	char SqlString[1024];
	char disp[300];

	memset(filename,0,100);
	GetPrivateProfileString("GENERAL","QRPath","",filename,300, __CONFIG_FILE__);
	time(&t); 
	sprintf(filename+strlen(filename),"\\%ld.bmp",t);//文件名

	//QRGeneration(pQrFunc->QRString,filename);//调用二维码生成程序

	memset(disp,0,100);
	sprintf(disp,"Just before writing to database: len=%d, %s",pQrFunc->Len,pQrFunc->QRString);
	__debugWindows(disp);

	AdjustQRStr(pQrFunc);//add for sql string including '
	memset(disp,0,300);
	sprintf(disp,"Just before writing to database,modified: len=%d, %s",pQrFunc->Len,pQrFunc->QRString);
	__debugWindows(disp);

	memset(SqlString,0,1024);
	sprintf(SqlString,"insert into aspdb.dbo.二维码表(QR,QRPicPos,Count,Expiration,Status) values('%s','%s',%d,'%s',%d)",     \
		               pQrFunc->QRString,filename,100000,"20301231",1);
	__debugWindows(SqlString);

	DoUpdate(SqlString,30);

	return;
}

void AdjustQRStr(QRFunc *pQrFunc)//查看是否存在 单引号
{
	int len = pQrFunc->Len;
	int i,j;
	int count=0;
	char tmpstr[300];

	memcpy(tmpstr,pQrFunc->QRString,pQrFunc->Len);
	i=0;
	j=0;
	while(i<len)
	{
		if(tmpstr[i] != '\'')
			pQrFunc->QRString[j] = tmpstr[i];
		else
		{
			pQrFunc->QRString[j] = tmpstr[i];
			j++;
			pQrFunc->QRString[j] = tmpstr[i];
		}

		i++;
		j++;
	}

	pQrFunc->Len = j;
	pQrFunc->QRString[ pQrFunc->Len ] = 0;

	//检查字符串最后是否有半个汉字
	if((BYTE)(pQrFunc->QRString[len-1])<=(BYTE)(0x7f))
		return;
	i=len-2;
	while((i>=0) && ((BYTE)(pQrFunc->QRString[i]) > (BYTE)(0x7f)))
	{
		count++;
		i--;
	}
	if(count % 2 == 0)
	{
		pQrFunc->QRString[len]=pQrFunc->QRString[len-1];
		pQrFunc->Len++;
	}

	return;
}

void SysStatusProc(SysStatus *pStatus)
{
	char SqlString[1024];

	memset(SqlString,0,1024);
	sprintf(SqlString,"insert into aspdb.dbo.门状态表(GateNo,GateStatus,SysStatus,BatteryStatus,PowerStatus,GPRSStatus) values(%d,%d,%d,%d,%d,%d)",   \
		               1,pStatus->gates[0],pStatus->sysState,pStatus->batteryState,pStatus->powerState,pStatus->gprsState);

	DoUpdate(SqlString,30);
	
	return;
}
*/
BYTE FindCommIndex(DWORD TargetIP,BYTE TargetTaskId)
{
	BYTE i;

	if(TargetIP == INVALID_SOCKET || TargetTaskId == 0)
	{
		DebugWindow("FindCommIndex(),Input Data Error!");
		return 0;
	}

	for(i = 0;i < MAX_SOCKET_NUM; i++)
	{
		if(SocketData[i].PeerIPAddr == TargetIP && SocketData[i].PeerTaskId == TargetTaskId)
			break;
	}

	if(i >= MAX_SOCKET_NUM)
	{
		DebugWindow("FindCommIndex(),Target Socket Index Not Found!");
		return 0;
	}

	return i;
}

int ConnectProc(int CmdNo,BYTE Index)
{
	int bSucc;

	switch(CmdNo)
	{
	case CM_LOGON://
#ifdef TEST
		DebugWindow("ConnectProc(),CM_LOGON!");
#endif
		bSucc = SendDataEx((char*)SLOGON,(WORD)20,Index);

		break;

	case CM_LOGOUT://
#ifdef TEST
		DebugWindow("ConnectProc(),received CM_LOGOUT!");
#endif
		clientOk = 0;

		SLOGON[16] = 0x02;
		SLOGON[18] = 0xA5;
		bSucc = SendDataEx((char*)SLOGON,(WORD)20,Index);

		break;

	case CM_TEST:
#ifdef TEST
		DebugWindow("ConnectProc(),Received CM_TEST!");
#endif
		SLOGON[16] = 0x03;
		SLOGON[18] = 0xA6;
		bSucc = SendDataEx((char*)SLOGON,(WORD)20,Index);
		break;

	default:
		DebugWindow("Received Unknown Message Type!");
		bSucc = 43;//unknown message type
		break;
	}

	return bSucc;
}

BOOL TcpSend(BYTE Index, WORD MsgLen, char * MsgHeadPtr)
{
	int status;
	BYTE i = Index;
	char *TempPtr = NULL;
//#ifdef TEST
//	char disp[200];
//#endif

	if (SocketData[i].Socket == INVALID_SOCKET)
	{
		DebugWindow("TcpSend(),Socket Global Data Error!");
		return FALSE;
	}

//#ifdef TEST
//	memset(disp,0,200);
//	DebugWindow("TcpSend(),Now Send data to wu!");
//#endif

	status = send(SocketData[i].Socket, (char *)MsgHeadPtr, MsgLen, NO_FLAG_SET);
	if (status == SOCKET_ERROR)
	{ 
		status = WSAGetLastError();
		if ((status != WSAEWOULDBLOCK) && (status != WSAEINPROGRESS))
		{	
			DebugWindow("TcpSend(),Send Data Error!");

			CloseSocketConnect(i);
			return FALSE;
		}
		else
			return InWaitSendQueue(i, MsgLen, (char *)MsgHeadPtr);
	}

	if (status != MsgLen)
	{ 	
		TempPtr = (char *)HeapAlloc(MyProcessHeapHandle, HEAP_ZERO_MEMORY, MsgLen);
		if (TempPtr == NULL)
		{
			DebugWindow("TcpSend(),Cannot alloc memory Error!");

			return FALSE;
		}
		
		memcpy(TempPtr, (BYTE *)MsgHeadPtr + status, MsgLen - status);
		if (InWaitSendQueue(i, (WORD)(MsgLen - status), TempPtr) == TRUE)
		{
			HeapFree(MyProcessHeapHandle, 0, MsgHeadPtr);
			return TRUE;
		}
		else
		{
			DebugWindow("TcpSend(),Set in wait queue Error!");
			HeapFree(MyProcessHeapHandle, 0, TempPtr);
			return FALSE;
		}
	}

	HeapFree(MyProcessHeapHandle, 0, MsgHeadPtr);
	return TRUE;	
}
BOOL InWaitSendQueue(BYTE Index, WORD hMsgLen, char * hMsg)
{
	BYTE i = Index;

	if (SocketData[i].WaitSendCount + 1 >= MAX_RESEND_NUM)
	{
		DebugWindow("InWaitSendQueue(),exceeding max_resend_num!");
	//the following line ,for center control GP
//		CloseSocketConnect(Index);

		return FALSE;
	}

	if (SocketData[i].WaitEnd < 0 || SocketData[i].WaitEnd >= MAX_RESEND_NUM)
	{
		DebugWindow("InWaitSendQueue(),Global Data Error!");
		ClearWaitSendQueue(i);
		return FALSE;
	}	

	TcpWaitSendQueue[i][SocketData[i].WaitEnd].hMsgBuf = hMsg;
	TcpWaitSendQueue[i][SocketData[i].WaitEnd].MsgLen = hMsgLen;

	SocketData[i].WaitSendCount++; 
	SocketData[i].WaitEnd = (++SocketData[i].WaitEnd) % MAX_RESEND_NUM;
	
	return TRUE;
}

void TcpResend(BYTE Index)
{
	char *TempPtr;
	char *MsgHeadPtr;
	BYTE i = Index, j;
	int status, MsgLen;	

	if (SocketData[i].WaitHead < 0 || SocketData[i].WaitHead >= MAX_RESEND_NUM)
	{
		DebugWindow("TcpResend(),exceeding max_resend_num!");
		ClearWaitSendQueue(i);
		return;
	}	

	if (SocketData[i].Socket == INVALID_SOCKET)
	{
		DebugWindow("TcpResend(),Socket Already Closed!");
		ClearWaitSendQueue(i);
		return;
	}

	for (j = SocketData[i].WaitSendCount; j > 0; j--)
	{	
//		DebugWindow("TcpReSend(),Now Send data to wu!");

		MsgHeadPtr = (char *)TcpWaitSendQueue[i][SocketData[i].WaitHead].hMsgBuf;
		MsgLen = TcpWaitSendQueue[i][SocketData[i].WaitHead].MsgLen;

		if ((MsgLen > MAX_MSG_LEN + sizeof(char)) || (MsgLen < 0))
		{
			DebugWindow("TcpResend(),Out of Buffer Length!");
			ClearWaitSendQueue(i);
			return;
		}
		
		status = send(SocketData[i].Socket, (char *)MsgHeadPtr, MsgLen, NO_FLAG_SET);
		if (status == SOCKET_ERROR)
		{   
			status = WSAGetLastError();
			if ((status != WSAEWOULDBLOCK) && (status != WSAEINPROGRESS))
			{
				DebugWindow("TcpResend(),Send Data Error,Clear Data!");
				CloseSocketConnect(i);
				return;
			}

			return;
		}				

		if (status != MsgLen)
		{ 	
			TempPtr = (char *)HeapAlloc(MyProcessHeapHandle, HEAP_ZERO_MEMORY, MsgLen);
			if (TempPtr == NULL)
			{
				DebugWindow("TcpResend(),Cannot alloc Memory!");
				CloseSocketConnect(i);
				return;
			}

			memcpy(TempPtr, (BYTE *)MsgHeadPtr + status, MsgLen - status);
			
			TcpWaitSendQueue[i][SocketData[i].WaitHead].hMsgBuf = TempPtr;
			TcpWaitSendQueue[i][SocketData[i].WaitHead].MsgLen = MsgLen - status;
				
			HeapFree(MyProcessHeapHandle, 0, MsgHeadPtr);
				
			return;
		}
		else
		{
			HeapFree(MyProcessHeapHandle, 0, MsgHeadPtr);

			SocketData[i].WaitSendCount--;
			SocketData[i].WaitHead = (++SocketData[i].WaitHead) % MAX_RESEND_NUM;
		}	
	}

	return;	
}

void   DebugWindow(char * DisplayMessage)
{
#ifdef TEST
	char context[MAX_DISPLAY_LEN];
#endif


#ifdef TEST
    if(strlen(DisplayMessage) < MAX_DISPLAY_LEN)
		strcpy(context,DisplayMessage);
	else
	{   
		strncpy(context,DisplayMessage,(MAX_DISPLAY_LEN - 1)*sizeof(char) );
		context[MAX_DISPLAY_LEN - 1] = '\0';
	}

    if( gY>380 )
    {
         OsClearWindow();
    }

    if(gHDC)
    {
       TextOut(gHDC,0,gY,context,strlen(context));
       gY+=16;
    }
#endif

	__debugWindows(DisplayMessage);

	return;
}

void   OsClearWindow(void)
{
    PAINTSTRUCT ps;
    RECT        rcPaint;
    
    // clear screen
    GetClientRect(hMainWnd,&rcPaint);
    InvalidateRect(hMainWnd,&rcPaint,TRUE);
    BeginPaint(hMainWnd,&ps);
    EndPaint(hMainWnd,&ps);
    gY=0;

	return;
}

void InitApp(void)
{
    gHDC = GetDC(hMainWnd);
    gY=0;

	return;
}

void __debugWindows(const char* fmt,...)
{
    va_list marker;
    char szBuf[256];
    
    va_start(marker, fmt);
    vsprintf(szBuf, fmt, marker);
    va_end(marker);

#ifdef TEST
    CommOutputDebugString(szBuf);
#else
    __DebugWindowUINT32(szBuf);
#endif

	return;
}

#define _MAX_SIZE_LIMITED_  1000000*30
static long position = 0;
#ifdef TEST
static void CommOutputDebugString(const char *debuginfo)
{
    char szDebug[400];
    time_t timeval;

    memset(szDebug,0,sizeof(szDebug));

#ifdef _MAX_SIZE_LIMITED_
    if (position >_MAX_SIZE_LIMITED_)
    {
       if (fp != NULL) 
       { 
           fseek(fp,0,SEEK_SET);
       }
    }
#endif

    if ( fp == NULL )
    {
        if ((fp = fopen(__DEBUG_FILE__,"at+")) == NULL)
           return ;
        fputs("================================================================\r\n",fp);
        fputs("================================================================\r\n",fp);
    }

    time(&timeval);
    sprintf(szDebug,"%s",ctime(&timeval));
    sprintf(&szDebug[strlen(szDebug)-1],"Info:%s\r\n",debuginfo);

	if(fp)
	{
		fputs(szDebug,fp);
		fflush(fp);

   //modify 10,29,2001
		#ifdef _MAX_SIZE_LIMITED_
		   position = ftell(fp);
		#endif
	//end
	}

	return;
}
#else

void   __DebugWindowUINT32(const char * DisplayMessage)
{ 
    char context[300];
    time_t timeval;

    memset(context,' ',sizeof(context));

    time(&timeval);
    sprintf(context,"%s",ctime(&timeval)); 
    sprintf(&context[strlen(context)-1]," %s",DisplayMessage);
    
#ifdef _MAX_SIZE_LIMITED_
    if (position >_MAX_SIZE_LIMITED_)
    {
       if (fp != NULL) 
       { 
           fseek(fp,0,SEEK_SET);
       }
    }
#endif

    memset(context+strlen(context),' ',sizeof(context)-strlen(context));
    context[sizeof(context)-3] = 0;  //set context[137]=0
    strcat(context,"\r\n");
       
    if( fp==NULL )
    {
       fp = fopen(__DEBUG_FILE__,"wt+");

       if (fp)
       {
           fputs("================================================================\r\n",fp);
           fputs("================================================================\r\n",fp);
       }   
    }

    if(fp)
    {
       int len;
       len = fputs(context,fp);
       fflush(fp);
	#ifdef _MAX_SIZE_LIMITED_
       position = ftell(fp);
	#endif
    }

	return;
}
#endif

BOOL SendData(char * pBuff,WORD MsgLen,SOCKET s)
{
	char * MsgHeadPtr;
	BYTE i;

	if(pBuff == NULL)
	{
		DebugWindow("SendData(),Input Msg Body Error!");
		return FALSE;
	}

	if(MsgLen <= 0 || MsgLen > MAX_MSG_LEN)
	{
		DebugWindow("SendData(),Input Msg Length Error!");
		return FALSE;
	}
	
	if(s == INVALID_SOCKET)
	{
		DebugWindow("SendData(),Send Socket Error!");
		return FALSE;
	}

	MsgHeadPtr = (char *)HeapAlloc(MyProcessHeapHandle, HEAP_ZERO_MEMORY, MsgLen);
	if (MsgHeadPtr == NULL)
	{
		DebugWindow("SendData(),Alloc Memory Error!");
		return FALSE;
	}

	memcpy(MsgHeadPtr, pBuff, MsgLen);

	for (i = 1; i < MAX_SOCKET_NUM; i++)
	{
		if(SocketData[i].Socket != INVALID_SOCKET && SocketData[i].Socket == s)
			break;
	}

	if (i == 0 || i >= MAX_SOCKET_NUM)
	{	  
		DebugWindow("SendData(),Buffer Overflow!");
		HeapFree(MyProcessHeapHandle, 0, MsgHeadPtr);						
		return FALSE;
	}

	if (TcpSend(i, MsgLen, MsgHeadPtr) != TRUE)
	{	
		DebugWindow("SendData(),Send Data Failed!");
		HeapFree(MyProcessHeapHandle, 0, MsgHeadPtr);
		return FALSE;
	}		 
	else
		return TRUE;
}

BOOL SendDataEx(char * pBuff,WORD MsgLen,unsigned char SockIndex)
{
	char * MsgHeadPtr;

	if(pBuff == NULL)
	{
		DebugWindow("SendDataEx(),Input Msg Body Error!");
		return FALSE;
	}

	if(MsgLen <= 0 || MsgLen > MAX_MSG_LEN)
	{
		DebugWindow("SendDataEx(),Input Msg Length Error!");
		return FALSE;
	}
	
	if (SockIndex == 0 || SockIndex >= MAX_SOCKET_NUM)
	{	  
		DebugWindow("SendDataEx(),Buffer Overflow!");
		return FALSE;
	}

	if(SocketData[SockIndex].Socket == INVALID_SOCKET)
	{
		DebugWindow("SendDataEx(),Send Socket Error!");
		return FALSE;
	}

	if(SocketData[SockIndex].LinkStatus == FALSE)
	{
#ifdef TEST
		DebugWindow("SendDataEx(),Send Socket Not Linked!");
#endif
		return FALSE;
	}

	MsgHeadPtr = (char *)HeapAlloc(MyProcessHeapHandle, HEAP_ZERO_MEMORY, MsgLen);
	if (MsgHeadPtr == NULL)
	{
		DebugWindow("SendDataEx(),Alloc Memory Error!");
		return FALSE;
	}

	memcpy(MsgHeadPtr, pBuff, MsgLen);

	if (TcpSend(SockIndex, MsgLen, MsgHeadPtr) != TRUE)
	{	
		DebugWindow("SendDataEx(),Send Data Failed!");
		HeapFree(MyProcessHeapHandle, 0, MsgHeadPtr);
		return FALSE;
	}		 
	else
		return TRUE;
}

void CheckSvrLink(void)
{
	if(SocketData[1].LinkStatus == TRUE)
		return;

	if(SocketData[1].Socket != INVALID_SOCKET)
	{
		CloseSocketConnect(1);
//		return;
	}

//	ReConnectSvrProc(inet_addr(SvrIp));
//	ReConnectSvrProc(SvrIPAddr);

	return;
}

BOOL DoConnect(char *szServer)
{
    RETCODE retcode;

	char     szConnStrIn[100];
	char     szConnStrOut[100];
	char	 sServer[30];
	SWORD    cbConnStrOut;
#ifdef TEST
	char disp[200];
#endif

	if(strcmp(szServer, "") == 0)
	{
		DebugWindow("DoConnect(),DB Server Name Is Empty!");
		return FALSE;
	}

	sprintf(sServer,szServer);

	retcode = SQLAllocEnv(&henv);
	if(retcode == SQL_ERROR) 
	{
		DebugWindow("DoConnect(),Unable Alloc Env Handle!");
		return FALSE;
	}

	retcode = SQLAllocConnect(henv,(HDBC far*)&hdbc);
	if(retcode == SQL_ERROR || retcode == SQL_SUCCESS_WITH_INFO || retcode == SQL_INVALID_HANDLE)
	{
		DebugWindow("DoConnect(),Unable Alloc Connect Handle!");
		return FALSE;
	}
	
	SQLSetConnectOption(hdbc, SQL_LOGIN_TIMEOUT, 10);
	sprintf(szConnStrIn , "DRIVER={SQL Server};SERVER=%s;UID=sa;PWD=%s", sServer, "1234567890");

	retcode = SQLDriverConnect(hdbc,
			 NULL,
			(UCHAR FAR *)szConnStrIn, (SWORD)strlen(szConnStrIn),
			(UCHAR FAR *)szConnStrOut, (SWORD)sizeof(szConnStrOut),
			(SWORD FAR *)&cbConnStrOut, SQL_DRIVER_NOPROMPT);
	
	if(retcode == SQL_ERROR || retcode == SQL_INVALID_HANDLE)
	{
		GetSQLError(NULL);
		DebugWindow("DoConnect(),Cannot Connect To Db!");
		return FALSE;
	}
#ifdef TEST
	else
	{
		memset(disp,0,200);
		sprintf(disp,"Has Connected to Database Server:%s",sServer);
		DebugWindow(disp);
	}
#endif


	return TRUE;
}

void  DoDisconnect(void)
{

	if(hdbc == NULL) 
	{
		DebugWindow("DoDisconnect(),Already Disconnect,Maybe Somewhere Error!");
		return;
	}

	SQLDisconnect(hdbc);
	SQLFreeConnect(hdbc);
	SQLFreeEnv(henv);
	hdbc = NULL;

	return;
}

BYTE  GetSQLError(HSTMT hstmt)
{
	RETCODE retcode;
   	char    szErrorMsg[200];
	SDWORD	dwErrCode;	
    char	szSqlState[6];
  	SWORD	cbErrorMsg = 0;

	cbErrorMsg = (SWORD)sizeof(szErrorMsg);
	
	retcode = SQLError(henv, hdbc, hstmt, (UCHAR FAR *)szSqlState, &dwErrCode,
					(UCHAR FAR *)szErrorMsg, cbErrorMsg, &cbErrorMsg);
    
	if(retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
	{
		if(strcmp(szSqlState, LINKDISCONNECT) == 0)
		{
			DebugWindow("GetSQLError(),Disconnect with SQL Server!");
			if(hstmt != NULL) 
				SQLFreeStmt(hstmt, SQL_DROP);
			return DB_SQL_LINKDISCONNECT;  
		}
	}

	szErrorMsg[99]=0;	
	DebugWindow(szErrorMsg);
	if(hstmt != NULL) 
		SQLFreeStmt(hstmt,SQL_DROP);

	return DB_SQL_GENERALLYERROR;
}	

BYTE DoUpdate(char *SQLString,UINT16 WaitTime)
{
	HSTMT    hstmt;
    RETCODE  retcode;
	SDWORD	 UpdateRow = 0;

	retcode = SQLAllocStmt(hdbc,&hstmt);
	if(RETCODE_IS_FAILURE(retcode)) 
		return DB_SQL_GENERALLYERROR;
  	
	retcode= SQLSetStmtOption(hstmt, SQL_QUERY_TIMEOUT, WaitTime);
	if(RETCODE_IS_FAILURE(retcode))
		return GetSQLError(hstmt);
  	
	retcode = SQLExecDirect(hstmt, (UCHAR FAR *)SQLString, SQL_NTS);
	if(RETCODE_IS_FAILURE(retcode)) 
    {
        BYTE errcode;

        errcode = GetSQLError(hstmt);
        if(errcode != DB_SQL_LINKDISCONNECT)
        {
            char temp[50];

            sprintf(temp, "begin transaction rollback transaction");
            retcode = SQLAllocStmt(hdbc, &hstmt);
	        if(RETCODE_IS_FAILURE(retcode)) 
		        return DB_SQL_GENERALLYERROR;
  	        
	        retcode= SQLSetStmtOption(hstmt, SQL_QUERY_TIMEOUT, 20);
	        if(RETCODE_IS_FAILURE(retcode))
		        return GetSQLError(hstmt);
  	        
	        retcode = SQLExecDirect(hstmt, (UCHAR FAR *)temp, SQL_NTS);
	        if(RETCODE_IS_FAILURE(retcode)) 
	            errcode = DB_SQL_LINKDISCONNECT;
           
            SQLFreeStmt(hstmt, SQL_DROP);
        }

        return errcode;
    }	
			
	retcode = SQLRowCount(hstmt, &UpdateRow);
	if(RETCODE_IS_FAILURE(retcode)) 
		return GetSQLError(hstmt);

	SQLFreeStmt(hstmt, SQL_DROP);
	if(UpdateRow == 0)
		return DB_SQL_NOROWUPDATED;

	return DB_SQL_SUCCESS;
}


INT32  dbSQLExecDirect(HSTMT hstmt, char *SQLString)
{
	INT32   BindColNumber = 0;
	RETCODE retcode;

	retcode= SQLSetStmtOption(hstmt, SQL_QUERY_TIMEOUT, 20);
	if(RETCODE_IS_FAILURE(retcode))
		return -1;
  	
	retcode = SQLExecDirect(hstmt, (UCHAR FAR *)SQLString, SQL_NTS);
	if(RETCODE_IS_FAILURE(retcode)) 
		return -1;

	retcode = SQLNumResultCols(hstmt, (SWORD FAR *)&BindColNumber);
	if(RETCODE_IS_FAILURE(retcode)) 
		return -1;

	return BindColNumber;
}

void rtrim(char *Instr)
{
	char *pchar;
    
    pchar = strrchr(Instr ,' ');
    if(pchar == NULL)
		return ;
    
    if(*(pchar+1) != 0)
		return;
    
    while(1)
    {
        if( *(--pchar) != ' ')
		{
			*(pchar+1) = 0;
            break;
        }
    }

	return;
}

BOOL Initdb(void)
{
	register int i;
	char DbIpSting[100];
	DWORD ret;
	UINT8 iniOk = TRUE;

	memset(DbIpSting,0x00,100);
	ret = GetPrivateProfileString("GENERAL","DBAddr","127.0.0.1",DbIpSting,100, __CONFIG_FILE__);

	if(ret > 0 && ret <= 15)
	{
		for(i = 0;i < (int)ret; i++)
		{
			if(!(DbIpSting[i] == '.' || (DbIpSting[i] <= '9' && DbIpSting[i] >= '0') ))
			{
				iniOk = FALSE;
				break;
			}
		}

		if(DbIpSting[0] == '.' || DbIpSting[0] >'2')
			iniOk = FALSE;
	}
	else
		iniOk = FALSE;

	if(iniOk == FALSE)
	{
		memset(DbIpSting,0,100);
		strcpy(DbIpSting,"127.0.0.1");
	}

	if(!DoConnect(DbIpSting))
	{
		DebugWindow("Initdb(),DB Connection Failed,System Exit!");
		PostQuitMessage(0);
		return FALSE;
	}

	return TRUE;
}

BYTE GetNotify(void)
{
	char SlqStr[300];
	BYTE   i;
	SDWORD  cbt[MAX_RET_COLNUMBER];
    char    sqlResult[4][MAX_COL_LEN];
	HSTMT   hstmt;
    RETCODE retcode;
	INT32   BindColNumber;
	char QRContent[MAX_QRSTR_LEN],Time[50];
	int Id,type;
	char fput[255];
//	time_t t;
//	char filename[100];

   	if(hdbc == NULL)
		return DB_SQL_GENERALLYERROR;

	retcode = SQLAllocStmt(hdbc, &hstmt);
	if(RETCODE_IS_FAILURE(retcode)) 
		return DB_SQL_GENERALLYERROR;

	memset(SlqStr,0,300);
	sprintf(SlqStr,"select id,QRContent,type,Time from aspdb.dbo.通知表 where Status = 1");
	BindColNumber = dbSQLExecDirect(hstmt,SlqStr);
	if(BindColNumber == -1 || BindColNumber != 4)
		return GetSQLError(hstmt);
    
	for(i=0; i<BindColNumber; i++)
	{
		retcode = SQLBindCol(hstmt,(UWORD)(i+1), SQL_C_CHAR, sqlResult[i], sizeof(sqlResult[i]), &cbt[i]);
		if(RETCODE_IS_FAILURE(retcode))
			return GetSQLError(hstmt);
	}
	
	while(1)
	{
		retcode = SQLFetch(hstmt);
		if(RETCODE_IS_FAILURE(retcode)) 
			return GetSQLError(hstmt);
	    
		if(retcode == SQL_NO_DATA_FOUND)
			break;

		rtrim(sqlResult[0]);
		rtrim(sqlResult[1]);
		rtrim(sqlResult[2]);
		rtrim(sqlResult[3]);
	
		if(strlen(sqlResult[1]) == 0 || strlen(sqlResult[1]) > 255)
		{
			DebugWindow("GetNotify(),QRContent too large in db!");
			continue;
		}
		if(strlen(sqlResult[2]) == 0 || strlen(sqlResult[2]) >1)
		{
			DebugWindow("GetNotify(),type length out of range in db!");
			continue;
		}
		if(strlen(sqlResult[3]) == 0 || strlen(sqlResult[3]) > 50)
		{
			DebugWindow("GetNotify(),Time too large in db!");
			continue;
		}
		memset(QRContent,0,MAX_QRSTR_LEN);
		memset(Time,0,50);
		memset(fput,0,255*sizeof(char));

		Id = atoi(sqlResult[0]);
		type = (UINT8)atoi(sqlResult[2]);
		strcpy(fput,sqlResult[1]);
//		strcpy(QRContent,sqlResult[1]);
		strcpy(Time,sqlResult[3]);

/*		time(&t); 
		memset(filename,0,100);
		sprintf(filename,"%ld.jpg",t);//文件名
		Str2QRPic(pQrFunc->QRString,filename);//调用二维码生成程序
*/
		Gb2312Utf(fput, strlen(fput), QRContent, MAX_QRSTR_LEN);
//		SendQRStr2GPRS(QRContent,strlen(QRContent)-1);//发给gprs
	}

	SQLFreeStmt(hstmt, SQL_DROP);

	memset(SlqStr,0,300);
	sprintf(SlqStr,"update aspdb.dbo.通知表 set Status=0 where status=1");
	DoUpdate(SlqStr,30);


	return DB_SQL_SUCCESS;
}
/*
void SendQRStr2GPRS(char *pQRStr,int QRlen)//QRlen=qr码串的长度
{
	BYTE QRStr[255];
	BYTE Index = 1;

	memset(QRStr,0,255);
	QRStr[0] = 0xaa;
	QRStr[1] = 0x55;
	QRStr[2] = 0x01;
	QRStr[3] = QRlen + 1;
	QRStr[4] = 0x0c;  // gate number
	memcpy(QRStr+5,pQRStr,QRlen);
//	memcpy(QRStr+4,pQRStr,len);
	QRStr[QRlen+5] = FormCheckSum((char *)(QRStr+2),QRlen+5);

	SendDataEx((char*)QRStr,(WORD)(QRlen+6),Index);//发给gprs

	return;
}
*/
void GetAppConfig(void)
{
	BOOL Ret;

	memset(SvrIp,0,sizeof(SvrIp));
	Ret = GetSvrIp(SvrIp);
	if(!Ret)
	{
		DebugWindow("GetAppConfig(),Get Svr Ip Failed!");
		return;
	}

	TcpPort = GetPrivateProfileInt("GENERAL","SockPort",PBCTRANS_TCP_PORT, __CONFIG_FILE__);

	return;
}

BOOL GetSvrIp(char *SvrIpSting)
{
	register int i;
	DWORD ret;

	ret = GetPrivateProfileString("GENERAL","SvrAddr","",SvrIpSting,100, __CONFIG_FILE__);

	if(ret <= 0 || ret > 15)
		return FALSE;

	for(i = 0;i < (int)ret; i++)
	{
		if(!(SvrIpSting[i] == '.' || (SvrIpSting[i] <= '9' && SvrIpSting[i] >= '0') ))
			return FALSE;
	}

	if(SvrIpSting[0] == '.' || SvrIpSting[0] >'2')
		return FALSE;

	SvrIPAddr = inet_addr(SvrIpSting);

	return TRUE;
}

BYTE FormCheckSum(char *InPtr,int Length)
{
	register int i;
	int sum = 0;
	int len = Length;

	for(i = 2;i < len;i++)
		sum += (BYTE)(*(InPtr + i));
	sum = sum & 0xff;

	sum = 256-sum;

	return (BYTE)sum;
}

BOOL IsCheckSumOk(char *InPtr,int Length)
{
	register int i;
	int sum = 0;
	char tempsum;
	int len = Length;

	for(i = 0;i < len;i++)
		sum += *(InPtr + i);
	tempsum = (char)(sum & 0xff);

	tempsum=(char)(tempsum + *(InPtr + len));

	if(tempsum==0)
		return TRUE;
	else
		return FALSE;
	
	return TRUE;
}

