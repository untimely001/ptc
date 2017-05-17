/*****************************************************************
*ModuleName: Wincomm               FileName:Wincomm.c            *
*CreateDate: 07/21/2k               Author:  yincy               *
*Version:    1.0                                                 *
*History:                                                        *
* Date       Version       Modifier           Activies           *
*================================================================*
*07/21/2000   1.0           yincy       create                   *
*12/25/2000   1.1           yincy       modify                   *
*02/22/2001   1.1           yincy       modify                   *
*07/08/2001   1.1           yincy       modify                   *
*****************************************************************/

#include "wincomm.h"
#include "ctlschemep.h"
#include "jtdb.h"
#include "PComm.h"

#pragma comment(lib,"pComm.lib")


#ifdef SERVICE
#include <tchar.h>
#include "service.h"
#endif

#ifdef SERVICE
HANDLE  hServerStopEvent = NULL;
extern SERVICE_STATUS          ssStatus;
extern SERVICE_STATUS_HANDLE   sshStatusHandle;
extern DWORD                   dwErr;
#endif

static HDC gHDC;
static int gY;
DWORD WinOsVerId = 4;
HWND hMainWnd;
FILE *fp = NULL;
char HostIp[16];
DWORD LocalIPAddr = 0;
extern char SvrIp[16];
DWORD SvrIPAddr = 0;
int TcpPort = 0;
int SerialStart = 1;
int SerialCommNum = 0;
//int CheckHuCount = 5;
int CalCrossTime = 1200;
int TestInterval = 3;
int ConnectSvrCount = 60;
extern UINT8 UseLcu;
extern int Card1;
extern int Card2;
extern CrossData_t CrossData[MAX_CROSS_NUM];

SerialBuff_t SerialBuff[MAX_CROSS_NUM];
SerialCtr ThreadCtr[MAX_CROSS_NUM] = {{NULL,0,0}};

SERIALINFO			gSerialInfo;
NPSERIALINFO		gnpSerialInfo;
BYTE				gCommBuff[BUFFERSIZE+1];

#ifdef TEST
BYTE				gCommBuffTest[100][BUFFERSIZE+1];
int					BufLen[100];
int					BuffIndex = 0;
int					bTestCount = 1;
//int invokecount = 0;
#endif
//int deccount = 0;
//int inccount = 0;

char			szAppName[NAME_LEN] = APPLICATION_NAME;
char			szTitle[NAME_LEN] = APPLICATION_TITLE;
HANDLE			MyProcessHeapHandle = NULL;
SocketDataNode	SocketData[MAX_SOCKET_NUM];
MsgWaitNode		TcpWaitSendQueue[MAX_SOCKET_NUM][MAX_RESEND_NUM];

SOCKET ClientSock,SvrSock;
struct sockaddr_in	remote,
					from;   
short UdpPort;
DWORD dwMulticastGroup;

HINSTANCE DllInst = NULL;
BOOL InitFaceDll(void);
void (__stdcall * ShowPopUp)(POINT * pMousePos);
void (__stdcall * GetSysParam)(long hWnd,char * pCross,char *pClient);

#ifdef _WIN32
HANDLE  svr_sem;
#endif

/*trace file name*/
#define __DEBUG_FILE__     ".\\jtdebug.inf"
DWORD    BaudTable[] =
         {
            CBR_110, CBR_300, CBR_600, CBR_1200, CBR_2400,
            CBR_4800, CBR_9600, CBR_14400, CBR_19200, CBR_38400,
            CBR_56000, CBR_128000, CBR_256000
         } ;

DWORD    ParityTable[] =
         {
            NOPARITY, EVENPARITY, ODDPARITY, MARKPARITY, SPACEPARITY
         } ;

DWORD    StopBitsTable[] =
         {
            ONESTOPBIT, ONE5STOPBITS, TWOSTOPBITS
         } ;


#if defined (WIN32)
	#define IS_WIN32 TRUE
#else
	#define IS_WIN32 FALSE
#endif

//---------------------------------------------------------------------------
//	void ServiceStart(DWORD dwArgc, LPTSTR *lpszArgv)
//  Description:
//     This is the Service Start !
//
//  
//  
//
//---------------------------------------------------------------------------
#ifdef SERVICE
void ServiceStart(DWORD dwArgc, LPTSTR *lpszArgv)
{
	DWORD Ret;
    HANDLE                  hEvents = NULL;
	MSG						msg;
	HWND Window;

    // report the status to the service control manager.
    if (!ReportStatusToSCMgr(
        SERVICE_START_PENDING, // service state
        NO_ERROR,              // exit code
        3000))                 // wait hint
        return;

    // create the event object. The control handler function signals
    // this event when it receives the "stop" control code.
    hServerStopEvent = CreateEvent(
        NULL,    // no security attributes
        TRUE,    // manual reset event
        FALSE,   // not-signalled
        NULL);   // no name

    if ( hServerStopEvent == NULL)
	{
		MessageBox(hMainWnd, "Create Event Failed!", "Warning", MB_ICONSTOP | MB_OK);
        return;
	}

    hEvents = hServerStopEvent;

    // report the status to the service control manager.
    if (!ReportStatusToSCMgr(
        SERVICE_START_PENDING, // service state
        NO_ERROR,              // exit code
        3000))                 // wait hint
	{
		MessageBox(hMainWnd, "Report to SC Manager 1 Failed!", "Warning", MB_ICONSTOP | MB_OK);
        return;
	}

	if ((Window = MakeWorkerWindow()) == NULL)
	{
		MessageBox(hMainWnd, "Create Work Window Failed!", "Warning", MB_ICONSTOP | MB_OK);
		return;
	}

	if (!InitTCPIP())	
	{
		MessageBox(hMainWnd, "TcpInit Failed!", "Warning", MB_ICONSTOP | MB_OK);
		return;
	}

	if (!Initdb())	
	{
		MessageBox(hMainWnd, "Connect to DB Failed!", "Warning", MB_ICONSTOP | MB_OK);
		return;
	}

/*	if(!InitSerialComm())
		return;
*/
	//initilize app uten   
	InitApp();

    // report the status to the service control manager.
    if (!ReportStatusToSCMgr(
        SERVICE_RUNNING,       // service state
        NO_ERROR,              // exit code
        0))                    // wait hint
	{
		MessageBox(hMainWnd, "Report to SC Manager 2 Failed!", "Warning", MB_ICONSTOP | MB_OK);
        return;
	}

	while (Ret = GetMessage(&msg, NULL, 0, 0))
	{
      if (Ret == -1)
         return;
 
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return;
}

//---------------------------------------------------------------------------
//  int PASCAL WinMain( HANDLE hInstance, HANDLE hPrevInstance,
//                      LPSTR lpszCmdLine, int nCmdShow )
//
//  Description:
//     This is the main window loop!
//
//  Parameters:
//     As documented for all WinMain() functions.
//
//---------------------------------------------------------------------------
#else
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

/*	if (!InitUdp())	
		return (FALSE);
*/
	GetAppConfig();

	if (!Initdb())
	{
		MessageBox(hMainWnd, "Connect to DB Failed!", "Warning", MB_ICONSTOP | MB_OK);
		return (FALSE);
	}	
	else
		PostMessage(hMainWnd,INITIALEVENT,0x10,0x20);
/*
	if(!InitSerialComm())
		return (FALSE);
*/
	ServiceInit();

	InitCrossComm();

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
	strcpy(NotifyData.szTip, "SLJTControl");

	Shell_NotifyIcon(NIM_ADD, &NotifyData);
	#ifndef FORDEBUG
	ShowWindow(hMainWnd, SW_HIDE);
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
    strcpy(NotifyData.szTip, "SLJTControl");

    Shell_NotifyIcon(NIM_DELETE, &NotifyData);
    }

	return msg.wParam;

//never reached here
//	lpCmdLine;
//	hPrevInstance;
}
#endif


#ifdef SERVICE
//---------------------------------------------------------------------------
//  HWND MakeWorkerWindow(void)
//
//  Description:
//     
//     
//
//  Parameters:
//     As documented for Window procedures.
//
//  Win-32 Porting Issues:
//     - WM_HSCROLL and WM_VSCROLL packing is different under Win-32.
//     - Needed LOWORD() of wParam for WM_CHAR messages.
//
//---------------------------------------------------------------------------
HWND MakeWorkerWindow(void)
{
	WNDCLASS wndclass;
	HWND Window;

	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = (WNDPROC)WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = NULL;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = szAppName;

	if (RegisterClass(&wndclass) == 0)
	{
		MessageBox(hMainWnd, "RegisterClass() Failed!", "Warning", MB_ICONSTOP | MB_OK);
//		printf("RegisterClass() failed with error %d\n", GetLastError());
		return NULL;
	}


	if ((Window = CreateWindow(
		szAppName,
		szTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL,
		NULL,
		NULL,
		NULL)) == NULL)
	{
//		printf("CreateWindow() failed with error %d\n", GetLastError());
		MessageBox(hMainWnd, "CreateWindow() Failed!", "Warning", MB_ICONSTOP | MB_OK);
		return NULL;
	}

	// Create a window.
	hMainWnd = Window;

	ShowWindow(hMainWnd, SW_HIDE);
	UpdateWindow(hMainWnd);
	
	//设置当前进程的高优先权
	SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);

	//设置异常过滤处理函数
	SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)MyUnhandledExceptionFilter);
	
	MyProcessHeapHandle = GetProcessHeap();

	return Window;
}
#endif

#ifdef SERVICE
//---------------------------------------------------------------------------
//  void ServiceStop(void)
//
//  Description:
//     
//     
//
//  Parameters:
//     As documented for Window procedures.
//
//  Win-32 Porting Issues:
//     - WM_HSCROLL and WM_VSCROLL packing is different under Win-32.
//     - Needed LOWORD() of wParam for WM_CHAR messages.
//
//---------------------------------------------------------------------------
void ServiceStop(void)
{
    if (hServerStopEvent)
	{
		PostMessage(hMainWnd,WM_QUIT,0,0);
        SetEvent(hServerStopEvent);
	}
}
#endif

//---------------------------------------------------------------------------
//  LRESULT FAR PASCAL WndProc( HWND hWnd, UINT uMsg,
//                                 WPARAM wParam, LPARAM lParam )
//
//  Description:
//     This is the Main Window Proc.  This handles ALL messages
//     to the window.
//
//  Parameters:
//     As documented for Window procedures.
//
//  Win-32 Porting Issues:
//     - WM_HSCROLL and WM_VSCROLL packing is different under Win-32.
//     - Needed LOWORD() of wParam for WM_CHAR messages.
//
//---------------------------------------------------------------------------
LRESULT FAR PASCAL WndProc(HWND hMainWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
//	int len;
	switch (message)
	{
		case WM_CREATE:

//			RegisterHotKey(hMainWnd, 0x01, MOD_CONTROL | MOD_ALT, VK_F12);
			
//			PostMessage(hMainWnd,INITIALEVENT,0,0);
			break;

		case WM_HOTKEY: //reInitialize	
			{
				BYTE i;
				
				DebugWindow("System ReInitialize Now.....");

				//close socket
				CancelBlocking();

				for (i = 0; i < MAX_SOCKET_NUM; i++)	
					CloseSocketConnect(i);
					
				//for (i = 0; i < MAX_CONNECT_NO; i++)	
				//	DoDisconnect(i);

				CancelBlocking();
				WSACleanup();

				//close serial comm
//				DestroySerialInfo() ;

				//reInitial
				if (!InitTCPIP())	
					return (FALSE);

/*				if(!InitSerialComm())
					return (FALSE);
*/
				DebugWindow("System ReInitialize Finished!");

			}
			break;

		case SENDCTR_EVENT:

			if(wParam == 0x1234 && lParam == 0x2345)
				Send2AllCross();

			break;

		case LPT_READ:

//#ifdef __TEST
//			DebugWindow("Received LPT_READ Event!");
//#endif
			GetLcuData(wParam,lParam);
			//TestGetLptData();

			break;

		case MULTICARD_READ:
			
			if(lParam == 0x9595)
				if(wParam > 0 && wParam <= MAX_CROSS_NUM)
					ReadMulSerialData((int)wParam);

			break;

		case EXTENDMSGSEND:
			
			if(wParam > 0  && wParam <= MAX_CROSS_NUM)
				SendMsgInQue((int)wParam,(int)lParam);

			break;

		case WSA_ACCEPT:		

			DebugWindow("To Accept Client!"); 
			AcceptConnect(wParam, lParam);
			break;

		case SVR_RECONNECT:

			ReConnectSvrProc((DWORD)wParam);
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
					//added 04,05,2001
					//CheckTheOprData(i);
					//added ends
					return -1;
				}

				switch (WSAGETSELECTEVENT(lParam))
				{
					case FD_READ:
						//DebugWindow("Read Data From Socket!");
						ReceiveData(i);
						break;

					case FD_WRITE:
						//DebugWindow("FD_WRITE,Can Send Data !");
						TcpResend(i);
						break;

					case FD_CLOSE:
						DebugWindow("A Socket Will Be Closed!");
						CloseSocketConnect(i);
						break;
				}
			}
			break;

			case SERIALNOTIFY:
				{

					unsigned char SvrAck = (unsigned char)0xa2;

					if((unsigned char)(*gCommBuff) != (unsigned char)0xa2 
						&& (unsigned char)(*gCommBuff) != (unsigned char)0xaa)
						if(!WriteCommBlock(&SvrAck, sizeof(unsigned char)))
							return 1;

					DebugWindow("Recieved SERIALNOTIFY Event!"); 
					if(wParam == PORT(gnpSerialInfo) && lParam != 0)
					{
						OnNotify((DWORD)lParam);
					}
					else
					{
						DebugWindow("Recieved Unknown Serial Message");
					}
				}
			break;

		case INITIALEVENT:
			break;

		case LCUBUF_RESEND:

			LcuBufResendProc((UINT8)wParam);

			break;

		case WM_TIMER:

			switch (wParam)
			{
				BYTE i;

				case REPEAT_SEND_WAIT_PACKET://1000
					
					for (i = 1; i < MAX_SOCKET_NUM; i++)	
					{
						if ((SocketData[i].Socket == INVALID_SOCKET) \
							|| (SocketData[i].WaitSendCount <= 0))
						continue;

						TcpResend(i);
					}

					CheckSignal();
					
					CheckForecedTime();

					if(CalCrossTime <= 0)
					{
						SendCurTime();
						//SendTest2Hu();
						//TestWrite2lcu();

						CalCrossTime = 3600 * 5;
					}
					else
						CalCrossTime--;

					AckTypeCmdTimeoutProc();

					//HuLcuResend();
					//SendTestMsg();

					//send test command to hu
					if(TestInterval <= 0)
					{
						SendTest2Hu();
						TestInterval = 25;
					}
					else
						TestInterval--;

					if(WinOsVerId == 4)//Only for NT4 
					{
						if(ConnectSvrCount <= 0)
						{
							CheckSvrLink();
							ConnectSvrCount = 60;
						}
						else
							ConnectSvrCount--;
					}
					break;

				case TIMEOUT_CHECK://100
					
					HuLcuTimeoutProc();

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

//				UnregisterHotKey(NULL, 0x01);


				//disconnect db  connection
//				for (i = 0; i < MAX_CONNECT_NO; i++)	
					//DoDisconnect(i);

				//close socket
				for (i = 1/*0*/; i < MAX_SOCKET_NUM; i++)
					CloseSocketConnect(i);

				CancelBlocking();
				WSACleanup();

//close serial comm
/*				DestroySerialInfo() ;

//added 08,24,2k
				#ifdef SERVICE
			    if (hServerStopEvent)
				{
			        CloseHandle(hServerStopEvent);
				}

			    if (sshStatusHandle)
					(void)ReportStatusToSCMgr(
						        SERVICE_STOPPED,
							    dwErr,
								0);

				#endif			
				//added ends
*/
				//added 02,19/2001
//				LptExit();

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

		//added 10,09,2001
		case WM_FACEUNLOAD:

			if(DllInst != NULL)
			{
				if(FreeLibrary(DllInst))
				{
					DllInst = NULL;
				}
			}
			
			break;

		case SYSICONNOTIFY:
			switch(lParam)
			{
				case WM_RBUTTONUP:
				{
					POINT MousePos;

					GetCursorPos(&MousePos);
					if(DllInst == NULL)
					{
						if(!InitFaceDll())
						{
							return 0;
						}
					}

					//ShowWindow(hMainWnd, SW_HIDE);
					ShowPopUp(&MousePos);

				}
					break;
				default:
					break;
			}
			break;
			//add end

         // fall through
		default:

			return (DefWindowProc(hMainWnd, message, wParam, lParam));
	}

	return 0;
}

//  FUNCTION: ServiceStop
//
//  PURPOSE: Stops the service
//
//  PARAMETERS:
//    none
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//    If a ServiceStop procedure is going to
//    take longer than 3 seconds to execute,
//    it should spawn a thread to execute the
//    stop code, and return.  Otherwise, the
//    ServiceControlManager will believe that
//    the service has stopped responding.
//    

//---------------------------------------------------------------------------
//  BOOL InitApplication( HANDLE hInstance )
//
//  Description:
//     First time initialization stuff.  This registers information
//     such as window classes.
//
//  Parameters:
//     HANDLE hInstance
//        Handle to this instance of the application.
//
//---------------------------------------------------------------------------

BOOL InitApplication(HINSTANCE hInstance)
{
	WNDCLASS wc;
	OSVERSIONINFO osVer;

/*	if (!IS_NT)		//This application only for NT
	{
		MessageBeep(MB_OK);
		MessageBox(NULL, "本程序只适用于 WINDOWS NT 平台!", "警告", MB_ICONSTOP | MB_OK);
		return FALSE;
	} 
*/
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

//---------------------------------------------------------------------------
//HWND InitInstance( HANDLE hInstance, int nCmdShow )
//
//  Description:
//     Initializes instance specific information.
//
//  Parameters:
//     HANDLE hInstance
//        Handle to instance
//
//     int nCmdShow
//        How do we show the window?
//
//---------------------------------------------------------------------------
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

/***************************************************************************
*	Function Name	: BOOL InitTCPIP(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,22,2000
*	Global			: None
*	Note			: None
****************************************************************************/	
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

//	TcpPort = GetPrivateProfileInt("GENERAL","SockPort",SLJK_TCP_PORT, __CONFIG_FILE__);
	local_sin.sin_port = htons((u_short)(TcpPort + 2));

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

	if (listen(TempSocket, MAX_PENDING_CONNECTS) == SOCKET_ERROR) 
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
	if(!ConnectSvrProc())
	{
		MessageBox(hMainWnd, "连接服务器失败!", "警告", MB_ICONSTOP | MB_OK);
		return FALSE;
	}

/*  if(1/*fromconfigfile*///)
/*  {
        SOCKET TempSocket;
        SOCKADDR_IN remote_sin, local_sin;
        BYTE i;
        
        local_sin.sin_family = AF_INET;
        local_sin.sin_port = 0;
        local_sin.sin_addr.s_addr = LocalIPAddr;
        
        remote_sin.sin_family = AF_INET;
        remote_sin.sin_port = htons((u_short)SLJK_TCP_PORT);
        
        for (i = 1; i <=1; i++)
        {
            if (SocketData[i].PeerIPAddr != INADDR_NONE)
            {
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
                
                SocketData[i].Socket = TempSocket;
                remote_sin.sin_addr.s_addr = SocketData[i].PeerIPAddr;
                
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
            }
        }
    }
*/
	SetTimer(hMainWnd, REPEAT_SEND_WAIT_PACKET, REPEAT_SEND_PACKET_TIME, NULL);
	if(UseLcu && (Card1 == 1 || Card2 == 1))
		SetTimer(hMainWnd, TIMEOUT_CHECK, TIMEOUT_CHECK_TIME, NULL);

	return TRUE;
}


/***************************************************************************
*	Function Name	: BOOL ConnectSvrProc(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,16,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
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
	remote_sin.sin_port = htons((u_short)TcpPort);//htons((u_short)SLJK_TCP_PORT);
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

/***************************************************************************
*	Function Name	: BOOL ReConnectSvrProc(DWORD SvrIpAddr)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,17,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
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


/***************************************************************************
*	Function Name	: BOOL	InitUdp(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/12,24,2000
*	Global			: None
*	Note			: None
****************************************************************************/	
BOOL	InitUdp(void)
{

	if(!InitUdpData())
	{
		DebugWindow("Initial UDP Data Failed!");
		return FALSE;
	}

	if(!InitUdpSvr())
	{
		DebugWindow("Initial UDP Sender Failed!");
		return FALSE;
	}

#ifdef _DEBUG
	if(!InitUdpClient())
	{
		DebugWindow("Initial UDP Reciever Failed!");
		return FALSE;
	}
#endif
	
	return TRUE;
}

/***************************************************************************
*	Function Name	: BOOL	InitUdpData(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/12,24,2000
*	Global			: None
*	Note			: None
****************************************************************************/	
BOOL InitUdpData(void)
{
	int errNo;
	WSADATA wsd;

	if(WSAStartup(MAKEWORD(1,1),&wsd) != 0)
	{
		errNo = WSAGetLastError();
		MessageBox(hMainWnd, "Udp consult version error!", "Warning!", MB_ICONSTOP | MB_OK);
		return FALSE;
	}

	if ((LOBYTE( wsd.wVersion ) != 1) || (HIBYTE( wsd.wVersion ) != 1))
	{
		MessageBeep(MB_OK);
		MessageBox(hMainWnd, "UDP Socket Version Not Match!", "Warning!", MB_ICONSTOP | MB_OK);
 		WSACleanup();
		return FALSE; 
	}

	ClientSock = INVALID_SOCKET;
	SvrSock = INVALID_SOCKET;

	UdpPort = 5151;
	dwMulticastGroup = inet_addr("234.5.6.7");

	remote.sin_family = AF_INET;
	remote.sin_port = htons(UdpPort);
	remote.sin_addr.s_addr = dwMulticastGroup;


	return TRUE;
}

/***************************************************************************
*	Function Name	: BOOL InitUdpClient(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/12,24,2000
*	Global			: None
*	Note			: None
****************************************************************************/	
BOOL InitUdpClient(void)
{
#ifdef _DEBUG
	int errNo;
#endif
	struct sockaddr_in local;
	struct ip_mreq mcast;
	int optval;
	
	if((ClientSock= socket(AF_INET,SOCK_DGRAM,0)) == INVALID_SOCKET)
	{
#ifdef _DEBUG
		errNo = WSAGetLastError();
#endif
		MessageBox(hMainWnd, "UDP create socket error!", "Warning!", MB_ICONSTOP | MB_OK);
		WSACleanup();
		return FALSE;
	}

	local.sin_family = AF_INET;
	local.sin_port = htons(UdpPort);
	local.sin_addr.s_addr/* = dwInterface*/ = htonl(INADDR_ANY);//inet_addr("210.45.234.140");


	if(bind(ClientSock,(struct sockaddr *)&local,sizeof(struct sockaddr)) == SOCKET_ERROR)
	{
#ifdef _DEBUG
		errNo = WSAGetLastError();
#endif
		MessageBox(hMainWnd, "UDP Bind Socket Error!", "Warning!", MB_ICONSTOP | MB_OK);

		closesocket(ClientSock);
		WSACleanup();
		return FALSE;
	}

	mcast.imr_multiaddr.s_addr = dwMulticastGroup;
	mcast.imr_interface.s_addr/* = dwInterface*/ = htonl(INADDR_ANY);//inet_addr("210.45.234.140");

	if(setsockopt(ClientSock,IPPROTO_IP,IP_ADD_MEMBERSHIP,
			(char *)&mcast,sizeof(mcast)) == SOCKET_ERROR)
	{
#ifdef _DEBUG
		errNo = WSAGetLastError();
#endif
		MessageBox(hMainWnd, "UDP Socket set Option Error!", "Warning!", MB_ICONSTOP | MB_OK);

		closesocket(ClientSock);
		WSACleanup();
		return FALSE;
	}


	optval = 0;//only in intranet
	if(setsockopt(ClientSock,IPPROTO_IP,IP_MULTICAST_TTL,
		(char *)&optval,sizeof(int)) == SOCKET_ERROR)
	{
#ifdef _DEBUG
		errNo = WSAGetLastError();
#endif
		MessageBox(hMainWnd, "UDP Socket set Option Error!", "Warning!", MB_ICONSTOP | MB_OK);

		closesocket(ClientSock);
		WSACleanup();
		return FALSE;
	}

	if (WSAAsyncSelect(ClientSock, hMainWnd, UDP_READ, FD_READ | FD_WRITE) == SOCKET_ERROR) 
	{
#ifdef _DEBUG
		errNo = WSAGetLastError();
#endif

		MessageBeep(MB_OK);
		MessageBox(hMainWnd, "UDP Socket Select Error", "warning", MB_ICONSTOP | MB_OK);
		WSACleanup();
		return FALSE;
	}
 
//in windows NT cannot success
/*	if(bLoopBack)
	{
		optval = 0;

		if((setsockopt(ClientSock,IPPROTO_IP,IP_MULTICAST_LOOP,
			(char *)&optval,sizeof(optval))) == SOCKET_ERROR)
		{
#ifdef _DEBUG
			errNo = WSAGetLastError();
#endif

			closesocket(ClientSock);
			WSACleanup();
			return FALSE;
		}
	}
*/

	return TRUE;

}

/***************************************************************************
*	Function Name	: BOOL InitUdpSvr(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/12,24,2000
*	Global			: None
*	Note			: None
****************************************************************************/	
BOOL InitUdpSvr(void)
{
#ifdef _DEBUG
	int errNo;
#endif

	if((SvrSock= socket(AF_INET,SOCK_DGRAM,0)) == INVALID_SOCKET)
	{
#ifdef _DEBUG
		errNo = WSAGetLastError();
#endif
		MessageBox(hMainWnd, "UDP Create Socket Error", "warning", MB_ICONSTOP | MB_OK);

		WSACleanup();
		return FALSE;
	}


	return TRUE;
}

/***************************************************************************
*	Function Name	: int UdpRecieve(char *buf)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/12,24,2000
*	Global			: None
*	Note			: None
****************************************************************************/	
int UdpRecieve(char *buf)
{
	int ret;
	int from_len = sizeof(struct sockaddr_in);
	unsigned char recvbuf[1024];

	if(buf == NULL)
	{
		DebugWindow("UdpRecieve(),Input Buffer Error!");
		return -1;
	}

	memset(recvbuf,0,1024);

	if((ret = recvfrom(ClientSock,recvbuf,1024,0,
		(struct sockaddr *)&from,&from_len)) == SOCKET_ERROR)
	{
#ifdef _DEBUG
		int no;
		no = WSAGetLastError();
#endif
		MessageBox(hMainWnd, "UDP Socket Recieve Data Error", "warning", MB_ICONSTOP | MB_OK);
		closesocket(ClientSock);
		WSACleanup();
		return -1;
	}

#ifdef _DEBUG
//	DebugWindow(recvbuf);
#endif

	memcpy(buf,recvbuf,ret);
	return ret;
}

/***************************************************************************
*	Function Name	: int UdpSend(char *buf,int len)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/12,24,2000
*	Global			: None
*	Note			: None
****************************************************************************/	
int UdpSend(char *buf,int len)
{
	int ret;
#ifdef _DEBUG
	int errNo;
#endif
	char sendbuf[1024];

	if(buf == NULL)
		return -1;
	if(len <= 0 || len >= 1024)
		return -1;

	memset(sendbuf,0,1024);
	memcpy(sendbuf,buf,len);

	if((ret = sendto(SvrSock,(char *)sendbuf,strlen(sendbuf),0,
		(struct sockaddr *)&remote,sizeof(remote))) == SOCKET_ERROR)
	{
#ifdef _DEBUG
		errNo = WSAGetLastError();
#endif
		MessageBox(hMainWnd, "UDP Socket Send Data Error", "warning", MB_ICONSTOP | MB_OK);
		closesocket(SvrSock);
		WSACleanup();
		return -1;
	}

	return ret;
}



/***************************************************************************
*	Function Name	: BOOL GetLocalIp(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,22,2000
*	Global			: None
*	Note			: None
****************************************************************************/	
BOOL GetLocalIp(void)
{
	BYTE i;

//deleted 08,20,2000
	char HostName[256] = {0};
	struct hostent * pHostent;
	struct	sockaddr_in dest;
//#ifdef TEST
	char * hIP;
//	char HostIp[16];
//#endif

	for (i = 0; i < MAX_SOCKET_NUM; i++)	
		InitSocketData(i);

	if(gethostname(HostName,15*sizeof(char)) == SOCKET_ERROR)
		return FALSE;

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


/***************************************************************************
*	Function Name	: void InitSocketData(BYTE i)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,22,2000
*	Global			: None
*	Note			: None
****************************************************************************/	
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


/***************************************************************************
*	Function Name	: void AcceptConnect(WPARAM wParam, LPARAM lParam)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,22,2000
*	Global			: None
*	Note			: None
****************************************************************************/	
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

//modify 07,16,2001	,for reconnect server
	//CloseSocketConnect(1);
//	SocketData[i].PeerIPAddr = acc_sin.sin_addr.s_addr;
	PostMessage(hMainWnd,SVR_RECONNECT,(WPARAM)(acc_sin.sin_addr.s_addr),0x2222);
	return;
//modify end

	InitSocketData(i);

	SocketData[i].Socket = TempSocket;
	SocketData[i].PeerIPAddr = acc_sin.sin_addr.s_addr;
	SocketData[i].LinkStatus = TRUE;

	return;
}

/***************************************************************************
*	Function Name	: void ConnectPeer(WPARAM wParam, LPARAM lParam)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,22,2000
*	Global			: None
*	Note			: None
****************************************************************************/	
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
	SendCheckin();

	return;
}

/***************************************************************************
*	Function Name	: void CancelBlocking(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,22,2000
*	Global			: None
*	Note			: None
****************************************************************************/	
void CancelBlocking(void)
{
	if (WSAIsBlocking() == TRUE)
		WSACancelBlockingCall();
}

/***************************************************************************
*	Function Name	: BYTE SocketInsertQueue(DWORD nIPAddr)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,22,2000
*	Global			: None
*	Note			: None
****************************************************************************/	
BYTE SocketInsertQueue(DWORD nIPAddr)
{
	BYTE i, nFree = 0;    //total items and index

	if(nIPAddr == INADDR_NONE)
	{
		DebugWindow("SocketInsertQueue(),Ip Address Error!");
		return 0;
	}

//delete 2001,01,09
//if the same client reconnect
/*	for (i = 1; i < MAX_SOCKET_NUM; i++)
	{
		if (SocketData[i].Socket != INVALID_SOCKET && SocketData[i].PeerIPAddr == nIPAddr)
		{	

			CloseSocketConnect(i);
			nFree = i;
			break;
		}
	}
	
	if(nFree > 0 && nFree < MAX_SOCKET_NUM)
		return nFree;
*/
//deleted end

//modify 07,16,2001
	return 1;
//modify end

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


/***************************************************************************
*	Function Name	: void ClearWaitSendQueue(BYTE Index)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,22,2000
*	Global			: None
*	Note			: None
****************************************************************************/	
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

/***************************************************************************
*	Function Name	: void CloseSocketConnect(BYTE Index)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,22,2000
*	Global			: None
*	Note			: None
****************************************************************************/	
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

//added 08,22,2k
/*	OprIndex = Comm2Opr(Index);	
	if(OprIndex == 0 || OprIndex >=MAX_OPR_NUM)
	{
//		DebugWindow("Process Function Global Data Exception!");//delete 11/09/2000
		return;
	}
*/
//	LogoutProc(OprIndex);		
//added ends 

	return;
}

/***************************************************************************
*	Function Name	: void ReceiveData(BYTE Index)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/04,05,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
void ReceiveData(BYTE Index)
{
	BYTE i = Index;
	int j;
	int status, ReceLen;
    char MsgBuff[5 * RECEIVE_BUFFER_LEN];
    CMsgFrame *MsgHeadPtr;
/*#ifdef TEST
	char disp[200];
#endif
*/
    SocketData[i].CheckLinkCount = 0;

    if (SocketData[i].LeftLen != 0)
    {    
        memcpy(MsgBuff, SocketData[i].ReceBuff, SocketData[i].LeftLen);
        memset(SocketData[i].ReceBuff,0, RECEIVE_BUFFER_LEN);
    }     

    ReceLen = recv(SocketData[i].Socket, MsgBuff + SocketData[i].LeftLen, 
					5 * RECEIVE_BUFFER_LEN - SocketData[i].LeftLen, NO_FLAG_SET);
	
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

/*    MsgHeadPtr = (CMsgFrame *)MsgBuff;
*/
/*
#ifdef TEST
	memset(disp,0,200);
	sprintf(disp,"Receved %d bytes",ReceLen);
	DebugWindow(disp);
#endif
*/
	ReceLen += SocketData[i].LeftLen;    
    SocketData[i].LeftLen = 0;

	j = 0;
	while (ReceLen > 0)  
    {    
//added 08,14,2001
		while(MsgBuff[j] != (char)MsgFlag)
		{
/*
		#ifdef TEST
			memset(disp,0,200);
			sprintf(disp,"Search MsgFlag,Start Position %d",j);
			DebugWindow(disp);
		#endif
*/
			ReceLen--;
			j++;
			if(ReceLen <= 0)
			{
/*		#ifdef TEST
				memset(disp,0,200);
				sprintf(disp,"Search MsgFlag,End Position %d",j);
				DebugWindow(disp);
		#endif
*/				return;
			}
		}

		MsgHeadPtr = (CMsgFrame *)(&MsgBuff[j]);
//added end       

		if (SocketData[i].ReceErrorCount > MAX_RECEIVE_ERROR_NUM)
        {
/*
		#ifdef TEST
			memset(disp,0,200);
			sprintf(disp,"Received MAX_RECEIVE_ERROR_NUM");
			DebugWindow(disp);
		#endif
*/
            CloseSocketConnect(i);
            return;
        }
        
        if (ReceLen < sizeof(CMsgFrame))
        {
            SocketData[i].LeftLen = ReceLen;
            memcpy(SocketData[i].ReceBuff, MsgHeadPtr, ReceLen);
/*
		#ifdef TEST
			memset(disp,0,200);
			sprintf(disp,"Left %d bytes < CMsgFrame",SocketData[i].LeftLen);
			DebugWindow(disp);
		#endif
*/
            return;
        }
  
		if(MsgHeadPtr->byFlag != MsgFlag)
		{
			DebugWindow("ReceiveData(),byFlag Error!");
            SocketData[i].LeftLen = 0;
            memset(SocketData[i].ReceBuff,0, RECEIVE_BUFFER_LEN);
            SocketData[i].ReceErrorCount++;
            return;
		}

		if(!IsCheckSumOk((char *)MsgHeadPtr))
		{
			DebugWindow("ReceiveData(),Checksum Error!");
            SocketData[i].LeftLen = 0;
            memset(SocketData[i].ReceBuff,0, RECEIVE_BUFFER_LEN);
            SocketData[i].ReceErrorCount++;
            return;
		}

        if (ReceLen < (int)(sizeof(CMsgFrame) + MsgHeadPtr->iLength)) 
        {    
            SocketData[i].LeftLen = ReceLen;
            memcpy(SocketData[i].ReceBuff, MsgHeadPtr, ReceLen);

/*		#ifdef TEST
			memset(disp,0,200);
			sprintf(disp,"Left %d bytes < CMsgFrame + iLength",SocketData[i].LeftLen);
			DebugWindow(disp);
		#endif
*/
            return;
			//break;
        }
        
        SocketData[i].ReceErrorCount = 0;
        
/*	#ifdef TEST
		memset(disp,0,200);
		sprintf(disp,"Invoking DispatchClientReqProc,Start Position %d,length=%d",j,sizeof(CMsgFrame) + MsgHeadPtr->iLength);
		DebugWindow(disp);
	#endif
*/
        DispatchClientReqProc(Index,(void *)MsgHeadPtr,(WORD)(sizeof(CMsgFrame) + MsgHeadPtr->iLength));
        
        ReceLen = ReceLen - MsgHeadPtr->iLength - sizeof(CMsgFrame);// - j;//modify 09,21,2001
        //MsgHeadPtr = (CMsgFrame *)((char *)MsgHeadPtr + MsgHeadPtr->iLength + sizeof(CMsgFrame)) + j;
		j = j + MsgHeadPtr->iLength + sizeof(CMsgFrame);

/*	#ifdef TEST
		memset(disp,0,200);
		sprintf(disp,"Invoked DispatchClientReqProc,Left length=%d,j=%d",ReceLen,j);
		DebugWindow(disp);
	#endif
*/
    }


	return;
}
//revised 07,19,2001
/*void ReceiveData(BYTE Index)
{
	BYTE i = Index;
	int status, ReceLen;
    char MsgBuff[5 * RECEIVE_BUFFER_LEN];
    MsgHead *MsgHeadPtr;

    SocketData[i].CheckLinkCount = 0;

    if (SocketData[i].LeftLen != 0)
    {    
        memcpy(MsgBuff, SocketData[i].ReceBuff, SocketData[i].LeftLen);
        memset(SocketData[i].ReceBuff,0, RECEIVE_BUFFER_LEN);
    }     

    ReceLen = recv(SocketData[i].Socket, MsgBuff + SocketData[i].LeftLen, 
					5 * RECEIVE_BUFFER_LEN - SocketData[i].LeftLen, NO_FLAG_SET);

	if(ReceLen == SOCKET_ERROR)	
	{
		status = WSAGetLastError();
		if ((status != WSAEWOULDBLOCK) && (status != WSAEINPROGRESS))
		{
			DebugWindow("ReceiveData(),Receive data Error!");

			CloseSocketConnect(i);
	//added 04,05,2001
			//CheckTheOprData(i);
	//added ends
			return;
		}
		else
		{
			memcpy(SocketData[i].ReceBuff, MsgBuff,SocketData[i].LeftLen);
			return;
		}
	}

    MsgHeadPtr = (MsgHead *)MsgBuff;
    if ((MsgHeadPtr->MsgLen > MAX_MSG_LEN) || (MsgHeadPtr->MsgLen < 0))
    {
        CloseSocketConnect(i);
        return ;
    }
    
    ReceLen += SocketData[i].LeftLen;    
    SocketData[i].LeftLen = 0;
    
    while (ReceLen > 0)  
    {    
        if (SocketData[i].ReceErrorCount > MAX_RECEIVE_ERROR_NUM)
        {
            CloseSocketConnect(i);
            return;
        }
        
        if (ReceLen < sizeof(MsgHead))
        {
            SocketData[i].LeftLen = ReceLen;
            memcpy(SocketData[i].ReceBuff, MsgHeadPtr, ReceLen);
            return;
        }
        
        if ((MsgHeadPtr->MsgLen > MAX_MSG_LEN) || (MsgHeadPtr->MsgLen < 0))
        {
            CloseSocketConnect(i);
            return;
        }
        
        if (ReceLen < (int)(sizeof(MsgHead) + MsgHeadPtr->MsgLen)) 
        {    
            SocketData[i].LeftLen = ReceLen;
            memcpy(SocketData[i].ReceBuff, MsgHeadPtr, ReceLen);
            break;
        }
        
        if (MsgHeadPtr->Packet1 != ~(MsgHeadPtr->MsgLen) || MsgHeadPtr->Packet2 != ~(MsgHeadPtr->Event))
        {
            ReceLen = ReceLen - MsgHeadPtr->MsgLen - sizeof(MsgHead);
            MsgHeadPtr = (MsgHead *)((char *)MsgHeadPtr + MsgHeadPtr->MsgLen + sizeof(MsgHead));
            SocketData[i].ReceErrorCount++;
            continue;
        }
        
        SocketData[i].ReceErrorCount = 0;
        
        if (MsgHeadPtr->Type == TCP_CHECK_TYPE)
        {
            ReceLen = ReceLen - MsgHeadPtr->MsgLen - sizeof(MsgHead);
            MsgHeadPtr = (MsgHead *)((char *)MsgHeadPtr + MsgHeadPtr->MsgLen + sizeof(MsgHead));
            continue;    
        }     

        DispatchClientReqProc(Index,(void *)MsgHeadPtr,(WORD)(sizeof(MsgHead) + MsgHeadPtr->MsgLen));
        
        ReceLen = ReceLen - MsgHeadPtr->MsgLen - sizeof(MsgHead);
        MsgHeadPtr = (MsgHead *)((char *)MsgHeadPtr + MsgHeadPtr->MsgLen + sizeof(MsgHead));
    }


	return;
}
*/
/********************************************************************************
*	Function Name	: void DispatchClientReqProc(BYTE Index,void *pMsg,WORD MsgLen)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/06,12,2001
*	Global			: None
*	Note			: None
*********************************************************************************/	
void DispatchClientReqProc(BYTE Index,void *pMsgIn,WORD MsgLen)
{
/*    CMsgFrame *MsgHeadPtr = (CMsgFrame *)pMsgIn;

	if(pMsgIn == NULL)
    {
        DebugWindow("DispatchClientReqProc(),MsgPtr Error!");
        return;
    }

    if (MsgLen > MAX_MSG_LEN || MsgLen < 0)
    {
        DebugWindow("DispatchClientReqProc(),MsgLen Out of Range!");
        return;
    }

	if(MsgHeadPtr->Type > TRANSFER_TYPE)
	{
	    char SendBuff[RECEIVE_BUFFER_LEN];
		SVR_CONFIRM *pConfirm = (SVR_CONFIRM *)SendBuff;
		BYTE TargetIndex;
		BOOL status;
		BYTE RetCode = 10;//can not find the taskid

		memset(SendBuff,0,RECEIVE_BUFFER_LEN);

		TargetIndex = FindCommIndex(MsgHeadPtr->TargetIP,MsgHeadPtr->TargetTaskId);
		if(TargetIndex > 0)
		{
			memcpy(SendBuff,MsgHeadPtr,MsgLen);
			status = SendDataEx(SendBuff,MsgLen,TargetIndex);

			if(status)
				return;

			RetCode = 11;//send data error
		}

		//somewhere error,send error to client
		pConfirm->cmdHeader.MsgLen = sizeof(DATA_SVR_CONFIRM);
		pConfirm->cmdHeader.Type = CONFIRM_TYPE;
		pConfirm->cmdHeader.SelfIP = 0;
		pConfirm->cmdHeader.SelfTaskId = 0;
		pConfirm->cmdHeader.TargetIP = MsgHeadPtr->SelfIP;
		pConfirm->cmdHeader.TargetTaskId = MsgHeadPtr->SelfTaskId;
		pConfirm->cmdHeader.CommandNo = MsgHeadPtr->Event;
		pConfirm->cmdHeader.Packet1 = ~(pConfirm->cmdHeader.MsgLen);
		pConfirm->cmdHeader.Packet2 = ~(pConfirm->cmdHeader.CommandNo);

		pConfirm->SvrConfirm.RetCode = RetCode;//can not find the taskid or send data error
		//pConfirm->SvrConfirm.TurnOver = MsgHeadPtr->

		status = SendDataEx(SendBuff,sizeof(SVR_CONFIRM),Index);

		if(!status)
			DebugWindow("DispatchClientReqProc(),Transfer Data Failed!");

		return;		
	}
	else
*/		InterpretClient((void *)pMsgIn,MsgLen,Index);

	return;
}


/********************************************************************************
*	Function Name	: BYTE FindCommIndex(DWORD TargetIP,BYTE TargetTaskId)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/06,13,2001
*	Global			: None
*	Note			: None
*********************************************************************************/	
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

/***************************************************************************
*	Function Name	: BOOL TcpSend(BYTE Index, WORD MsgLen, char * MsgHeadPtr)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,22,2000
*	Global			: None
*	Note			: None
****************************************************************************/	
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

/***************************************************************************
*	Function Name	: BOOL InWaitSendQueue(BYTE Index, WORD hMsgLen, char * hMsg)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,22,2000
*	Global			: None
*	Note			: None
****************************************************************************/	
BOOL InWaitSendQueue(BYTE Index, WORD hMsgLen, char * hMsg)
{
	BYTE i = Index;

	if (SocketData[i].WaitSendCount + 1 >= MAX_RESEND_NUM)
	{
		DebugWindow("InWaitSendQueue(),exceeding max_resend_num!");
	//the following line is added 07,27,2001,for center control GP
//		CloseSocketConnect(Index);
	//added end

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

/***************************************************************************
*	Function Name	: void TcpResend(BYTE Index)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,22,2000
*	Global			: None
*	Note			: None
****************************************************************************/	
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


/***************************************************************************
*	Function Name	: BOOL CreateSerialInfo(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,22,2000
*	Global			: None
*	Note			: None
****************************************************************************/	
BOOL CreateSerialInfo(void)
{

	NPSERIALINFO   npSerialInfo ;
	gnpSerialInfo = &gSerialInfo;

 // initialize Serial info structure
	npSerialInfo = gnpSerialInfo;

	
	COMDEV( npSerialInfo )        = 0 ;
	CONNECTED( npSerialInfo )     = FALSE ;
	PORT( npSerialInfo )          = GetPrivateProfileInt("GENARAL", "SerPort",1, __CONFIG_FILE__);
	BAUDRATE( npSerialInfo )      = CBR_9600;//CBR_4800 ;
	BYTESIZE( npSerialInfo )      = 8 ;
   //FLOWCTRL( npSerialInfo )      = FC_RTSCTS ;
	FLOWCTRL( npSerialInfo )      = FC_XONXOFF ;
	PARITY( npSerialInfo )        = NOPARITY;////EVENPARITY ;
	STOPBITS( npSerialInfo )      = ONESTOPBIT ;
	XONXOFF( npSerialInfo )       = FALSE ;
	USECNRECEIVE( npSerialInfo )  = TRUE ;
	DISPLAYERRORS( npSerialInfo ) = TRUE ;
	WRITE_OS( npSerialInfo ).Offset = 0 ;
	WRITE_OS( npSerialInfo ).OffsetHigh = 0 ;
	READ_OS( npSerialInfo ).Offset = 0 ;
	READ_OS( npSerialInfo ).OffsetHigh = 0 ;
	TERMWND( npSerialInfo ) = hMainWnd ;

   // create I/O event used for overlapped reads / writes

	READ_OS(npSerialInfo).hEvent = CreateEvent(NULL,    // no security
                                              TRUE,    // explicit reset req
                                              FALSE,   // initial event reset
                                              NULL) ; // no name
	if (READ_OS(npSerialInfo).hEvent == NULL)
	{
		DebugWindow("CreateSerialInfo(),READ_OS( npSerialInfo ).hEvent Error!");
//		LocalFree( npSerialInfo ) ;
		return (FALSE) ;
	}
	WRITE_OS(npSerialInfo).hEvent = CreateEvent(NULL,    // no security
                                               TRUE,    // explicit reset req
                                               FALSE,   // initial event reset
                                               NULL) ; // no name
	if (NULL == WRITE_OS(npSerialInfo).hEvent)
	{
		DebugWindow("CreateSerialInfo(),READ_OS( npSerialInfo ).hEvent Error!");
		CloseHandle(READ_OS( npSerialInfo).hEvent ) ;
//      LocalFree( npSerialInfo ) ;
		return (FALSE) ;
	}

#ifdef _WIN32
	svr_sem = CreateSemaphore(NULL,1,1,NULL);

	if(svr_sem == NULL)
	{
		DebugWindow("Create Semaphore Error!");
		return FALSE;
	}
#endif

	return TRUE;

} // end of CreateSerialInfo()

//---------------------------------------------------------------------------
//  BOOL DestroySerialInfo( HWND hWnd )
//
//  Description:
//     Destroys block associated with Main window handle.
//
//  Parameters:
//     HWND hWnd
//        handle to Main window
//
//  Win-32 Porting Issues:
//     - Needed to clean up event objects created during initialization.
//
//---------------------------------------------------------------------------

BOOL DestroySerialInfo(void)
{
	NPSERIALINFO npSerialInfo ;
	
	
	npSerialInfo = gnpSerialInfo;
	// force connection closed (if not already closed)
	
	if(CONNECTED(npSerialInfo))
		CloseConnection();

	// clean up event objects
	CloseHandle(READ_OS(npSerialInfo).hEvent);
	CloseHandle(WRITE_OS(npSerialInfo).hEvent);
	CloseHandle(POSTEVENT(npSerialInfo));

	return (TRUE);

} // end of DestroySerialInfo()



//---------------------------------------------------------------------------
//  BOOL ProcessSerialCharacter(BYTE bOut )
//
//  Description:
//     This simply writes a character to the port and echos it
//     to the TTY screen if fLocalEcho is set.  Some minor
//     keyboard mapping could be performed here.
//
//  Parameters:
//     BYTE bOut
//        byte from keyboard
//
//---------------------------------------------------------------------------

BOOL ProcessSerialCharacter(BYTE bOut )
{
	NPSERIALINFO  npSerialInfo ;

	npSerialInfo = gnpSerialInfo;

	if (!CONNECTED(npSerialInfo ))
	{
		DebugWindow("ProcessSerialCharacter(),Not Connect!");
		return (FALSE) ;
	}

   // a robust app would take appropriate steps if WriteCommBlock failed
	WriteCommBlock((char *)&bOut, 1 ) ;

	return (TRUE) ;

} // end of ProcessTTYCharacter()

//---------------------------------------------------------------------------
//  BOOL OpenConnection(void)
//
//  Description:
//     Opens communication port specified in the SERIALINFO struct.
//     It also sets the CommState and notifies the window via
//     the fConnected flag in the SERIALINFO struct.
//
//  Parameters:
//
//  Win-32 Porting Issues:
//     - OpenComm() is not supported under Win-32.  Use CreateFile()
//       and setup for OVERLAPPED_IO.
//     - Win-32 has specific communication timeout parameters.
//     - Created the secondary thread for event notification.
//
//---------------------------------------------------------------------------

BOOL OpenConnection(void)
{
	char       szPort[15], szTemp[10] ;
	BOOL       fRetVal ;
	NPSERIALINFO  npSerialInfo ;

	HANDLE        hCommWatchThread ;
	DWORD         dwThreadID ;
	COMMTIMEOUTS  CommTimeOuts ;

	npSerialInfo = gnpSerialInfo;

	// load the COM prefix string and append port number

	strcpy(szTemp,"COM");	
	wsprintf( szPort, "%s%d", (LPSTR) szTemp, PORT( npSerialInfo ) ) ;

	// open COMM device

	if ((COMDEV( npSerialInfo ) =
		CreateFile( szPort, 
					GENERIC_READ | GENERIC_WRITE,
					0,                    // exclusive access
					NULL,                 // no security attrs
					OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL |
					FILE_FLAG_OVERLAPPED, // overlapped I/O
					NULL )) == (HANDLE) -1 )
	{
		DebugWindow("Creating File Error!");
		return ( FALSE ) ;
	}
	else
	{
      // get any early notifications

		SetCommMask( COMDEV( npSerialInfo ), EV_RXCHAR ) ;

      // setup device buffers

		SetupComm( COMDEV(npSerialInfo), 1024, 1024 ) ;

      // purge any information in the buffer

		PurgeComm(COMDEV(npSerialInfo), PURGE_TXABORT | PURGE_RXABORT |
			                              PURGE_TXCLEAR | PURGE_RXCLEAR ) ;

	  //perhaps followed by flushfilebuffers  2000.07.16

      // set up for overlapped I/O

		CommTimeOuts.ReadIntervalTimeout = 10*CBR_4800/BAUDRATE(npSerialInfo);//1000;//0xFFFFFFFF ;
		CommTimeOuts.ReadTotalTimeoutMultiplier = 10*CBR_4800/BAUDRATE(npSerialInfo);//1000;//0 ;
		CommTimeOuts.ReadTotalTimeoutConstant = 1000;//2000;
      // CBR_4800 is approximately 1byte/2ms. For our purposes, allow
      // double the expected time per character for a fudge factor.
		CommTimeOuts.WriteTotalTimeoutMultiplier = 4*CBR_4800/BAUDRATE(npSerialInfo);//4800
		CommTimeOuts.WriteTotalTimeoutConstant = 1000;//0 ;
		SetCommTimeouts(COMDEV(npSerialInfo),&CommTimeOuts) ;
	}

	fRetVal = SetupConnection() ;

	if (fRetVal)
	{
		CONNECTED( npSerialInfo ) = TRUE ;

      // Create a secondary thread
      // to watch for an event.

		if (NULL == (hCommWatchThread =
                      CreateThread( (LPSECURITY_ATTRIBUTES) NULL,
                                    0,
                                    (LPTHREAD_START_ROUTINE) CommWatchProc,
                                    (LPVOID) npSerialInfo,
                                    0, &dwThreadID )))
		{
			DebugWindow("Creating Comm Thread Error!");
			CONNECTED( npSerialInfo ) = FALSE ;
			CloseHandle( COMDEV( npSerialInfo ) ) ;
			fRetVal = FALSE ;
		}
		else
		{
			THREADID( npSerialInfo ) = dwThreadID ;
			HTHREAD( npSerialInfo ) = hCommWatchThread ;

         // assert DTR

			EscapeCommFunction( COMDEV(npSerialInfo), SETDTR) ;

		}
	}
	else
	{
		CONNECTED( npSerialInfo ) = FALSE ;
		CloseHandle( COMDEV( npSerialInfo ) ) ;
	}


	return ( fRetVal ) ;

} // end of OpenConnection()

//---------------------------------------------------------------------------
//  BOOL SetupConnection(void)
//
//  Description:
//     This routines sets up the DCB based on settings in the
//     Serial info structure and performs a SetCommState().
//
//  Parameters:
//
//  Win-32 Porting Issues:
//     - Win-32 requires a slightly different processing of the DCB.
//       Changes were made for configuration of the hardware handshaking
//       lines.
//
//---------------------------------------------------------------------------

BOOL SetupConnection(void)
{
	BOOL       fRetVal;
	BYTE       bSet;
	DCB        dcb;
	NPSERIALINFO  npSerialInfo;

	npSerialInfo = gnpSerialInfo;

	dcb.DCBlength = sizeof(DCB);

	GetCommState(COMDEV(npSerialInfo ),&dcb) ;

	dcb.BaudRate = BAUDRATE(npSerialInfo);
	dcb.ByteSize = BYTESIZE(npSerialInfo);
	dcb.Parity = PARITY(npSerialInfo);
	dcb.StopBits = STOPBITS(npSerialInfo);

   // setup hardware flow control

	bSet = (BYTE) ((FLOWCTRL(npSerialInfo) & FC_DTRDSR) != 0) ;
	dcb.fOutxDsrFlow = bSet ;
	if (bSet)
		dcb.fDtrControl = DTR_CONTROL_HANDSHAKE ;
	else
		dcb.fDtrControl = DTR_CONTROL_ENABLE ;

	bSet = (BYTE) ((FLOWCTRL( npSerialInfo ) & FC_RTSCTS) != 0) ;
	dcb.fOutxCtsFlow = bSet ;
	if (bSet)
		dcb.fRtsControl = RTS_CONTROL_HANDSHAKE ;
	else
		dcb.fRtsControl = RTS_CONTROL_ENABLE ;

   // setup software flow control

	bSet = (BYTE) ((FLOWCTRL( npSerialInfo ) & FC_XONXOFF) != 0) ;

	dcb.fInX = dcb.fOutX = bSet ;
	dcb.XonChar = ASCII_XON ;
	dcb.XoffChar = ASCII_XOFF ;
	dcb.XonLim = 100 ;
	dcb.XoffLim = 100 ;

   // other various settings

	dcb.fBinary = TRUE ;
	dcb.fParity = TRUE ;

	fRetVal = SetCommState( COMDEV( npSerialInfo ), &dcb ) ;

	return ( fRetVal ) ;

} // end of SetupConnection()

//---------------------------------------------------------------------------
//  BOOL CloseConnection()
//
//  Description:
//     Closes the connection to the port.  Resets the connect flag
//     in the SERIALINFO struct.
//
//  Parameters:
//
//  Win-32 Porting Issues:
//     - Needed to stop secondary thread.  SetCommMask() will signal the
//       WaitCommEvent() event and the thread will halt when the
//       CONNECTED() flag is clear.
//     - Use new PurgeComm() API to clear communications driver before
//       closing device.
//
//---------------------------------------------------------------------------

BOOL CloseConnection(void)
{
	NPSERIALINFO  npSerialInfo ;

	npSerialInfo = gnpSerialInfo;
   // set connected flag to FALSE

	CONNECTED( npSerialInfo ) = FALSE ;

   // disable event notification and wait for thread
   // to halt

	SetCommMask( COMDEV( npSerialInfo ), 0 ) ;

   // block until thread has been halted

	while(THREADID(npSerialInfo) != 0);

   // drop DTR

	EscapeCommFunction( COMDEV( npSerialInfo ), CLRDTR ) ;

   // purge any outstanding reads/writes and close device handle

	PurgeComm( COMDEV( npSerialInfo ), PURGE_TXABORT | PURGE_RXABORT |
                                   PURGE_TXCLEAR | PURGE_RXCLEAR ) ;
	CloseHandle( COMDEV( npSerialInfo ) ) ;

   // change the selectable items in the menu

	return ( TRUE ) ;

} // end of CloseConnection()

//---------------------------------------------------------------------------
//  int ReadCommBlock(LPSTR lpszBlock, int nMaxLength )
//
//  Description:
//     Reads a block from the COM port and stuffs it into
//     the provided buffer.
//
//  Parameters:
//     LPSTR lpszBlock
//        block used for storage
//
//     int nMaxLength
//        max length of block to read
//
//  Win-32 Porting Issues:
//     - ReadComm() has been replaced by ReadFile() in Win-32.
//     - Overlapped I/O has been implemented.
//
//---------------------------------------------------------------------------

int ReadCommBlock(LPSTR lpszBlock, int nMaxLength )
{
	BOOL       fReadStat ;
	COMSTAT    ComStat ;
	DWORD      dwErrorFlags;
	DWORD      dwLength;
	DWORD      dwError;
	char       szError[ 10 ] ;
	NPSERIALINFO  npSerialInfo ;
	BOOL		  bSucc = TRUE;
	long Result = -1;

#ifdef _WIN32
	int ret_code;
#endif

	npSerialInfo = gnpSerialInfo;

   // only try to read number of bytes in queue
	ClearCommError( COMDEV( npSerialInfo ), &dwErrorFlags, &ComStat ) ;
	dwLength = min( (DWORD) nMaxLength, ComStat.cbInQue ) ;
	if(dwLength == 0)
	{
		DebugWindow("ReadCommBlock(),Need not Read!");
		return 0;
	}

#ifdef _WIN32
    ret_code = WaitForSingleObject(svr_sem,WAIT_SYN_TIME);
    if (ret_code == WAIT_FAILED || ret_code == WAIT_TIMEOUT)
	{
		DebugWindow("Waiting for SingleObject Failed!");
		return 0;
	}
#else
	if( TOKEN(npSerialInfo) != SM_IDLESTATUS)
	{
		DebugWindow("Token not released when read comm data,discard it!");
		return 0;
   }
   TOKEN(npSerialInfo) = SM_BUSYSTATUS;
#endif

   if(dwLength > 0)
   {
//added 08,22/2k
		Sleep(50);
		// only try to read number of bytes in queue
		ClearCommError( COMDEV( npSerialInfo ), &dwErrorFlags, &ComStat ) ;
		dwLength = min( (DWORD) nMaxLength, ComStat.cbInQue);
//added ends

		fReadStat = ReadFile( COMDEV( npSerialInfo ), lpszBlock,
		                    dwLength, &dwLength, &READ_OS( npSerialInfo ) ) ;
		if (!fReadStat)
		{
			if (GetLastError() == ERROR_IO_PENDING)
			{
				DebugWindow("I/O is pending!");
				// We have to wait for read to complete.
				// This function will timeout according to the
				// CommTimeOuts.ReadTotalTimeoutConstant variable
				// Every time it times out, check for port errors
				while(!GetOverlappedResult( COMDEV( npSerialInfo ),
						&READ_OS( npSerialInfo ), &dwLength, TRUE ))
				{
					dwError = GetLastError();

					if(dwError == ERROR_IO_INCOMPLETE)
						// normal result if not finished
						continue;
					else
					{
						// an error occurred, try to recover
						wsprintf( szError, "<CE-%u>", dwError ) ;
						ClearCommError( COMDEV( npSerialInfo ), &dwErrorFlags, &ComStat ) ;
						if ((dwErrorFlags > 0) && DISPLAYERRORS( npSerialInfo ))
						{
							wsprintf( szError, "<CE-%u>", dwErrorFlags ) ;
						}

     					DebugWindow(szError);
						bSucc = FALSE;
						break;
					}

				}

			}
			else
			{
				// some other error occurred
				dwLength = 0 ;
				ClearCommError( COMDEV( npSerialInfo ), &dwErrorFlags, &ComStat ) ;
				if ((dwErrorFlags > 0) && DISPLAYERRORS( npSerialInfo ))
				{
					wsprintf( szError, "<CE-%u>", dwErrorFlags ) ;
					DebugWindow(szError);
					bSucc = FALSE;
				}
			}
		}
	}
	
	if(bSucc == TRUE)
	{
		unsigned char SvrAck = (unsigned char)0xa2;

		memset((void *)gCommBuff,0x00,sizeof(BYTE)*(BUFFERSIZE+1));
		memcpy((void *)gCommBuff,(void *)lpszBlock,dwLength);

#ifdef TEST
		memcpy(gCommBuffTest[BuffIndex],(void *)lpszBlock,dwLength);
		BufLen[BuffIndex] = dwLength;
		if(BuffIndex < 99)
			BuffIndex++;
		else 
			BuffIndex = 0;
#endif

		if((unsigned char)(*gCommBuff) != (unsigned char)0xa2 
				&& (unsigned char)(*gCommBuff) != (unsigned char)0xaa)
			if(!WriteCommBlock(&SvrAck, sizeof(unsigned char)))
			{
				DebugWindow("ReadCommBlock(),Write Ack to Device error!");
				return 0;
			}

//		if(SendMessageTimeout(hMainWnd, SERIALNOTIFY,PORT(npSerialInfo),(LONG)dwLength, \
//						SMTO_BLOCK + SMTO_ABORTIFHUNG,10000,(LPDWORD)&Result) == FALSE)
//			return 0;
		PostMessage(hMainWnd, SERIALNOTIFY,PORT(npSerialInfo),(LONG)dwLength );
	}

#ifdef _WIN32
    ReleaseSemaphore( svr_sem,1,NULL);
#else
	TOKEN(npSerialInfo) = SM_IDLESTATUS;
#endif

	return (dwLength) ;

} // end of ReadCommBlock()

//---------------------------------------------------------------------------
//  BOOL WriteCommBlock(/* HWND hWnd, */BYTE *pByte )
//
//  Description:
//     Writes a block of data to the COM port specified in the associated
//     info structure.
//
//  Parameters:
//     BYTE *pByte
//        pointer to data to write to port
//
//  Win-32 Porting Issues:
//     - WriteComm() has been replaced by WriteFile() in Win-32.
//     - Overlapped I/O has been implemented.
//
//---------------------------------------------------------------------------

BOOL WriteCommBlock(LPSTR lpByte , DWORD dwBytesToWrite)
{

	BOOL        fWriteStat = 0;
	DWORD       dwBytesWritten ;
	NPSERIALINFO   npSerialInfo ;
	DWORD       dwErrorFlags;
	DWORD	   	dwError;
	DWORD       dwBytesSent=0;
	COMSTAT     ComStat;
	char        szError[128] ;
#ifdef TEST
	int i;
#endif


	npSerialInfo = gnpSerialInfo;

	if(!CONNECTED(npSerialInfo)) //connect?
	{
		DebugWindow("Comm device not open when write comm data");
		return FALSE;
	}

	fWriteStat = WriteFile( COMDEV( npSerialInfo ), lpByte, dwBytesToWrite,
                           &dwBytesWritten, &WRITE_OS( npSerialInfo ) ) ;

   // Note that normally the code will not execute the following
   // because the driver caches write operations. Small I/O requests
   // (up to several thousand bytes) will normally be accepted
   // immediately and WriteFile will return true even though an
   // overlapped operation was specified

	if (!fWriteStat)
	{
#ifdef TEST
		i = GetLastError(); 
#endif
		if(GetLastError() == ERROR_IO_PENDING)
		{
         // We should wait for the completion of the write operation
         // so we know if it worked or not

         // This is only one way to do this. It might be beneficial to
         // place the write operation in a separate thread
         // so that blocking on completion will not negatively
         // affect the responsiveness of the UI

         // If the write takes too long to complete, this
         // function will timeout according to the
         // CommTimeOuts.WriteTotalTimeoutMultiplier variable.
         // This code logs the timeout but does not retry
         // the write.

			while(!GetOverlappedResult( COMDEV( npSerialInfo ),
				&WRITE_OS( npSerialInfo ), &dwBytesWritten, TRUE ))
			{
				dwError = GetLastError();

				if(dwError == ERROR_IO_INCOMPLETE)
				{
				// normal result if not finished
					dwBytesSent += dwBytesWritten;
					continue;
				}
				else
				{
               // an error occurred, try to recover
					wsprintf(szError, "<CE-%u>", dwError);
					ClearCommError( COMDEV( npSerialInfo), &dwErrorFlags, &ComStat);

					if ((dwErrorFlags > 0) && DISPLAYERRORS( npSerialInfo ))
					{
						wsprintf( szError, "<CE-%u>", dwErrorFlags);
						DebugWindow(szError);
					}
					break;
				}
			}

			dwBytesSent += dwBytesWritten;

			if( dwBytesSent != dwBytesToWrite )
			{
				wsprintf(szError,"Probable Write Timeout: Total of %ld bytes sent", dwBytesSent);
//				ClearCommError( COMDEV( npSerialInfo ), &dwErrorFlags, &ComStat ) ;
			
			}
			else
				wsprintf(szError,"%ld bytes written", dwBytesSent);

//     		DebugWindow(szError);

		}
		else
		{
         // some other error occurred
			ClearCommError( COMDEV( npSerialInfo ), &dwErrorFlags, &ComStat ) ;
			if ((dwErrorFlags > 0) && DISPLAYERRORS( npSerialInfo ))
			{
				wsprintf( szError, "<CE-%u>", dwErrorFlags ) ;
				DebugWindow(szError);
			}

			return FALSE;
		}
	}

	return TRUE;

} // end of WriteCommBlock()


//************************************************************************
//  DWORD FAR PASCAL CommWatchProc( LPSTR lpData )
//
//  Description:
//     A secondary thread that will watch for COMM events.
//
//  Parameters:
//     LPSTR lpData
//        32-bit pointer argument
//
//  Win-32 Porting Issues:
//     - Added this thread to watch the communications device and
//       post notifications to the associated window.
//
//************************************************************************

DWORD FAR PASCAL CommWatchProc( LPSTR lpData )
{
	DWORD		dwEvtMask ;
	NPSERIALINFO   npSerialInfo = (NPSERIALINFO) lpData ;
	OVERLAPPED  os ;
	int			nLength ;
	BYTE		abIn[MAXBLOCK + 1];
	COMSTAT		comstat;
	DWORD dwError = 0;
	BOOL bResult = FALSE;

	memset( &os, 0, sizeof(OVERLAPPED) ) ;

   // create I/O event used for overlapped read

	os.hEvent = CreateEvent( NULL,    // no security
		                     TRUE,    // explicit reset req
			                 FALSE,   // initial event reset
				             NULL ) ; // no name
	if (os.hEvent == NULL)
	{
		MessageBox( NULL, "Failed to create event for thread!", "WatchComm Error!",
                  MB_ICONEXCLAMATION | MB_OK ) ;
		return ( FALSE ) ;
	}

	if (!SetCommMask( COMDEV( npSerialInfo ), EV_RXCHAR ))
	{
		DebugWindow("CommWatchProc(),Set CommMask Error!");
		return ( FALSE ) ;
	}

	while ( CONNECTED( npSerialInfo ) )
	{
		dwEvtMask = 0 ;
		
		if(!WaitCommEvent(COMDEV(npSerialInfo), &dwEvtMask, NULL));
//			continue;

//added 08,21,2k
		bResult = ClearCommError(COMDEV(npSerialInfo), &dwError, &comstat);

		if (comstat.cbInQue == 0)
			continue;
//added ends

		if ((dwEvtMask & EV_RXCHAR) == EV_RXCHAR)
		{
			do
			{
				memset((char *)abIn,0x00,sizeof(BYTE)*(MAXBLOCK + 1));
				nLength = ReadCommBlock((LPSTR)abIn, MAXBLOCK);
			}
			while (/* nLength > 0 &&*/ nLength == MAXBLOCK) ;
		}
	}

	// get rid of event handle

	CloseHandle( os.hEvent ) ;

   // clear information in structure (kind of a "we're done flag")

	THREADID( npSerialInfo ) = 0 ;
	HTHREAD( npSerialInfo ) = NULL ;

	return(TRUE) ;

} // end of CommWatchProc()

/***************************************************************************
*	Function Name	: void OnNotify(DWORD dwLength)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,22,2000
*	Global			: None
*	Note			: None
****************************************************************************/	
void OnNotify(DWORD dwLength)
{
	NPSERIALINFO  npSerialInfo ;
	BYTE CommBuffer[BUFFERSIZE+1];
#ifdef _WIN32
	int ret_code;
#endif

	npSerialInfo = gnpSerialInfo;

#ifdef _WIN32
    ret_code = WaitForSingleObject(svr_sem,WAIT_SYN_TIME);
    if (ret_code == WAIT_FAILED || ret_code == WAIT_TIMEOUT)
	{
	   DebugWindow("Waiting for SingleObject Failed!");
	   return;
	}
#else
	if( TOKEN(npSerialInfo) != SM_IDLESTATUS)
	{
		DebugWindow("Token not released when read comm data,discard it!");
		return;
	}
	TOKEN(npSerialInfo) = SM_BUSYSTATUS;
#endif

	memset((void *)CommBuffer,0x00,sizeof(BYTE)*(BUFFERSIZE+1));
	memcpy((void *)CommBuffer,(void *)gCommBuff,sizeof(BYTE)*dwLength/*(BUFFERSIZE+1)*/);
	memset((void *)gCommBuff,0x00,sizeof(BYTE)*(BUFFERSIZE+1));

#ifdef _WIN32
    ReleaseSemaphore( svr_sem,1,NULL);
#else
	TOKEN(npSerialInfo) = SM_IDLESTATUS;
#endif

//	Dev2SvrProc((char *)CommBuffer,dwLength);

	return;
}



/***************************************************************************
*	Function Name	: void   DebugWindow(char * DisplayMessage)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,22,2000
*	Global			: None
*	Note			: None
****************************************************************************/	
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

/***************************************************************************
*	Function Name	: void   OsClearWindow(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,22,2000
*	Global			: None
*	Note			: None
****************************************************************************/	
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

/***************************************************************************
*	Function Name	: void InitApp(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,22,2000
*	Global			: None
*	Note			: None
****************************************************************************/	
void InitApp(void)
{
	/* initialize display window */
    gHDC = GetDC(hMainWnd);
    gY=0;

	return;
}


/***************************************************************************
*	Function Name	: BOOL InitSerialComm(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,22,2000
*	Global			: None
*	Note			: None
****************************************************************************/	
BOOL InitSerialComm(void)
{
	if(!CreateSerialInfo())
	{
		MessageBox( hMainWnd, "Serial Initial failed.", szAppName,MB_ICONEXCLAMATION ) ;
		return FALSE;
	}

	if (!OpenConnection())
	{
		MessageBox( hMainWnd, "Serial Connection failed.", szAppName,MB_ICONEXCLAMATION ) ;
		return FALSE;
	}

	return TRUE;
}


/***************************************************************************
*	Function Name	: void __debugWindows(const char* fmt,...)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,22,2000
*	Global			: None
*	Note			: None
****************************************************************************/	
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

/***************************************************************************
*	Function Name	: static void CommOutputDebugString(const char *debuginfo)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,22,2000
*	Global			: None
*	Note			: None
****************************************************************************/	
#define _MAX_SIZE_LIMITED_  1000000*30
static long position = 0;
#ifdef TEST
static void CommOutputDebugString(const char *debuginfo)
{
    char szDebug[400];
    time_t timeval;

    memset(szDebug,0,sizeof(szDebug));

   //modify 10,29,2001
#ifdef _MAX_SIZE_LIMITED_
    if (position >_MAX_SIZE_LIMITED_)
    {
       if (fp != NULL) 
       { 
           fseek(fp,0,SEEK_SET);
       }
    }
#endif
	//end

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

//#define _MAX_SIZE_LIMITED_  10000*300
//static long position = 0;
 
/***************************************************************************
*	Function Name	: void   __DebugWindowUINT32(const char * DisplayMessage)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,22,2000
*	Global			: None
*	Note			: None
****************************************************************************/	
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

/***************************************************************************
*	Function Name	: BOOL SendData(char * pBuff,WORD MsgLen,SOCKET s)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,22,2000
*	Global			: None
*	Note			: None
****************************************************************************/	
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


/**************************************************************************************
*	Function Name	: BOOL SendDataEx(char * pBuff,WORD MsgLen,unsigned char SockIndex)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/03,16,2001
*	Global			: None
*	Note			: None
***************************************************************************************/	
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


/***************************************************************************
*	Function Name	: BOOL MultiSerialInit(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,11,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
BOOL MultiSerialInit(void)
{
	int i;
	BOOL Flag = TRUE;

/*	SerialCommNum = GetPrivateProfileInt("GENERAL","SerCommNum",0, __CONFIG_FILE__);

	if(SerialCommNum == 0 || SerialCommNum >= MAX_CROSS_NUM)
	{
		DebugWindow("MultiSerialInit(),SerialCommNum Out Of Range!");
		SerialCommNum = 16;
//		return FALSE;
	}
*/
	for(i = SerialStart;i < SerialStart + SerialCommNum;i++)
	{
		if(!OpenPort(i))
		{
			char disp[200];

			memset(disp,0,200);
			sprintf(disp,"MultiSerialInit(),Port %d Not Opened!",i);
			DebugWindow(disp);
			Flag = FALSE;
		}
		else
			Sleep(20);
	}

/*	OpenPort(8);
	OpenPort(5);
	OpenPort(6);
	OpenPort(10);
	Sleep(100);
*/
	return Flag;
}


/***************************************************************************
*	Function Name	: BOOL OpenPort(int port)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,11,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
BOOL OpenPort(int port)
{
    int ret;
	DWORD dwThreadID;
	HANDLE hDBThread;

	COMMPARAM CommParam;

	if(port <= 0 || port > MAX_CROSS_NUM)
	{
		DebugWindow("OpenPort(),Port No out of Range!");
		return FALSE;
	}

	ret = sio_open(port);
	if(ret != SIO_OK)
	{
        DebugWindow("OpenPort(),sio_open error!");
        return FALSE;
    }

	CommParam.Port = port;
	CommParam.BaudRate = B9600;
    CommParam.Parity = P_NONE;
	CommParam.ByteSize = BIT_8;
	CommParam.StopBits = STOP_1;

	CommParam.Hw = FALSE;
	CommParam.Sw = FALSE;
	CommParam.Dtr = FALSE;
	CommParam.Rts = FALSE;

	ret = PortSet(port,&CommParam);
	if(!ret)
	{
        sio_close(port);
        return FALSE;
    }

	ret = sio_flush (port,2);
	if (ret != SIO_OK)
	{
        sio_close(port);
        return FALSE;
	}

	ret = InitIrq(port);
	if(!ret)
	{
        ClearIrq(port);
        sio_close(port);
        return FALSE;
    }

	if (NULL == (hDBThread =
                  CreateThread((LPSECURITY_ATTRIBUTES) NULL,
                                0,
                                (LPTHREAD_START_ROUTINE)SerialProc,
                                (LPVOID)port,
                                0, &dwThreadID)))
	{
		DebugWindow("Creating Serial Working Thread Failed!");
		hDBThread = NULL;
		dwThreadID = 0;
		ThreadCtr[port - 1].bUse = 0;

        ClearIrq(port);
        sio_close(port);

        return FALSE;
	}
	else
	{
		ThreadCtr[port - 1].hThread = hDBThread;
		ThreadCtr[port - 1].dwThreadID = dwThreadID;
		ThreadCtr[port - 1].bUse = 1;
	}

	memset((void *)(&SerialBuff[port - 1]),0,sizeof(SerialBuff_t));

	ClearOpenLeftTimes(port);
	IncOpenportTimes(port);

    return TRUE;
}

/***************************************************************************
*	Function Name	: BOOL OpenPortEx(int port)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/04,08,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
BOOL OpenPortEx(int port)
{
    int ret;
	COMMPARAM CommParam;

	if(port <= 0 || port > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("OpenPortEx(),Port No out of Range!");
#endif
		return FALSE;
	}

	ret = sio_open(port);
	if(ret != SIO_OK)
	{
#ifdef TEST
        DebugWindow("OpenPortEx(),sio_open error!");
#endif
        return FALSE;
    }

	CommParam.Port = port;
	CommParam.BaudRate = B9600;
    CommParam.Parity = P_NONE;
	CommParam.ByteSize = BIT_8;
	CommParam.StopBits = STOP_1;

	CommParam.Hw = FALSE;
	CommParam.Sw = FALSE;
	CommParam.Dtr = FALSE;
	CommParam.Rts = FALSE;

	ret = PortSet(port,&CommParam);
	if(!ret)
	{
        sio_close(port);
        return FALSE;
    }

	ret = sio_flush (port,2);
	if (ret != SIO_OK)
	{
        sio_close(port);
        return FALSE;
	}

	ret = InitIrq(port);
	if(!ret)
	{
        ClearIrq(port);
        sio_close(port);
        return FALSE;
    }

	memset((void *)(&SerialBuff[port - 1]),0,sizeof(SerialBuff_t));

	ClearOpenLeftTimes(port);
	IncOpenportTimes(port);

	if(ThreadCtr[port - 1].bUse != 1)
		ThreadCtr[port - 1].bUse = 1;

    return TRUE;
}


/***************************************************************************
*	Function Name	: DWORD WINAPI SerialProc(LPSTR lpData)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,11,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
DWORD WINAPI SerialProc(LPSTR lpData)
{
	MSG msg;
	int port;
	UINT16 MsgLen;
	char *pMsgData;
	char MsgBody[MAX_MSG_LEN];
	BOOL ret = 0;
	int len;
	int resendnum;

	port = (int)lpData;
	while(1)
	{
		ret = GetMessage(&msg,NULL,0,0);
		if(ret == 0)
		{
			//quit??
			return TRUE;
		}

		switch(msg.message)
		{
			case SERIAL_READ:
			{
		#ifdef TEST
				DebugWindow("SerialProc(),May Use Later!");  		
		#endif
			}
			break;

			case SERIAL_WRITE:
			{
				MsgLen = (UINT16)(msg.lParam);

				if(MsgLen == 0 || MsgLen >= MAX_MSG_LEN)
				{
			#ifdef TEST
					DebugWindow("SerialProc(),msg.lParam Error When Recieve SERIAL_WRITE!");
			#endif
					continue;
				}

				pMsgData = (char *)(msg.wParam);
				if(pMsgData == NULL)
				{
			#ifdef TEST
					DebugWindow("SerialProc(),msg.wParam Error When Recieve SERIAL_WRITE!");  		
			#endif
					continue;
				}

				if(ThreadCtr[port - 1].bUse == 0)
				{
			#ifdef TEST
					DebugWindow("SerialProc(),Target Thread not Use!");
			#endif
					HeapFree(MyProcessHeapHandle,0,pMsgData);
					continue;
				}

				memset(MsgBody,0,MAX_MSG_LEN);
				memcpy(MsgBody,pMsgData,MsgLen);
				HeapFree(MyProcessHeapHandle,0,pMsgData);
				len = sio_write(port,MsgBody,MsgLen);

				if(len >= 0 && len != MsgLen)
				{	
					resendnum = 0;
					while(resendnum <= 3)
					{
						resendnum++;
						Sleep(10);
						len = sio_write(port,MsgBody,MsgLen);

						if(len == MsgLen)
							break;
					}

					if(resendnum > 3)
					{
						char disp[200];
						memset(disp,0,200);
						sprintf(disp,"SerialProc(),Port=%d,len!=MsgLen(len=%d)",port,len);
						DebugWindow(disp);
					}
				}
/*				else
				{
					char disp[200];

					memset(disp,0,200);
					if(len == SIO_BADPORT)
					{
						sprintf(disp,"SerialProc(),Port=%d,Port is not opened!",port);
					}
					else if(len == SIO_BADPARM)
					{
						sprintf(disp,"SerialProc(),Port=%d,Bad parameter!",port);
					}
					else if(len == SIO_ABORTWRITE)
					{
						sprintf(disp,"SerialProc(),Port=%d,User abort blocked write!",port);
					}
					else if(len == SIO_WRITETIMEOUT)
					{
						sprintf(disp,"SerialProc(),Port=%d,Write timeout has happened!",port);
					}
					else
					{
						DWORD ret;
						
				 		ret = GetLastError();
						sprintf(disp,"SerialProc(),Port=%d,ErrorCode=%d",port,ret);
					}
					
					if(strlen(disp) > 0)
					{
				#ifdef TEST
						DebugWindow(disp);
				#endif
					}
				}
*/				
				//added 08,08,2001
			//following lines deleted at 08,10,2001,we'll handle it at timeout
/*				if(len != MsgLen)
				{
					if(IsSanlian((UINT8)port))
					{
						DeleteDownBufHead((UINT8)port);
						SetBuf2Ready((UINT8)port);
					}
				}
*/				//end
		
			}
			break;

			default:
#ifdef TEST
				DebugWindow("SerialProc(),Error Msg in msg.message!");  		
#endif
			break;

		}//endswitch
	}//endwhile

	return TRUE;
}

/***************************************************************************
*	Function Name	: BOOL Notify(int TaskNo,UINT16 Event, UINT8 *In, UINT16 InLen)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,11,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
BOOL Notify(int TaskNo,UINT16 Event, UINT8 *In, UINT16 InLen)
{
	BOOL status;
	char *MsgPtr = NULL;
	LPARAM combine = InLen;
	BYTE i;

	if(TaskNo <= 0 || TaskNo >= MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("Notify(),TaskNo Out of Range!");
#endif
		return FALSE;
	}

	if(ThreadCtr[TaskNo - 1].dwThreadID == 0)
	{
#ifdef TEST
		DebugWindow("Notify(),Target ThreadId Error!");
#endif
		return FALSE;
	}

	if(ThreadCtr[TaskNo - 1].bUse == 0)
	{
#ifdef TEST
		DebugWindow("Notify(),Target Thread not Use!");
#endif
		return FALSE;
	}

	if(InLen >= MAX_MSG_LEN)
	{
#ifdef TEST
		DebugWindow("Notify(),Message Length Too Long!");
#endif
		return FALSE;
	}

	if(InLen > 0)
	{
		if(In == NULL)
		{
#ifdef TEST
			DebugWindow("Notify(),Message Body Is Empty!");
#endif
			return FALSE;
		}

		MsgPtr = (char *)HeapAlloc(MyProcessHeapHandle, HEAP_ZERO_MEMORY, InLen);
		if (MsgPtr == NULL)
		{
#ifdef TEST
			DebugWindow("Notify(),Cannot Alloc Memory!");
#endif
			return FALSE;
		}

		memcpy(MsgPtr,In,InLen);
	}

	status = PostThreadMessage(ThreadCtr[TaskNo - 1].dwThreadID,Event,(WPARAM)MsgPtr,InLen);
	if(!status)
	{
		for(i = 0;i <MAX_POST_NUM;i++)
		{
			Sleep(10);
			status = PostThreadMessage(ThreadCtr[TaskNo - 1].dwThreadID,Event,(WPARAM)MsgPtr,InLen);
			if(status)
				break;
		}

		if(!status)
		{
#ifdef TEST
			DebugWindow("Notify(),Post Message Failure!!!!");
#endif
			if(InLen > 0)
				HeapFree(MyProcessHeapHandle, 0, MsgPtr);
			return FALSE;
		}
	}

	if(IsSanlian((UINT8)TaskNo))
	{	
#ifdef TEST
		char disp[350];
#endif
//		int i;
#ifdef TEST
		memset(disp,0,sizeof(disp));

		sprintf(disp,"Send Data to Com%d!",TaskNo);
		DebugWindow(disp);
#endif

/*		i = 0;
		memset(disp,0,sizeof(disp));
		while(i < InLen && i < 100)
		{
#ifdef TEST
			sprintf(disp + i * 3," %02x",(UINT8)(*(MsgPtr + i)) );
#endif
			i++;
		}

#ifdef TEST
		DebugWindow(disp);
#endif
*/
	}

	return TRUE;
}


/***************************************************************************
*	Function Name	: void CheckSvrLink(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/11,19,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
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
	ReConnectSvrProc(SvrIPAddr);

	return;
}


/***************************************************************************
*	Function Name	: BOOL PortSet(int port,COMMPARAM *pCommParam)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,11,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
BOOL PortSet(int port,COMMPARAM *pCommParam)
{
    int  mode = pCommParam->Parity | pCommParam->ByteSize | pCommParam->StopBits;
    int  hw = pCommParam->Hw ? 3 : 0;      /* bit0 and bit1 */
    int  sw = pCommParam->Sw ? 12 : 0;     /* bit2 and bit3 */
    int  ret ;
    DWORD tout;

	if(port <= 0 || port > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("PortSet(),PORT No out of Range!");
#endif
		return FALSE;
	}

	if(pCommParam == NULL)
	{
#ifdef TEST
		DebugWindow("PortSet(),CommParam is NULL!");
#endif
		return FALSE;
	}

	ret = sio_ioctl(port,pCommParam->BaudRate,mode);
    if(ret != SIO_OK)
	{
#ifdef TEST
        DebugWindow("PortSet(),sio_ioctl!");
#endif
        return FALSE;
    }

    ret = sio_flowctrl(port,hw|sw);
	if(ret != SIO_OK)
	{
#ifdef TEST
        DebugWindow("PortSet(),sio_flowctrl!");
#endif
        return FALSE;
    }

    ret = sio_DTR(port,(pCommParam->Dtr ? 1 : 0));
	if(ret != SIO_OK)
	{
#ifdef TEST
        DebugWindow("PortSet(),sio_DTR!");
#endif
        return FALSE;
    }

	if(!(pCommParam->Hw))	
	{
        ret = sio_RTS(port,(pCommParam->Rts ? 1 : 0));
		if(ret != SIO_OK)
		{
#ifdef TEST
			DebugWindow("PortSet(),sio_RTS!");
#endif
            return FALSE;
        }
    }

    tout = 1000 / sio_getbaud(port);  /* ms/byte */
    if (tout < 1)
        tout = 1;

	if(IsSanlian((UINT8)port))
		tout = tout * 100 * 10;             /* 100 byte; '*10' is for delay */
	else
		tout = tout * 200 * 10;              /* 200 byte; '*10' is for delay */

    sio_SetWriteTimeouts(port, tout);

    return TRUE;
}

/***************************************************************************
*	Function Name	: BOOL ClosePort(int port)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,11,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
BOOL ClosePort(int port)
{
	if(port <= 0 || port > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("ClosePort(),input port out of range!");
#endif
		return FALSE;
	}

    ClearIrq(port);
    sio_close(port);

	if(ThreadCtr[port - 1].bUse != 0)
		ThreadCtr[port - 1].bUse = 0;

    return TRUE;
}

/***************************************************************************
*	Function Name	: VOID CALLBACK CntIrq(int port)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,11,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
VOID CALLBACK CntIrq(int port)
{
#ifdef TEST
	char disp[200];
#endif

//#ifdef TEST
//	invokecount++;
//#endif

	if(SerialBuff[port - 1].HighCount > 10)//max times
	{
		SerialBuff[port - 1].HighCount = 0;
		SerialBuff[port - 1].PostFlag = 0;
	}

	if(SerialBuff[port - 1].PostFlag == 1)
	{
		SerialBuff[port - 1].HighCount++;
		return;
	}
	else if(SerialBuff[port - 1].PostFlag == 0)
	{
		SerialBuff[port - 1].PostFlag = 1;
		SerialBuff[port - 1].HighCount = 0;
		PostMessage(hMainWnd,MULTICARD_READ,port,0x9595);
	}
#ifdef TEST
	else
	{
		memset(disp,0,200);
		sprintf(disp,"Port = %d,Invoking CntIrq(),But We Not Post Msg to Main!",port);
//		DebugWindow(disp);
	}
#endif

	return;
}

/***************************************************************************
*	Function Name	: BOOL InitIrq(int port)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,11,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
BOOL InitIrq(int port)
{
    int Ret;

	if(port <= 0 || port > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("InitIrq(),port No out of Range!");
#endif
		return FALSE;
	}

	Ret = sio_cnt_irq(port,CntIrq,1);
    if(Ret != SIO_OK)
	{
#ifdef TEST
        DebugWindow("InitIrq(),invoking sio_cnt_irq Failed!");
#endif
        return FALSE;
    }

    return TRUE;
}

/***************************************************************************
*	Function Name	: void ClearIrq(int port)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,11,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
void ClearIrq(int port)
{
    int ret;

	if(port <= 0 || port > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("ClearIrq(),port No out of Range!");
#endif
		return;
	}

    ret = sio_cnt_irq(port,NULL,0);
    if(ret != SIO_OK)
	{
#ifdef TEST
		DebugWindow("ClearIrq(),invoking sio_cnt_irq Failed!");
#endif
	}

	return;
}


/***************************************************************************
*	Function Name	: int ReadMulSerialData(int port)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,11,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
int ReadMulSerialData(int port)
{
//	int ret;
	SerialBits *pSerialBit; 
	int len;
	char buf[1024];
//#ifdef TEST
//	char disp[200];
//#endif

	int i;

/*	if(SerialBuff[port - 1].PostFlag == 1)
	{
		deccount++;
		InterlockedDecrement(&SerialBuff[port - 1].PostFlag);
	}
*/
	InterlockedCompareExchange((PVOID *)(&SerialBuff[port - 1].PostFlag),(PVOID)0,(PVOID)1);

	memset(buf,0,1024);
	if(SerialBuff[port - 1].LeftLen > 0)
		memcpy(buf,SerialBuff[port - 1].CommBuff,SerialBuff[port - 1].LeftLen);

	len = sio_read(port,buf + SerialBuff[port - 1].LeftLen,1024 - SerialBuff[port - 1].LeftLen);
/*
	ret = sio_data_status(port);
	if (ret < 0)
	{
#ifdef TEST
		DebugWindow("ReadMulSerialData(),Failed!");
#endif
		return 0;
	}
	else
	{
		if (ret & 0x01)
		{
#ifdef TEST
			DebugWindow("ReadMulSerialData(),parity error!");
#endif
			return 0;
		}
		else if (ret & 0x02)
		{
			DebugWindow("ReadMulSerialData(),framing error!");
			return 0;
		}
	}
*/
	if(len <= 0)
	{
#ifdef FORDEBUG
		char errmsg[200];

		memset(errmsg,0,200);
#endif
		if(len == 0)
		{
#ifdef FORDEBUG
			sprintf(errmsg,"ReadMulSerialData(),Port=%d,No Data!",port);
//			DebugWindow(errmsg);
#endif
			return 0;
		}
		else if(len == SIO_BADPORT)
		{
#ifdef FORDEBUG
			sprintf(errmsg,"ReadMulSerialData(),Port=%d,SIO_BADPORT!",port);
#endif
		}
		else if(len == SIO_BADPARM)
		{
#ifdef FORDEBUG
			sprintf(errmsg,"ReadMulSerialData(),Port=%d,SIO_BADPARM!",port);
#endif	
		}
		else
		{
#ifdef FORDEBUG
			int ret;

			ret = GetLastError();
			sprintf(errmsg,"ReadMulSerialData(),Port=%d,ErrorCode=%d!",port,ret);
#endif
		}

#ifdef FORDEBUG
		DebugWindow(errmsg);
#endif
	}
/*#ifdef TEST
	if(len > 6)
	{
		char disp[200];
		
		memset(disp,0,200);
		sprintf(disp,"Received %d bytes serial data!",len);
		DebugWindow(disp);
	}
#endif
*/
    len += SerialBuff[port - 1].LeftLen;    
    SerialBuff[port - 1].LeftLen = 0;
	memset(SerialBuff[port - 1].CommBuff,0,BUFFERSIZE);

	if(IsSanlian((UINT8)port))
	{
		i = 0;

		while(len > 0)
		{
			if(len <  HU_HEAD_LEN)
			{
				SerialBuff[port - 1].LeftLen = len;
				memcpy(SerialBuff[port - 1].CommBuff,&buf[i],len);
				return 0;
			}

			if(buf[i] != (char)0xaa/* || buf[i + 1] != (char)port*/)
			{
#ifdef TEST
				DebugWindow("ReadMulSerialData(),Head != 0xaa!");
#endif
				SerialBuff[port - 1].LeftLen = 0;
				memset(SerialBuff[port - 1].CommBuff,0,BUFFERSIZE);
				return 0;
			}

			if(len < HU_HEAD_LEN + buf[i + 4] + 1)//+checksum
			{
				SerialBuff[port - 1].LeftLen = len;
				memcpy(SerialBuff[port - 1].CommBuff,&buf[i],len);
				return 0;
			}
				
			if(!IsHuChecksumOk(&buf[i],HU_HEAD_LEN + buf[i + 4])) 
			{
		#ifdef TEST
				DebugWindow("ReadMulSerialData(),Checksum Error!");
		#endif
				SerialBuff[port - 1].LeftLen = 0;
				memset(SerialBuff[port - 1].CommBuff,0,BUFFERSIZE);
				return 0;
			}

			SanlianSerialProc(port,&buf[i],HU_HEAD_LEN + buf[i + 4] + 1);

			len -= ( HU_HEAD_LEN + buf[i + 4] + 1 );
			i += ( HU_HEAD_LEN + buf[i + 4] + 1 );
		}

	}
	else if(IsJingSan((UINT8)port))
	{
	//we now parse the serial data for jinsan,in order to process
		int ylfLen,ylfDataLen;
		UINT8 Msg1[MAX_YLF_LEN];

		i = 0;
		while(len >= 3)
		{
			if(buf[i] == 0x41 && buf[i + 1] == 0x41)
			{
				if(len < MIN_YLF_LEN)
					break;

				ylfDataLen = UnPackYlfShort((BYTE)buf[i + 6],(BYTE)buf[i + 7]);
				if(ylfDataLen >= MAX_ENCRIPT_LEN || (ylfDataLen % 2) == 1)
				{
					SerialBuff[port - 1].LeftLen = 0;
					memset(SerialBuff[port - 1].CommBuff,0,BUFFERSIZE);
				#ifdef TEST
					DebugWindow("Receive JS Append Command Error!");
				#endif
					SerialBuff[port - 1].ReceErrorCount++;

					len = 0;

					return 0;
				}

				ylfDataLen += MIN_YLF_LEN;
				if(len < ylfDataLen)
					break;
				
				if(!IsYlfChecksumOk(&buf[i],ylfDataLen - 1)) 
				{
	#ifdef TEST
					DebugWindow("ReadMulSerialData(),YLF Checksum Error!");
	#endif
					SerialBuff[port - 1].LeftLen = 0;
					memset(SerialBuff[port - 1].CommBuff,0,BUFFERSIZE);

					SerialBuff[port - 1].ReceErrorCount++;

					len = 0;

					return 0;
				}

				memset(Msg1,0,MAX_YLF_LEN);
				ylfLen = UnPackYlfForm(Msg1,&buf[i],ylfDataLen - 1);
				if(ylfLen < EFFECT_YLF_HEAD_LEN || ylfLen >= MAX_YLF_LEN)
				{
	#ifdef TEST
					DebugWindow("ReadMulSerialData(),YLF Checksum Error???!");
	#endif

					len -= ylfDataLen;
					i += ylfDataLen;
					continue;
/*
					SerialBuff[port - 1].LeftLen = 0;
					memset(SerialBuff[port - 1].CommBuff,0,BUFFERSIZE);
					return 0;
*/
				}

				if(/* port == Msg1[0] && */ylfLen ==  (Msg1[2] + EFFECT_YLF_HEAD_LEN) )//ylfDataLen
				{
					SerialBuff[port - 1].ReceErrorCount = 0;
			#ifdef TEST
					DebugWindow("Successfully Received bytes for JSSerialProc!");
			#endif
					JSSerialProc(port,Msg1[1],&Msg1[EFFECT_YLF_HEAD_LEN],ylfLen - EFFECT_YLF_HEAD_LEN);
				}
				else
				{
	#ifdef TEST
					DebugWindow("ReadMulSerialData(),Port Or MsgLen Not Match!");
	#endif
				}

				len -= ylfDataLen;
				i += ylfDataLen;

			}
			else
			{
				pSerialBit = (SerialBits *)(&buf[i]);
				if(pSerialBit->BitWise.CharType == A_CHAR)
				{
					pSerialBit = (SerialBits *)(&buf[i + 1]);
					if(pSerialBit->BitWise.CharType == B_CHAR)
					{
						pSerialBit = (SerialBits *)(&buf[i + 2]);
						if( (pSerialBit->BitWise.CharType <= MAX_CHAR_VALUE						\
											&& pSerialBit->BitWise.CharType >= C_CHAR)			\
											|| pSerialBit->BitWise.CharType == 0xff )
						{
					#ifdef TEST
							//DebugWindow("Successfully Received 3 byte from Cross!");
					#endif
							if(pSerialBit->BitWise.CharType == C_CHAR)
								SerialBuff[port - 1].ReceErrorCount = 0;

							SerialDataProc(port,&buf[i]);
							SerialBuff[port - 1].ReceErrorCount = 0;

							len -= 3;
							i += 3;
						}
						else
						{
						#ifdef TEST
							DebugWindow("the third data not correct!");
						#endif
							len -= 2;
							i += 2;
							SerialBuff[port - 1].ReceErrorCount++;
						}
					}
					else
					{
					#ifdef TEST
						DebugWindow("the second data not correct!");
					#endif
						len -= 2;
						i += 2;
						SerialBuff[port - 1].ReceErrorCount++;
					}
				}
				else
				{
				#ifdef TEST
					DebugWindow("the first data not correct!");
				#endif
					len --;
					i++;
					SerialBuff[port - 1].ReceErrorCount++;
				}
			}//endif(buf[i] == 0x41 && buf[i + 1] == 0x41)

			if(SerialBuff[port - 1].ReceErrorCount >= MAX_RECEIVE_ERROR_NUM)
			{
	#ifdef TEST
				DebugWindow("ReadMulSerialData(),Too Many Errors Occurred!");
	#endif

				if(SerialBuff[port - 1].LeftLen != 0)
				{
					SerialBuff[port - 1].LeftLen = 0;
					memset(SerialBuff[port - 1].CommBuff,0,BUFFERSIZE);
				}

				len = 0;

				return 0;
			}

		}//end while,parsed end

		if(len > 0)
		{
			SerialBuff[port - 1].LeftLen = len;
			memcpy(SerialBuff[port - 1].CommBuff,&buf[i],len);
		}
	}
	else
	{
		DebugWindow("ReadMulSerialData(),Unkown Cross Type!");

		if(SerialBuff[port - 1].LeftLen != 0)
		{
			SerialBuff[port - 1].LeftLen = 0;
			memset(SerialBuff[port - 1].CommBuff,0,BUFFERSIZE);
		}

		return 0;
	}

	return 0;
}

/***************************************************************************
*	Function Name	: BOOL SerialPost(int Port,char *MsgPtr,UINT16 MsgLen)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,11,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
BOOL SerialPost(int Port,char *MsgPtr,UINT16 MsgLen)
{
	BOOL Ret;

	if(MsgPtr == NULL)
	{
#ifdef TEST
		DebugWindow("SerialPost(),MsgPtr Is NULL!");
#endif
		return FALSE;
	}

	if(MsgLen == 0 || MsgLen >= MAX_MSG_LEN)
	{
#ifdef TEST
		DebugWindow("SerialPost(),MsgLen Out of Range!");
#endif
		return FALSE;
	}

	Ret = Notify(Port,SERIAL_WRITE,MsgPtr,MsgLen);
	if(!Ret)
	{
#ifdef TEST
		DebugWindow("SerialPost(),Invoking Notify Failed!");
#endif
		return FALSE;
	}

	return TRUE;
}


/**************************************************************
*	Function Name	: BOOL InitFaceDll(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/10,09,2001
*	Global			: None
*	Note			: None
***************************************************************/	
BOOL InitFaceDll(void)
{
#ifdef __TEST
	int ErrNo;
#endif

	if(DllInst == NULL)
	{
		DllInst = LoadLibrary("face.dll");
	}

	if(DllInst)
	{
		GetSysParam = (void (__stdcall *)(long,char *,char *))GetProcAddress(DllInst,"GetSysParam");
		ShowPopUp = (void (__stdcall *)(POINT *))GetProcAddress(DllInst,"ShowPopUp");
		if(GetSysParam && ShowPopUp)
		{
			GetSysParam((long)hMainWnd,(char *)CrossData,(char *)NULL);

			return TRUE;
		}
#ifdef __TEST
		else
			ErrNo = GetLastError();
#endif

		GetSysParam = NULL;
		ShowPopUp = NULL;
	}

	return FALSE;
}


