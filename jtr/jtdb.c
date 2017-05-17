/*****************************************************************
*ModuleName: Jtdb                   FileName:Jtdb.c				 *
*CreateDate: 12/28/2k               Author:  yincy               *
*Version:    1.0                                                 *
*History:                                                        *
* Date       Version       Modifier           Activies           *
*================================================================*
*12/28/2000   1.0           yincy       create                   *
*02/06/2001   1.1           yincy       modify                   *
*06/08/2001   2.0           yincy       modify                   *
*02/19/2002   2.0           yincy       modify                   *
*****************************************************************/
#include <math.h>
#include "wincomm.h"
#include "ctlschemep.h"
#include "jtdb.h"
#ifdef IOCARD1_DRV_DLL
	#include "IOCard1_Drv.h"
#else
	#include "agciolib.h"
#endif

#pragma comment(lib,"ODBC32.lib")
#pragma comment(lib,"winmm.lib")
#ifdef IOCARD1_DRV_DLL
	#pragma comment(lib,"IOCard1_Drv.lib")
#else
	#pragma comment(lib,"agciolib.lib")
#endif

HENV henv = NULL;
HDBC hdbc = NULL;

UINT8 UseLcu = 0;
int Card1 = 1;
int Card2 = 0;
int IntMode = 1;
int IntNumber = 11;
extern char HostIp[16];
char HostId[10] = {"通讯机1"};
char SvrIp[16];
char SvrId[10] = {"总控台"};
extern DWORD SvrIPAddr;
extern int TcpPort;

UINT8 SendInterval = 200;
long LcuNotRead[2] = {0,0};
UINT8 LcuHighCount[2] = {0,0};
int MidDayCount;

CrossData_t CrossData[MAX_CROSS_NUM];
//DownMsgWaitNode LcuSpool[MAX_CROSS_NUM * MAX_RESEND_NUM];
RecBuf SendingBuf;
//RecBuf SendSpool[MAX_SPOOL_NUM];

CrossData_t *gpDevMap;
RecBuf *gpSpool;

extern int SerialStart;
extern int SerialCommNum;
extern HANDLE MyProcessHeapHandle;	//堆句柄
extern HWND hMainWnd;
extern SocketDataNode SocketData[MAX_SOCKET_NUM];
extern SerialCtr ThreadCtr[MAX_CROSS_NUM];

/*********************************************
*	Function Name	: BOOL LptInit(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/01,19,2001
*	Global			: None
*	Note			: None
**********************************************/	
BOOL LptInit(void)
{
#ifdef IOCARD1_DRV_DLL
	BOOL ret;
#else
	int ret;
#endif
	int ShortIn;

#ifdef IOCARD1_DRV_DLL
	ret = Open(0x360,0x367,(UINT8)IntNumber);
	if(!ret)
	{
		DebugWindow("LptInit(),Open Error!");
		return FALSE;
	}

	if(IntMode == 1)
	{
		ret = EnableIntrrupt(IntHandler);
		if(!ret)
		{
			DebugWindow("LptInit(),EnableIntrrupt Error!");
			Close();

			return FALSE;
		}
	}

	ShortIn = (int)ReadPortB(PORT_361);//force id2 is low

#else
	ret = AgcInitIO();
	if(ret == 0)
	{
		DebugWindow("LptInit(),AgcInitIO Error!");
		return FALSE;
	}

	ShortIn = AgcInport(PORT_361);//force id2 is low
#endif

	return TRUE;
}

/***************************************************************************
*	Function Name	: void IntHandler(PVOID pDrv)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/11,15,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
void IntHandler(PVOID pDrv)
{
#ifdef IOCARD1_DRV_DLL
	static DWORD a = 0;
	static int TurnOver = 1;
	DWORD dwIntCount;
	BYTE ShortIn;
	BYTE StatusMask = 0x01;
	BOOL Status;

	if(Card1 > 0 && Card2 > 0)
	{
		if(TurnOver == 1)
		{
			ShortIn = ReadPortB(PORT_362);

			Status = ShortIn & StatusMask;
			if(Status)
			{
				PostMessage(hMainWnd,LPT_READ,(WPARAM)1,(LPARAM)Card1);
			}
			else
			{
				PostMessage(hMainWnd,LPT_READ,(WPARAM)2,(LPARAM)Card2);
			}

			TurnOver = 2;
		}
		else
		{
			ShortIn = ReadPortB(PORT_366);
			Status = ShortIn & StatusMask;
			if(Status)
			{
				PostMessage(hMainWnd,LPT_READ,(WPARAM)2,(LPARAM)Card2);
			}
			else
			{
				PostMessage(hMainWnd,LPT_READ,(WPARAM)1,(LPARAM)Card1);
			}

			TurnOver = 1;
		}
	}
	else if(Card1)
	{
		PostMessage(hMainWnd,LPT_READ,(WPARAM)1,(LPARAM)Card1);
	}
	else if(Card2)
	{
		PostMessage(hMainWnd,LPT_READ,(WPARAM)2,(LPARAM)Card2);
	}
	else
	{
		DebugWindow("Card1 And Card2 Not Equal 1!");
	}
	
	dwIntCount = GetIntCount();	 

	if(dwIntCount != a + 1)
	{
		DebugWindow("IntHandler(),IntCount Error!");
	}

	a = dwIntCount;

#endif
}

/***************************************************************************
*	Function Name	: void GetLcuData(WPARAM CardNo,LPARAM CrossType)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/09,24,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
void GetLcuData(WPARAM CardNo,LPARAM CrossType)
{

	if(CrossType == 1)//sanlian
		GetLptHuData(CardNo);
	else if(CrossType == 2)//jinsan
		GetLptData(CardNo);
	else
	{
#ifdef FORDEBUG
		DebugWindow("GetLcuData(),CrossType Error!");
#endif
		return;
	}

	return;
}

/***************************************************************************
*	Function Name	: void IncOpenportTimes(int Port)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/11,01,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
void IncOpenportTimes(int Port)
{
	if(Port <= 0 || Port >= MAX_CROSS_NUM)
		return;

//	if(UseLcu != 0)
//		return;

	CrossData[Port - 1].OpenTimes++;

	return;
}

/***************************************************************************
*	Function Name	: void IncOpenportTimes(int Port)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/03,06,2002
*	Global			: None
*	Note			: None
****************************************************************************/	
void ClearOpenLeftTimes(int Port)
{
//	SYSTEMTIME time;

	if(Port <= 0 || Port >= MAX_CROSS_NUM)
		return;

//	if(UseLcu != 0)
//		return;

	CrossData[Port - 1].ReopenLeftCount = 1200;	

/*	GetLocalTime(&time);

	CrossData[Port - 1].LastConnectTime.wYear = time.wYear;
	CrossData[Port - 1].LastConnectTime.wMonth = time.wMonth;
	CrossData[Port - 1].LastConnectTime.wDay = time.wDay;
	CrossData[Port - 1].LastConnectTime.wHour = time.wHour;
	CrossData[Port - 1].LastConnectTime.wMinute = time.wMinute;
	CrossData[Port - 1].LastConnectTime.wSecond = time.wSecond;
	CrossData[Port - 1].LastConnectTime.wDayOfWeek = time.wDayOfWeek;
*/

	return;
}

/***************************************************************************
*	Function Name	: void GetLptHuData(WPARAM CardNo)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/09,24,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
void GetLptHuData(WPARAM CardNo)
{
	int StatusMask = 0x00000001;
	int DataMask = 0x000000ff;
	UINT8 MsgData[MAX_LPT_LEN];
	UINT8 MsgHead[2],MsgTail[2],TempMsgLen[2];
	int ShortIn;
	int MsgLen;
#ifndef IOCARD1_DRV_SMAN
	UINT8 Data;
	int i;
#endif
	UINT16 BasePort;
//	UINT8 PortAdjust;
#ifdef TEST
//	char disp[200];
#endif

	if(CardNo == 1)
	{
		BasePort = PORT_360;
//		PortAdjust = 0;
	}
	else if(CardNo == 2)
	{
		BasePort = PORT_364;
//		PortAdjust = NO_PER_CARD;
	}
	else
	{
		DebugWindow("!");
		DebugWindow("!");
		DebugWindow("---GetLptHuData(),CardNo Error!---");
		return;
	}

//	if(LcuNotRead[CardNo - 1] == 1)
//		InterlockedDecrement(&LcuNotRead[CardNo - 1]);
	if(IntMode == 0)
		InterlockedCompareExchange((PVOID *)(&LcuNotRead[CardNo - 1]),(PVOID)0,(PVOID)1);

	memset(MsgData,0,MAX_LPT_LEN);

//	memset(disp,0,200);
	//get msg head
#ifdef IOCARD1_DRV_SMAN
	if(!ReadPortS((WORD)BasePort,(char *)MsgHead,(WORD)2))//PORT_360);
	{
		DebugWindow("GetLptHuData(),Read Message Head Error!");
		ShortIn = (int)ReadPortB((UINT16)(BasePort + 1));//PORT_361);

		return;
	}
#else
	for(i = 0;i < 2;i++)
	{
	#ifdef IOCARD1_DRV_DLL
		ShortIn = (int)ReadPortB(BasePort);//PORT_360);
	#else
		ShortIn = AgcInport(BasePort);//PORT_360);
	#endif
		Data = (UINT8)(ShortIn & DataMask);
		MsgHead[i] = Data;
	
//		sprintf(disp + i*3,"%02X ",(UINT8)Data);
	}
#endif

	if(MsgHead[0] != 0xaa || MsgHead[1] != 0xbb)
	{
#ifdef FORDEBUG
		if(MsgHead[0] != 0x66)
			DebugWindow("GetLptHuData(),Message Head Error!");
#endif

		//force id2 to low Status
#ifdef IOCARD1_DRV_DLL
		ShortIn = (int)ReadPortB((UINT16)(BasePort + 1));//PORT_361);
#else
		ShortIn = AgcInport((UINT16)(BasePort + 1));//PORT_361);
#endif
		return;
	}

	//get msg length
#ifdef IOCARD1_DRV_SMAN
	if(!ReadPortS((WORD)BasePort,(char *)TempMsgLen,(WORD)2))//PORT_360);
	{
		DebugWindow("GetLptHuData(),Read Message Length Error!");
		ShortIn = (int)ReadPortB((UINT16)(BasePort + 1));//PORT_361);

		return;
	}
#else
	for(i = 0;i < 2;i++)
	{
	#ifdef IOCARD1_DRV_DLL
		ShortIn = (int)ReadPortB(BasePort);//PORT_360);
	#else
		ShortIn = AgcInport(BasePort);//PORT_360);
	#endif
		Data = (UINT8)(ShortIn & DataMask);
		TempMsgLen[i] = Data;
//		sprintf(disp + (2+i)*3,"%02X ",(UINT8)Data);
	}
#endif

	MsgLen = TempMsgLen[0] + ( (TempMsgLen[1]) << 8);
	if(MsgLen <= 0 || MsgLen >= MAX_LPT_LEN)
	{
		DebugWindow("GetLptHuData(),Message Length Out Of Range!");
		//Sleep(50);
		//force id2 to low Status
#ifdef IOCARD1_DRV_DLL
		ShortIn = (int)ReadPortB((UINT16)(BasePort + 1));//PORT_361);
#else
		ShortIn = AgcInport((UINT16)(BasePort + 1));//PORT_361);
#endif
		return;
	}
		
	//get msg body
#ifdef IOCARD1_DRV_SMAN
	if(!ReadPortS((WORD)BasePort,(char *)MsgData,(WORD)MsgLen))//PORT_360);
	{
		DebugWindow("GetLptHuData(),Read Message Body Error!");
		ShortIn = (int)ReadPortB((UINT16)(BasePort + 1));//PORT_361);

		return;
	}
#else
	for(i = 0;i < MsgLen;i++)
	{
	#ifdef IOCARD1_DRV_DLL
		ShortIn = (int)ReadPortB(BasePort);//PORT_360);
	#else
		ShortIn = AgcInport(BasePort);//PORT_360);
	#endif
		Data = (UINT8)(ShortIn & DataMask);
		MsgData[i] = Data;
//		sprintf(disp + (4+i)*3,"%02X ",(UINT8)Data);
	}
#endif

	//get msg tail
#ifdef IOCARD1_DRV_SMAN
	if(!ReadPortS((WORD)BasePort,(char *)MsgTail,(WORD)2))//PORT_360);
	{
		DebugWindow("GetLptHuData(),Read Message Tail Error!");
		ShortIn = (int)ReadPortB((UINT16)(BasePort + 1));//PORT_361);

		return;
	}
#else
	for(i = 0;i < 2;i++)
	{
	#ifdef IOCARD1_DRV_DLL
		ShortIn = (int)ReadPortB(BasePort);//PORT_360);
	#else
		ShortIn = AgcInport(BasePort);//PORT_360);
	#endif
		Data = (UINT8)(ShortIn & DataMask);
		MsgTail[i] = Data;
//		sprintf(disp + (4+MsgLen+i)*3,"%02X ",(UINT8)Data);
	}
#endif

//	DebugWindow(disp);
	if(MsgTail[0] != 0x55 || MsgTail[1] != 0x66)
	{
#ifdef FORDEBUG
		DebugWindow("GetLptHuData(),Message Tail Error!");
#endif

		//force id2 to low Status
#ifdef IOCARD1_DRV_DLL
		ShortIn = (int)ReadPortB((UINT16)(BasePort + 1));//PORT_361);
#else
		ShortIn = AgcInport((UINT16)(BasePort + 1));//PORT_361);
#endif
		return;
	}

	if(MsgLen == 2)//zhao ack
	{
		if(MsgData[0] == 0/* && MsgData[1] == 0xfe*/)
		{
#ifdef FORDEBUG
			DebugWindow("GetLptHuData(),Received Zhao Error!");
#endif			
			ZhaoErrEventProc();
		}
		else
		{
			if( (MsgData[0] == SendingBuf.CrossNo) && (MsgData[1] == SendingBuf.CmdNo) )
			{
#ifdef FORDEBUG
				char disp[200];

				memset(disp,0,200);
				sprintf(disp,"Cross=%d,Receive Zhao Ack!",MsgData[0]);
				DebugWindow(disp);
#endif
				ZhaoAckEventProc(MsgData[0]/* + PortAdjust*/,MsgData[1]);
			}
			else
			{
#ifdef FORDEBUG
				DebugWindow("Received 2-Byte Chars Or Data Already Deleted!");
#endif
			}
		}
	}
	else
	{
#ifdef FORDEBUG
		DebugWindow("GetLptHuData(),Received Zhao Data!");
#endif

		if(MsgData[0] > 0 && MsgData[0] <= MAX_CROSS_NUM)
		{
/*			int Port;
		#ifdef TEST
			char  disp2[200];
		#endif

			Port = MsgData[0];
			CrossData[Port - 1].NoSignalCount = 0;
			if(CrossData[Port - 1].Status != CONNECT_STATUS)
			{
		#ifdef TEST
				memset(disp2,0,200);
				sprintf(disp2,"Send Connect To Port %d!",Port);
				DebugWindow(disp2);
		#endif
				SendCommStatus2Wu(Port,0);
				CrossData[Port - 1].Status = CONNECT_STATUS;
			}
*/		
//			CrossData[MsgData[0] - 1].NoSignalCount = 0;
		}

		SanlianLcuProc(MsgData[0]/* + PortAdjust*/,&MsgData[0],MsgLen);		
	}

	//force id2 is low
#ifdef IOCARD1_DRV_DLL
	ShortIn = (int)ReadPortB((UINT16)(BasePort + 1));//PORT_361);
#else
	ShortIn = AgcInport((UINT16)(BasePort + 1));//PORT_361);
#endif

 	return;
}

/***************************************************************************
*	Function Name	: void GetLptData(WPARAM CardNo)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/03,27,2001
*	Modify			: yincy/06,20,2001
*	Modify			: yincy/09,24,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
void GetLptData(WPARAM CardNo)
{
	int StatusMask = 0x00000001;
	int DataMask = 0x000000ff;
	UINT8 MsgData[MAX_LPT_LEN];
	UINT8 MsgHead[2],MsgTail[2],TempMsgLen[2];
	int ShortIn;
	int MsgLen;
	int MsgCount;
#ifndef IOCARD1_DRV_SMAN
	UINT8 Data;
#endif
	UINT16 BasePort;
//	UINT8 PortAdjust;
	int i;

	if(CardNo == 1)
	{
		BasePort = PORT_360;
//		PortAdjust = 0;
	}
	else if(CardNo == 2)
	{
		BasePort = PORT_364;
//		PortAdjust = NO_PER_CARD;
	}
	else
	{
		DebugWindow("!");
		DebugWindow("!");
		DebugWindow("---GetLptData(),CardNo Error!---");
		return;
	}
 
//	if(LcuNotRead[CardNo - 1] == 1)
//		InterlockedDecrement(&LcuNotRead[CardNo - 1]);
	if(IntMode == 0)
		InterlockedCompareExchange((PVOID *)(&LcuNotRead[CardNo - 1]),(PVOID)0,(PVOID)1);

	memset(MsgData,0,MAX_LPT_LEN);

	//get msg head
#ifdef IOCARD1_DRV_SMAN
	if(!ReadPortS((WORD)BasePort,(char *)MsgHead,(WORD)2))//PORT_360);
	{
		DebugWindow("GetLptData(),Read Message Head Error!");
		ShortIn = (int)ReadPortB((UINT16)(BasePort + 1));//PORT_361);

		return;
	}
#else
	for(i = 0;i < 2;i++)
	{
	#ifdef IOCARD1_DRV_DLL
		ShortIn = (int)ReadPortB(BasePort);//PORT_360);
	#else
		ShortIn = AgcInport(BasePort);//PORT_360);
	#endif
		Data = (UINT8)(ShortIn & DataMask);
		MsgHead[i] = Data;
	}
#endif

	if(MsgHead[0] != 0xaa || MsgHead[1] != 0xbb)
	{
#ifdef FORDEBUG
		if(MsgHead[0] != 0x66)
			DebugWindow("GetLptData(),Message Header Error!");
#endif

		//force id2 to low Status
#ifdef IOCARD1_DRV_DLL
		ShortIn = (int)ReadPortB((UINT16)(BasePort + 1));//PORT_361);
#else
		ShortIn = AgcInport((UINT16)(BasePort + 1));//PORT_361);
#endif
		return;
	}

	//get msg length
#ifdef IOCARD1_DRV_SMAN
	if(!ReadPortS((WORD)BasePort,(char *)TempMsgLen,(WORD)2))//PORT_360);
	{
		DebugWindow("GetLptData(),Read Message Length Error!");
		ShortIn = (int)ReadPortB((UINT16)(BasePort + 1));//PORT_361);

		return;
	}
#else
	for(i = 0;i < 2;i++)
	{
	#ifdef IOCARD1_DRV_DLL
		ShortIn = (int)ReadPortB(BasePort);//PORT_360);
	#else
		ShortIn = AgcInport(BasePort);//PORT_360);
	#endif
		Data = (UINT8)(ShortIn & DataMask);
		TempMsgLen[i] = Data;
	}
#endif
	MsgLen = TempMsgLen[0] + ( (TempMsgLen[1]) << 8);
	if(MsgLen <= 0 || MsgLen >= MAX_LPT_LEN || (MsgLen % 4) != 0)
	{
		DebugWindow("GetLptData(),Message Length Out of Range!");

		//force id2 to low Status
#ifdef IOCARD1_DRV_DLL
		ShortIn = (int)ReadPortB((UINT16)(BasePort + 1));//PORT_361);
#else
		ShortIn = AgcInport((UINT16)(BasePort + 1));//PORT_361);
#endif
		return;
	}
		
	//get msg body
#ifdef IOCARD1_DRV_SMAN
	if(!ReadPortS((WORD)BasePort,(char *)MsgData,(WORD)MsgLen))//PORT_360);
	{
		DebugWindow("GetLptData(),Read Message Body Error!");
		ShortIn = (int)ReadPortB((UINT16)(BasePort + 1));//PORT_361);

		return;
	}
#else
	for(i = 0;i < MsgLen;i++)
	{
	#ifdef IOCARD1_DRV_DLL
		ShortIn = (int)ReadPortB(BasePort);//PORT_360);
	#else
		ShortIn = AgcInport(BasePort);//PORT_360);
	#endif
		Data = (UINT8)(ShortIn & DataMask);
		MsgData[i] = Data;
	}
#endif

	//get msg tail
#ifdef IOCARD1_DRV_SMAN
	if(!ReadPortS((WORD)BasePort,(char *)MsgTail,(WORD)2))//PORT_360);
	{
		DebugWindow("GetLptData(),Read Message Tail Error!");
		ShortIn = (int)ReadPortB((UINT16)(BasePort + 1));//PORT_361);

		return;
	}
#else
	for(i = 0;i < 2;i++)
	{
	#ifdef IOCARD1_DRV_DLL
		ShortIn = (int)ReadPortB(BasePort);//PORT_360);
	#else
		ShortIn = AgcInport(BasePort);//PORT_360);
	#endif
		Data = (UINT8)(ShortIn & DataMask);
		MsgTail[i] = Data;
	}
#endif

	if(MsgTail[0] != 0x55 || MsgTail[1] != 0x66)
	{
#ifdef FORDEBUG
		DebugWindow("GetLptData(),Message Tail Error!");
#endif

		//force id2 to low Status
#ifdef IOCARD1_DRV_DLL
		ShortIn = (int)ReadPortB((UINT16)(BasePort + 1));//PORT_361);
#else
		ShortIn = AgcInport((UINT16)(BasePort + 1));//PORT_361);
#endif
		return;
	}

	MsgCount = MsgLen / 4;
	//DebugWindow("Disp!");

	i = 0;
	while(MsgLen > 0)
	{
		if(MsgData[i + 1] == 0xbb || MsgData[i + 1] == 0xcc)
		{
			//DebugWindow("GetLptData(),Maybe Received 0xcc Or 0xbb!");
			//Send data to wu
			i += 4;
			MsgLen -= 4;
			continue;
			//return;
		}

		SerialDataProc(MsgData[i]/* + PortAdjust*/,&MsgData[i + 1]);
		i += 4;
		MsgLen -= 4;
	}
	
	//force id2 is low
#ifdef IOCARD1_DRV_DLL
	ShortIn = (int)ReadPortB((UINT16)(BasePort + 1));//PORT_361);
#else
	ShortIn = AgcInport((UINT16)(BasePort + 1));//PORT_361);
#endif

 	return;
}

/***************************************************************************
*	Function Name	: void TestGetLptData(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,04,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
void TestGetLptData(void)
{
	int DataMask = 0x000000ff;
	UINT8 MsgData[64];
	int ShortIn;
#ifndef IOCARD1_DRV_SMAN
	UINT8 Data;
	int i;
#endif
	char disp[200];

	memset(MsgData,0,64);

//	if(LcuNotRead[1] == 1)
//		InterlockedDecrement(&LcuNotRead[1]);
	if(IntMode == 0)
		InterlockedCompareExchange((PVOID *)(&LcuNotRead[1]),(PVOID)0,(PVOID)1);

	memset(&SendingBuf,0,sizeof(RecBuf));

	//following line modified 11,23,2001 
	ReadPortS(PORT_364,(char *)MsgData,64);

	memset(disp,0,200);
/*	sprintf(disp,"Rece:");
	for(i = 0;i < 26;i++)
	{
#ifdef IOCARD1_DRV_DLL
		ShortIn = (int)ReadPortB(PORT_364);
#else
		ShortIn = AgcInport(PORT_364);
#endif
		Data = (UINT8)(ShortIn & DataMask);
		MsgData[i] = Data;

		sprintf(disp + 5 + i * 3,"%02X ",(UINT8)Data );

	}

	DebugWindow(disp);
*/
/*	if(MsgData[0] == 0x66 || MsgData[1] == 0x66)
	{
//		DebugWindow("this is not a msg,Discard!");
		//force id2 to low Status
#ifdef IOCARD1_DRV_DLL
		ShortIn = (int)ReadPortB(PORT_365);
#else
		ShortIn = AgcInport(PORT_365);
#endif
		return;
	}
*/
//	memset(disp,0,200);
/*	if(MsgData[24] != 0x55 || MsgData[25] != 0x66)
	{
		//force id2 to low Status
#ifdef IOCARD1_DRV_DLL
		ShortIn = (int)ReadPortB(PORT_365);
#else
		ShortIn = AgcInport(PORT_365);
#endif
		return;
	}
*/
	//force id2 is low
#ifdef IOCARD1_DRV_DLL
	ShortIn = (int)ReadPortB(PORT_365);
#else
	ShortIn = AgcInport(PORT_365);
#endif
	//sprintf(disp,"All Rec Times = %d,Err Rec Times = %d",allrecelcu,errrecelcu);
	//DebugWindow(disp);

 	return;
}

/***************************************************************************
*	Function Name	: void TestWrite2lcu(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,04,2001
*	Modify			: yincy/10,10,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
void TestWrite2lcu(void)
{
	char buff[130];
	int i;
	int len;

	if(!UseLcu)
		return;

	memset(buff,0,130);

	buff[0] = (char)0xaa;
	buff[1] = (char)0xbb;
	buff[2] = 32;
	buff[3] = 0;

	len = buff[2] + 6;

	for(i = 4; i < 130;i++)
		buff[i] = 0x29;

/*	buff[4] = 4;
	buff[9] = 2;
	buff[14] = 3;
	buff[19] = 0;
*/
	buff[len - 2] = 0x55;
	buff[len - 1] = 0x66;


	AdjustData4Zhao((UINT8 *)buff,(int)len);

	Write2Lpt(buff,(UINT16)len,2);

	return;
}

/*********************************************************************************
*	Function Name	: void AdjustData4Zhao(UINT8 *buff,int len)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/10,11,2001
*	Global			: None
*	Note			: None
**********************************************************************************/	
void AdjustData4Zhao(UINT8 *buff,int len)
{
	UINT8 Mask = 0x80;

	if(buff == NULL)
		return;

	if(len < 6)//aa bb ll lh 55 66
		return;

	if(*(buff + 2) == 0)
		return;

	*(buff + 2) = ( *(buff + 2) ) | Mask;

	return;
}

/*********************************************************************************
*	Function Name	: BOOL Write2Lpt(UINT8 *Buff,UINT8 Len)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/01,19,2001
*	Modify			: yincy/09,24,2001
*	Global			: None
*	Note			: None
**********************************************************************************/	
BOOL Write2Lpt(UINT8 *Buff,UINT16 Len,UINT8 CardNo)
{
#ifdef IOCARD1_DRV_DLL
	BOOL ret;
#else
	int ret;
#endif
	UINT16 BasePort;
#ifdef __TEST
	char Disp1[300];

	#ifndef IOCARD1_DRV_SMAN
		int i;
		int data;
		char disp[300];
		int len;
	#endif
#endif
	
	if(Buff == NULL)
	{
		DebugWindow("Write2Lpt Input Data Error!");
		return FALSE;
	}

	if(Len == 0 || Len > MAX_LPT_LEN)
	{
		DebugWindow("Write2Lpt Input Data Error!");
		return FALSE;
	}

	if(CardNo == 1)
		BasePort = PORT_360;
	else
		BasePort = PORT_364;

#ifdef __TEST
	memset(Disp1,0,sizeof(Disp1));
//	if(Len != 64)
//		return FALSE;
	memcpy(Disp1,Buff,Len);
	if(strlen(Disp1) >= 250)
		return FALSE;

//	DebugWindow(Disp1);
#endif

#ifdef __TEST
//	memset(disp,0,sizeof(disp));
//	sprintf(disp,"Send %d:",sendindex++);
//	len = strlen(disp);
#endif

#ifdef IOCARD1_DRV_DLL
	ret = WritePortB((UINT16)(BasePort + 1),(int)1);//Clear fifo
	if(ret != TRUE)
	{
		DebugWindow("Write2Lpt() Write Data to 361 Error!");
		return FALSE;
	}
#else
	ret = AgcOutport((UINT16)(BasePort + 1),(int)1);
	if(ret != (int)1)
	{
		DebugWindow("Write2Lpt() Write Data to 361 Error!");
		return FALSE;
	}
#endif

#ifdef IOCARD1_DRV_SMAN
	if(!WritePortS((WORD)BasePort,(char *)Buff,(WORD)Len))
	{
		DebugWindow("Write2Lpt(),Write Message Operation Error!");

		return FALSE;
	}
#else
	memset(disp,0,200);
	sprintf(disp,"Send:");
	len = strlen(disp);
	for(i = 0;i < Len; i++)
/*
	#ifdef IOCARD1_DRV_DLL
		ret = WritePortB(PORT_360,(int)(*(Buff + i)));
	#else
		ret = AgcOutport(PORT_360,(int)(*(Buff + i)));
	#endif
*/
	#ifdef __TEST
	{
	#endif
		data = (int)(*(Buff + i));

	#ifdef FORDEBUG
		sprintf(disp + len + i*3,"%02X ",(UINT8)data );
		sprintf(disp,"%02X ",(UINT8)data );
		DebugWindow(disp);
	#endif

	#ifdef IOCARD1_DRV_DLL
		ret = WritePortB(BasePort,(UINT8)data);
		if(ret != TRUE)
		{
			DebugWindow("Write2Lpt Write Data to 360 Error!");
			DebugWindow(disp);
			return FALSE;
		}
	#else
		ret = AgcOutport(BasePort,data);
		if(ret != data)
		{
			DebugWindow("Write2Lpt Write Data to 360 Error!");
			DebugWindow(disp);
			return FALSE;
		}
	#endif

	#ifdef __TEST
	}
	#endif

	#ifdef __TEST
//	DebugWindow(disp);
	#endif

#endif

#ifdef IOCARD1_DRV_DLL
	ret = WritePortB((UINT16)(BasePort + 2),(int)1);//notify lcu to read
#else
	ret = AgcOutport((UINT16)(BasePort + 2),(int)1);//notify lcu to read
#endif

	//DebugWindow("Send data to Lpt!");

	return TRUE;
}

/*************************************************************
*	Function Name	: void SetRealTimer(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/01,19,2001
*	Global			: None
*	Note			: None
**************************************************************/	
void SetRealTimer(void)
{
	UINT dwUser = 0;

	if(UseLcu && IntMode == 0)
		timeSetEvent(5,1,(LPTIMECALLBACK)RealTimerProc,dwUser,TIME_PERIODIC);
	else
		timeSetEvent(1000,10,(LPTIMECALLBACK)RealTimerProc,dwUser,TIME_PERIODIC);

	return;
}

/****************************************************************************************************
*	Function Name	: void CALLBACK RealTimerProc(UINT uID,UINT uMsg,DWORD dwUser,DWORD dw1,DWORD dw2)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/01,19,2001
*	Modify			: yincy/09,24,2001 
*	Modify			: yincy/10,15,2001 for syn
*	Modify			: yincy/11,17,2001
*	Global			: None
*	Note			: None
*****************************************************************************************************/	
void CALLBACK RealTimerProc(UINT uID,UINT uMsg,DWORD dwUser,DWORD dw1,DWORD dw2)
{
	int ShortIn;
	int StatusMask = 0x00000001;
	BOOL Status;

	if(UseLcu && IntMode == 0)
	{
		if(SendInterval <= 0)
		{
			PostMessage(hMainWnd,SENDCTR_EVENT,(WPARAM)0x1234,(LPARAM)0x2345);
			SendInterval = 200;
		}
		else
			SendInterval--;

		if(Card1 > 0)
		{
			if(LcuHighCount[0] > 200)
			{
				DebugWindow("RealTimerProc(),Auto Set LcuNotRead[0]=0");
				LcuHighCount[0] = 0;
				LcuNotRead[0] = 0;
			}

			if(LcuNotRead[0])
			{
				LcuHighCount[0]++;
				return;
			}
			else
				LcuHighCount[0] = 0;

#ifdef IOCARD1_DRV_DLL
			ShortIn = (int)ReadPortB(PORT_362);
#else
			ShortIn = AgcInport(PORT_362);
#endif
	/*		if(ShortIn == 0x000000ff)//Not insert card,But ivoked init function,further consideration later
				return;
	*/
			Status = ShortIn & StatusMask;
			if(Status)
			{
				PostMessage(hMainWnd,LPT_READ,(WPARAM)1,(LPARAM)Card1);
				LcuNotRead[0] = 1;
			}
		}

		if(Card2 > 0)
		{
			if(LcuHighCount[1] > 200)
			{
				DebugWindow("RealTimerProc(),Auto Set LcuNotRead[1]=0");
				LcuHighCount[1] = 0;
				LcuNotRead[1] = 0;
			}

			if(LcuNotRead[1])
			{
				LcuHighCount[1]++;
				return;
			}
			else
				LcuHighCount[1] = 0;

#ifdef IOCARD1_DRV_DLL
			ShortIn = (int)ReadPortB(PORT_366);
#else
			ShortIn = AgcInport(PORT_366);
#endif
	/*		if(ShortIn == 0x000000ff)//Not insert card,But ivoked init function,further consideration later
				return;
	*/
			Status = ShortIn & StatusMask;
			if(Status)
			{
				PostMessage(hMainWnd,LPT_READ,(WPARAM)2,(LPARAM)Card2);
				LcuNotRead[1] = 1;
			}
		}

		//HuLcuTimeoutProc();
	}
	else
		PostMessage(hMainWnd,SENDCTR_EVENT,(WPARAM)0x1234,(LPARAM)0x2345);


	return;
}

/**********************************************************************
*	Function Name	: void LptExit(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/01,19,2001
*	Global			: None
*	Note			: None
***********************************************************************/	
void LptExit(void)
{
#ifdef IOCARD1_DRV_DLL
	if(UseLcu && IntMode == 1)
		DisableIntrrupt();

	Close();
#else
	AgcExitIO();
#endif

	return;
}

/*******************************************************************************************************
*	Function Name	: UINT8 JSSetStepTableProc(SetStepTable *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/02,19,2002
*	Global			: None
*	Note			: None
********************************************************************************************************/	
UINT8 JSSetStepTableProc(SetStepTable *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex)
{
	BYTE ylfData[MAX_YLF_LEN];
	BYTE ylfEnCyptData[MAX_ENCRIPT_LEN];
	UINT32 EncriptLen;
	int i;
	register int j;

	if(Data == NULL)
	{
#ifdef TEST
		DebugWindow("JSSetStepTableProc(),Data=NULL!");
#endif
		return 50;
	}

	if(CrossNo == 0 || CrossNo > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("JSSetStepTableProc(),bCrossNo=0!");
#endif
		return 30;
	}

	if(!UseLcu)
	{
		memset(ylfData,0,MAX_YLF_LEN);
		ylfData[0] = (UINT8)CrossNo;
		ylfData[1] = (UINT8)CmdNo;
		ylfData[2] = 2 + 32;

		for(i = 0;i < Data->bStepNum;i++)
		{
			if(i >= MaxStepNum)
			{
	#ifdef TEST
				DebugWindow("JSSetStepTableProc(),StepCount Out Of Range!");
	#endif
				return 51;
			}

			ylfData[3] = i;
			ylfData[4] = Data->bStepNum;

			for(j = 0;j < MaxLampNum;j++)
			{
				if(Data->LampColor[i][j] < LT_UNKNOWN)
					ylfData[5 + j] = Data->LampColor[i][j];
				else
					ylfData[5 + j] = 0xff;
			}

			memset(ylfEnCyptData,0,MAX_ENCRIPT_LEN);
			EncriptLen = Pack2YlfFormat(ylfEnCyptData,ylfData,EFFECT_YLF_HEAD_LEN + ylfData[2]);
			if(EncriptLen <= 0 || EncriptLen >= MAX_ENCRIPT_LEN)
			{
	#ifdef TEST
				DebugWindow("JSSetStepTableProc(),EncriptLen Out Of Range!");
	#endif
				continue;
			}
				
			ylfEnCyptData[EncriptLen] = FormYlfChecksum(ylfEnCyptData,EncriptLen);

			if(!HuSerialPost(CrossNo,(UINT8)CmdNo,ylfEnCyptData,(UINT16)(EncriptLen + 1)))
			{
	#ifdef TEST
				DebugWindow("JSSetStepTableProc(),Invoking HuSerialPost Failed!");
	#endif
				return 52;
			}
		}
	}
	else
	{
#ifdef TEST
		DebugWindow("JSSetStepTableProc(),Type Not Implemented!");
#endif
		return 60;
	}

	return 0;
}

/*******************************************************************************************************
*	Function Name	: UINT8 JSGetStepTableProc(SetStepTable *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/02,20,2002
*	Global			: None
*	Note			: None
********************************************************************************************************/	
UINT8 JSGetStepItemProc(Cmd_GetStepTable *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex)
{
	BYTE ylfData[MAX_YLF_LEN];
	BYTE ylfEnCyptData[MAX_ENCRIPT_LEN];
	UINT32 EncriptLen;
	int count;
	int i;

	if(Data == NULL)
	{
#ifdef TEST
		DebugWindow("JSGetStepItemProc(),Data=NULL!");
#endif
		return 50;
	}

	if(CrossNo == 0 || CrossNo > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("JSGetStepItemProc(),bCrossNo=0!");
#endif
		return 30;
	}

	if(!UseLcu)
	{
		memset(ylfData,0,MAX_YLF_LEN);
		ylfData[0] = (UINT8)CrossNo;
		ylfData[1] = (UINT8)CmdNo;
		ylfData[2] = 1;

		if(Data->iReserved1 == 0 && Data->iReserved2 == 0)
		{
			Data->iReserved1 = 1;
			Data->iReserved2 = MaxStepNum;
		}

		count = 0;
		for(i = Data->iReserved1;i < Data->iReserved2 + 1;i++)
		{
			count++;
			if(count > MaxStepNum)
			{
	#ifdef TEST
				DebugWindow("JSGetStepItemProc(),StepCount Out Of Range!");
	#endif
				return 51;
			}
			
			if(i > 0)
				ylfData[3] = i - 1;
			else
				continue;

			memset(ylfEnCyptData,0,MAX_ENCRIPT_LEN);
			EncriptLen = Pack2YlfFormat(ylfEnCyptData,ylfData,EFFECT_YLF_HEAD_LEN + ylfData[2]);
			if(EncriptLen <= 0 || EncriptLen >= MAX_ENCRIPT_LEN)
			{
	#ifdef TEST
				DebugWindow("JSGetStepItemProc(),EncriptLen Out Of Range!");
	#endif
				continue;
			}
				
			ylfEnCyptData[EncriptLen] = FormYlfChecksum(ylfEnCyptData,EncriptLen);

			if(!HuSerialPost(CrossNo,(UINT8)CmdNo,ylfEnCyptData,(UINT16)(EncriptLen + 1)))
			{
	#ifdef TEST
				DebugWindow("JSGetStepItemProc(),Invoking HuSerialPost Failed!");
	#endif
				return 52;
			}
		}
	}
	else
	{
#ifdef TEST
		DebugWindow("JSGetStepItemProc(),Type Not Implemented!");
#endif
		return 60;
	}

	return 0;
}

/*******************************************************************************************************
*	Function Name	: UINT8 JSSetStepTimePlanProc(SetStepTimePlan *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/02,20,2002
*	Global			: None
*	Note			: None
********************************************************************************************************/	
UINT8 JSSetStepTimePlanProc(SetStepTimePlan *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex)
{
	BYTE ylfData[MAX_YLF_LEN];
	BYTE ylfEnCyptData[MAX_ENCRIPT_LEN];
	UINT32 EncriptLen;
	int count;
//	int i;
	register int j;
	BYTE Flag;

	if(Data == NULL)
	{
#ifdef TEST
		DebugWindow("JSSetStepTimePlanProc(),Data=NULL!");
#endif
		return 50;
	}

	if(CrossNo == 0 || CrossNo > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("JSSetStepTimePlanProc(),bCrossNo=0!");
#endif
		return 30;
	}

	if(!UseLcu)
	{
		memset(ylfData,0,MAX_YLF_LEN);
		ylfData[0] = (UINT8)CrossNo;
		ylfData[1] = (UINT8)CmdNo;

	//	for(i = 0;i < MaxStepTimePlanNum;i++)
	//	{
	//		if(Data->bStepNum[i] == 0 || Data->bStepNum[i] > MaxStepNum)
	//			continue;

			if(Data->bStepNum <= MaxStepNum)
				ylfData[2] = 2 + Data->bStepNum/*[i]*/;//data segment len
			else
				ylfData[2] = 2 + MaxStepNum;//data segment len

			if(Data->bStepTimePlanNum > 0)
				ylfData[3] = /*(UINT8)i*/Data->bStepTimePlanNum - 1;
			else
				ylfData[3] = /*(UINT8)i*/Data->bStepTimePlanNum - 1;

			if(Data->bStepNum <= MaxStepNum)
				ylfData[4] = Data->bStepNum/*[i]*/;
			else
				ylfData[4] = MaxStepNum;

			count = 0;
			Flag = TRUE;
			for(j = 0;j < Data->bStepNum/*[i]*/;j++)
			{
				count++;
				if(count > MaxStepNum)
				{
	#ifdef TEST
					DebugWindow("JSSetStepTimePlanProc(),StepNum Out Of Range!");
	#endif
					Flag = FALSE;
					break;
				}

				ylfData[5 + j] = Data->StepLen/*[i]*/[j];

			}

	//		if(!Flag)
	//			continue;

			memset(ylfEnCyptData,0,MAX_ENCRIPT_LEN);
			EncriptLen = Pack2YlfFormat(ylfEnCyptData,ylfData,EFFECT_YLF_HEAD_LEN + ylfData[2]);
			if(EncriptLen <= 0 || EncriptLen >= MAX_ENCRIPT_LEN)
			{
	#ifdef TEST
				DebugWindow("JSSetStepTimePlanProc(),EncriptLen Out Of Range!");
	#endif
				return 51;
				///continue;
			}
				
			ylfEnCyptData[EncriptLen] = FormYlfChecksum(ylfEnCyptData,EncriptLen);

			if(!HuSerialPost(CrossNo,(UINT8)CmdNo,ylfEnCyptData,(UINT16)(EncriptLen + 1)))
			{
	#ifdef TEST
				DebugWindow("JSSetStepTimePlanProc(),Invoking HuSerialPost Failed!");
	#endif
				return 52;
			}
	//	}
	}
	else
	{
#ifdef TEST
		DebugWindow("JSSetStepTimePlanProc(),Type Not Implemented!");
#endif
		return 60;
	}

	return 0;
}

/*******************************************************************************************************
*	Function Name	: UINT8 JSGetStepTimePlanProc(Cmd_GetStepTimePlan *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/02,20,2002
*	Global			: None
*	Note			: None
********************************************************************************************************/	
UINT8 JSGetStepTimePlanProc(Cmd_GetStepTimePlan *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex)
{
	BYTE ylfData[MAX_YLF_LEN];
	BYTE ylfEnCyptData[MAX_ENCRIPT_LEN];
	UINT32 EncriptLen;
	int count;
	int i;

	if(Data == NULL)
	{
#ifdef TEST
		DebugWindow("JSGetStepTimePlanProc(),Data=NULL!");
#endif
		return 50;
	}

	if(CrossNo == 0 || CrossNo > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("JSGetStepTimePlanProc(),bCrossNo=0!");
#endif
		return 30;
	}

	if(!UseLcu)
	{
		memset(ylfData,0,MAX_YLF_LEN);
		ylfData[0] = (UINT8)CrossNo;
		ylfData[1] = (UINT8)CmdNo;
		ylfData[2] = 1;

		//added 04,01,2002
		if(Data->iReserved1 == 0 || Data->iReserved2 == 0)
		{
			Data->iReserved1 = 1;
			Data->iReserved2 = MaxStepTimePlanNum;
		}
		//added end

		count = 0;
		for(i = Data->iReserved1;i < Data->iReserved2 + 1;i++)
		{
			count++;
			if(count >= MaxStepTimePlanNum)
			{
	#ifdef TEST
				DebugWindow("JSGetStepTimePlanProc(),StepTimePlanNum Out Of Range!");
	#endif
				return 51;
			}
			
			if(i > 0)
				ylfData[3] = i - 1;
			else
				continue;

			memset(ylfEnCyptData,0,MAX_ENCRIPT_LEN);
			EncriptLen = Pack2YlfFormat(ylfEnCyptData,ylfData,EFFECT_YLF_HEAD_LEN + ylfData[2]);
			if(EncriptLen <= 0 || EncriptLen >= MAX_ENCRIPT_LEN)
			{
	#ifdef TEST
				DebugWindow("JSGetStepTimePlanProc(),EncriptLen Out Of Range!");
	#endif
				continue;
			}
				
			ylfEnCyptData[EncriptLen] = FormYlfChecksum(ylfEnCyptData,EncriptLen);

			if(!HuSerialPost(CrossNo,(UINT8)CmdNo,ylfEnCyptData,(UINT16)(EncriptLen + 1)))
			{
	#ifdef TEST
				DebugWindow("JSGetStepTimePlanProc(),Invoking HuSerialPost Failed!");
	#endif
				return 52;
			}
		}
	}
	else
	{
#ifdef TEST
		DebugWindow("JSGetStepTimePlanProc(),Type Not Implemented!");
#endif
		return 60;
	}

	return 0;
}

/*******************************************************************************************************
*	Function Name	: UINT8 JSTimePlanReqProc(TimeBandDown *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/02,20,2002
*	Global			: None
*	Note			: None
********************************************************************************************************/	
UINT8 JSTimePlanReqProc(TimeBandDown *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex)
{
	BYTE ylfData[MAX_YLF_LEN];
	BYTE ylfEnCyptData[MAX_ENCRIPT_LEN];
	UINT32 EncriptLen;
	UINT32 Hour,Minute,Second;
	int count;
	int i;

	if(Data == NULL)
	{
#ifdef TEST
		DebugWindow("JSTimePlanReqProc(),Data=NULL!");
#endif
		return 50;
	}

	if(CrossNo == 0 || CrossNo > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("JSTimePlanReqProc(),bCrossNo=0!");
#endif
		return 30;
	}

	if(!UseLcu)
	{
		memset(ylfData,0,MAX_YLF_LEN);
		ylfData[0] = (UINT8)CrossNo;
		ylfData[1] = (UINT8)CmdNo;
		ylfData[2] = 2 + Data->byTimeBandNum * 4;
		ylfData[3] = Data->byTimeBandPlanNo;
		ylfData[4] = Data->byTimeBandNum;

		count = 0;
		for(i = 0;i < Data->byTimeBandNum;i++)
		{
			count++;
			if(count > MaxTimeBandNum)
			{
	#ifdef TEST
				DebugWindow("JSTimePlanReqProc(),TimeBandNum Out Of Range!");
	#endif
				return 51;
			}

			Minute = (UINT32)(Data->StartTime[i]);
			Second = (UINT32)( (Data->StartTime[i] - (float)Minute + 0.001) * 60.0);
			Hour = Minute / 60;
			Minute = Minute % 60;

			ylfData[5 + 4 * i] = (BYTE)Hour;
			ylfData[5 + 4 * i + 1] = (BYTE)Minute;
			ylfData[5 + 4 * i + 2] = (BYTE)Second;
			ylfData[5 + 4 * i + 3] = Data->byPlanNo[i];

		}

		memset(ylfEnCyptData,0,MAX_ENCRIPT_LEN);
		EncriptLen = Pack2YlfFormat(ylfEnCyptData,ylfData,EFFECT_YLF_HEAD_LEN + ylfData[2]);
		if(EncriptLen <= 0 || EncriptLen >= MAX_ENCRIPT_LEN)
		{
	#ifdef TEST
			DebugWindow("JSTimePlanReqProc(),EncriptLen Out Of Range!");
	#endif
			return 53;
		}
			
		ylfEnCyptData[EncriptLen] = FormYlfChecksum(ylfEnCyptData,EncriptLen);

		if(!HuSerialPost(CrossNo,(UINT8)CmdNo,ylfEnCyptData,(UINT16)(EncriptLen + 1)))
		{
	#ifdef TEST
			DebugWindow("JSTimePlanReqProc(),Invoking HuSerialPost Failed!");
	#endif
			return 52;
		}
	}
	else
	{
#ifdef TEST
		DebugWindow("JSTimePlanReqProc(),Type Not Implemented!");
#endif
		return 60;
	}

	return 0;
}

/*******************************************************************************************************
*	Function Name	: UINT8 GetTimeScheduleProc(Cmd_GetTimeBand *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/02,20,2002
*	Global			: None
*	Note			: None
********************************************************************************************************/	
UINT8 GetTimeScheduleProc(Cmd_GetTimeBand *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex)
{
	BYTE ylfData[MAX_YLF_LEN];
	BYTE ylfEnCyptData[MAX_ENCRIPT_LEN];
	UINT32 EncriptLen;
	int i;

	if(Data == NULL)
	{
#ifdef TEST
		DebugWindow("GetTimeScheduleProc(),Data=NULL!");
#endif
		return 50;
	}

	if(CrossNo == 0 || CrossNo > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("GetTimeScheduleProc(),bCrossNo=0!");
#endif
		return 30;
	}

	//added 04,01,2002
	if(Data->iReserved1 == 0)
	{
		Data->iReserved1 = 1;
		Data->iReserved2 = MaxTimeBandNum;
	}
	//added end

	if(!UseLcu)
	{
		memset(ylfData,0,MAX_YLF_LEN);
		ylfData[0] = (UINT8)CrossNo;
		ylfData[1] = (UINT8)CmdNo;
		ylfData[2] = 1;

		if(Data->iReserved2 != MaxTimeBandNum)
		{
			if(Data->iReserved1 > 0)
				ylfData[3] = Data->iReserved1 - 1;
			else
				ylfData[3] = Data->iReserved1 - 1;
	//			return 54;

			memset(ylfEnCyptData,0,MAX_ENCRIPT_LEN);
			EncriptLen = Pack2YlfFormat(ylfEnCyptData,ylfData,EFFECT_YLF_HEAD_LEN + ylfData[2]);
			if(EncriptLen <= 0 || EncriptLen >= MAX_ENCRIPT_LEN)
			{
		#ifdef TEST
				DebugWindow("GetTimeScheduleProc(),EncriptLen Out Of Range!");
		#endif
				return 53;
			}
				
			ylfEnCyptData[EncriptLen] = FormYlfChecksum(ylfEnCyptData,EncriptLen);

			if(!HuSerialPost(CrossNo,(UINT8)CmdNo,ylfEnCyptData,(UINT16)(EncriptLen + 1)))
			{
		#ifdef TEST
				DebugWindow("GetTimeScheduleProc(),Invoking HuSerialPost Failed!");
		#endif
				return 52;
			}
		}
		else
		{
			for(i = Data->iReserved1;i < Data->iReserved2 + 1;i++)
			{
				ylfData[3] = i - 1;

				memset(ylfEnCyptData,0,MAX_ENCRIPT_LEN);
				EncriptLen = Pack2YlfFormat(ylfEnCyptData,ylfData,EFFECT_YLF_HEAD_LEN + ylfData[2]);
				if(EncriptLen <= 0 || EncriptLen >= MAX_ENCRIPT_LEN)
				{
			#ifdef TEST
					DebugWindow("GetTimeScheduleProc(),EncriptLen Out Of Range!");
			#endif
					return 53;
				}
					
				ylfEnCyptData[EncriptLen] = FormYlfChecksum(ylfEnCyptData,EncriptLen);

				if(!HuSerialPost(CrossNo,(UINT8)CmdNo,ylfEnCyptData,(UINT16)(EncriptLen + 1)))
				{
			#ifdef TEST
					DebugWindow("GetTimeScheduleProc(),Invoking HuSerialPost Failed!");
			#endif
					return 52;
				}
			}
		}
	}
	else
	{
#ifdef TEST
		DebugWindow("GetTimeScheduleProc(),Type Not Implemented!");
#endif
		return 60;
	}

	return 0;
}

/*******************************************************************************************************
*	Function Name	: UINT8 JSGetTimeScheduleProc(Cmd_GetTimeBand *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/02,20,2002
*	Global			: None
*	Note			: None
********************************************************************************************************/	
UINT8 JSGetTimeScheduleProc(Cmd_GetTimeBand *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex)
{
	BYTE ylfData[MAX_YLF_LEN];
	BYTE ylfEnCyptData[MAX_ENCRIPT_LEN];
	UINT32 EncriptLen;

	if(Data == NULL)
	{
#ifdef TEST
		DebugWindow("JSGetTimeScheduleProc(),Data=NULL!");
#endif
		return 50;
	}

	if(CrossNo == 0 || CrossNo > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("JSGetTimeScheduleProc(),bCrossNo=0!");
#endif
		return 30;
	}

	if(!UseLcu)
	{
		memset(ylfData,0,MAX_YLF_LEN);
		ylfData[0] = (UINT8)CrossNo;
		ylfData[1] = (UINT8)CmdNo;
		ylfData[2] = 1;

		if(Data->iReserved1 > 0)
			ylfData[3] = Data->iReserved1 - 1;
		else
			return 54;

		memset(ylfEnCyptData,0,MAX_ENCRIPT_LEN);
		EncriptLen = Pack2YlfFormat(ylfEnCyptData,ylfData,EFFECT_YLF_HEAD_LEN + ylfData[2]);
		if(EncriptLen <= 0 || EncriptLen >= MAX_ENCRIPT_LEN)
		{
	#ifdef TEST
			DebugWindow("JSGetTimeScheduleProc(),EncriptLen Out Of Range!");
	#endif
			return 53;
		}
			
		ylfEnCyptData[EncriptLen] = FormYlfChecksum(ylfEnCyptData,EncriptLen);

		if(!HuSerialPost(CrossNo,(UINT8)CmdNo,ylfEnCyptData,(UINT16)(EncriptLen + 1)))
		{
	#ifdef TEST
			DebugWindow("JSGetTimeScheduleProc(),Invoking HuSerialPost Failed!");
	#endif
			return 52;
		}
	}
	else
	{
#ifdef TEST
		DebugWindow("JSGetTimeScheduleProc(),Type Not Implemented!");
#endif
		return 60;
	}

	return 0;
}

/*******************************************************************************************************
*	Function Name	: UINT8 JSDatePlanProc(DatePlanDown *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/02,20,2002
*	Global			: None
*	Note			: None
********************************************************************************************************/	
UINT8 JSDatePlanProc(DatePlanDown *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex)
{
	BYTE ylfData[MAX_YLF_LEN];
	BYTE ylfEnCyptData[MAX_ENCRIPT_LEN];
	UINT32 EncriptLen;
	BYTE year,month;
	int count;
	int i;

	if(Data == NULL)
	{
#ifdef TEST
		DebugWindow("JSDatePlanProc(),Data=NULL!");
#endif
		return 50;
	}

	if(CrossNo == 0 || CrossNo > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("JSDatePlanProc(),bCrossNo=0!");
#endif
		return 30;
	}

	if(!UseLcu)
	{
		memset(ylfData,0,MAX_YLF_LEN);
		ylfData[0] = (UINT8)CrossNo;
		ylfData[1] = (UINT8)CmdNo;
		ylfData[2] = 6;

		count = 0;
		for(i = 0;i < Data->byDatePlanNum;i++)
		{
			count++;
			if(count > MaxDatePlanNum)
			{
	#ifdef TEST
				DebugWindow("JSDatePlanProc(),DatePlanNum Out Of Range!");
	#endif
				return 51;
			}

			ylfData[3] = Data->byDateTypeNo[i];

			year = (BYTE)(Data->StartDate[i]);
			month = (BYTE)((Data->StartDate[i] - (float)year + 0.001) * 100);
			ylfData[4] = year;
			ylfData[5] = month;

			year = (BYTE)(Data->EndDate[i]);
			month = (BYTE)((Data->EndDate[i] - (float)year + 0.001) * 100);
			ylfData[6] = year;
			ylfData[7] = month;

			ylfData[8] = Data->byPlanGroupNo[i];

			memset(ylfEnCyptData,0,MAX_ENCRIPT_LEN);
			EncriptLen = Pack2YlfFormat(ylfEnCyptData,ylfData,EFFECT_YLF_HEAD_LEN + ylfData[2]);
			if(EncriptLen <= 0 || EncriptLen >= MAX_ENCRIPT_LEN)
			{
	#ifdef TEST
				DebugWindow("JSDatePlanProc(),EncriptLen Out Of Range!");
	#endif
				continue;
			}
				
			ylfEnCyptData[EncriptLen] = FormYlfChecksum(ylfEnCyptData,EncriptLen);

			if(!HuSerialPost(CrossNo,(UINT8)CmdNo,ylfEnCyptData,(UINT16)(EncriptLen + 1)))
			{
	#ifdef TEST
				DebugWindow("JSDatePlanProc(),Invoking HuSerialPost Failed!");
	#endif
				return 52;
			}
		}
	}
	else
	{
#ifdef TEST
		DebugWindow("JSDatePlanProc(),Type Not Implemented!");
#endif
		return 60;
	}

	return 0;
}

/*******************************************************************************************************
*	Function Name	: UINT8 GetDataPlanProc(GetDatePlanItem *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/02,20,2002
*	Global			: None
*	Note			: None
********************************************************************************************************/	
UINT8 GetDataPlanProc(GetDatePlanItem *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex)
{
	BYTE ylfData[MAX_YLF_LEN];
	BYTE ylfEnCyptData[MAX_ENCRIPT_LEN];
	UINT32 EncriptLen;
	int count;
	int i;
	int flag = 0;

	if(Data == NULL)
	{
#ifdef TEST
		DebugWindow("GetDataPlanProc(),Data=NULL!");
#endif
		return 50;
	}

	if(CrossNo == 0 || CrossNo > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("GetDataPlanProc(),bCrossNo=0!");
#endif
		return 30;
	}

	if(Data->byDatePlanNum == 0)
	{
		flag = 1;
		Data->byDatePlanNum = MaxDatePlanNum;
	}

	if(!UseLcu)
	{
		memset(ylfData,0,MAX_YLF_LEN);
		ylfData[0] = (UINT8)CrossNo;
		ylfData[1] = (UINT8)CmdNo;
		ylfData[2] = 1;

		count = 0;
		for(i = 0;i < Data->byDatePlanNum;i++)
		{
			count++;
			if(count > MaxDatePlanNum)
			{
	#ifdef TEST
				DebugWindow("GetDataPlanProc(),DatePlanNum Out Of Range!");
	#endif
				return 51;
			}
			
			if(flag == 0)
			{
				if(Data->byDateTypeNo[i] > 0)
					ylfData[3] = Data->byDateTypeNo[i] - 1;
				else
					continue;
			}
			else
			{
				ylfData[3] = i;
			}

			memset(ylfEnCyptData,0,MAX_ENCRIPT_LEN);
			EncriptLen = Pack2YlfFormat(ylfEnCyptData,ylfData,EFFECT_YLF_HEAD_LEN + ylfData[2]);
			if(EncriptLen <= 0 || EncriptLen >= MAX_ENCRIPT_LEN)
			{
	#ifdef TEST
				DebugWindow("GetDataPlanProc(),EncriptLen Out Of Range!");
	#endif
				continue;
			}
				
			ylfEnCyptData[EncriptLen] = FormYlfChecksum(ylfEnCyptData,EncriptLen);

			if(!HuSerialPost(CrossNo,(UINT8)CmdNo,ylfEnCyptData,(UINT16)(EncriptLen + 1)))
			{
	#ifdef TEST
				DebugWindow("GetDataPlanProc(),Invoking HuSerialPost Failed!");
	#endif
				return 52;
			}
		}
	}
	else
	{
#ifdef TEST
		DebugWindow("GetDataPlanProc(),Type Not Implemented!");
#endif
		return 60;
	}

	return 0;
}

/*******************************************************************************************************
*	Function Name	: UINT8 JSBenchMarkReqProc(DATA_BENCHMARK_TIME *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/02,20,2002
*	Global			: None
*	Note			: None
********************************************************************************************************/	
UINT8 JSBenchMarkReqProc(DATA_BENCHMARK_TIME *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex)
{
	BYTE ylfData[MAX_YLF_LEN];
	BYTE ylfEnCyptData[MAX_ENCRIPT_LEN];
	UINT32 EncriptLen;

	if(Data == NULL)
	{
#ifdef TEST
		DebugWindow("JSBenchMarkReqProc(),Data=NULL!");
#endif
		return 50;
	}

	if(CrossNo == 0 || CrossNo > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("JSBenchMarkReqProc(),bCrossNo=0!");
#endif
		return 30;
	}

	if(!UseLcu)
	{
		memset(ylfData,0,MAX_YLF_LEN);
		ylfData[0] = (UINT8)CrossNo;
		ylfData[1] = (UINT8)CmdNo;
		ylfData[2] = 7;

		ylfData[3] = Data->Year;
		ylfData[4] = Data->Month;
		ylfData[5] = Data->Day;
		ylfData[6] = Data->Hour;
		ylfData[7] = Data->Minute;
		ylfData[8] = Data->Second;

	/*	if(Data->Week == 0)
			ylfData[9] = 7;
		else 
	*/		ylfData[9] = Data->Week;

		memset(ylfEnCyptData,0,MAX_ENCRIPT_LEN);
		EncriptLen = Pack2YlfFormat(ylfEnCyptData,ylfData,EFFECT_YLF_HEAD_LEN + ylfData[2]);
		if(EncriptLen <= 0 || EncriptLen >= MAX_ENCRIPT_LEN)
		{
	#ifdef TEST
			DebugWindow("JSBenchMarkReqProc(),EncriptLen Out Of Range!");
	#endif
			return 53;
		}
			
		ylfEnCyptData[EncriptLen] = FormYlfChecksum(ylfEnCyptData,EncriptLen);

		if(!HuSerialPost(CrossNo,(UINT8)CmdNo,ylfEnCyptData,(UINT16)(EncriptLen + 1)))
		{
	#ifdef TEST
			DebugWindow("JSBenchMarkReqProc(),Invoking HuSerialPost Failed!");
	#endif
			return 52;
		}
	}
	else
	{
#ifdef TEST
		DebugWindow("JSBenchMarkReqProc(),Type Not Implemented!");
#endif
		return 60;
	}

	return 0;
}


/*******************************************************************************************************
*	Function Name	: UINT8 JSCrossTimeReqProc(char *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/02,20,2002
*	Global			: None
*	Note			: None
********************************************************************************************************/	
UINT8 JSCrossTimeReqProc(char *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex)
{
	BYTE ylfData[MAX_YLF_LEN];
	BYTE ylfEnCyptData[MAX_ENCRIPT_LEN];
	UINT32 EncriptLen;

	if(CrossNo == 0 || CrossNo > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("JSCrossTimeReqProc(),bCrossNo=0!");
#endif
		return 30;
	}

	if(!UseLcu)
	{
		memset(ylfData,0,MAX_YLF_LEN);
		ylfData[0] = (UINT8)CrossNo;
		ylfData[1] = (UINT8)CmdNo;
		ylfData[2] = 0;

		memset(ylfEnCyptData,0,MAX_ENCRIPT_LEN);
		EncriptLen = Pack2YlfFormat(ylfEnCyptData,ylfData,EFFECT_YLF_HEAD_LEN + ylfData[2]);
		if(EncriptLen <= 0 || EncriptLen >= MAX_ENCRIPT_LEN)
		{
	#ifdef TEST
			DebugWindow("JSCrossTimeReqProc(),EncriptLen Out Of Range!");
	#endif
			return 53;
		}
			
		ylfEnCyptData[EncriptLen] = FormYlfChecksum(ylfEnCyptData,EncriptLen);

		if(!HuSerialPost(CrossNo,(UINT8)CmdNo,ylfEnCyptData,(UINT16)(EncriptLen + 1)))
		{
	#ifdef TEST
			DebugWindow("JSCrossTimeReqProc(),Invoking HuSerialPost Failed!");
	#endif
			return 52;
		}
	}
	else
	{
#ifdef TEST
		DebugWindow("JSCrossTimeReqProc(),Type Not Implemented!");
#endif
		return 60;
	}

	return 0;
}

/**********************************************************************************
*	Function Name	: UINT32 Pack2YlfFormat(BYTE *Dest,BYTE *Src,UINT32 Length)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/02,20,2002
*	Global			: None
*	Note			: None
***********************************************************************************/	
UINT32 UnPackYlfForm(BYTE *Dest,BYTE *Src,UINT32 Length)
{
	UINT32 Len = 0,EnLen = 2; 

	if(Length < MIN_YLF_LEN - 1 || Length >= MAX_ENCRIPT_LEN || (Length % 2 == 1))
	{
#ifdef TEST
		DebugWindow("UnPackYlfForm(),Length Out Of Range!");
#endif
		return 0;
	}

	if(Dest == NULL || Src == NULL)
	{
#ifdef TEST
		DebugWindow("UnPackYlfForm(),In or Out Buffer=NULL!");
#endif
		return 0;
	}

	while(EnLen < Length)
	{
		*(Dest + Len) = (BYTE)UnPackYlfShort(*(Src + EnLen),*(Src + EnLen + 1));
		Len++;
		EnLen += 2;
	}

	*(Dest + 2) = *(Dest + 2) / 2;

	return Len;
}

/**********************************************************************************
*	Function Name	: BYTE UnPackYlfShort(BYTE a,BYTE b)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/02,20,2002
*	Global			: None
*	Note			: None
***********************************************************************************/	
UINT16 UnPackYlfShort(BYTE bHigh,BYTE bLow)
{
	UINT16 Value;

	if(bHigh < 0x30 || bLow < 0x30)
		return MAX_ENCRIPT_LEN + 1; 

	Value = (UINT16)(((bHigh << 4) & 0xf0) + (bLow - 0x30));

	return Value;
}

/**********************************************************************************
*	Function Name	: UINT32 Pack2YlfFormat(BYTE *Dest,BYTE *Src,UINT32 Length)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/02,20,2002
*	Global			: None
*	Note			: None
***********************************************************************************/	
UINT32 Pack2YlfFormat(BYTE *Dest,BYTE *Src,UINT32 Length)
{
	UINT32 Len,EnLen; 

	if(Length < EFFECT_YLF_HEAD_LEN || Length >= MAX_YLF_LEN)
	{
#ifdef TEST
		DebugWindow("Pack2YlfFormat(),Length Out Of Range!");
#endif
		return 0;
	}

	if(Dest == NULL || Src == NULL)
	{
#ifdef TEST
		DebugWindow("Pack2YlfFormat(),In or Out Buffer=NULL!");
#endif
		return 0;
	}

//	memset(Dest,0,MAX_ENCRIPT_LEN);
	*Dest = (BYTE)0x41;
	*(Dest + 1) = (BYTE)0x41;

	Len = 0;
	EnLen = 2;

	while(Len < Length)
	{
		EnLen++; 
		if(EnLen >= MAX_ENCRIPT_LEN)
		{
#ifdef TEST
			DebugWindow("Pack2YlfFormat(),EnLen Out Of Range!");
#endif
			return 0;
		}
		*(Dest + EnLen - 1) = (BYTE)( 0x30 + ((*(Src + Len) >> 4) & 0x0f) );

		EnLen++; 
		if(EnLen >= MAX_ENCRIPT_LEN)
		{
#ifdef TEST
			DebugWindow("Pack2YlfFormat(),EnLen Out Of Range!");
#endif
			return 0;
		}
		*(Dest + EnLen - 1) = (BYTE)( 0x30 + (*(Src + Len) & 0x0f) );

		Len++;
	}

	*(Dest + 6) = (BYTE)( 0x30 + ((*(Src + 2) >> 3) & 0x0f) );
	*(Dest + 7) = (BYTE)( 0x30 + ((*(Src + 2) << 1) & 0x0f) );

	return EnLen;
}


/*
	note:	Return 0 if we handled successfully
			Error Message Range 30----44
			We can return it to Wu if he likes to know what happened
*/
/********************************************************************************
*	Function Name	: void InterpretClient(char * pMsgBuf,int RecLen,BYTE Index)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/06,14,2001
*	Global			: None
*	Note			: None
*********************************************************************************/	
int InterpretClient(void * pMsgBuf,int RecLen,BYTE Index)
{
    CMsgFrame *pHeader = (CMsgFrame *)pMsgBuf;
	BYTE RecieveBuf[MAX_MSG_LEN + 1];
//	SVR_CONFIRM conEvent;
	UINT32 commandNo;
	UINT32 iCrossNo;
	UINT8 bCrossNo;
	BYTE bSucc = 0;//default is success
	int len2;

	if(pMsgBuf == NULL)
	{
#ifdef TEST
		DebugWindow("InterpretClient(),MsgPtr Fatal Error!");
#endif
		return 0;
	}

    if(RecLen > MAX_MSG_LEN || RecLen < 0)
    {
#ifdef TEST
        DebugWindow("InterpretClient(),MsgLen Out of Range!");
#endif
        return 0;
    }

	if(Index == 0 || Index > MAX_SOCKET_NUM)
	{
#ifdef TEST
		DebugWindow("InterpretClient(),Data Link Fatal Error!");
#endif
		return 0;
	}

	memset((void *)RecieveBuf,0x00,MAX_MSG_LEN);
	memcpy((void *)RecieveBuf,(void *)pMsgBuf,RecLen);
	pHeader = (CMsgFrame *)RecieveBuf;
	commandNo = pHeader->MsgType;
	iCrossNo = pHeader->iCrossNo;

//	if(iCrossNo == 0)
//		iCrossNo = 0;
#ifdef TEST
	DebugWindow("----------Received One Command----------");
#endif
	bCrossNo = FindCrossNo(iCrossNo);
	if(bCrossNo == 0)
	{
		if(commandNo == CM_TIMESYNC)
		{
			CMD_BENCHMARK_TIME *p = (CMD_BENCHMARK_TIME *)pMsgBuf;
			DATA_BENCHMARK_TIME *Data = &(p->Time_Data);

#ifdef TEST
			DebugWindow("Received CM_INNERTIMESYNC Event!");
#endif
			bSucc = InnerBenchMarkProc(Data);

			//modified 10,29,2001
			SendCurTime();

			return bSucc;
		}
		else
		{
#ifdef TEST
			DebugWindow("InterpretClient(),Cannot Find CrossNo!");
#endif
			return 42;//Cannot find CrossNo
		}
	}
	
//	if(bCrossNo != 1)
//		bCrossNo = bCrossNo;

	len2 = GetMsgLen(commandNo,CrossData[bCrossNo - 1].Type);
	if( len2 == MAX_REC_DATA || len2 != (int)(RecLen - sizeof(CMsgFrame) ) )
	{
#ifdef TEST
		DebugWindow("InterpretClient(),May Cross Type Mismatch Or Cannot Handled!");
#endif
		return 50;//Cross Type Mismatch
	}

	if(IsSanlian(bCrossNo))
	{
#ifdef TEST
		SanlianProc((char *)RecieveBuf,RecLen,bCrossNo);
#endif
		return 0;
	}

	if(CrossData[bCrossNo - 1].Type != 2)
	{
#ifdef TEST
		DebugWindow("InterpretClient(),Cross Type Error!");
#endif
		return 45;//Cross Type Error
	}

	switch(commandNo)
	{
	case CM_FIRSTSTEPPARAMETER://首次下发参数
#ifdef TEST
		DebugWindow("InterpretClient(),Received CM_FIRSTSTEPPARAMETER!");
#endif
		{
			Cmd_FirstStepsDown *p = (Cmd_FirstStepsDown *)RecieveBuf;
			CStepsDown *Data = &(p->StepsDown);
			//if(bCrossNo == 10)
			//	bCrossNo = 10;
			bSucc = FirstStepParameterProc(bCrossNo,Data);
		}
		break;

	case CM_STEPPARAMETER://通讯机在收到参数后从下一个周期开始运行该参数
#ifdef TEST
		DebugWindow("InterpretClient(),received CM_STEPPARAMETER!");
#endif
		{
			Cmd_StepsDown *p = (Cmd_StepsDown *)RecieveBuf;
			CStepsDown *Data = &(p->StepsDown);

			bSucc = StepParameterProc(bCrossNo,Data);
		}
		break;

	case CM_SLSTEPNEXT:
		{
			CMsgFrame *p = (CMsgFrame *)pMsgBuf;
#ifdef TEST
			DebugWindow("InterpretClient(),Received CM_SLSTEPNEXT!");
#endif
			bSucc = JsNextstepReqProc(bCrossNo,(UINT8)(p->iReserved1));
		}
		break;

	case CM_ONLINE://在线控制指令(开始步进控制)

#ifdef TEST
			DebugWindow("received CM_ONLINE!");
#endif

			bSucc = OnlineProc(bCrossNo);
		break;

	case CM_OFFLINE://离线控制指令

#ifdef TEST
			DebugWindow("InterpretClient(),received CM_OFFLINE!");
#endif
			bSucc = OfflineProc(bCrossNo);
		break;

//added 08,19,2001
	case CM_YELLOWBLINK:
		{
			CMD_YELLOWLAMP_FLASH *p = (CMD_YELLOWLAMP_FLASH *)pMsgBuf;

			DebugWindow("InterpretClient(),Received CM_YELLOWBLINK Event!");
			bSucc = SetYellowOnProc(bCrossNo,p->iReserved1);
		}
		break;

	case CM_ENDYELLOWBLINK:
#ifdef TEST
			DebugWindow("InterpretClient(),Received CM_ENDYELLOWBLINK Event!");
#endif
			bSucc = SetYellowOffProc(bCrossNo);
		break;

	case CM_LAMPOFF:
		{
			CMD_BLACK_OUT *p = (CMD_BLACK_OUT *)pMsgBuf;
#ifdef TEST
			DebugWindow("InterpretClient(),Received CM_LAMPOFF Event!");
#endif
			bSucc = SetBlackOnProc(bCrossNo,p->iReserved1);
		}
		break;

	case CM_ENDLAMPOFF:
		{
#ifdef TEST
			DebugWindow("InterpretClient(),Received CM_ENDLAMPOFF Event!");
#endif
			bSucc = SetBlackOffProc(bCrossNo);
		}
		break;
//added end

	case CM_SETCOLOR://强制保持指定阶梯，用于VIP路线等控制时

#ifdef TEST
		DebugWindow("InterpretClient(),received CM_SETCOLOR!");
#endif
		{
			CSetStepOn iData;
			Cmd_SetStepOn *p = (Cmd_SetStepOn *)RecieveBuf;

			iData.MaxTime = p->iReserved1;
			iData.ForcedStep = p->iReserved2;

			bSucc = SetStepOnProc(bCrossNo,&iData);
		}
		break;

	case CM_ENDCOLOR://解除强制保持指定阶梯

#ifdef TEST
		DebugWindow("InterpretClient(),received CM_ENDCOLOR!");
#endif
		{
//			Cmd_SetStepOff *p = (Cmd_SetStepOff *)RecieveBuf;
//			CSetStepOff *Data = &(p->SetStepOff);

			bSucc = SetStepOffProc(bCrossNo,NULL);//Data);
		}
		break;

	case CM_CHANGECTRLTYPE://改变控制方式，E:D1-D2，标准／特1／特2／特3

#ifdef TEST
		DebugWindow("InterpretClient(),CM_CHANGECTRLTYPE Not Handled So Far!");
#endif
		bSucc = 44;//unhandled message type
		break;

	case CM_STARTREALSTATUS://控制台通知总控，开始需要路口实时参数
#ifdef TEST
		DebugWindow("InterpretClient(),received CM_STARTREALSTATUS!");
#endif

		bSucc = StartRealProc(bCrossNo);

		break;

	case CM_STOPREALSTATUS://控制台通知总控，结束路口实时参数传送
#ifdef TEST
		DebugWindow("InterpretClient(),received CM_STOPREALSTATUS!");
#endif

		bSucc = StopRealProc(bCrossNo);

		break;

	case CM_SETSTEPTABLE://阶梯表设置
		{
		Cmd_SetStepTable *p = (Cmd_SetStepTable *)pMsgBuf;
		SetStepTable *Data = &(p->StepTableData);

#ifdef TEST
		DebugWindow("InterpretClient(),received CM_SETSTEPTABLE!");
#endif
		JSCommStatusCheck(bCrossNo);

		bSucc = JSSetStepTableProc(Data,0x01,bCrossNo,1);
		}
		break;

	case CM_GETSTEPLAMPCOLOR://请求上发阶梯表
		{
			Cmd_GetStepTable *Data = (Cmd_GetStepTable *)pMsgBuf;

#ifdef TEST
		DebugWindow("InterpretClient(),received CM_GETSTEPLAMPCOLOR!");
#endif
		JSCommStatusCheck(bCrossNo);

		bSucc = JSGetStepItemProc(Data,0x11,bCrossNo,1);
		}
		break;

	case CM_SETSTEPTIMEPLAN://阶梯时长设置
		{
		Cmd_SetStepTimePlan *p = (Cmd_SetStepTimePlan *)pMsgBuf;
		SetStepTimePlan *Data = &(p->StepTimePlanData);

#ifdef TEST
		DebugWindow("InterpretClient(),received CM_SETSTEPTIMEPLAN!");
#endif
		JSCommStatusCheck(bCrossNo);

		bSucc = JSSetStepTimePlanProc(Data,0x02,bCrossNo,1);
		}
		break;

	case CM_GETSTEPTIMEPLAN://请求上发阶梯时长
		{
		Cmd_GetStepTimePlan *Data = (Cmd_GetStepTimePlan *)pMsgBuf;
#ifdef TEST
		DebugWindow("InterpretClient(),received CM_GETSTEPTIMEPLAN!");
#endif
		JSCommStatusCheck(bCrossNo);

		bSucc = JSGetStepTimePlanProc(Data,0x12,bCrossNo,1);
		}
		break;

	case CM_SETTIMEBAND://给路口机设置各时段
		{
			Cmd_TimeBandDown *p = (Cmd_TimeBandDown *)pMsgBuf;
			TimeBandDown *Data = &(p->PlanDownData);

#ifdef TEST
			DebugWindow("InterpretClient(),Received CM_SETTIMEBAND Event!");
#endif
		JSCommStatusCheck(bCrossNo);

		bSucc = JSTimePlanReqProc(Data,0x03,bCrossNo,1);
		}
		break;

	case CM_GETTIMEBANDPLAN://请求上发时间段方案
		{
			Cmd_GetTimeBand *Data = (Cmd_GetTimeBand *)pMsgBuf;

#ifdef TEST
			DebugWindow("InterpretClient(),Received CM_GETTIMEBANDPLAN Event!");
#endif
		JSCommStatusCheck(bCrossNo);

		bSucc = GetTimeScheduleProc(Data,0x13,bCrossNo,1);
		}
		break;

	case CM_SETSCHEDULE://给路口机设置时段方案运行时间表
		{
			Cmd_DatePlanDown *p = (Cmd_DatePlanDown *)pMsgBuf;
			DatePlanDown *Data = &(p->DatePlanData);

#ifdef TEST
			DebugWindow("InterpretClient(),Received CM_SETSCHEDULE Event!");
#endif
		JSCommStatusCheck(bCrossNo);

			bSucc = JSDatePlanProc(Data,0x04,bCrossNo,1);
		}
		break;

	case CM_GETDATAPLAN://请求上发日期方案
		{
			Cmd_GetDatePlanItem *p = (Cmd_GetDatePlanItem *)pMsgBuf;
			GetDatePlanItem *Data = &(p->GetDatePlanItemData);

#ifdef TEST
			DebugWindow("InterpretClient(),Received CM_GETDATAPLAN Event!");
#endif
		JSCommStatusCheck(bCrossNo);

			bSucc = GetDataPlanProc(Data,0x14,bCrossNo,1);
		}
		break;

	case CM_TIMESYNC://
		{
			CMD_BENCHMARK_TIME *p = (CMD_BENCHMARK_TIME *)pMsgBuf;
			DATA_BENCHMARK_TIME *Data = &(p->Time_Data);

#ifdef TEST
			DebugWindow("InterpretClient(),Received CM_TIMESYNC Event!");
#endif
			JSCommStatusCheck(bCrossNo);

			InnerBenchMarkProc(Data);

			bSucc = JSBenchMarkReqProc(Data,0x06,bCrossNo,1);
		}
		break;

	case CM_GETBENCHMARK://请求上发时间
#ifdef TEST
			DebugWindow("InterpretClient(),Received CM_GETBENCHMARK Event!");
#endif
		JSCommStatusCheck(bCrossNo);

			bSucc = JSCrossTimeReqProc(NULL,0x16,bCrossNo,1);
		break;

	default:

		DebugWindow("InterpretClient(),Received Unknown Message Type!");
		bSucc = 43;//unknown message type
		break;
	}

	return bSucc;
}
/*int InterpretClient(void * pMsgBuf,int RecLen,BYTE Index)
{
	Header *pHeader;
	BYTE RecieveBuf[MAX_MSG_LEN + 1];
	SVR_CONFIRM conEvent;
	UINT32 commandNo;
	BOOL status;
	BYTE bSucc = 0;//default is success

	if(pMsgBuf == NULL)
	{
		DebugWindow("InterpretClient(),MsgPtr Fatal Error!");
		return 0;
	}

    if(RecLen > MAX_MSG_LEN || RecLen < 0)
    {
        DebugWindow("InterpretClient(),MsgLen Out of Range!");
        return 0;
    }

	if(Index == 0 || Index > MAX_SOCKET_NUM)
	{
		DebugWindow("InterpretClient(),Data Link Fatal Error!");
		return 0;
	}

	memset((void *)&RecieveBuf,0x00,MAX_MSG_LEN);
	memcpy((void *)&RecieveBuf,(void *)pMsgBuf,RecLen);
	pHeader = (Header *)RecieveBuf;
	commandNo = pHeader->CommandNo;

	switch(commandNo)
	{
	case 0x01:
		{
			CMD_VSM_CONTENT *p = (CMD_VSM_CONTENT *)RecieveBuf;
			DATA_VSM_CONTENT *Data = &(p->Vsm_Data);

		}
		break;


		break;

	default:

		DebugWindow("InterpretClient(),Received Unknown Message Type!");
		bSucc = 20;//unknown message type
		break;
	}

	//send confirmation to client
	memset((void *)&conEvent,0x00,sizeof(SVR_CONFIRM));

	conEvent.cmdHeader.MsgLen = sizeof(DATA_SVR_CONFIRM);
	conEvent.cmdHeader.Type = CONFIRM_TYPE;
	conEvent.cmdHeader.SelfIP = 0;  
 	conEvent.cmdHeader.SelfTaskId = 0;
	conEvent.cmdHeader.TargetIP = pHeader->SelfIP;
	conEvent.cmdHeader.TargetTaskId = pHeader->SelfTaskId;
	conEvent.cmdHeader.CommandNo = pHeader->CommandNo;
	conEvent.cmdHeader.Packet1 = ~(conEvent.cmdHeader.MsgLen);
	conEvent.cmdHeader.Packet2 = ~(conEvent.cmdHeader.CommandNo);

	conEvent.SvrConfirm.RetCode = bSucc;
	//conEvent.SvrConfirm.TurnOver = pHeader->

	status = SendDataEx((char *)&conEvent,sizeof(SVR_CONFIRM),Index);

	if(!status)
		DebugWindow("InterpretClient(),Send Error To Client Failed!");

	return 0;

}
*/

/********************************************************************************
*	Function Name	: UINT8 SetBlackOnProc(UINT8 bCrossNo,unsigned short Data)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/09,19,2001
*	Global			: None
*	Note			: None
*********************************************************************************/	
void ClearSynFlag(int port)
{
	int i;

	if(port == 0 || port > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("ClearSynFlag(),port == 0!");
#endif
		return;
	}

	if(CrossData[port - 1].BaseCross != 0)
	{
		CrossData[port - 1].BaseCross = 0;
		CrossData[port - 1].DeltaPhi = 0;
		CrossData[port - 1].SynErrCount = 0;
	}

	for(i = 0;i < MAX_CROSS_NUM;i++)
	{
		if(CrossData[i].BaseCross == port)
		{
			CrossData[i].BaseCross = 0;
			CrossData[i].DeltaPhi = 0;
			CrossData[i].SynErrCount = 0;
		}
	}

	return;
}

/********************************************************************************
*	Function Name	: UINT8 SetBlackOnProc(UINT8 bCrossNo,unsigned short Data)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/09,19,2001
*	Global			: None
*	Note			: None
*********************************************************************************/	
UINT8 SetBlackOnProc(UINT8 bCrossNo,unsigned short Data)
{
	UINT16 LastingTime;

	if(bCrossNo == 0 || bCrossNo > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("SetBlackOnProc(),bCrossNo == 0!");
#endif
		return 30;
	}

	if(CrossData[bCrossNo - 1].Status == IDLE_STATUS || CrossData[bCrossNo - 1].Status == INITIAL_STATUS \
		|| CrossData[bCrossNo - 1].Status == OFFLINE_STATUS)
	{
#ifdef TEST
		DebugWindow("SetBlackOnProc(),Cross Status Invalid!");
#endif
		return 35;
	}

	if(CrossData[bCrossNo - 1].Status == FORCED_STATUS)
	{
		if(CrossData[bCrossNo - 1].ForcedStep == 211)
			return 0;
		else
			return 38;//re-forcedstep,rejected
	}

	LastingTime = Data * 60 + 15;
	if(Data == 0)
		LastingTime = 30 * 60;//default time;

	if(CrossData[bCrossNo - 1].Status == NORMAL_STATUS)
	{
		CrossData[bCrossNo - 1].ForcedStep = 211;
		CrossData[bCrossNo - 1].ForecedLeftTime = LastingTime;
		CrossData[bCrossNo - 1].AulStatus = NORMAL_STATUS;
		CrossData[bCrossNo - 1].Status = FORCED_STATUS;
	}
	else if(CrossData[bCrossNo - 1].Status == TRANSIT_STATUS)
	{
		CrossData[bCrossNo - 1].ForcedStep = 211;
		CrossData[bCrossNo - 1].ForecedLeftTime = LastingTime;
		CrossData[bCrossNo - 1].AulStatus = TRANSIT_STATUS;
		CrossData[bCrossNo - 1].Status = FORCED_STATUS;
	}

	//Status changed,Clear VIP data 08,15,2001
	if(CrossData[bCrossNo - 1].BaseCross != 0)
	{
		CrossData[bCrossNo - 1].SynErrCount = 0;
		CrossData[bCrossNo - 1].BaseCross = 0;
		CrossData[bCrossNo - 1].DeltaPhi = 0;
	}

	ClearSynFlag(bCrossNo);

	return 0;
}

/********************************************************************************
*	Function Name	: UINT8 SetBlackOffProc(UINT8 bCrossNo)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/09,19,2001
*	Global			: None
*	Note			: None
*********************************************************************************/	
UINT8 SetBlackOffProc(UINT8 bCrossNo)
{

	if(bCrossNo == 0 || bCrossNo > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("SetBlackOffProc(),bCrossNo == 0!");
#endif
		return 30;
	}

	if(CrossData[bCrossNo - 1].Status != FORCED_STATUS \
					|| CrossData[bCrossNo - 1].ForcedStep != 211)
	{
#ifdef TEST
		DebugWindow("SetBlackOffProc(),Cross Status Invalid Or Not In Black!");
#endif
		return 35;
	}

	//following deleted 03,20,2001 for yin's and lee's response not identical 
///*
	CrossData[bCrossNo - 1].CurStep = 0;
	CrossData[bCrossNo - 1].NeedStep = 0;
	CrossData[bCrossNo - 1].CycleAulCount += CrossData[bCrossNo - 1].CycleTimeOffset;
	CrossData[bCrossNo - 1].CycleTimeOffset = 0;
//*/
	CrossData[bCrossNo - 1].ForcedStep = 0;
	CrossData[bCrossNo - 1].ForecedLeftTime = 0;//??????????
	CrossData[bCrossNo - 1].Status = NORMAL_STATUS;

	return 0;
}

/********************************************************************************
*	Function Name	: UINT8 SetYellowOnProc(UINT8 bCrossNo,unsigned short Data)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/09,19,2001
*	Global			: None
*	Note			: None
*********************************************************************************/	
UINT8 SetYellowOnProc(UINT8 bCrossNo,unsigned short Data)
{
	UINT16 LastingTime;

	if(bCrossNo == 0 || bCrossNo > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("SetYellowOnProc(),bCrossNo == 0!");
#endif
		return 30;
	}

	if(CrossData[bCrossNo - 1].Status == IDLE_STATUS || CrossData[bCrossNo - 1].Status == INITIAL_STATUS \
		|| CrossData[bCrossNo - 1].Status == OFFLINE_STATUS)
	{
#ifdef TEST
		DebugWindow("SetYellowOnProc(),Cross Status Invalid!");
#endif
		return 35;
	}

	if(CrossData[bCrossNo - 1].Status == FORCED_STATUS)
	{
		if(CrossData[bCrossNo - 1].ForcedStep == 201)
			return 0;
		else
			return 38;//re-forcedstep,rejected
	}

	LastingTime = Data * 60 + 15;
	if(Data == 0)
		LastingTime = 30 * 60;//default time;

	CrossData[bCrossNo - 1].InitalWait = 1;//only a flag

	if(CrossData[bCrossNo - 1].Status == NORMAL_STATUS)
	{
		CrossData[bCrossNo - 1].ForcedStep = 201;
		CrossData[bCrossNo - 1].ForecedLeftTime = LastingTime;
		CrossData[bCrossNo - 1].AulStatus = NORMAL_STATUS;
		CrossData[bCrossNo - 1].Status = FORCED_STATUS;
	}
	else if(CrossData[bCrossNo - 1].Status == TRANSIT_STATUS)
	{
		CrossData[bCrossNo - 1].ForcedStep = 201;
		CrossData[bCrossNo - 1].ForecedLeftTime = LastingTime;
		CrossData[bCrossNo - 1].AulStatus = TRANSIT_STATUS;
		CrossData[bCrossNo - 1].Status = FORCED_STATUS;
	}

	//Status changed,Clear VIP data 08,15,2001
	if(CrossData[bCrossNo - 1].BaseCross != 0)
	{
		CrossData[bCrossNo - 1].SynErrCount = 0;
		CrossData[bCrossNo - 1].BaseCross = 0;
		CrossData[bCrossNo - 1].DeltaPhi = 0;
	}

	ClearSynFlag(bCrossNo);

	return 0;
}

/********************************************************************************
*	Function Name	: UINT8 SetYellowOffProc(UINT8 bCrossNo,CSetStepOff *Data)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/09,19,2001
*	Global			: None
*	Note			: None
*********************************************************************************/	
UINT8 SetYellowOffProc(UINT8 bCrossNo)
{

	if(bCrossNo == 0 || bCrossNo > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("SetYellowOffProc(),bCrossNo == 0!");
#endif
		return 30;
	}

	if(CrossData[bCrossNo - 1].Status != FORCED_STATUS \
				|| CrossData[bCrossNo - 1].ForcedStep != 201)
	{
#ifdef TEST
		DebugWindow("SetYellowOffProc(),Cross Status Invalid Or Not In Yellow Blink!");
#endif
		return 35;
	}

	//following deleted 03,20,2001 for yin's and lee's response not identical 
///*
	CrossData[bCrossNo - 1].CurStep = 0;
	CrossData[bCrossNo - 1].NeedStep = 0;
	CrossData[bCrossNo - 1].CycleAulCount += CrossData[bCrossNo - 1].CycleTimeOffset;
	CrossData[bCrossNo - 1].CycleTimeOffset = 0;
//*/
	CrossData[bCrossNo - 1].ForcedStep = 0;
	CrossData[bCrossNo - 1].ForecedLeftTime = 0;//??????????
	CrossData[bCrossNo - 1].Status = NORMAL_STATUS;

	return 0;
}

/********************************************************************************
*	Function Name	: void CommStatusCheck(int CrossNo)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/10,24,2001 baoding
*   Modify		    : yincy/04,08,2002
*	Global			: None
*	Note			: None
*********************************************************************************/	
void CommStatusCheck(int CrossNo)
{
#ifdef TEST
	char disp[200];
#endif

	if(CrossNo == 0 || CrossNo > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("CommStatusCheck(),CrossNo Out Of Range!");
#endif
		return;
	}

	if(UseLcu)//use serial comm?
		return;

	if(CrossData[CrossNo - 1].Status != DISCONNECT_STATUS)//status ok?
		return;
	
	if(CrossData[CrossNo - 1].ReopenLeftCount > 0)//reopen just now
		return;

#ifdef TEST
	memset(disp,0,200);
	sprintf(disp,"CommStatusCheck(),ReOpen Port=%d!",CrossNo);
	DebugWindow(disp);
#endif

	ClearSanlianSendQueue((UINT8)CrossNo);

	if(ClosePort(CrossNo))
	{
//following lines not neccessary,baoding
//		if(TerminateThread(ThreadCtr[CrossNo].hThread,(DWORD)0))
//		{
//			ThreadCtr[i + 1].hThread = NULL;
//		}
//		else
//			DebugWindow("CommStatusCheck(),Terminate Failed");
//
		if(OpenPortEx(CrossNo))
			Sleep(20);
		else
		{
#ifdef TEST
			DebugWindow("CommStatusCheck(),OpenPort Failed");
#endif
		}
	}
	else
	{
#ifdef TEST
		DebugWindow("CommStatusCheck(),ClosePort Failed");
#endif
	}
	
	CrossData[CrossNo - 1].NoSignalCount = 20;
	CrossData[CrossNo - 1].ReopenLeftCount = 60;
	CrossData[CrossNo - 1].DisconnectCount = 0;
	
	return;
}

/********************************************************************************
*	Function Name	: void JSCommStatusCheck(int CrossNo)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/03,06,2002
*   Modify		    : yincy/04,08,2002
*	Global			: None
*	Note			: None
*********************************************************************************/	
void JSCommStatusCheck(int CrossNo)
{
#ifdef TEST
	char disp[200];
#endif

	if(CrossNo == 0 || CrossNo > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("JSCommStatusCheck(),CrossNo Out Of Range!");
#endif
		return;
	}

	if(UseLcu)//use serial comm?
		return;

	if(CrossData[CrossNo - 1].LinkFlag == TRUE)//status ok?
		return;
	
	if(CrossData[CrossNo - 1].ReopenLeftCount > 0)//reopen just now
		return;

#ifdef TEST
	memset(disp,0,200);
	sprintf(disp,"JSCommStatusCheck(),ReOpen Port=%d!",CrossNo);
	DebugWindow(disp);
#endif

	ClearSanlianSendQueue((UINT8)CrossNo);

	if(ClosePort(CrossNo))
	{
		if(OpenPortEx(CrossNo))
			Sleep(20);
		else
		{
#ifdef TEST
			DebugWindow("JSCommStatusCheck(),OpenPort Failed");
#endif
		}
	}
	else
	{
#ifdef TEST
		DebugWindow("JSCommStatusCheck(),ClosePort Failed");
#endif
	}
	
//	CrossData[CrossNo - 1].NoSignalCount = 20;
	CrossData[CrossNo - 1].ReopenLeftCount = 60;
	CrossData[CrossNo - 1].	DisconnectCount = 0;
	
	return;
}


/********************************************************************************
*	Function Name	: int SanlianProc(char * RecieveBuf,int RecLen,BYTE Index)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/08,06,2001
*	Global			: None
*	Note			: None
*********************************************************************************/	
int SanlianProc(char * pMsgBuf,int RecLen,BYTE CrossNo)
{
	CMsgFrame *pHeader;
	UINT32 commandNo;
	BOOL bSucc = FALSE;
//#ifdef TEST
//	char disp[100];
//#endif

	if(pMsgBuf == NULL)
	{
#ifdef TEST
		DebugWindow("SanlianProc(),Fatal Error!");
#endif
		return 30;
	}

	if(CrossNo == 0 || CrossNo > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("SanlianProc(),CrossNo Error!");
#endif
		return 30;
	}

	pHeader = (CMsgFrame *)pMsgBuf;
	commandNo = pHeader->MsgType;

	CommStatusCheck(CrossNo);

	switch(commandNo)
	{
/*	case CM_VMSTEXT://01
		{
			CMD_VMSTEXT *p = (CMD_VMSTEXT *)pMsgBuf;
			DATA_VMSTEXT *Data = &(p->Vms_Data);

#ifdef TEST
			DebugWindow("Received CM_VMSTEXT Event!");
#endif
			bSucc = VmsTextReqProc(Data,commandNo,CrossNo,1);

		}
		break;
*/
	case CM_TIMESYNC://02
		{
			CMD_BENCHMARK_TIME *p = (CMD_BENCHMARK_TIME *)pMsgBuf;
			DATA_BENCHMARK_TIME *Data = &(p->Time_Data);

#ifdef TEST
			DebugWindow("Received CM_TIMESYNC Event!");
#endif
			InnerBenchMarkProc(Data);

			bSucc = BenchMarkReqProc(Data,commandNo,CrossNo,1);
		}
		break;

	case CM_YELLOWBLINK://03路口黄闪指令
		{
			CMD_YELLOWLAMP_FLASH *p = (CMD_YELLOWLAMP_FLASH *)pMsgBuf;
#ifdef TEST
			DebugWindow("Received CM_YELLOWBLINK Event!");
#endif
			bSucc = YellowLampFlashReqProc(p->iReserved1,commandNo,CrossNo,1);
		}
		break;

	case CM_ENDYELLOWBLINK://03结束黄闪指令
#ifdef TEST
			DebugWindow("Received CM_ENDYELLOWBLINK Event!");
#endif
			bSucc = EndYellowBlinckReqProc(NULL,commandNo,CrossNo,1);
		break;

	case CM_LAMPOFF://04
		{
			CMD_BLACK_OUT *p = (CMD_BLACK_OUT *)pMsgBuf;
#ifdef TEST
			DebugWindow("Received CM_LAMPOFF Event!");
#endif
			bSucc = BlackOutReqProc(p->iReserved1,commandNo,CrossNo,1);
		}
		break;

	case CM_ENDLAMPOFF://04
		{
#ifdef TEST
			DebugWindow("Received CM_ENDLAMPOFF Event!");
#endif
			bSucc = EndBlackReqProc(NULL,commandNo,CrossNo,1);
		}
		break;

	case CM_SETSPLIT://05
		{
			Cmd_SplitPlanDown *p = (Cmd_SplitPlanDown *)pMsgBuf;
			SplitPlanDown *Data = &(p->SplitPlanData);
#ifdef TEST
			DebugWindow("Received CM_SETSPLIT Event!");
#endif
			bSucc = MonentPhaseReqProc(Data,commandNo,CrossNo,1);
		}
		break;

	case CM_SETTIMEBAND://06给路口机设置各时段 52byte
		{
			Cmd_TimeBandDown *p = (Cmd_TimeBandDown *)pMsgBuf;
			TimeBandDown *Data = &(p->PlanDownData);

#ifdef TEST
			DebugWindow("Received CM_SETTIMEBAND Event!");
#endif
			bSucc = TimePlanReqProc(Data,commandNo,CrossNo,1);
		}
		break;

	case CM_SETCYCLEPLAN://07给路口机设置配时方案(new)33 byte
		{
			Cmd_CyclePlanDown *p = (Cmd_CyclePlanDown *)pMsgBuf;
			CyclePlanDown *Data = &(p->CyclePlanData);
#ifdef TEST
			DebugWindow("Received CM_SETCYCLEPLAN Event!");
#endif
			bSucc = PeriodTimeReqProc(Data,commandNo,CrossNo,1);
		}
		break;

	case CM_SETSCHEDULE://08给路口机设置时段方案运行时间表(new)
		{
			Cmd_DatePlanDown *p = (Cmd_DatePlanDown *)pMsgBuf;
			DatePlanDown *Data = &(p->DatePlanData);

#ifdef TEST
			DebugWindow("Received CM_SETSCHEDULE Event!");
#endif
			bSucc = TimeScheduleReqProc(Data,commandNo,CrossNo,1);
		}
		break;

	case CM_SETCONTROL://09
		{
			CMsgFrame *p = (CMsgFrame *)pMsgBuf;

#ifdef TEST
			DebugWindow("Received CM_SETCONTROL Event!");
#endif
			bSucc = SlSetControlReqProc(p->iReserved1,commandNo,CrossNo,1);
		}
		break;

	case CM_SLSTEPNEXT://0a
		{
			CMsgFrame *p = (CMsgFrame *)pMsgBuf;
#ifdef TEST
			DebugWindow("SanlianProc(),Received CM_SLSTEPNEXT!");
#endif
			bSucc = SlNextstepReqProc((UINT8)(p->iReserved1),CrossNo,1);
		}
		break;

	case CM_SETCOLOR://17强制灯色指令
		{
			CMD_LightDataDown *p = (CMD_LightDataDown *)pMsgBuf;
			LightDataDown *Data = &(p->LightDownData);
#ifdef TEST
			DebugWindow("Received CM_SETCOLOR Event!");
#endif
			bSucc = SetColorReqProc(Data,commandNo,CrossNo,1);
		}
		break;

	case CM_SETUSEGROUPPLAN://18
		{
			Cmd_CurScheme *p = (Cmd_CurScheme *)pMsgBuf;
#ifdef TEST
			DebugWindow("Received CM_SETUSEGROUPPLAN Event!");
#endif
			bSucc = CurSchemeReqProc(p->iReserved1,commandNo,CrossNo,1);
		}
		break;

	case CM_SETUSEPLAN://19
		{
			Cmd_CurScheme *p = (Cmd_CurScheme *)pMsgBuf;
#ifdef TEST
			DebugWindow("Received CM_SETUSEPLAN Event!");
#endif
			bSucc = CurSchReqProc(p->iReserved1,commandNo,CrossNo,1);
		}

		break;

	case CM_ENDCOLOR://29解除强制灯色指令

#ifdef TEST
			DebugWindow("Received CM_ENDCOLOR Event!");
#endif
			bSucc = EndColorReqProc(NULL,commandNo,CrossNo,1);

		break;

	case CM_STARTREALSTATUS://80控制台通知总控，开始需要路口实时参数
		{
			CMsgFrame *p = (CMsgFrame *)pMsgBuf;
#ifdef TEST
			char disp[200];
			memset(disp,0,200);
			sprintf(disp,"SanlianProc(),Received Cross=%d CM_STARTREALSTATUS",CrossNo);

			DebugWindow(disp);
#endif
			bSucc = QryAllStateReqProc(p->iReserved1,CrossNo,1);
		}
		break;

	case CM_STOPREALSTATUS://80控制台通知总控，结束路口实时参数传送
		{
#ifdef TEST
		char disp[200];
		memset(disp,0,200);
		sprintf(disp,"SanlianProc(),Received Cross=%d CM_STOPREALSTATUS",CrossNo);
		DebugWindow(disp);
#endif
		bSucc = EndAllStateReqProc(CrossNo,1);
		}
		break;

	case CM_GETSLLAMPCONDITION:   //81控制台通知总控，得到路口机的信号机好坏状态参数
		{
			CMsgFrame *p = (CMsgFrame *)pMsgBuf;
#ifdef TEST
			DebugWindow("SanlianProc(),Received CM_GETSLLAMPCONDITION!");
#endif
			bSucc = StartLampOkReqProc(p->iReserved1,CrossNo,1);
		}
		break;

	case CM_STARTSLCOUNTDATA: //82控制台通知总控，开始需要路口的车辆计数参数
		{
			CMsgFrame *p = (CMsgFrame *)pMsgBuf;
#ifdef TEST
			DebugWindow("SanlianProc(),Received CM_STARTSLCOUNTDATA!");
#endif
			bSucc = StartSlCountReqProc(p->iReserved1,CrossNo,1);
		}
		break;

	case CM_STOPSLCOUNTDATA:      //82控制台通知总控，结束路口的交通参数
#ifdef TEST
		DebugWindow("SanlianProc(),Received CM_STOPSLCOUNTDATA!");
#endif
		bSucc = EndSlCountReqProc(CrossNo,1);

		break;

	case CM_STARTSLAIRDATA:     //83
		{
			CMsgFrame *p = (CMsgFrame *)pMsgBuf;
#ifdef TEST
			DebugWindow("SanlianProc(),Received CM_STARTSLAIRDATA!");
#endif
			bSucc = StartSlAirReqProc(p->iReserved1,CrossNo,1);
		}
		break;

	case CM_STOPSLAIRDATA:      //83
#ifdef TEST
		DebugWindow("SanlianProc(),Received CM_STOPSLAIRDATA!");
#endif
		bSucc = EndSlAirReqProc(CrossNo,1);

		break;

	case CM_STARTSLFLOWDATA:     //84
		{
			CMsgFrame *p = (CMsgFrame *)pMsgBuf;
#ifdef TEST
			DebugWindow("SanlianProc(),Received CM_STARTSLFLOWDATA!");
#endif
			bSucc = StartSlFlowReqProc(p->iReserved1,CrossNo,1);
		}
		break;

	case CM_STOPSLFLOWDATA:      //84
#ifdef TEST
		DebugWindow("SanlianProc(),Received CM_STOPSLFLOWDATA!");
#endif
		bSucc = EndSlFlowReqProc(CrossNo,1);

		break;

	case CM_SLCOMTEST: //85
		{
			CMsgFrame *p = (CMsgFrame *)pMsgBuf;
#ifdef TEST
			DebugWindow("SanlianProc(),Received CM_SLCOMTEST!");
#endif
			bSucc = StartTestReqProc(p->iReserved1,CrossNo,1);
		}

		break;

	case CM_STARTSLCURSTATUS:     //86控制台通知总控，开始需要路口的当前状态参数
		{
			CMsgFrame *p = (CMsgFrame *)pMsgBuf;
#ifdef TEST
			DebugWindow("SanlianProc(),Received CM_STARTSLCURSTATUS!");
#endif
			bSucc = StartCurStatusReqProc(p->iReserved1,CrossNo,1);
		}
		break;

	case CM_STOPSLCURSTATUS:      //86控制台通知总控，结束路口的当前状态参数
#ifdef TEST
		DebugWindow("SanlianProc(),Received CM_STOPSLCURSTATUS!");
#endif
		bSucc = EndCurStatusReqProc(CrossNo,1);

		break;

	default:
		DebugWindow("SanlianProc(),Received Unknown Message Type!");
		break;
	}

//upsend com

	return bSucc;
}

/*******************************************************************************************
*	Function Name	: BOOL SlNextstepReqProc(BYTE Param,UINT32 CrossNo,UINT8 CommIndex)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/09,25,2001
*	Global			: None
*	Note			: None
*******************************************************************************************/	
BOOL SlNextstepReqProc(WORD Param,UINT32 CrossNo,UINT8 CommIndex)
{
	if(CrossNo == 0 || CrossNo > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("SlNextstepReqProc(),Cross No Fatal Error!");
#endif
		return FALSE;
	}

//	if(Param != (BYTE)0x66)
//		return FALSE;
	Param = (BYTE)0x66;

	if(!UseLcu)
	{
		BYTE HuData[7];
		
		memset(HuData,0,sizeof(HuData));
		HuData[0] = (char)0xaa;
		HuData[1] = (UINT8)CrossNo;
		HuData[2] = (UINT8)(0x0a);
		HuData[3] = 0;
		HuData[4] = 1;

		HuData[5] = (BYTE)Param;

		HuData[6] = FormHuChecksum(HuData,6);
		if(HuSerialPost(CrossNo,(UINT8)0x0a,HuData,7))
		{
//			CrossData[CrossNo - 1].NeedRealData = 0;
			return TRUE;
		}
		else
			return FALSE;
	}
	else
	{
 		BYTE HuData[10];
		
		memset(HuData,0,sizeof(HuData));
		HuData[0] = (char)0xaa;
		HuData[1] = (char)0xbb;
		HuData[2] = 1 + ZHAO_ADD_LEN;
		HuData[3] = 0;
		HuData[4] = (UINT8)CrossNo;
		HuData[5] = (UINT8)(0x0a);
		HuData[6] = 0;

		HuData[7] = (BYTE)Param;

		HuData[8] = (char)0x55;
		HuData[9] = (char)0x66;

		if(HuLcuPost(CrossNo,(UINT8)0x0a,HuData,10,CrossData[CrossNo - 1].CardNo))
		{
//			CrossData[CrossNo - 1].NeedRealData = 0;
			return TRUE;
		}
		else
			return FALSE;
	}

	return TRUE;
}


/*******************************************************************************************
*	Function Name	: BOOL StartTestReqProc(BYTE Param,UINT32 CrossNo,UINT8 CommIndex)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/09,25,2001
*	Global			: None
*	Note			: None
*******************************************************************************************/	
BOOL StartTestReqProc(WORD Param,UINT32 CrossNo,UINT8 CommIndex)
{
	UINT8 Flag = 0;

	if(CrossNo == 0 || CrossNo > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("StartTestReqProc(),Cross No Fatal Error!");
#endif
		return FALSE;
	}

	if(CommIndex == 0xff)
		Flag = 0xff;

	if(!UseLcu)
	{
		BYTE HuData[7];
		
		memset(HuData,0,sizeof(HuData));
		HuData[0] = (char)0xaa;
		HuData[1] = (UINT8)CrossNo;
		HuData[2] = (UINT8)(0x85);
		HuData[3] = Flag;
		HuData[4] = 1;

		HuData[5] = (BYTE)Param;

		HuData[6] = FormHuChecksum(HuData,6);
		if(HuSerialPost(CrossNo,(UINT8)0x85,HuData,7))
		{
//			CrossData[CrossNo - 1].NeedRealData = 0;
			return TRUE;
		}
		else
			return FALSE;
	}
	else
	{
 		BYTE HuData[10];
		
		memset(HuData,0,sizeof(HuData));
		HuData[0] = (char)0xaa;
		HuData[1] = (char)0xbb;
		HuData[2] = 1 + ZHAO_ADD_LEN;
		HuData[3] = 0;
		HuData[4] = (UINT8)CrossNo;
		HuData[5] = (UINT8)(0x85);
		HuData[6] = Flag;

		HuData[7] = (BYTE)Param;

		HuData[8] = (char)0x55;
		HuData[9] = (char)0x66;

		if(HuLcuPost(CrossNo,(UINT8)0x85,HuData,10,CrossData[CrossNo - 1].CardNo))
		{
//			CrossData[CrossNo - 1].NeedRealData = 0;
			return TRUE;
		}
		else
			return FALSE;
	}

	return TRUE;
}

/*******************************************************************************************
*	Function Name	: BOOL StartLampOkReqProc(BYTE Param,UINT32 CrossNo,UINT8 CommIndex)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/09,25,2001
*	Global			: None
*	Note			: None
*******************************************************************************************/	
BOOL StartLampOkReqProc(WORD Param,UINT32 CrossNo,UINT8 CommIndex)
{
	if(CrossNo == 0 || CrossNo > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("StartLampOkReqProc(),Cross No Fatal Error!");
#endif
		return FALSE;
	}

	if(!UseLcu)
	{
		BYTE HuData[7];
		
		memset(HuData,0,sizeof(HuData));
		HuData[0] = (char)0xaa;
		HuData[1] = (UINT8)CrossNo;
		HuData[2] = (UINT8)(0x81);
		HuData[3] = 0;
		HuData[4] = 1;

		HuData[5] = 0;//Param;

		HuData[6] = FormHuChecksum(HuData,6);
		if(HuSerialPost(CrossNo,(UINT8)0x81,HuData,7))
		{
//			CrossData[CrossNo - 1].NeedRealData = 1;
			return TRUE;
		}
		else
			return FALSE;
	}
	else
	{
 		BYTE HuData[10];
		
		memset(HuData,0,sizeof(HuData));
		HuData[0] = (char)0xaa;
		HuData[1] = (char)0xbb;
		HuData[2] = 1 + ZHAO_ADD_LEN;
		HuData[3] = 0;
		HuData[4] = (UINT8)CrossNo;
		HuData[5] = (UINT8)(0x81);
		HuData[6] = 0;

		HuData[7] = 0;//Param;

		HuData[8] = (char)0x55;
		HuData[9] = (char)0x66;

		if(HuLcuPost(CrossNo,(UINT8)0x81,HuData,10,CrossData[CrossNo - 1].CardNo))
		{
//			CrossData[CrossNo - 1].NeedRealData = 1;
			return TRUE;
		}
		else
			return FALSE;
	}

	return TRUE;
}

/*******************************************************************************************
*	Function Name	: BOOL EndCurStatusReqProc(BYTE Param,UINT32 CrossNo,UINT8 CommIndex)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/09,25,2001
*	Global			: None
*	Note			: None
*******************************************************************************************/	
BOOL EndCurStatusReqProc(UINT32 CrossNo,UINT8 CommIndex)
{
	if(CrossNo == 0 || CrossNo > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("EndCurStatusReqProc(),Cross No Fatal Error!");
#endif
		return FALSE;
	}

	if(!UseLcu)
	{
		BYTE HuData[7];
		
		memset(HuData,0,sizeof(HuData));
		HuData[0] = (char)0xaa;
		HuData[1] = (UINT8)CrossNo;
		HuData[2] = (UINT8)(0x86);
		HuData[3] = 0;
		HuData[4] = 1;

		HuData[5] = (UINT8)0xfe;

		HuData[6] = FormHuChecksum(HuData,6);
		if(HuSerialPost(CrossNo,(UINT8)0x86,HuData,7))
		{
//			CrossData[CrossNo - 1].NeedRealData = 0;
			return TRUE;
		}
		else
			return FALSE;
	}
	else
	{
 		BYTE HuData[10];
		
		memset(HuData,0,sizeof(HuData));
		HuData[0] = (char)0xaa;
		HuData[1] = (char)0xbb;
		HuData[2] = 1 + ZHAO_ADD_LEN;
		HuData[3] = 0;
		HuData[4] = (UINT8)CrossNo;
		HuData[5] = (UINT8)(0x86);
		HuData[6] = 0;

		HuData[7] = (UINT8)0xfe;

		HuData[8] = (char)0x55;
		HuData[9] = (char)0x66;

		if(HuLcuPost(CrossNo,(UINT8)0x86,HuData,10,CrossData[CrossNo - 1].CardNo))
		{
//			CrossData[CrossNo - 1].NeedRealData = 0;
			return TRUE;
		}
		else
			return FALSE;
	}

	return TRUE;
}

/*******************************************************************************************
*	Function Name	: BOOL StartCurStatusReqProc(BYTE Param,UINT32 CrossNo,UINT8 CommIndex)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/09,25,2001
*	Global			: None
*	Note			: None
*******************************************************************************************/	
BOOL StartCurStatusReqProc(WORD Param,UINT32 CrossNo,UINT8 CommIndex)
{
	if(CrossNo == 0 || CrossNo > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("StartCurStatusReqProc(),Cross No Fatal Error!");
#endif
		return FALSE;
	}

	if(!UseLcu)
	{
		BYTE HuData[7];
		
		memset(HuData,0,sizeof(HuData));
		HuData[0] = (char)0xaa;
		HuData[1] = (UINT8)CrossNo;
		HuData[2] = (UINT8)(0x86);
		HuData[3] = 0;
		HuData[4] = 1;

		HuData[5] = (BYTE)Param;

		HuData[6] = FormHuChecksum(HuData,6);
		if(HuSerialPost(CrossNo,(UINT8)0x86,HuData,7))
		{
//			CrossData[CrossNo - 1].NeedRealData = 1;
			return TRUE;
		}
		else
			return FALSE;
	}
	else
	{
 		BYTE HuData[10];
		
		memset(HuData,0,sizeof(HuData));
		HuData[0] = (char)0xaa;
		HuData[1] = (char)0xbb;
		HuData[2] = 1 + ZHAO_ADD_LEN;
		HuData[3] = 0;
		HuData[4] = (UINT8)CrossNo;
		HuData[5] = (UINT8)(0x86);
		HuData[6] = 0;

		HuData[7] = (BYTE)Param;

		HuData[8] = (char)0x55;
		HuData[9] = (char)0x66;

		if(HuLcuPost(CrossNo,(UINT8)0x86,HuData,10,CrossData[CrossNo - 1].CardNo))
		{
//			CrossData[CrossNo - 1].NeedRealData = 1;
			return TRUE;
		}
		else
			return FALSE;
	}

	return TRUE;
}

/*******************************************************************************************
*	Function Name	: BOOL EndSlAirReqProc(BYTE Param,UINT32 CrossNo,UINT8 CommIndex)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/09,25,2001
*	Global			: None
*	Note			: None
*******************************************************************************************/	
BOOL EndSlAirReqProc(UINT32 CrossNo,UINT8 CommIndex)
{
	if(CrossNo == 0 || CrossNo > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("EndSlAirReqProc(),Cross No Fatal Error!");
#endif
		return FALSE;
	}

	if(!UseLcu)
	{
		BYTE HuData[7];
		
		memset(HuData,0,sizeof(HuData));
		HuData[0] = (char)0xaa;
		HuData[1] = (UINT8)CrossNo;
		HuData[2] = (UINT8)(0x83);
		HuData[3] = 0;
		HuData[4] = 1;

		HuData[5] = (UINT8)0xfe;

		HuData[6] = FormHuChecksum(HuData,6);
		if(HuSerialPost(CrossNo,(UINT8)0x83,HuData,7))
		{
//			CrossData[CrossNo - 1].NeedRealData = 0;
			return TRUE;
		}
		else
			return FALSE;
	}
	else
	{
 		BYTE HuData[10];
		
		memset(HuData,0,sizeof(HuData));
		HuData[0] = (char)0xaa;
		HuData[1] = (char)0xbb;
		HuData[2] = 1 + ZHAO_ADD_LEN;
		HuData[3] = 0;
		HuData[4] = (UINT8)CrossNo;
		HuData[5] = (UINT8)(0x83);
		HuData[6] = 0;

		HuData[7] = (UINT8)0xfe;

		HuData[8] = (char)0x55;
		HuData[9] = (char)0x66;

		if(HuLcuPost(CrossNo,(UINT8)0x83,HuData,10,CrossData[CrossNo - 1].CardNo))
		{
//			CrossData[CrossNo - 1].NeedRealData = 0;
			return TRUE;
		}
		else
			return FALSE;
	}

	return TRUE;
}

/*******************************************************************************************
*	Function Name	: BOOL StartSlAirReqProc(BYTE Param,UINT32 CrossNo,UINT8 CommIndex)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/09,25,2001
*	Global			: None
*	Note			: None
*******************************************************************************************/	
BOOL StartSlAirReqProc(WORD Param,UINT32 CrossNo,UINT8 CommIndex)
{
	if(CrossNo == 0 || CrossNo > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("StartSlAirReqProc(),Cross No Fatal Error!");
#endif
		return FALSE;
	}

	if(!UseLcu)
	{
		BYTE HuData[7];
		
		memset(HuData,0,sizeof(HuData));
		HuData[0] = (char)0xaa;
		HuData[1] = (UINT8)CrossNo;
		HuData[2] = (UINT8)(0x83);
		HuData[3] = 0;
		HuData[4] = 1;

		HuData[5] = (BYTE)Param;

		HuData[6] = FormHuChecksum(HuData,6);
		if(HuSerialPost(CrossNo,(UINT8)0x83,HuData,7))
		{
//			CrossData[CrossNo - 1].NeedRealData = 1;
			return TRUE;
		}
		else
			return FALSE;
	}
	else
	{
 		BYTE HuData[10];
		
		memset(HuData,0,sizeof(HuData));
		HuData[0] = (char)0xaa;
		HuData[1] = (char)0xbb;
		HuData[2] = 1 + ZHAO_ADD_LEN;
		HuData[3] = 0;
		HuData[4] = (UINT8)CrossNo;
		HuData[5] = (UINT8)(0x83);
		HuData[6] = 0;

		HuData[7] = (BYTE)Param;

		HuData[8] = (char)0x55;
		HuData[9] = (char)0x66;

		if(HuLcuPost(CrossNo,(UINT8)0x83,HuData,10,CrossData[CrossNo - 1].CardNo))
		{
//			CrossData[CrossNo - 1].NeedRealData = 1;
			return TRUE;
		}
		else
			return FALSE;
	}

	return TRUE;
}

/*******************************************************************************************
*	Function Name	: BOOL EndSlFlowReqProc(BYTE Param,UINT32 CrossNo,UINT8 CommIndex)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/09,25,2001
*	Global			: None
*	Note			: None
*******************************************************************************************/	
BOOL EndSlFlowReqProc(UINT32 CrossNo,UINT8 CommIndex)
{
	if(CrossNo == 0 || CrossNo > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("EndSlFlowReqProc(),Cross No Fatal Error!");
#endif
		return FALSE;
	}

	if(!UseLcu)
	{
		BYTE HuData[7];
		
		memset(HuData,0,sizeof(HuData));
		HuData[0] = (char)0xaa;
		HuData[1] = (UINT8)CrossNo;
		HuData[2] = (UINT8)(0x84);
		HuData[3] = 0;
		HuData[4] = 1;

		HuData[5] = (UINT8)0xfe;

		HuData[6] = FormHuChecksum(HuData,6);
		if(HuSerialPost(CrossNo,(UINT8)0x84,HuData,7))
		{
//			CrossData[CrossNo - 1].NeedRealData = 0;
			return TRUE;
		}
		else
			return FALSE;
	}
	else
	{
 		BYTE HuData[10];
		
		memset(HuData,0,sizeof(HuData));
		HuData[0] = (char)0xaa;
		HuData[1] = (char)0xbb;
		HuData[2] = 1 + ZHAO_ADD_LEN;
		HuData[3] = 0;
		HuData[4] = (UINT8)CrossNo;
		HuData[5] = (UINT8)(0x84);
		HuData[6] = 0;

		HuData[7] = (UINT8)0xfe;

		HuData[8] = (char)0x55;
		HuData[9] = (char)0x66;

		if(HuLcuPost(CrossNo,(UINT8)0x84,HuData,10,CrossData[CrossNo - 1].CardNo))
		{
//			CrossData[CrossNo - 1].NeedRealData = 0;
			return TRUE;
		}
		else
			return FALSE;
	}

	return TRUE;
}

/*******************************************************************************************
*	Function Name	: BOOL StartSlFlowReqProc(BYTE Param,UINT32 CrossNo,UINT8 CommIndex)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/09,25,2001
*	Global			: None
*	Note			: None
*******************************************************************************************/	
BOOL StartSlFlowReqProc(WORD Param,UINT32 CrossNo,UINT8 CommIndex)
{
	if(CrossNo == 0 || CrossNo > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("StartSlFlowReqProc(),Cross No Fatal Error!");
#endif
		return FALSE;
	}

	if(!UseLcu)
	{
		BYTE HuData[7];
		
		memset(HuData,0,sizeof(HuData));
		HuData[0] = (char)0xaa;
		HuData[1] = (UINT8)CrossNo;
		HuData[2] = (UINT8)(0x84);
		HuData[3] = 0;
		HuData[4] = 1;

		HuData[5] = (BYTE)Param;

		HuData[6] = FormHuChecksum(HuData,6);
		if(HuSerialPost(CrossNo,(UINT8)0x84,HuData,7))
		{
//			CrossData[CrossNo - 1].NeedRealData = 1;
			return TRUE;
		}
		else
			return FALSE;
	}
	else
	{
 		BYTE HuData[10];
		
		memset(HuData,0,sizeof(HuData));
		HuData[0] = (char)0xaa;
		HuData[1] = (char)0xbb;
		HuData[2] = 1 + ZHAO_ADD_LEN;
		HuData[3] = 0;
		HuData[4] = (UINT8)CrossNo;
		HuData[5] = (UINT8)(0x84);
		HuData[6] = 0;

		HuData[7] = (BYTE)Param;

		HuData[8] = (char)0x55;
		HuData[9] = (char)0x66;

		if(HuLcuPost(CrossNo,(UINT8)0x84,HuData,10,CrossData[CrossNo - 1].CardNo))
		{
//			CrossData[CrossNo - 1].NeedRealData = 1;
			return TRUE;
		}
		else
			return FALSE;
	}

	return TRUE;
}

/********************************************************************************
*	Function Name	: int GetMsgLen(INT32 CommandNo,BYTE Type)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/08,21,2001
*	Global			: None
*	Note			: None
*********************************************************************************/	
int GetMsgLen(INT32 CommandNo,BYTE Type)
{
	int MsgLen = MAX_REC_DATA;

	switch(CommandNo)
	{
	case CM_STARTSLCOUNTDATA:
		if(Type == 1)
			MsgLen = 0;
		else if(Type == 2)
		{
#ifdef TEST
			DebugWindow("CM_STARTSLCOUNTDATA Event Length Error!");
#endif
			MsgLen = MAX_REC_DATA;
		}
		break;

	case CM_STOPSLCOUNTDATA:
		if(Type == 1)
			MsgLen = 0;
		else if(Type == 2)
		{
#ifdef TEST
			DebugWindow("CM_STOPSLCOUNTDATA Event Length Error!");
#endif
			MsgLen = MAX_REC_DATA;
		}
		break;

	case CM_STARTSLFLOWDATA:
		if(Type == 1)
			MsgLen = 0;
		else if(Type == 2)
		{
#ifdef TEST
			DebugWindow("CM_STARTSLFLOWDATA Event Length Error!");
#endif
			MsgLen = MAX_REC_DATA;
		}
		break;

	case CM_STOPSLFLOWDATA:
		if(Type == 1)
			MsgLen = 0;
		else if(Type == 2)
		{
#ifdef TEST
			DebugWindow("CM_STOPSLFLOWDATA Event Length Error!");
#endif
			MsgLen = MAX_REC_DATA;
		}
		break;

	case CM_STARTSLAIRDATA:
		if(Type == 1)
			MsgLen = 0;
		else if(Type == 2)
		{
#ifdef TEST
			DebugWindow("CM_STARTSLAIRDATA Event Length Error!");
#endif
			MsgLen = MAX_REC_DATA;
		}
		break;

	case CM_STOPSLAIRDATA:
		if(Type == 1)
			MsgLen = 0;
		else if(Type == 2)
		{
#ifdef TEST
			DebugWindow("CM_STOPSLAIRDATA Event Length Error!");
#endif
			MsgLen = MAX_REC_DATA;
		}
		break;

	case CM_STARTSLCURSTATUS:
		if(Type == 1)
			MsgLen = 0;
		else if(Type == 2)
		{
#ifdef TEST
			DebugWindow("CM_STARTSLCURSTATUS Event Length Error!");
#endif
			MsgLen = MAX_REC_DATA;
		}
		break;

	case CM_STOPSLCURSTATUS:
		if(Type == 1)
			MsgLen = 0;
		else if(Type == 2)
		{
#ifdef TEST
			DebugWindow("CM_STOPSLCURSTATUS Event Length Error!");
#endif
			MsgLen = MAX_REC_DATA;
		}
		break;

	case CM_GETSLLAMPCONDITION:
		if(Type == 1)
			MsgLen = 0;
		else if(Type == 2)
		{
#ifdef TEST
			DebugWindow("CM_GETSLLAMPCONDITION Event Length Error!");
#endif
			MsgLen = MAX_REC_DATA;
		}
		break;

	case CM_SLCOMTEST:
		
		if(Type == 1)
			MsgLen = 0;
		else if(Type == 2)
		{
#ifdef TEST
			DebugWindow("CM_SLCOMTEST Event Length Error!");
#endif
			MsgLen = MAX_REC_DATA;
		}

		break;
		
	case CM_SLSTEPNEXT:
	case CM_STARTREALSTATUS:
	case CM_STOPREALSTATUS://控制台通知总控，结束路口实时参数传送
		MsgLen = 0;
		break;
		
	case CM_SETUSEPLAN:

		if(Type == 1)
			MsgLen = 0;
		else if(Type == 2)
		{
#ifdef TEST
			DebugWindow("CM_SETUSEPLAN Event Length Error!");
#endif
			MsgLen = MAX_REC_DATA;
		}

		break;
		
	case CM_SETUSEGROUPPLAN://设置路口运行方案号

		if(Type == 1)
			MsgLen = 0;
		else if(Type == 2)
		{
#ifdef TEST
			DebugWindow("CM_SETUSEGROUPPLAN Event Length Error!");
#endif
			MsgLen = MAX_REC_DATA;
		}

		break;

	case CM_SETSCHEDULE://设置路口日期方案选择表

		if(Type == 1)
			MsgLen = sizeof(DatePlanDown);
		else if(Type == 2)
		{
			MsgLen = sizeof(DatePlanDown);//02,19,2001
//#ifdef TEST
//			DebugWindow("CM_SETSCHEDULE Event Length Error!");
//#endif
//			MsgLen = MAX_REC_DATA;
		}

		break;
		
	case CM_SETTIMEBAND://设置路口时间段表

		if(Type == 1)
			MsgLen = sizeof(TimeBandDown);
		else if(Type == 2)
		{
			MsgLen = sizeof(TimeBandDown);//02,19,2001
//#ifdef TEST
//			DebugWindow("CM_SETTIMEBAND Event Length Error!");
//#endif
//			MsgLen = MAX_REC_DATA;
		}

		break;
		
	case CM_SETCYCLEPLAN://设置路口配时方案

		if(Type == 1)
			MsgLen = sizeof(CyclePlanDown);
		else if(Type == 2)
		{
#ifdef TEST
			DebugWindow("CM_SETCYCLEPLAN Event Length Error!");
#endif
			MsgLen = MAX_REC_DATA;
		}

		break;
		
	case CM_SETSPLIT://设置路口相位方案

		if(Type == 1)
			MsgLen = sizeof(SplitPlanDown);
		else if(Type == 2)
		{
#ifdef TEST
			DebugWindow("CM_SETSPLIT Event Length Error!");
#endif
			MsgLen = MAX_REC_DATA;
		}

		break;
		
	case CM_SETCOLOR://强制灯色指令

		if(Type == 1)
			MsgLen = sizeof(LightDataDown);
		else if(Type == 2)
			MsgLen = 0;//sizeof(CSetStepOn);

		break;
		
	case CM_ENDCOLOR://解除强制灯色指令

		if(Type == 1)
			MsgLen = 0;
		else if(Type == 2)
			MsgLen = 0;//sizeof(CSetStepOff);

		break;
		
	case CM_TIMESYNC://时间同步指令

		if(Type == 1)
			MsgLen = sizeof(DATA_BENCHMARK_TIME);
		else if(Type == 2)
		{
			MsgLen = sizeof(DATA_BENCHMARK_TIME);//02,19,2001
//#ifdef TEST
//			DebugWindow("CM_TIMESYNC Event Length Error!");
//#endif
//			MsgLen = MAX_REC_DATA;
		}

		break;
		
	case CM_ONLINE://在线指令

		if(Type == 1)
		{
#ifdef TEST
			DebugWindow("CM_ONLINE Event Length Error!");
#endif
			MsgLen = MAX_REC_DATA;
		}
		else if(Type == 2)
			MsgLen = 0;

		break;
		
	case CM_OFFLINE://离线指令

		if(Type == 1)
		{
#ifdef TEST
			DebugWindow("CM_OFFLINE Event Length Error!");
#endif
			MsgLen = MAX_REC_DATA;
		}
		else if(Type == 2)
			MsgLen = 0;

		break;
		
	case CM_YELLOWBLINK://路口黄闪指令

		if(Type == 1)
			MsgLen = 0;//sizeof(unsigned short);
		else if(Type == 2)
			MsgLen = 0;//sizeof(unsigned short);

		break;
		
	case CM_ENDYELLOWBLINK://结束黄闪指令

		if(Type == 1)
			MsgLen = 0;
		else if(Type == 2)
			MsgLen = 0;

		break;
		
	case CM_LAMPOFF://路口灯熄灭指令

		if(Type == 1)
			MsgLen = 0;//sizeof(unsigned short);
		else if(Type == 2)
			MsgLen = 0;//sizeof(unsigned short);

		break;
		
	case CM_ENDLAMPOFF://路口灯结束熄灭指令

		if(Type == 1)
			MsgLen = 0;
		else if(Type == 2)
			MsgLen = 0;

		break;
		
	case CM_FIRSTSTEPPARAMETER://第一次向通讯机下发信号阶梯控制参数

		if(Type == 1)
		{
#ifdef TEST
			DebugWindow("CM_FIRSTSTEPPARAMETER Event Length Error!");
#endif
			MsgLen = MAX_REC_DATA;
		}
		else if(Type == 2)
			MsgLen = sizeof(CStepsDown);


		break;

	case CM_STEPPARAMETER://向通讯机下发信号阶梯控制参数,

		if(Type == 1)
		{
#ifdef TEST
			DebugWindow("CM_STEPPARAMETER Event Length Error!");
#endif
			MsgLen = MAX_REC_DATA;
		}
		else if(Type == 2)
			MsgLen = sizeof(CStepsDown);

		break;

	case CM_SETCONTROL://设置控制方式

		if(Type == 1)
			MsgLen = 0;
		else if(Type == 2)
		{
#ifdef TEST
			DebugWindow("GetMsgLen(),Received CM_SETCONTROL Not Handled!");
#endif
			MsgLen = MAX_REC_DATA;
		}
		break;
		
	case CM_CHANGECTRLTYPE:

		if(Type == 1)
		{
#ifdef TEST
			DebugWindow("GetMsgLen(),CM_CHANGECTRLTYPE Not Handled!");
#endif
			MsgLen = MAX_REC_DATA;
		}
		else if(Type == 2)
		{
#ifdef TEST
			DebugWindow("GetMsgLen(),CM_CHANGECTRLTYPE Not Handled!");
#endif
			MsgLen = MAX_REC_DATA;
		}
		break;

	case CM_SETSTEPTABLE://阶梯表设置

		if(Type == 1)
		{
#ifdef TEST
			DebugWindow("GetMsgLen(),CM_SETSTEPTABLE Not Handled!");
#endif
			MsgLen = MAX_REC_DATA;
		}
		else if(Type == 2)
			MsgLen = sizeof(SetStepTable);

		break;

	case CM_GETSTEPLAMPCOLOR://请求上发阶梯表

		if(Type == 1)
		{
#ifdef TEST
			DebugWindow("GetMsgLen(),CM_GETSTEPLAMPCOLOR Not Handled!");
#endif
			MsgLen = MAX_REC_DATA;
		}
		else if(Type == 2)
			MsgLen = 0;

		break;
	

	case CM_SETSTEPTIMEPLAN://阶梯时长设置

		if(Type == 1)
		{
#ifdef TEST
			DebugWindow("GetMsgLen(),CM_SETSTEPTIMEPLAN Not Handled!");
#endif
			MsgLen = MAX_REC_DATA;
		}
		else if(Type == 2)
			MsgLen = sizeof(SetStepTimePlan);

		break;

	case CM_GETSTEPTIMEPLAN://请求上发阶梯时长

		if(Type == 1)
		{
#ifdef TEST
			DebugWindow("GetMsgLen(),CM_GETSTEPTIMEPLAN Not Handled!");
#endif
			MsgLen = MAX_REC_DATA;
		}
		else if(Type == 2)
			MsgLen = 0;

		break;
	case CM_GETTIMEBANDPLAN://请求上发时间段方案

		if(Type == 1)
		{
#ifdef TEST
			DebugWindow("GetMsgLen(),CM_GETTIMEBANDPLAN Not Handled!");
#endif
			MsgLen = MAX_REC_DATA;
		}
		else if(Type == 2)
			MsgLen = 0;

		break;
	case CM_GETDATAPLAN://请求上发日期方案

		if(Type == 1)
		{
#ifdef TEST
			DebugWindow("GetMsgLen(),CM_GETDATAPLAN Not Handled!");
#endif
			MsgLen = MAX_REC_DATA;
		}
		else if(Type == 2)
			MsgLen = sizeof(GetDatePlanItem);

		break;
	case CM_GETBENCHMARK://请求上发时间

		if(Type == 1)
		{
#ifdef TEST
			DebugWindow("GetMsgLen(),CM_GETBENCHMARK Not Handled!");
#endif
			MsgLen = MAX_REC_DATA;
		}
		else if(Type == 2)
			MsgLen = 0;

		break;
	
	default:
#ifdef TEST
		DebugWindow("GetMsgLen(),Received Unknown Message Type!");
#endif
		MsgLen = MAX_REC_DATA;
		break;
	}

	return MsgLen;
}

/*****************************************************************************************************
*	Function Name	: BOOL SetColorReqProc(LightDataDown *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/08,13,2001
*	Global			: None
*	Note			: None
******************************************************************************************************/	
BOOL SetColorReqProc(LightDataDown *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex)
{
	int i;
	int timeLen;
//	int LampNum;
	SYSTEMTIME time;
	int temp;
	BYTE endhour;
	BYTE endmin;

	if(Data == NULL)
	{
#ifdef TEST
		DebugWindow("SetColorReqProc(),Process Function Input Value Error!");
#endif
		return FALSE;
	}

	if(CrossNo == 0 || CrossNo > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("SetColorReqProc(),Cross No Fatal Error!");
#endif
		return FALSE;
	}

	timeLen = Data->byTime;
	if(timeLen <= 0)
		return TRUE;

	GetLocalTime(&time);
	temp = time.wMinute + timeLen;

	endmin = temp % 60;
	endhour = (time.wHour + temp / 60) % 24;
/*	if(endhour < time.wHour)
	{
		endhour = 23;
		endmin = 59;
	}
*/

/*	if(CrossData[CrossNo - 1].Status == NORMAL_STATUS \
			|| CrossData[CrossNo - 1].Status == FORCED_STATUS)
	{
		DebugWindow("--Warning--Already Send CM_SETCOLOR To Cross!");
//		return TRUE;
	}

	CrossData[CrossNo - 1].Status = NORMAL_STATUS;
	CrossData[CrossNo - 1].ForecedLeftTime = timeLen * 60 + 90;
*/
/*	LampNum = Data->byLampNum;
	if(LampNum < 0 || LampNum > 24)
		LampNum = 24;
*/
	if(!UseLcu)
	{
		BYTE HuData[86];
		
		memset(HuData,0,sizeof(HuData));
		HuData[0] = (char)0xaa;
		HuData[1] = (UINT8)CrossNo;
		HuData[2] = (UINT8)(0x17);
		HuData[3] = 0;
		HuData[4] = 80;

		for(i = 0;i < 20;i++)
		{
			if(Data->byStatus[i] == LT_GREEN)
			{
				HuData[5 + 3 * i] = 0;
				HuData[5 + 3 * i + 1] = 0;
				HuData[5 + 3 * i + 2] = 1;
			}
			else if(Data->byStatus[i] == LT_YELLOW)
			{
				HuData[5 + 3 * i] = 1;
				HuData[5 + 3 * i + 1] = 0;
				HuData[5 + 3 * i + 2] = 0;
			}
			else if(Data->byStatus[i] == LT_RED)
			{
				HuData[5 + 3 * i] = 0;
				HuData[5 + 3 * i + 1] = 1;
				HuData[5 + 3 * i + 2] = 0;
			}
			else
			{
				HuData[5 + 3 * i] = 0;
				HuData[5 + 3 * i + 1] = 0;
				HuData[5 + 3 * i + 2] = 0;
			}
		}
		for(i = 0;i < 8;i++)
		{
			if(Data->byStatus[20 + i] == LT_GREEN)
			{
				HuData[65 + 2 * i] = 0;
				HuData[65 + 2 * i + 1] = 1;
			}
			else  if(Data->byStatus[20 + i] == LT_RED)
			{
				HuData[65 + 2 * i] = 1;
				HuData[65 + 2 * i + 1] = 0;
			}
			else
			{
				HuData[65 + 2 * i] = 0;
				HuData[65 + 2 * i + 1] = 0;
			}
		}

		HuData[81] = (BYTE)(time.wHour);
		HuData[82] = (BYTE)(time.wMinute);
		HuData[83] = endhour;
		HuData[84] = endmin;

		HuData[85] = FormHuChecksum(HuData,85);

		if(HuSerialPost(CrossNo,(UINT8)(0x17),HuData,86))
			return TRUE;
		else
			return FALSE;

		return TRUE;
	}
	else
	{
		BYTE HuData[89];
		
		memset(HuData,0,sizeof(HuData));
		HuData[0] = (char)0xaa;
		HuData[1] = (char)0xbb;
		HuData[2] = 80 + ZHAO_ADD_LEN;
		HuData[3] = 0;

		HuData[4] = (UINT8)CrossNo;
		HuData[5] = (UINT8)(0x17);
		HuData[6] = 0;

		for(i = 0;i < 20;i++)
		{
			if(Data->byStatus[i] == LT_GREEN)
			{
				HuData[7 + 3 * i] = 0;
				HuData[7 + 3 * i + 1] = 0;
				HuData[7 + 3 * i + 2] = 1;
			}
			else if(Data->byStatus[i] == LT_YELLOW)
			{
				HuData[7 + 3 * i] = 1;
				HuData[7 + 3 * i + 1] = 0;
				HuData[7 + 3 * i + 2] = 0;
			}
			else if(Data->byStatus[i] == LT_RED)
			{
				HuData[7 + 3 * i] = 0;
				HuData[7 + 3 * i + 1] = 1;
				HuData[7 + 3 * i + 2] = 0;
			}
			else
			{
				HuData[7 + 3 * i] = 0;
				HuData[7 + 3 * i + 1] = 0;
				HuData[7 + 3 * i + 2] = 0;
			}
		}
		for(i = 0;i < 8;i++)
		{
			if(Data->byStatus[20 + i] == LT_GREEN)
			{
				HuData[67 + 2 * i] = 0;
				HuData[67 + 2 * i + 1] = 1;
			}
			else  if(Data->byStatus[20 + i] == LT_RED)
			{
				HuData[67 + 2 * i] = 1;
				HuData[67 + 2 * i + 1] = 0;
			}
			else
			{
				HuData[67 + 2 * i] = 0;
				HuData[67 + 2 * i + 1] = 0;
			}
		}

		HuData[83] = (BYTE)(time.wHour);
		HuData[84] = (BYTE)(time.wMinute);
		HuData[85] = endhour;
		HuData[86] = endmin;

		HuData[87] = (char)0x55;
		HuData[88] = (char)0x66;

		if(HuLcuPost(CrossNo,(UINT8)(0x17),HuData,89,CrossData[CrossNo - 1].CardNo))
			return TRUE;
		else
			return FALSE;

		return TRUE;
	}

	return TRUE;
}

/*****************************************************************************************************
*	Function Name	: BOOL EndColorReqProc(char *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/08,13,2001
*	Global			: None
*	Note			: None
******************************************************************************************************/	
BOOL EndColorReqProc(char *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex)
{

	if(CrossNo == 0 || CrossNo > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("EndColorReqProc(),Cross No Fatal Error!");
#endif
		return FALSE;
	}

#ifdef TEST
	DebugWindow("In EndColor Function!");
#endif
/*
	if(CrossData[CrossNo - 1].Status != NORMAL_STATUS \
			&& CrossData[CrossNo - 1].Status != FORCED_STATUS)
	{
		DebugWindow("EndColorReqProc(),Status Error But We Do This Job!");
//		return FALSE;//this line deleted 09,28,2001
	}

	CrossData[CrossNo - 1].Status = IDLE_STATUS;
	CrossData[CrossNo - 1].ForecedLeftTime = 0;
*/
	if(!UseLcu)
	{
		BYTE HuData[7];
		
		memset(HuData,0,sizeof(HuData));
		HuData[0] = (char)0xaa;
		HuData[1] = (UINT8)CrossNo;
		HuData[2] = (UINT8)(0x29);
		HuData[3] = 0;
		HuData[4] = 1;

		HuData[5] = 1;
		HuData[6] = FormHuChecksum(HuData,6);

		if(HuSerialPost(CrossNo,(UINT8)(0x29),HuData,7))
			return TRUE;
		else
			return FALSE;

		return TRUE;
	}
	else
	{
		BYTE HuData[10];
		
		memset(HuData,0,sizeof(HuData));
		HuData[0] = (char)0xaa;
		HuData[1] = (char)0xbb;
		HuData[2] = 1 + ZHAO_ADD_LEN;
		HuData[3] = 0;
		HuData[4] = (UINT8)CrossNo;
		HuData[5] = (UINT8)(0x29);
		HuData[6] = 0;

		HuData[7] = 1;
		HuData[8] = (char)0x55;
		HuData[9] = (char)0x66;

		if(HuLcuPost(CrossNo,(UINT8)(0x29),HuData,10,CrossData[CrossNo - 1].CardNo))
			return TRUE;
		else
			return FALSE;

		return TRUE;
	}

	return TRUE;
}

/*****************************************************************************************************
*	Function Name	: BOOL BenchMarkReqProc(DATA_BENCHMARK_TIME *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/08,13,2001
*	Global			: None
*	Note			: None
******************************************************************************************************/	
BOOL BenchMarkReqProc(DATA_BENCHMARK_TIME *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex)
{
	BYTE WeekOfHu;

	if(Data == NULL)
	{
#ifdef TEST
		DebugWindow("BenchMarkReqProc(),Process Function Input Value Error!");
#endif
		return FALSE;
	}

	if(CrossNo == 0 || CrossNo > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("BenchMarkReqProc(),Cross No Fatal Error!");
#endif
		return FALSE;
	}

/*	if(Data->Week >= 100)//only InnerBenchMarkProc
	{
		return InnerBenchMarkProc(Data);
	}
*/
	if(Data->Week == 0)
		WeekOfHu = 7;
	else 
		WeekOfHu = Data->Week;

	if(!UseLcu)
	{
		BYTE HuData[13];
		
		memset(HuData,0,sizeof(HuData));
		HuData[0] = (char)0xaa;
		HuData[1] = (UINT8)CrossNo;
		HuData[2] = (UINT8)(0x02);
		HuData[3] = 0;
		HuData[4] = 7;

		HuData[5] = Data->Year;
		HuData[6] = Data->Month;				
		HuData[7] = Data->Day;
		HuData[8] = WeekOfHu;
		HuData[9] = Data->Hour;
		HuData[10] = Data->Minute;
		HuData[11] = Data->Second;				
		HuData[12] = FormHuChecksum(HuData,12);

		if(HuSerialPost(CrossNo,(UINT8)(0x02),HuData,13))
			return TRUE;
		else
			return FALSE;

		return TRUE;
	}
	else
	{
		BYTE HuData[16];
		
		memset(HuData,0,sizeof(HuData));
		HuData[0] = (char)0xaa;
		HuData[1] = (char)0xbb;
		HuData[2] = 7 + ZHAO_ADD_LEN;
		HuData[3] = 0;
		HuData[4] = (UINT8)CrossNo;
		HuData[5] = (UINT8)(0x02);
		HuData[6] = 0;

		HuData[7] = Data->Year;
		HuData[8] = Data->Month;				
		HuData[9] = Data->Day;
		HuData[10] = WeekOfHu;
		HuData[11] = Data->Hour;
		HuData[12] = Data->Minute;
		HuData[13] = Data->Second;				
		HuData[14] = (char)0x55;
		HuData[15] = (char)0x66;

		if(HuLcuPost(CrossNo,(UINT8)(0x02),HuData,16,CrossData[CrossNo - 1].CardNo))
			return TRUE;
		else
			return FALSE;

		return TRUE;
	}

	return TRUE;
}

/*****************************************************************************************************
*	Function Name	: UINT8 InnerBenchMarkProc(DATA_BENCHMARK_TIME *Data)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/09,20,2001
*	Global			: None
*	Note			: None
******************************************************************************************************/	
UINT8 InnerBenchMarkProc(DATA_BENCHMARK_TIME *Data)
{
	SYSTEMTIME time;

	if(Data == NULL)
	{
#ifdef TEST
		DebugWindow("InnerBenchMarkProc(),Process Function Input Value Error!");
#endif
		return FALSE;
	}

/*	if(Data->Week < 100)
	{
		DebugWindow("InnerBenchMarkProc(),Week Value Error!");
		return FALSE;
	}
*/
	memset(&time,0,sizeof(SYSTEMTIME));
	time.wYear = Data->Year + 2000;
	time.wMonth = Data->Month;
	time.wDay = Data->Day;
	time.wHour = Data->Hour;
	time.wMinute = Data->Minute;
	time.wSecond = Data->Second;
	time.wDayOfWeek = Data->Week;

	if(	time.wSecond < 60  && 
		time.wMinute < 60  &&
		time.wHour  < 24  &&
		time.wYear < 2100 &&  time.wYear >= 2001 &&
		time.wMonth <= 12 &&  time.wMonth >= 1   &&
		time.wDay   <= 31 &&  time.wDay  >= 1)
	{
		MidDayCount = time.wHour * 3600 + time.wMinute * 60 + time.wSecond;
		if(MidDayCount <= 12 * 3600)
			MidDayCount = 12 * 3600 - MidDayCount;
		else
			MidDayCount = 36 * 3600 - MidDayCount;//24 * 360 - (MidDayCount - 12 * 360);

		SetLocalTime(&time); 
	}
	else
		return FALSE;

	return TRUE;
}

/*****************************************************************************************************
*	Function Name	: BOOL EndYellowBlinckReqProc(DATA_ENDYELLOWBLINCK *Data,UINT8 CmdNo,UINT8 CrossNo,UINT8 CommIndex)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/08,13,2001
*	Global			: None
*	Note			: None
******************************************************************************************************/	
BOOL EndYellowBlinckReqProc(char *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex)
{
	SYSTEMTIME time;

	if(CrossNo == 0 || CrossNo > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("EndYellowBlinckReqProc(),Cross No Fatal Error!");
#endif
		return FALSE;
	}

	GetLocalTime(&time);

	if(!UseLcu)
	{
		BYTE HuData[11];
		
		memset(HuData,0,sizeof(HuData));
		HuData[0] = (char)0xaa;
		HuData[1] = (UINT8)CrossNo;
		HuData[2] = (UINT8)(0x03);
		HuData[3] = 0;
		HuData[4] = 5;

		HuData[5] = (BYTE)(time.wHour);
		HuData[6] = (BYTE)(time.wMinute);
		HuData[7] = (BYTE)(time.wHour);
		HuData[8] = (BYTE)(time.wMinute);

		HuData[9] = 0;
		HuData[10] = FormHuChecksum(HuData,10);

		if(HuSerialPost(CrossNo,(UINT8)(0x03),HuData,11))
			return TRUE;
		else
			return FALSE;

		return TRUE;
	}
	else
	{
		BYTE HuData[14];

		memset(HuData,0,sizeof(HuData));
		HuData[0] = (char)0xaa;
		HuData[1] = (char)0xbb;
		HuData[2] = 5 + ZHAO_ADD_LEN;//low
		HuData[3] = 0;//high
		HuData[4] = (UINT8)CrossNo;
		HuData[5] = (UINT8)(0x03);
		HuData[6] = 0;//reserved

		HuData[7] = (BYTE)(time.wHour);
		HuData[8] = (BYTE)(time.wMinute);
		HuData[9] = (BYTE)(time.wHour);
		HuData[10] = (BYTE)(time.wMinute);
		HuData[11] = 0;
		HuData[12] = (char)0x55;
		HuData[13] = (char)0x66;

		if(HuLcuPost(CrossNo,(UINT8)(0x03),HuData,14,CrossData[CrossNo - 1].CardNo))
			return TRUE;
		else
			return FALSE;

		return TRUE;
	}


	return TRUE;
}

/*****************************************************************************************************
*	Function Name	: BOOL YellowLampFlashReqProc(DATA_YELLOWLAMP_FLASH *Data,UINT8 CmdNo,UINT8 CrossNo,UINT8 CommIndex)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/08,06,2001
*	Global			: None
*	Note			: None
******************************************************************************************************/	
BOOL YellowLampFlashReqProc(unsigned short Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex)
{
	SYSTEMTIME time;
	int temp;
	BYTE endhour;
	BYTE endmin;

	if(CrossNo == 0 || CrossNo > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("YellowLampFlashReqProc(),Cross No Fatal Error!");
#endif
		return FALSE;
	}

	GetLocalTime(&time);
	temp = time.wMinute + Data;

	endmin = temp % 60;
	endhour = (time.wHour + temp / 60) % 24;
/*	if(endhour < time.wHour)
	{
		endhour = 23;
		endmin = 59;
	}
*/
	if(!UseLcu)
	{
		BYTE HuData[11];
		
		memset(HuData,0,sizeof(HuData));
		HuData[0] = (char)0xaa;
		HuData[1] = (UINT8)CrossNo;
		HuData[2] = (UINT8)(0x03);
		HuData[3] = 0;
		HuData[4] = 5;

		HuData[5] = (BYTE)(time.wHour);
		HuData[6] = (BYTE)(time.wMinute);
		HuData[7] = (BYTE)endhour;
		HuData[8] = (BYTE)endmin;
		HuData[9] = 1;
		HuData[10] = FormHuChecksum(HuData,10);

		if(HuSerialPost(CrossNo,(UINT8)(0x03),HuData,11))
			return TRUE;
		else
			return FALSE;

		return TRUE;
	}
	else
	{
		BYTE HuData[14];

		memset(HuData,0,sizeof(HuData));
		HuData[0] = (char)0xaa;
		HuData[1] = (char)0xbb;
		HuData[2] = 5 + ZHAO_ADD_LEN;//low
		HuData[3] = 0;//high
		HuData[4] = (UINT8)CrossNo;
		HuData[5] = (UINT8)(0x03);
		HuData[6] = 0;//reserved

		HuData[7] = (BYTE)(time.wHour);
		HuData[8] = (BYTE)(time.wMinute);
		HuData[9] = (BYTE)endhour;
		HuData[10] = (BYTE)endmin;
		HuData[11] = 1;
		HuData[12] = (char)0x55;
		HuData[13] = (char)0x66;

		if(HuLcuPost(CrossNo,(UINT8)(0x03),HuData,14,CrossData[CrossNo - 1].CardNo))
			return TRUE;
		else
			return FALSE;

		return TRUE;
	}

	return TRUE;
}

/****************************************************************************************
*	Function Name	: BOOL HuLcuPost(int Port,UINT8 CmdNo,char *MsgPtr,UINT16 MsgLen)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/09,24,2001
*	Global			: None
*	Note			: None
*****************************************************************************************/	
BOOL HuLcuPost(int Port,UINT8 CmdNo,char *MsgPtr,UINT16 MsgLen,UINT8 CardNo)
{

	if(MsgPtr == NULL)
	{
#ifdef TEST
		DebugWindow("HuLcuPost(),MsgPtr Is NULL!");
#endif
		return FALSE;
	}

	if(MsgLen == 0 || MsgLen >= MAX_MSG_LEN)
	{
#ifdef TEST
		DebugWindow("HuLcuPost(),MsgLen Out of Range!");
#endif
		return FALSE;
	}

	if(CardNo == 0 || CardNo > 2)
	{
#ifdef TEST
		DebugWindow("HuLcuPost(),CardNo Out of Range!");
#endif
		return FALSE;		
	}

	//following added 10,11,2001
	AdjustData4Zhao((UINT8 *)MsgPtr,(int)MsgLen);
	
	if(LcuCanSend())
	{
		memset(&SendingBuf,0,sizeof(RecBuf));
		SendingBuf.Status = BUF_SEND_STATUS;
		SendingBuf.CrossNo = (BYTE)Port;
		SendingBuf.CmdNo = CmdNo;
		SendingBuf.CardNo = CardNo;
		SendingBuf.ReSendTimes = 1;
		SendingBuf.Timeout = 2;
		SendingBuf.MsgLen = MsgLen;
		memcpy(SendingBuf.MsgData,MsgPtr,MsgLen);

		Write2Lpt(SendingBuf.MsgData,SendingBuf.MsgLen,CardNo);
	}
	else
	{
		if(!SanlianInQue((BYTE)Port,CmdNo,MsgPtr,MsgLen))
		{
#ifdef TEST
			DebugWindow("HuLcuPost(),Queue Overflow!");
#endif
			return FALSE;
		}
	}

	return TRUE;
}

/****************************************************************************************
*	Function Name	: BOOL LcuCanSend(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/09,24,2001
*	Global			: None
*	Note			: None
*****************************************************************************************/	
BOOL LcuCanSend(void)
{
	if(SendingBuf.Status == BUF_IDLE_STATUS)
		return TRUE;
	else
		return FALSE;
}

/****************************************************************************************
*	Function Name	: BOOL HuSerialPost(int Port,UINT8 CmdNo,char *MsgPtr,UINT16 MsgLen)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/08,10,2001
*	Global			: None
*	Note			: None
*****************************************************************************************/	
BOOL HuSerialPost(int Port,UINT8 CmdNo,char *MsgPtr,UINT16 MsgLen)
{
	BOOL Ret;
	BOOL RealSend = FALSE;

	if(MsgPtr == NULL)
	{
#ifdef TEST
		DebugWindow("HuSerialPost(),MsgPtr Is NULL!");
#endif
		return FALSE;
	}

	if(MsgLen == 0 || MsgLen >= MAX_MSG_LEN)
	{
#ifdef TEST
		DebugWindow("HuSerialPost(),MsgLen Out of Range!");
#endif
		return FALSE;
	}

	if(NowCanSend((UINT8)Port))
	{
		//demostrating the queue is empty
		RealSend = TRUE;
		Ret = Notify(Port,SERIAL_WRITE,MsgPtr,MsgLen);
		if(!Ret)
		{
#ifdef TEST
			DebugWindow("HuSerialPost(),Invoking Notify Failed,May not Reached!");
#endif

			if(SanlianInQue((BYTE)Port,CmdNo,MsgPtr,MsgLen))
				SetBuf2Que((UINT8)Port,FALSE);
			else
				return FALSE;

			return TRUE;
		}
	}

	if(SanlianInQue((BYTE)Port,CmdNo,MsgPtr,MsgLen))
	{
		if(RealSend)
			SetBuf2Busy((UINT8)Port,FALSE);
		else
			SetBuf2Que((UINT8)Port,FALSE);
	}
	else
	{
/*		if(RealSend)
		{
			//may never reach
		}
		else
		{
			//may reach
		}
*/		
		return FALSE;
	}

	return TRUE;
}


/*********************************************************************************
*	Function Name	: BOOL CanSendDirect(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/08,06,2001
*	Global			: None
*	Note			: None
**********************************************************************************/	
BOOL CanSendDirect(void)
{
//	if(SendingBuf.Status == SENDBUF_READY_STATUS)
//		return TRUE;

	return FALSE;
}

/*******************************************************************
*	Function Name	: UINT8 FindPos(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/01,19,2001
*	Global			: None
*	Note			: None
********************************************************************/	
UINT32 FindPos(void)
{
	RecBuf *pSpool = gpSpool;
	register int i;

	//完全相同的命令的覆盖，暂时不考虑
	for(i = 0; i < MAX_SPOOL_NUM; i++)
	{
//		if( (pSpool + i)->Status == SENDBUF_READY_STATUS)
//			break;
//		else
//			continue;
	}

	if(i < 0 || i >= MAX_SPOOL_NUM)
		return 0;
	else 
		return i+1;

}

/*****************************************************************************************************
*	Function Name	: BOOL BlackOutReqProc(DATA_BLACK_OUT *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/08,06,2001
*	Global			: None
*	Note			: None
******************************************************************************************************/	
BOOL BlackOutReqProc(unsigned short Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex)
{
	SYSTEMTIME time;
	int temp;
	BYTE endhour;
	BYTE endmin;

	if(CrossNo == 0 || CrossNo > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("YellowLampFlashReqProc(),Cross No Fatal Error!");
#endif
		return FALSE;
	}

	GetLocalTime(&time);
	temp = time.wMinute + Data;

	endmin = temp % 60;
	endhour = (time.wHour + temp / 60) % 24;
/*	if(endhour < time.wHour)
	{
		endhour = 23;
		endmin = 59;
	}
*/
	if(!UseLcu)
	{
		BYTE HuData[11];
		
		memset(HuData,0,sizeof(HuData));
		HuData[0] = (char)0xaa;
		HuData[1] = (UINT8)CrossNo;
		HuData[2] = (UINT8)(0x04);
		HuData[3] = 0;
		HuData[4] = 5;

		HuData[5] = (BYTE)(time.wHour);
		HuData[6] = (BYTE)(time.wMinute);
		HuData[7] = (BYTE)endhour;
		HuData[8] = (BYTE)endmin;
		HuData[9] = 1;
		HuData[10] = FormHuChecksum(HuData,10);

		if(HuSerialPost(CrossNo,(UINT8)(0x04),HuData,11))
			return TRUE;
		else
			return FALSE;

		return TRUE;
	}
	else
	{
		BYTE HuData[14];

		memset(HuData,0,sizeof(HuData));
		HuData[0] = (char)0xaa;
		HuData[1] = (char)0xbb;
		HuData[2] = 5 + ZHAO_ADD_LEN;//low
		HuData[3] = 0;//high
		HuData[4] = (UINT8)CrossNo;
		HuData[5] = (UINT8)(0x04);
		HuData[6] = 0;//reserved

		HuData[7] = (BYTE)(time.wHour);
		HuData[8] = (BYTE)(time.wMinute);
		HuData[9] = (BYTE)endhour;
		HuData[10] = (BYTE)endmin;
		HuData[11] = 1;
		HuData[12] = (char)0x55;
		HuData[13] = (char)0x66;

		if(HuLcuPost(CrossNo,(UINT8)(0x04),HuData,14,CrossData[CrossNo - 1].CardNo))
			return TRUE;
		else
			return FALSE;

		return TRUE;
	}

	return TRUE;
}

/*****************************************************************************************************
*	Function Name	: BOOL EndBlackReqProc(DATA_END_BLACK *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/08,06,2001
*	Global			: None
*	Note			: None
******************************************************************************************************/	
BOOL EndBlackReqProc(char *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex)
{
	SYSTEMTIME time;

	if(CrossNo == 0 || CrossNo > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("EndYellowBlinckReqProc(),Cross No Fatal Error!");
#endif
		return FALSE;
	}

	GetLocalTime(&time);

	if(!UseLcu)
	{
		BYTE HuData[11];
		
		memset(HuData,0,sizeof(HuData));
		HuData[0] = (char)0xaa;
		HuData[1] = (UINT8)CrossNo;
		HuData[2] = (UINT8)(0x04);
		HuData[3] = 0;
		HuData[4] = 5;

		HuData[5] = (BYTE)(time.wHour);
		HuData[6] = (BYTE)(time.wMinute);
		HuData[7] = (BYTE)(time.wHour);
		HuData[8] = (BYTE)(time.wMinute);

		HuData[9] = 0;
		HuData[10] = FormHuChecksum(HuData,10);

		if(HuSerialPost(CrossNo,(UINT8)(0x04),HuData,11))
			return TRUE;
		else
			return FALSE;

		return TRUE;
	}
	else
	{
		BYTE HuData[14];

		memset(HuData,0,sizeof(HuData));
		HuData[0] = (char)0xaa;
		HuData[1] = (char)0xbb;
		HuData[2] = 5 + ZHAO_ADD_LEN;//low
		HuData[3] = 0;//high
		HuData[4] = (UINT8)CrossNo;
		HuData[5] = (UINT8)(0x04);
		HuData[6] = 0;//reserved

		HuData[7] = (BYTE)(time.wHour);
		HuData[8] = (BYTE)(time.wMinute);
		HuData[9] = (BYTE)(time.wHour);
		HuData[10] = (BYTE)(time.wMinute);
		HuData[11] = 0;
		HuData[12] = (char)0x55;
		HuData[13] = (char)0x66;

		if(HuLcuPost(CrossNo,(UINT8)(0x04),HuData,14,CrossData[CrossNo - 1].CardNo))
			return TRUE;
		else
			return FALSE;

		return TRUE;
	}

	return TRUE;
}


/*****************************************************************************************************
*	Function Name	: BOOL MonentPhaseReqProc(SplitPlanDown *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/08,13,2001
*	Global			: None
*	Note			: None
******************************************************************************************************/	
BOOL MonentPhaseReqProc(SplitPlanDown *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex)
{
	int i,j;
	int byFlowGroupNum;
//	int byLampNum;

	if(Data == NULL)
	{
#ifdef TEST
		DebugWindow("MonentPhaseReqProc(),Process Function Input Value Error!");
#endif
		return FALSE;
	}

	if(CrossNo == 0 || CrossNo > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("MonentPhaseReqProc(),Cross No Fatal Error!");
#endif
		return FALSE;
	}

	byFlowGroupNum = Data->byFlowGroupNum;
	if(byFlowGroupNum < 0 || byFlowGroupNum > 8)
		byFlowGroupNum = 8;
//	byLampNum = Data->byLampNum[i];
//	if(byLampNum < 0 || byLampNum > 11)
//		byLampNum = 11;

	if(!UseLcu)
	{
		BYTE HuData[87];
		
		memset(HuData,0,sizeof(HuData));
		HuData[0] = (char)0xaa;
		HuData[1] = (UINT8)CrossNo;
		HuData[2] = (UINT8)(0x05);
		HuData[3] = 0;
		HuData[4] = 81;

		HuData[5] = byFlowGroupNum;
		for(i = 0;i < 8;i++)
		{
			for(j = 0;j < 10;j++)
			{
				HuData[6 + 10 * i + j] = Data->byLampNo[i][j];
			}
		}
			
		HuData[86] = FormHuChecksum(HuData,86);

		if(HuSerialPost(CrossNo,(UINT8)(0x05),HuData,87))
			return TRUE;
		else
			return FALSE;

		return TRUE;
	}
	else
	{
		BYTE HuData[90];
		
		memset(HuData,0,sizeof(HuData));
		HuData[0] = (char)0xaa;
		HuData[1] = (char)0xbb;
		HuData[2] = 81 + ZHAO_ADD_LEN;
		HuData[3] = 0;
		HuData[4] = (UINT8)CrossNo;
		HuData[5] = (UINT8)(0x05);
		HuData[6] = 0;

		HuData[7] = byFlowGroupNum;
		for(i = 0;i < 8;i++)
		{
			for(j = 0;j < 10;j++)
			{
				HuData[8 + 10 * i + j] = Data->byLampNo[i][j];
			}
		}
			
		HuData[88] = (char)0x55;
		HuData[89] = (char)0x66;

		if(HuLcuPost(CrossNo,(UINT8)(0x05),HuData,90,CrossData[CrossNo - 1].CardNo))
			return TRUE;
		else
			return FALSE;

		return TRUE;
	}

	return TRUE;
}

/*****************************************************************************************************
*	Function Name	: BOOL PeriodTimeReqProc(CyclePlanDown *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/08,06,2001
*	Global			: None
*	Note			: None
******************************************************************************************************/	
BOOL PeriodTimeReqProc(CyclePlanDown *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex)
{
	int i;

	if(Data == NULL)
	{
#ifdef TEST
		DebugWindow("MonentPhaseReqProc(),Process Function Input Value Error!");
#endif
		return FALSE;
	}

	if(CrossNo == 0 || CrossNo > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("MonentPhaseReqProc(),Cross No Fatal Error!");
#endif
		return FALSE;
	}

	if(!UseLcu)
	{
		BYTE HuData[39];
		
		memset(HuData,0,sizeof(HuData));
		HuData[0] = (char)0xaa;
		HuData[1] = (UINT8)CrossNo;
		HuData[2] = (UINT8)(0x07);
		HuData[3] = 0;
		HuData[4] = 33;

		HuData[5] = Data->byPlanNo;
		for(i = 0;i < 8;i++)
		{
			HuData[6 + i] = (BYTE)(Data->iGrTime[i]);
			HuData[6 + 8 + i] = Data->byGrBlinkTime[i];
			HuData[6 + 16 + i] = Data->byYellowTime[i];
			HuData[6 + 24 + i] = Data->byAllRedTime[i];
		}
			
		HuData[38] = FormHuChecksum(HuData,38);

		if(HuSerialPost(CrossNo,(UINT8)(0x07),HuData,39))
			return TRUE;
		else
			return FALSE;

		return TRUE;
	}
	else
	{
		BYTE HuData[42];
		
		memset(HuData,0,sizeof(HuData));
		HuData[0] = (char)0xaa;
		HuData[1] = (char)0xbb;
		HuData[2] = 33 + ZHAO_ADD_LEN;
		HuData[3] = 0;
		HuData[4] = (UINT8)CrossNo;
		HuData[5] = (UINT8)(0x07);
		HuData[6] = 0;

		HuData[7] = Data->byPlanNo;
		for(i = 0;i < 8;i++)
		{
			HuData[8 + i] = (BYTE)(Data->iGrTime[i]);
			HuData[8 + 8 + i] = Data->byGrBlinkTime[i];
			HuData[8 + 16 + i] = Data->byYellowTime[i];
			HuData[8 + 24 + i] = Data->byAllRedTime[i];
		}
			
		HuData[40] = (char)0x55;
		HuData[41] = (char)0x66;

		if(HuLcuPost(CrossNo,(UINT8)(0x07),HuData,42,CrossData[CrossNo - 1].CardNo))
			return TRUE;
		else
			return FALSE;

		return TRUE;
	}

	return TRUE;
}

/*****************************************************************************************************
*	Function Name	: BOOL TimePlanReqProc(TIME_PLAN *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/08,06,2001
*	Global			: None
*	Note			: None
******************************************************************************************************/	
BOOL TimePlanReqProc(TimeBandDown *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex)
{
	int i;
	BYTE hour,min;

	if(Data == NULL)
	{
#ifdef TEST
		DebugWindow("TimePlanReqProc(),Process Function Input Value Error!");
#endif
		return FALSE;
	}

	if(CrossNo == 0 || CrossNo > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("TimePlanReqProc(),Cross No Fatal Error!");
#endif
		return FALSE;
	}

	if(!UseLcu)
	{
		BYTE HuData[58];
		
		memset(HuData,0,sizeof(HuData));
		HuData[0] = (char)0xaa;
		HuData[1] = (UINT8)CrossNo;
		HuData[2] = (UINT8)(0x06);
		HuData[3] = 0;
		HuData[4] = 52;

		HuData[5] = Data->byTimeBandPlanNo;
		HuData[6] = Data->byTimeBandNum;
		for(i = 0;i < 10;i++)
		{
			hour = (BYTE)(Data->StartTime[i]);
			min = (BYTE)( (Data->StartTime[i] - (float)hour) * 60);
			HuData[7 + 5 * i] = hour;
			HuData[7 + 5 * i + 1] = min;
			hour = (BYTE)(Data->EndTime[i]);
			min = (BYTE)( (Data->EndTime[i] - (float)hour) * 60);
			HuData[7 + 5 * i + 2] = hour;
			HuData[7 + 5 * i + 3] = min;
			HuData[7 + 5 * i + 4] = Data->byPlanNo[i];
		}
			
		HuData[57] = FormHuChecksum(HuData,57);

		if(HuSerialPost(CrossNo,(UINT8)(0x06),HuData,58))
			return TRUE;
		else
			return FALSE;

		return TRUE;
	}
	else
	{
		BYTE HuData[61];
		
		memset(HuData,0,sizeof(HuData));
		HuData[0] = (char)0xaa;
		HuData[1] = (char)0xbb;
		HuData[2] = 52 + ZHAO_ADD_LEN;
		HuData[3] = 0;
		HuData[4] = (UINT8)CrossNo;
		HuData[5] = (UINT8)(0x06);
		HuData[6] = 0;

		HuData[7] = Data->byTimeBandPlanNo;
		HuData[8] = Data->byTimeBandNum;
		for(i = 0;i < 10;i++)
		{
			hour = (BYTE)(Data->StartTime[i]);
			min = (BYTE)( (Data->StartTime[i] - (float)hour) * 60);
			HuData[9 + 5 * i] = hour;
			HuData[9 + 5 * i + 1] = min;
			hour = (BYTE)(Data->EndTime[i]);
			min = (BYTE)( (Data->EndTime[i] - (float)hour) * 60);
			HuData[9 + 5 * i + 2] = hour;
			HuData[9 + 5 * i + 3] = min;
			HuData[9 + 5 * i + 4] = Data->byPlanNo[i];
		}
			
		HuData[59] = (char)0x55;
		HuData[60] = (char)0x66;

		if(HuLcuPost(CrossNo,(UINT8)(0x06),HuData,61,CrossData[CrossNo - 1].CardNo))
			return TRUE;
		else
			return FALSE;

		return TRUE;
	}

	return TRUE;
}

/*****************************************************************************************************
*	Function Name	: BOOL SlSetControlReqProc(WORD Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/08,06,2001
*	Global			: None
*	Note			: None
******************************************************************************************************/	
BOOL SlSetControlReqProc(WORD Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex)
{
	if(CrossNo == 0 || CrossNo > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("SlSetControlReqProc(),Cross No Fatal Error!");
#endif
		return FALSE;
	}

	if(!UseLcu)
	{
		BYTE HuData[7];
		
		memset(HuData,0,sizeof(HuData));
		HuData[0] = (char)0xaa;
		HuData[1] = (UINT8)CrossNo;
		HuData[2] = (UINT8)(0x09);
		HuData[3] = 0;
		HuData[4] = 1;

		HuData[5] = (BYTE)Data;

		HuData[6] = FormHuChecksum(HuData,6);
		if(HuSerialPost(CrossNo,(UINT8)0x09,HuData,7))
			return TRUE;
		else
			return FALSE;
	}
	else
	{
 		BYTE HuData[10];
		
		memset(HuData,0,sizeof(HuData));
		HuData[0] = (char)0xaa;
		HuData[1] = (char)0xbb;
		HuData[2] = 1 + ZHAO_ADD_LEN;
		HuData[3] = 0;
		HuData[4] = (UINT8)CrossNo;
		HuData[5] = (UINT8)(0x09);
		HuData[6] = 0;

		HuData[7] = (BYTE)Data;

		HuData[8] = (char)0x55;
		HuData[9] = (char)0x66;

		if(HuLcuPost(CrossNo,(UINT8)0x09,HuData,10,CrossData[CrossNo - 1].CardNo))
			return TRUE;
		else
			return FALSE;
	}

	return TRUE;
}


/*****************************************************************************************************
*	Function Name	: BOOL TimeScheduleReqProc(DatePlanDown *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/08,06,2001
*	Global			: None
*	Note			: None
******************************************************************************************************/	
BOOL TimeScheduleReqProc(DatePlanDown *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex)
{
	int i;
	BYTE year,month;
	BYTE Num;
	BOOL Flag = TRUE;
	BYTE WeekOfHu;

	if(Data == NULL)
	{
#ifdef TEST
		DebugWindow("TimeScheduleReqProc(),Process Function Input Value Error!");
#endif
		return FALSE;
	}

	if(CrossNo == 0 || CrossNo > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("TimeScheduleReqProc(),Cross No Fatal Error!");
#endif
		return FALSE;
	}

	if(!UseLcu)
	{
		BYTE HuData[12];
		
		memset(HuData,0,sizeof(HuData));
		HuData[0] = (char)0xaa;
		HuData[1] = (UINT8)CrossNo;
		HuData[2] = (UINT8)(0x08);
		HuData[3] = 0;
		HuData[4] = 6;

		Num = Data->byDatePlanNum;
		if(Num <= 0)
			return FALSE;
		if(Num > 19)
			Num = 19;

		for(i = 0;i < Num;i++)
		{
			if(Data->byDateTypeNo[i] == 0)
				WeekOfHu = 7;
			else if(Data->byDateTypeNo[i] >= 7)
				WeekOfHu = Data->byDateTypeNo[i] + 1;
			else 
				WeekOfHu = Data->byDateTypeNo[i];

			HuData[5] = WeekOfHu;
			year = (BYTE)(Data->StartDate[i]);
			month = (BYTE)( (Data->StartDate[i] - (float)year) * 100);
			HuData[6] = year;
			HuData[7] = month;
			year = (BYTE)(Data->EndDate[i]);
			month = (BYTE)( (Data->EndDate[i] - (float)year) * 100);
			HuData[8] = year;
			HuData[9] = month;
			HuData[10] = Data->byPlanGroupNo[i];
			
			HuData[11] = FormHuChecksum(HuData,11);
			if(!HuSerialPost(CrossNo,(UINT8)(0x08),HuData,12))
				Flag = FALSE;
		}

		return Flag;
	}
	else
	{
		BYTE HuData[15];
		
		memset(HuData,0,sizeof(HuData));
		HuData[0] = (char)0xaa;
		HuData[1] = (char)0xbb;
		HuData[2] = 6 + ZHAO_ADD_LEN;
		HuData[3] = 0;

		HuData[4] = (UINT8)CrossNo;
		HuData[5] = (UINT8)(0x08);
		HuData[6] = 0;

		Num = Data->byDatePlanNum;
		if(Num <= 0)
			return FALSE;
		if(Num > 17)
			Num = 17;

		for(i = 0;i < Num;i++)
		{
			if(Data->byDateTypeNo[i] == 0)
				WeekOfHu = 7;
			else if(Data->byDateTypeNo[i] >= 7)
				WeekOfHu = Data->byDateTypeNo[i] + 1;
			else 
				WeekOfHu = Data->byDateTypeNo[i];

			HuData[7] = WeekOfHu;
			year = (BYTE)(Data->StartDate[i]);
			month = (BYTE)( (Data->StartDate[i] - (float)year) * 100);
			HuData[8] = year;
			HuData[9] = month;
			year = (BYTE)(Data->EndDate[i]);
			month = (BYTE)( (Data->EndDate[i] - (float)year) * 100);
			HuData[10] = year;
			HuData[11] = month;
			HuData[12] = Data->byPlanGroupNo[i];
			
			HuData[13] = (char)0x55;
			HuData[14] = (char)0x66;
			if(!HuLcuPost(CrossNo,(UINT8)(0x08),HuData,15,CrossData[CrossNo - 1].CardNo))
				Flag = FALSE;
		}

		return Flag;
	}

	return TRUE;
}

/*****************************************************************************************************
*	Function Name	: BOOL CurSchemeReqProc(BYTE Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/08,06,2001
*	Global			: None
*	Note			: None
******************************************************************************************************/	
BOOL CurSchemeReqProc(WORD Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex)
{
	BYTE WeekOfHu;

	if(CrossNo == 0 || CrossNo > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("CurSchemeReqProc(),Cross No Fatal Error!");
#endif
		return FALSE;
	}

	if(Data == 0)
		WeekOfHu = 7;
	else if(Data >= 7)
		WeekOfHu = (BYTE)(Data + 1);
	else
		WeekOfHu = (BYTE)Data;

	if(!UseLcu)
	{
		BYTE HuData[7];
		
		memset(HuData,0,sizeof(HuData));
		HuData[0] = (char)0xaa;
		HuData[1] = (UINT8)CrossNo;
		HuData[2] = (UINT8)(0x18);
		HuData[3] = 0;
		HuData[4] = 1;

		HuData[5] = (BYTE)Data;//WeekOfHu;
		HuData[6] = FormHuChecksum(HuData,6);
		if(HuSerialPost(CrossNo,(UINT8)(0x18),HuData,7))
			return TRUE;
		else
			return FALSE;
	}
	else
	{
		BYTE HuData[10];
		
		memset(HuData,0,sizeof(HuData));
		HuData[0] = (char)0xaa;
		HuData[1] = (char)0xbb;
		HuData[2] = 1 + ZHAO_ADD_LEN;
		HuData[3] = 0;

		HuData[4] = (UINT8)CrossNo;
		HuData[5] = (UINT8)(0x18);
		HuData[6] = 0;

		HuData[7] = (BYTE)Data;//WeekOfHu;
		HuData[8] = (char)0x55;
		HuData[9] = (char)0x66;
		if(HuLcuPost(CrossNo,(UINT8)(0x18),HuData,10,CrossData[CrossNo - 1].CardNo))
			return TRUE;
		else
			return FALSE;
	}

	return TRUE;
}

/*****************************************************************************************************
*	Function Name	: BOOL CurSchReqProc(BYTE Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/10,16,2001
*	Global			: None
*	Note			: None
******************************************************************************************************/	
BOOL CurSchReqProc(WORD Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex)
{

	if(CrossNo == 0 || CrossNo > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("CurSchemeReqProc(),Cross No Fatal Error!");
#endif
		return FALSE;
	}

	if(!UseLcu)
	{
		BYTE HuData[7];
		
		memset(HuData,0,sizeof(HuData));
		HuData[0] = (char)0xaa;
		HuData[1] = (UINT8)CrossNo;
		HuData[2] = (UINT8)(0x19);
		HuData[3] = 0;
		HuData[4] = 1;

		HuData[5] = (BYTE)Data;
		HuData[6] = FormHuChecksum(HuData,6);
		if(HuSerialPost(CrossNo,(UINT8)(0x19),HuData,7))
			return TRUE;
		else
			return FALSE;
	}
	else
	{
		BYTE HuData[10];
		
		memset(HuData,0,sizeof(HuData));
		HuData[0] = (char)0xaa;
		HuData[1] = (char)0xbb;
		HuData[2] = 1 + ZHAO_ADD_LEN;
		HuData[3] = 0;

		HuData[4] = (UINT8)CrossNo;
		HuData[5] = (UINT8)(0x19);
		HuData[6] = 0;

		HuData[7] = (BYTE)Data;
		HuData[8] = (char)0x55;
		HuData[9] = (char)0x66;
		if(HuLcuPost(CrossNo,(UINT8)(0x19),HuData,10,CrossData[CrossNo - 1].CardNo))
			return TRUE;
		else
			return FALSE;
	}

	return TRUE;
}

/*******************************************************************************************
*	Function Name	: BOOL EndSlCountReqProc(UINT32 CrossNo,UINT8 CommIndex)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/09,25,2001
*	Global			: None
*	Note			: None
*******************************************************************************************/	
BOOL EndSlCountReqProc(UINT32 CrossNo,UINT8 CommIndex)
{
	if(CrossNo == 0 || CrossNo > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("EndSlCountReqProc(),Cross No Fatal Error!");
#endif
		return FALSE;
	}

	if(!UseLcu)
	{
		BYTE HuData[7];
		
		memset(HuData,0,sizeof(HuData));
		HuData[0] = (char)0xaa;
		HuData[1] = (UINT8)CrossNo;
		HuData[2] = (UINT8)(0x82);
		HuData[3] = 0;
		HuData[4] = 1;

		HuData[5] = (UINT8)0xfe;

		HuData[6] = FormHuChecksum(HuData,6);
		if(HuSerialPost(CrossNo,(UINT8)0x82,HuData,7))
		{
//			CrossData[CrossNo - 1].NeedRealData = 0;
			return TRUE;
		}
		else
			return FALSE;
	}
	else
	{
 		BYTE HuData[10];
		
		memset(HuData,0,sizeof(HuData));
		HuData[0] = (char)0xaa;
		HuData[1] = (char)0xbb;
		HuData[2] = 1 + ZHAO_ADD_LEN;
		HuData[3] = 0;
		HuData[4] = (UINT8)CrossNo;
		HuData[5] = (UINT8)(0x82);
		HuData[6] = 0;

		HuData[7] = (UINT8)0xfe;

		HuData[8] = (char)0x55;
		HuData[9] = (char)0x66;

		if(HuLcuPost(CrossNo,(UINT8)0x82,HuData,10,CrossData[CrossNo - 1].CardNo))
		{
//			CrossData[CrossNo - 1].NeedRealData = 0;
			return TRUE;
		}
		else
			return FALSE;
	}

	return TRUE;
}

/*******************************************************************************************
*	Function Name	: BOOL EndAllStateReqProc(UINT32 CrossNo,UINT8 CommIndex)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/09,25,2001
*	Global			: None
*	Note			: None
*******************************************************************************************/	
BOOL EndAllStateReqProc(UINT32 CrossNo,UINT8 CommIndex)
{
	if(CrossNo == 0 || CrossNo > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("EndAllStateReqProc(),Cross No Fatal Error!");
#endif
		return FALSE;
	}

	if(!UseLcu)
	{
		BYTE HuData[7];
		
		memset(HuData,0,sizeof(HuData));
		HuData[0] = (char)0xaa;
		HuData[1] = (UINT8)CrossNo;
		HuData[2] = (UINT8)(0x80);
		HuData[3] = 0;
		HuData[4] = 1;

		HuData[5] = (UINT8)0xfe;

		HuData[6] = FormHuChecksum(HuData,6);
		if(HuSerialPost(CrossNo,(UINT8)0x80,HuData,7))
		{
//			CrossData[CrossNo - 1].NeedRealData = 0;
			return TRUE;
		}
		else
			return FALSE;
	}
	else
	{
 		BYTE HuData[10];
		
		memset(HuData,0,sizeof(HuData));
		HuData[0] = (char)0xaa;
		HuData[1] = (char)0xbb;
		HuData[2] = 1 + ZHAO_ADD_LEN;
		HuData[3] = 0;
		HuData[4] = (UINT8)CrossNo;
		HuData[5] = (UINT8)(0x80);
		HuData[6] = 0;

		HuData[7] = (UINT8)0xfe;

		HuData[8] = (char)0x55;
		HuData[9] = (char)0x66;

		if(HuLcuPost(CrossNo,(UINT8)0x80,HuData,10,CrossData[CrossNo - 1].CardNo))
		{
//			CrossData[CrossNo - 1].NeedRealData = 0;
			return TRUE;
		}
		else
			return FALSE;
	}

	return TRUE;
}

/*******************************************************************************************
*	Function Name	: BOOL StartSlCountReqProc(BYTE Data,UINT32 CrossNo,UINT8 CommIndex)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/09,25,2001
*	Global			: None
*	Note			: None
*******************************************************************************************/	
BOOL StartSlCountReqProc(WORD Param,UINT32 CrossNo,UINT8 CommIndex)
{
	if(CrossNo == 0 || CrossNo > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("StartSlCountReqProc(),Cross No Fatal Error!");
#endif
		return FALSE;
	}

	if(!UseLcu)
	{
		BYTE HuData[7];
		
		memset(HuData,0,sizeof(HuData));
		HuData[0] = (char)0xaa;
		HuData[1] = (UINT8)CrossNo;
		HuData[2] = (UINT8)(0x82);
		HuData[3] = 0;
		HuData[4] = 1;

		HuData[5] = (BYTE)Param;

		HuData[6] = FormHuChecksum(HuData,6);
		if(HuSerialPost(CrossNo,(UINT8)0x82,HuData,7))
		{
//			CrossData[CrossNo - 1].NeedRealData = 1;
			return TRUE;
		}
		else
			return FALSE;
	}
	else
	{
 		BYTE HuData[10];
		
		memset(HuData,0,sizeof(HuData));
		HuData[0] = (char)0xaa;
		HuData[1] = (char)0xbb;
		HuData[2] = 1 + ZHAO_ADD_LEN;
		HuData[3] = 0;
		HuData[4] = (UINT8)CrossNo;
		HuData[5] = (UINT8)(0x82);
		HuData[6] = 0;

		HuData[7] = (BYTE)Param;

		HuData[8] = (char)0x55;
		HuData[9] = (char)0x66;

		if(HuLcuPost(CrossNo,(UINT8)0x82,HuData,10,CrossData[CrossNo - 1].CardNo))
		{
//			CrossData[CrossNo - 1].NeedRealData = 1;
			return TRUE;
		}
		else
			return FALSE;
	}

	return TRUE;
}

/*******************************************************************************************
*	Function Name	: BOOL QryAllStateReqProc(BYTE Data,UINT32 CrossNo,UINT8 CommIndex)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/08,06,2001
*	Global			: None
*	Note			: None
*******************************************************************************************/	
BOOL QryAllStateReqProc(WORD Param,UINT32 CrossNo,UINT8 CommIndex)
{
	if(CrossNo == 0 || CrossNo > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("CurSchemeReqProc(),Cross No Fatal Error!");
#endif
		return FALSE;
	}

	if(!UseLcu)
	{
		BYTE HuData[7];
		
		memset(HuData,0,sizeof(HuData));
		HuData[0] = (char)0xaa;
		HuData[1] = (UINT8)CrossNo;
		HuData[2] = (UINT8)(0x80);
		HuData[3] = 0;
		HuData[4] = 1;

		HuData[5] = (BYTE)Param;

		HuData[6] = FormHuChecksum(HuData,6);
		if(HuSerialPost(CrossNo,(UINT8)0x80,HuData,7))
		{
//			CrossData[CrossNo - 1].NeedRealData = 1;
			return TRUE;
		}
		else
			return FALSE;
	}
	else
	{
 		BYTE HuData[10];
		
		memset(HuData,0,sizeof(HuData));
		HuData[0] = (char)0xaa;
		HuData[1] = (char)0xbb;
		HuData[2] = 1 + ZHAO_ADD_LEN;
		HuData[3] = 0;
		HuData[4] = (UINT8)CrossNo;
		HuData[5] = (UINT8)(0x80);
		HuData[6] = 0;

		HuData[7] = (BYTE)Param;

		HuData[8] = (char)0x55;
		HuData[9] = (char)0x66;

		if(HuLcuPost(CrossNo,(UINT8)0x80,HuData,10,CrossData[CrossNo - 1].CardNo))
		{
//			CrossData[CrossNo - 1].NeedRealData = 1;
			return TRUE;
		}
		else
			return FALSE;
	}

	return TRUE;
}

/*****************************************************************************************************
*	Function Name	: BOOL QryLampStateReqProc(IMPL_TYPE *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/08,06,2001
*	Global			: None
*	Note			: None
******************************************************************************************************/	
BOOL QryLampStateReqProc(IMPL_TYPE *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex)
{
	return TRUE;
}

/*****************************************************************************************************
*	Function Name	: BOOL QryCheckNumReqProc(IMPL_TYPE *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/08,06,2001
*	Global			: None
*	Note			: None
******************************************************************************************************/	
BOOL QryCheckNumReqProc(IMPL_TYPE *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex)
{
	return TRUE;
}

/*****************************************************************************************************
*	Function Name	: BOOL QryPollutingReqProc(IMPL_TYPE *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/08,06,2001
*	Global			: None
*	Note			: None
******************************************************************************************************/	
BOOL QryPollutingReqProc(IMPL_TYPE *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex)
{
	return TRUE;
}

/*****************************************************************************************************
*	Function Name	: BOOL OprTestReqProc(IMPL_TYPE *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/08,06,2001
*	Global			: None
*	Note			: None
******************************************************************************************************/	
BOOL OprTestReqProc(IMPL_TYPE *Data,UINT32 CmdNo,UINT32 CrossNo,UINT8 CommIndex)
{
	return TRUE;
}

/********************************************************************************
*	Function Name	: BOOL IsSanlian(UINT8 bCrossNo)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/08,06,2001
*	Global			: None
*	Note			: None
*********************************************************************************/	
BOOL IsSanlian(UINT8 bCrossNo)
{
	if(bCrossNo == 0 || bCrossNo > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("IsSanlian(),bCrossNo == 0!");
#endif
		return FALSE;
	}

	if(CrossData[bCrossNo - 1].Type == 1)
		return TRUE;
	else
		return FALSE;
}

/********************************************************************************
*	Function Name	: BOOL IsJingSan(UINT8 bCrossNo)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/02,20,2002
*	Global			: None
*	Note			: None
*********************************************************************************/	
BOOL IsJingSan(UINT8 bCrossNo)
{
	if(bCrossNo == 0 || bCrossNo > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("IsJingSan(),bCrossNo == 0!");
#endif
		return FALSE;
	}

	if(CrossData[bCrossNo - 1].Type == 2)
		return TRUE;
	else
		return FALSE;
}

/********************************************************************************
*	Function Name	: BOOL NowCanSend(UINT8 Port)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/08,08,2001
*	Global			: None
*	Note			: None
*********************************************************************************/	
BOOL NowCanSend(UINT8 Port)
{
	if(Port == 0 || Port > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("NowCanSend(),Port == 0!");
#endif
		return FALSE;
	}

	if(CrossData[Port - 1].WaitSendCount <= 0)
		return TRUE;
	else
		return FALSE;
}

/********************************************************************************
*	Function Name	: void DeleteDownBuf((UINT8 port)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/08,08,2001
*	Global			: None
*	Note			: None
*********************************************************************************/	
void DeleteDownBufHead(UINT8 Port)
{
	char *MsgHeadPtr;
	BYTE i;
	int MsgLen;	

	if(Port == 0 || Port > MAX_CROSS_NUM)
	{
		DebugWindow("DeleteDownBufHead(),Port == 0!");
		return;
	}

	i = Port - 1;
	if(CrossData[i].WaitHead < 0 || CrossData[i].WaitHead >= MAX_RESEND_NUM)
	{
#ifdef TEST
		DebugWindow("DeleteDownBuf(),Exceeding Max_Resend_Num!");
#endif
		ClearSanlianSendQueue(Port);
		return;
	}	

	if(CrossData[i].WaitSendCount <= 0 || CrossData[i].WaitSendCount > MAX_RESEND_NUM)
	{
#ifdef TEST
		DebugWindow("DeleteDownBuf(),WaitSendCount error!");
#endif
		ClearSanlianSendQueue(Port);
		return;
	}	

	MsgHeadPtr = (char *)CrossData[i].DownMsgBuf[CrossData[i].WaitHead].hMsgBuf;
	MsgLen = CrossData[i].DownMsgBuf[CrossData[i].WaitHead].MsgLen;

	if((MsgLen > MAX_MSG_LEN) || (MsgLen < 0))
	{
#ifdef TEST
		DebugWindow("DeleteDownBuf(),Out of Buffer Length!");
#endif
		ClearSanlianSendQueue(Port);
		return;
	}
	
	if(MsgHeadPtr == NULL)
	{
#ifdef TEST
		DebugWindow("DeleteDownBuf(),Msg Ptr is NULL!");
#endif
		ClearSanlianSendQueue(Port);
		return;
	}

	if(MsgHeadPtr != NULL)
		HeapFree(MyProcessHeapHandle, 0, MsgHeadPtr);

	//added 08,14,2001
	CrossData[i].DownMsgBuf[CrossData[i].WaitHead].hMsgBuf = NULL;
	CrossData[i].DownMsgBuf[CrossData[i].WaitHead].MsgLen = 0;
	//add end

	CrossData[i].WaitSendCount--;
	CrossData[i].WaitHead = (++CrossData[i].WaitHead) % MAX_RESEND_NUM;

	return;
}

/********************************************************************************
*	Function Name	: void SetBuf2Ready(UINT8 Port)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/08,08,2001
*	Global			: None
*	Note			: None
*********************************************************************************/	
void SetBuf2Ready(UINT8 Port,UINT8 Head)
{
	UINT8 Index;

	if(Port == 0 || Port > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("SetBuf2Busy(),Port == 0!");
#endif
		return;
	}

	if(Head)
		Index = CrossData[Port - 1].WaitHead;
	else
		Index = ( CrossData[Port - 1].WaitEnd + (MAX_RESEND_NUM - 1) ) % MAX_RESEND_NUM;

	CrossData[Port - 1].DownMsgBuf[Index].Status = BUF_IDLE_STATUS;
	CrossData[Port - 1].DownMsgBuf[Index].SendNum = 0;
	CrossData[Port - 1].DownMsgBuf[Index].Timeout = MAX_BUF_WAIT;

	return;
}

/********************************************************************************
*	Function Name	: void SetBuf2Busy(UINT8 Port,UINT8 Head)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/08,08,2001
*	Global			: None
*	Note			: None
*********************************************************************************/	
void SetBuf2Busy(UINT8 Port,UINT8 Head)
{
	UINT8 Index;

	if(Port == 0 || Port > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("SetBuf2Busy(),Port == 0!");
#endif
		return;
	}

	if(Head)
		Index = CrossData[Port - 1].WaitHead;
	else
		Index = ( CrossData[Port - 1].WaitEnd + (MAX_RESEND_NUM - 1) ) % MAX_RESEND_NUM;

	CrossData[Port - 1].DownMsgBuf[Index].Status = BUF_SEND_STATUS;
	CrossData[Port - 1].DownMsgBuf[Index].SendNum = 1;
	CrossData[Port - 1].DownMsgBuf[Index].Timeout = MAX_HU_WAIT;//80以上的命令应立即返回值

	return;
}

/********************************************************************************
*	Function Name	: void SetBuf2Que(UINT8 Port)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/08,08,2001
*	Global			: None
*	Note			: None
*********************************************************************************/	
void SetBuf2Que(UINT8 Port,UINT8 Head)
{
	UINT8 Index;

	if(Port == 0 || Port > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("SetBuf2Busy(),Port == 0!");
#endif
		return;
	}

	if(Head)
		Index = CrossData[Port - 1].WaitHead;
	else
		Index = ( CrossData[Port - 1].WaitEnd + (MAX_RESEND_NUM - 1) ) % MAX_RESEND_NUM;

	CrossData[Port - 1].DownMsgBuf[Index].Status = BUF_QUE_STATUS;
	CrossData[Port - 1].DownMsgBuf[Index].SendNum = 0;
	CrossData[Port - 1].DownMsgBuf[Index].Timeout = MAX_BUF_WAIT;

	return;
}

/********************************************************************************
*	Function Name	: BOOL SanlianInQue(UINT8 Index,UINT8 CmdNo,char *hMsg,UINT16 hMsgLen)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/08,08,2001
*	Global			: None
*	Note			: None
*********************************************************************************/	
BOOL SanlianInQue(UINT8 Port,UINT8 CmdNo,char *hMsg,UINT16 hMsgLen)
{
	BYTE i;
	char * MsgHeadPtr;

	if(Port == 0 || Port > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("SanlianInQue(),Port == 0!");
#endif
		return FALSE;
	}

	if(hMsg == NULL)
	{
#ifdef TEST
		DebugWindow("SanlianInQue(),Input Msg Body Error!");
#endif
		return FALSE;
	}

	if(hMsgLen <= 0 || hMsgLen > MAX_MSG_LEN)
	{
#ifdef TEST
		DebugWindow("SanlianInQue(),Input Msg Length Error!");
#endif
		return FALSE;
	}

	i = Port - 1;

	MsgHeadPtr = (char *)HeapAlloc(MyProcessHeapHandle, HEAP_ZERO_MEMORY, hMsgLen);
	if(MsgHeadPtr == NULL)
	{
#ifdef TEST
		DebugWindow("SanlianInQue(),Alloc Memory Error!");
#endif
		return FALSE;
	}

	memcpy(MsgHeadPtr, hMsg, hMsgLen);

	if(CrossData[i].WaitSendCount + 1 >= MAX_RESEND_NUM)
	{
#ifdef TEST
		DebugWindow("SanlianInQue(),Exceeding Max_Resend_Num!");
#endif
		if(MsgHeadPtr != NULL)
		{
			HeapFree(MyProcessHeapHandle,0,MsgHeadPtr);//added 03,26,2002
			MsgHeadPtr = NULL;
		}

		return FALSE;
	}

	if(CrossData[i].WaitEnd < 0 || CrossData[i].WaitEnd >= MAX_RESEND_NUM)
	{
#ifdef TEST
		DebugWindow("SanlianInQue(),Global Data Error!");
#endif
		ClearSanlianSendQueue(Port);//(i + 1);
		return FALSE;
	}	

	CrossData[i].DownMsgBuf[CrossData[i].WaitEnd].Status = BUF_IDLE_STATUS;
	CrossData[i].DownMsgBuf[CrossData[i].WaitEnd].SendNum = 0;
	CrossData[i].DownMsgBuf[CrossData[i].WaitEnd].Timeout = MAX_BUF_WAIT;

	CrossData[i].DownMsgBuf[CrossData[i].WaitEnd].hMsgBuf = MsgHeadPtr;
	CrossData[i].DownMsgBuf[CrossData[i].WaitEnd].MsgLen = hMsgLen;
	CrossData[i].DownMsgBuf[CrossData[i].WaitEnd].CmdNo = CmdNo;

	CrossData[i].WaitSendCount++; 
	CrossData[i].WaitEnd = (++CrossData[i].WaitEnd) % MAX_RESEND_NUM;
	
	return TRUE;
}

/********************************************************************************
*	Function Name	: void ClearSanlianSendQueue(BYTE Port)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/08,08,2001
*	Global			: None
*	Note			: None
*********************************************************************************/	
void ClearSanlianSendQueue(BYTE Port)
{	
	BYTE i,j;

	if(Port == 0 || Port > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("ClearSanlianSendQueue(),Port == 0!");
#endif
		return;
	}

	i = Port - 1;
	if(CrossData[i].WaitSendCount <= 0)
	{
		CrossData[i].WaitSendCount = 0;
		CrossData[i].WaitHead = 0;
		CrossData[i].WaitEnd = 0;
//		CrossData[i].BufStatus = SENDBUF_READY_STATUS;

		return;
	}

	for(j = CrossData[i].WaitSendCount; j > 0; j--)
	{
		if(CrossData[i].DownMsgBuf[CrossData[i].WaitHead].hMsgBuf != NULL)
			HeapFree(MyProcessHeapHandle,0,CrossData[i].DownMsgBuf[CrossData[i].WaitHead].hMsgBuf);

		CrossData[i].DownMsgBuf[CrossData[i].WaitHead].hMsgBuf = NULL;
		CrossData[i].DownMsgBuf[CrossData[i].WaitHead].MsgLen = 0;

		CrossData[i].WaitHead = (++CrossData[i].WaitHead) % MAX_RESEND_NUM;
	}

	CrossData[i].WaitSendCount = 0;
	CrossData[i].WaitHead = 0;
	CrossData[i].WaitEnd = 0;
//	CrossData[i].BufStatus = SENDBUF_READY_STATUS;

	return;
}


/********************************************************************************
*	Function Name	: UINT8 OnlineProc(UINT8 bCrossNo)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,21,2001
*	Global			: None
*	Note			: None
*********************************************************************************/	
UINT8 OnlineProc(UINT8 bCrossNo)
{
	UINT8 StepNo;
	int i;

	if(bCrossNo == 0 || bCrossNo > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("OnlineProc(),bCrossNo == 0!");
#endif
		return 30;
	}

	if(CrossData[bCrossNo - 1].Status != OFFLINE_STATUS)
	{
#ifdef TEST
		DebugWindow("OnlineProc(),Cross Status Invalid!");
#endif
		return 35;
	}

	//check StepTable in order to run with it
	StepNo = CrossData[bCrossNo - 1].TotalStepNo;
	if(StepNo < 4 || StepNo > MAX_STEP_NUM)
	{
#ifdef TEST
		DebugWindow("OnlineProc(),StepNo Out of Range!");
#endif
		return 31;
	}

	for(i = 0;i < StepNo;i++)
	{
		if(CrossData[bCrossNo - 1].StepTable[i] <= 0 || CrossData[bCrossNo - 1].StepTable[i] > 180)//???????
		{
#ifdef TEST
			DebugWindow("OnlineProc(),StepValue Invalid!");
#endif
			return 32;
		}
	}
		
	CrossData[bCrossNo - 1].AulStatus = OFFLINE_STATUS;
	CrossData[bCrossNo - 1].Status = TRANSIT_STATUS;

	return 0;
}

/********************************************************************************
*	Function Name	: UINT8 OfflineProc(UINT8 bCrossNo)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,21,2001
*	Global			: None
*	Note			: None
*********************************************************************************/	
UINT8 OfflineProc(UINT8 bCrossNo)
{
	if(bCrossNo == 0 || bCrossNo > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("OfflineProc(),bCrossNo == 0!");
#endif
		return 30;
	}

	if(CrossData[bCrossNo - 1].Status == OFFLINE_STATUS)
		return 0;

	if(CrossData[bCrossNo - 1].Status == IDLE_STATUS || CrossData[bCrossNo - 1].Status == INITIAL_STATUS \
			|| CrossData[bCrossNo - 1].Status == FORCED_STATUS)
	{
#ifdef TEST
		DebugWindow("OfflineProc(),Cross Status Invalid!");
#endif
		return 35;
	}

	CrossData[bCrossNo - 1].InitalWait = 20;//send 30 times

	if(CrossData[bCrossNo - 1].Status == TRANSIT_STATUS)
	{
		CrossData[bCrossNo - 1].Status = OFFLINE_STATUS;
	}
	else if(CrossData[bCrossNo - 1].Status == NORMAL_STATUS)
	{
		CrossData[bCrossNo - 1].Status = OFFLINE_STATUS;
	}

	//Status changed,Clear VIP data 08,15,2001
	if(CrossData[bCrossNo - 1].BaseCross != 0)
	{
		CrossData[bCrossNo - 1].SynErrCount = 0;
		CrossData[bCrossNo - 1].BaseCross = 0;
		CrossData[bCrossNo - 1].DeltaPhi = 0;
	}

	ClearSynFlag(bCrossNo);

	return 0;
}

/********************************************************************************
*	Function Name	: UINT8 JsNextstepReqProc(UINT8 bCrossNo,UINT8 Data)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/09,25,2001
*	Global			: None
*	Note			: None
*********************************************************************************/	
UINT8 JsNextstepReqProc(UINT8 bCrossNo,UINT8 Data)
{
	if(bCrossNo == 0 || bCrossNo > MAX_CROSS_NUM)
	{
		DebugWindow("JsNextstepReqProc(),bCrossNo == 0!");
		return 30;
	}

	if(CrossData[bCrossNo - 1].ManCtrFlag != 1)
	{
		Rep_CrossCtrStatus CrossCtrStatus;

		CrossData[bCrossNo - 1].ManCtrFlag = 2;

		//send to wu
		memset(&CrossCtrStatus,0,sizeof(Rep_CrossCtrStatus));
		CrossCtrStatus.byFlag = MsgFlag;
		CrossCtrStatus.MsgType = CM_CONTROLSTATUS;

		strcpy(CrossCtrStatus.SourceIP,HostIp);
		strcpy(CrossCtrStatus.SourceID,HostId);
		strcpy(CrossCtrStatus.TargetIP,SvrIp);
		strcpy(CrossCtrStatus.TargetID,SvrId);

		CrossCtrStatus.iCrossNo = CrossData[bCrossNo - 1].lkbh;
		CrossCtrStatus.iLength = 0;
		CrossCtrStatus.iReserved1 = CrossData[bCrossNo - 1].ManCtrFlag;

		CrossCtrStatus.byCheckSum = FormCheckSum((char *)&CrossCtrStatus);

		SendDataEx((char *)&CrossCtrStatus,sizeof(Rep_CrossCtrStatus),(UINT8)1);
	}

	if(CrossData[bCrossNo - 1].BaseCross != 0)
	{
		CrossData[bCrossNo - 1].SynErrCount = 0;
		CrossData[bCrossNo - 1].BaseCross = 0;
		CrossData[bCrossNo - 1].DeltaPhi = 0;
	}

	ClearSynFlag(bCrossNo);

	return 0;
}

/********************************************************************************
*	Function Name	: UINT8 SetStepOnProc(UINT8 bCrossNo,CSetStepOn *Data)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,24,2001
*	Global			: None
*	Note			: None
*********************************************************************************/	
UINT8 SetStepOnProc(UINT8 bCrossNo,CSetStepOn *Data)
{
	UINT16 LastingTime;

	if(bCrossNo == 0 || bCrossNo > MAX_CROSS_NUM)
	{
		DebugWindow("SetStepOnProc(),bCrossNo == 0!");
		return 30;
	}

	if(Data == NULL)
	{
		DebugWindow("SetStepOnProc(),Data == NULL!");
		return 36;
	}

	if(CrossData[bCrossNo - 1].Status == IDLE_STATUS || CrossData[bCrossNo - 1].Status == INITIAL_STATUS \
		|| CrossData[bCrossNo - 1].Status == OFFLINE_STATUS)
	{
		DebugWindow("SetStepOnProc(),Cross Status Invalid!");
		return 35;
	}

	if(Data->ForcedStep == 0 || Data->ForcedStep > CrossData[bCrossNo - 1].TotalStepNo)
	{
		DebugWindow("SetStepOnProc(),Specified Step Invalid!");
		return 37;
	}

	if(CrossData[bCrossNo - 1].Status == FORCED_STATUS)
	{
		if(CrossData[bCrossNo - 1].ForcedStep == Data->ForcedStep - 1)
			return 0;
		else
			return 38;//re-forcedstep,rejected
	}

	LastingTime = Data->MaxTime * 60 + 1000;//we'll adjust it later for 15s
	if(Data->MaxTime == 0)
		LastingTime = 30 * 60 + 1000;//default time;

//	CrossData[bCrossNo - 1].InitalWait = Data->MaxTime;//added 03,20,2002
//	if(Data->MaxTime == 0)
//		CrossData[bCrossNo - 1].InitalWait = 30;//added 03,20,2002
	CrossData[bCrossNo - 1].InitalWait = 1;//only a flag

	if(CrossData[bCrossNo - 1].Status == NORMAL_STATUS)
	{
		CrossData[bCrossNo - 1].ForcedStep = Data->ForcedStep - 1;
		CrossData[bCrossNo - 1].ForecedLeftTime = LastingTime;
		CrossData[bCrossNo - 1].AulStatus = NORMAL_STATUS;
		CrossData[bCrossNo - 1].Status = FORCED_STATUS;
	}
	else if(CrossData[bCrossNo - 1].Status == TRANSIT_STATUS)
	{
		CrossData[bCrossNo - 1].ForcedStep = Data->ForcedStep - 1;
		CrossData[bCrossNo - 1].ForecedLeftTime = LastingTime;
		CrossData[bCrossNo - 1].AulStatus = TRANSIT_STATUS;
		CrossData[bCrossNo - 1].Status = FORCED_STATUS;
	}

	//Status changed,Clear VIP data 08,15,2001
	if(CrossData[bCrossNo - 1].BaseCross != 0)
	{
		CrossData[bCrossNo - 1].SynErrCount = 0;
		CrossData[bCrossNo - 1].BaseCross = 0;
		CrossData[bCrossNo - 1].DeltaPhi = 0;
	}

	ClearSynFlag(bCrossNo);

	return 0;
}

/********************************************************************************
*	Function Name	: UINT8 SetStepOffProc(UINT8 bCrossNo,CSetStepOff *Data)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,21,2001
*	Modify			: yincy/09,19,2001
*	Global			: None
*	Note			: None
*********************************************************************************/	
UINT8 SetStepOffProc(UINT8 bCrossNo,CSetStepOff *Data)
{
//	UINT8 ForcedStep;

	if(bCrossNo == 0 || bCrossNo > MAX_CROSS_NUM)
	{
		DebugWindow("SetStepOffProc(),bCrossNo == 0!");
		return 30;
	}

/*	if(Data == NULL)
	{
		DebugWindow("SetStepOffProc(),Data == NULL!");
		return 36;
	}
*/
	if(CrossData[bCrossNo - 1].Status != FORCED_STATUS)
	{
		DebugWindow("SetStepOffProc(),Cross Status Invalid!");
		return 35;
	}

/*	
	ForcedStep = Data->ForcedStep;
	if(ForcedStep == 0 || ForcedStep > CrossData[bCrossNo - 1].TotalStepNo \
		|| CrossData[bCrossNo - 1].ForcedStep != ForcedStep - 1)
	{
		DebugWindow("SetStepOffProc(),Specified Step Invalid!");
		return 37;
	}
*/
	SetStepoffAdjust(bCrossNo);

	CrossData[bCrossNo - 1].ForcedStep = 0;
	CrossData[bCrossNo - 1].ForecedLeftTime = 0;//added 09,19,2001???????
	CrossData[bCrossNo - 1].Status = TRANSIT_STATUS;

	CrossData[bCrossNo - 1].InitalWait = 0;//added 03,20,2002

	return 0;
}

/********************************************************************************
*	Function Name	: UINT8 StartRealProc(UINT8 bCrossNo)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,21,2001
*	Global			: None
*	Note			: None
*********************************************************************************/	
UINT8 StartRealProc(UINT8 bCrossNo)
{
	UINT8 Num;

	if(bCrossNo == 0 || bCrossNo > MAX_CROSS_NUM)
	{
		DebugWindow("StartRealProc(),bCrossNo == 0!");
		return 30;
	}

	if(CrossData[bCrossNo - 1].Status == IDLE_STATUS)
	{
		DebugWindow("StartRealProc(),Cross Status Not Match!");
		return 40;
	}

	if(CrossData[bCrossNo - 1].Status == INITIAL_STATUS \
			&& CrossData[bCrossNo - 1].CurStep > MAX_STEP_NUM)
	{
		DebugWindow("StartRealProc(),Cross May Disconnected!");
		return 41;
	}

	Num = GetRealtimeNum();
	if(Num > MAX_REALTIME_NUM)
	{
		DebugWindow("StartRealProc(),Too Many Real Data Cross Num!");
		return 39;
	}

	if(CrossData[bCrossNo - 1].NeedRealData == 0)
		CrossData[bCrossNo - 1].NeedRealData = 1;

	return 0;
}

/********************************************************************************
*	Function Name	: UINT8 StopRealProc(UINT8 bCrossNo)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,21,2001
*	Global			: None
*	Note			: None
*********************************************************************************/	
UINT8 StopRealProc(UINT8 bCrossNo)
{
	if(bCrossNo == 0 || bCrossNo > MAX_CROSS_NUM)
	{
		DebugWindow("StopRealProc(),bCrossNo == 0!");
		return 30;
	}

	if(CrossData[bCrossNo - 1].Status == IDLE_STATUS)
	{
		DebugWindow("StopRealProc(),Cross Status Not Match!");
		return 40;
	}

	if(CrossData[bCrossNo - 1].Status == INITIAL_STATUS \
			&& CrossData[bCrossNo - 1].CurStep > MAX_STEP_NUM)
	{
		DebugWindow("StopRealProc(),Cross May Disconnected!");
		return 41;
	}

	if(CrossData[bCrossNo - 1].NeedRealData > 0)
		CrossData[bCrossNo - 1].NeedRealData = 0;
	else
		return 39;

	return 0;
}

/********************************************************************************
*	Function Name	: UINT8 GetRealtimeNum(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,23,2001
*	Global			: None
*	Note			: None
*********************************************************************************/	
UINT8 GetRealtimeNum(void)
{
	int i;
	UINT8 num = 0;

	for(i = 0;i < MAX_CROSS_NUM;i++)
	{
		if(CrossData[i].Status == IDLE_STATUS || CrossData[i].Status == INITIAL_STATUS)
		{
			//a remedy
			if(CrossData[i].NeedRealData > 0)
				CrossData[i].NeedRealData = 0;
			continue;
		}

		if(CrossData[i].NeedRealData > 0)
			num++;
	}

	return num;
}

/********************************************************************************
*	Function Name	: UINT8 FirstStepParameterProc(UINT8 bCrossNo,CStepsDown *Data)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,25,2001
*	Modify			: yincy/08,08,2001
*   Modify		    : yincy/11,11,2001
*   Modify		    : yincy/02,27,2002
*   Modify		    : yincy/04,16,2002
*   Modify		    : yincy/04,17,2002
*	Global			: None
*	Note			: None
*********************************************************************************/	
UINT8 FirstStepParameterProc(UINT8 bCrossNo,CStepsDown *Data)
{
	int i;
	int j;
	UINT8 StepNo;
	UINT8 BaseCrossNo;
	int EffectDelt;
	int CycleLen;
	UINT8 byDetectorNum;
#ifdef TEST
	char disp[200];
#endif

	if(bCrossNo == 0 || bCrossNo > MAX_CROSS_NUM)
	{
		DebugWindow("FirstStepParameterProc(),bCrossNo == 0!");
		return 30;
	}

	if(Data == NULL)
	{
		DebugWindow("FirstStepParameterProc(),Data == NULL!");
		return 36;
	}

	if(CrossData[bCrossNo - 1].Status == TRANSIT_STATUS || CrossData[bCrossNo - 1].Status == NORMAL_STATUS \
		|| CrossData[bCrossNo - 1].Status == FORCED_STATUS || CrossData[bCrossNo - 1].Status == OFFLINE_STATUS)
	{
		DebugWindow("FirstStepParameterProc(),Cross Status Invalid!");
		return 35;
	}

	StepNo = Data->byStep;
	if(StepNo < 4 || StepNo > MAX_STEP_NUM)
	{
		DebugWindow("FirstStepParameterProc(),StepNo Out of Range!");
		return 31;
	}

	for(i = 0;i < StepNo;i++)
	{
		if(Data->iTime[i] <= 0 || Data->iTime[i] > 180)//???????
		{
			DebugWindow("FirstStepParameterProc(),StepValue invalid!");
			return 32;
		}
	}

	byDetectorNum = Data->byDetectorNum;
	if(byDetectorNum == 0 || byDetectorNum > MaxDetectorNum)
	{
		DebugWindow("FirstStepParameterProc(),byDetectorNum invalid!");
		return 33;
	}

/*	for(i = 0;i < byDetectorNum;i++)
	{
		if(Data->byDetectorGreen1[i] == 0 || Data->byDetectorGreen1[i] > MAX_STEP_NUM \
			|| Data->byDetectorGreen2[i] == 0 || Data->byDetectorGreen2[i] > MAX_STEP_NUM \
			|| Data->byDetectorGreen1[i] > Data->byDetectorGreen2[i])
		{
			DebugWindow("FirstStepParameterProc(),Green Lamp Value Invalid!");
			return 34;
		}
	}
*/	

//set data now
	CycleLen = GetCycleLenEx(Data);
	if(CycleLen <= 0 || CycleLen >= 10000)
	{
		DebugWindow("FirstStepParameterProc(),Fatal Error---Cycle Length!");
		return 60;
	}

	EffectDelt = (Data->iOffset + CycleLen) % CycleLen;
/*	if(Data->iCoCrossNo <= 0 || Data->iCoCrossNo >= MAX_CROSS_NUM)
	{
//		BaseCrossNo = 0;
		CrossData[bCrossNo - 1].BaseCross = 0;
		CrossData[bCrossNo - 1].DeltaPhi = 0;
	}
	else
	{
*/	
	if(Data->iCoCrossNo > 0)
		BaseCrossNo = FindCrossNo(Data->iCoCrossNo);
	else
		BaseCrossNo = 0;
	if( (BaseCrossNo <= 0 || BaseCrossNo >= MAX_CROSS_NUM) || BaseCrossNo == bCrossNo )
	{
#ifdef TEST
		DebugWindow("FirstStepParameterProc(),Set BaseCrossNo=0");
#endif
//		BaseCrossNo = 0;
		CrossData[bCrossNo - 1].BaseCross = 0;
		CrossData[bCrossNo - 1].DeltaPhi = 0;
	}
	else if(CrossData[bCrossNo - 1].Type == 2 && CrossData[BaseCrossNo - 1].Type == 2)
	{
		CrossData[bCrossNo - 1].BaseCross = BaseCrossNo;
		CrossData[bCrossNo - 1].DeltaPhi = EffectDelt;
	}
	else
	{
		CrossData[bCrossNo - 1].BaseCross = 0;
		CrossData[bCrossNo - 1].DeltaPhi = 0;
	}
//	}

	CrossData[bCrossNo - 1].SynErrCount = 0;
	CrossData[bCrossNo - 1].TotalStepNo = StepNo;
	memset(CrossData[bCrossNo - 1].jcqbh,0,MAX_JCQ_NUM);
	memset(CrossData[bCrossNo - 1].jcqStartGreenStep,0,4 * MAX_JCQ_NUM);
	memset(CrossData[bCrossNo - 1].jcqEndGreenStep,0,4 * MAX_JCQ_NUM);

	for(i = 0;i < byDetectorNum;i++)
		CrossData[bCrossNo - 1].jcqbh[i] = Data->byDetectorNo[i];

	for(i = 0;i < 4;i++)
	{
		for(j = 0;j < byDetectorNum;j++)
		{
			CrossData[bCrossNo - 1].jcqStartGreenStep[i][j] = Data->byDetectorGreen1[i][j];
			CrossData[bCrossNo - 1].jcqEndGreenStep[i][j] = Data->byDetectorGreen2[i][j];
	#ifdef TEST
			if(Data->byDetectorGreen1[i][j] != 0)
			{
				memset(disp,0,200);
				sprintf(disp,"port=%3d,GreenStart[%d]=%2d,GreenEnd[%d]=%2d",bCrossNo,i + 1,\
					Data->byDetectorGreen1[i][j],i + 1,Data->byDetectorGreen2[i][j]);
				DebugWindow(disp);
			}
	#endif
		}
	}

	//wait for cross data
	memset(CrossData[bCrossNo - 1].StepTable,0,MAX_STEP_NUM * sizeof(UINT16));
	memset(CrossData[bCrossNo - 1].MinStepTable,0,MAX_STEP_NUM * sizeof(UINT16));
	memset(CrossData[bCrossNo - 1].MaxStepTable,0,MAX_STEP_NUM * sizeof(UINT16));
	for(i = 0;i < StepNo;i++)
	{
		CrossData[bCrossNo - 1].StepTable[i] = Data->iTime[i];
		if(Data->iTime[i] >= Data->MinStepTime[i] && Data->iTime[i] <= Data->MaxStepTime[i])
		{
			CrossData[bCrossNo - 1].MinStepTable[i] = Data->MinStepTime[i];
			CrossData[bCrossNo - 1].MaxStepTable[i] = Data->MaxStepTime[i];
		}
#ifdef TEST
		memset(disp,0,200);
		sprintf(disp,"port=%3d,Step[%02d]=%d",bCrossNo,i,Data->iTime[i]);
		DebugWindow(disp);
#endif
	}

	if(CrossData[bCrossNo - 1].Status != INITIAL_STATUS)
	{
		CrossData[bCrossNo - 1].Status = INITIAL_STATUS;
		CrossData[bCrossNo - 1].InitalWait = 3;
		CrossData[bCrossNo - 1].CurStep = MAX_STEP_NUM + 1;
		CrossData[bCrossNo - 1].NeedStep = MAX_STEP_NUM + 2;
		CrossData[bCrossNo - 1].CurCtrMode = 0xff;//D当前的控制模式
		CrossData[bCrossNo - 1].CurCValue = 0xff;//C当前的C值
		CrossData[bCrossNo - 1].Left5Min = 5 * 60;
	}

	return 0;
}

/********************************************************************************
*	Function Name	: UINT8 StepParameterProc(UINT8 bCrossNo,CStepsDown *Data)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,20,2001
*	Modify			: yincy/08,08,2001
*   Modify		    : yincy/11,11,2001
*   Modify		    : yincy/02,27,2002
*   Modify		    : yincy/04,16,2002
*   Modify		    : yincy/04,17,2002
*	Global			: None
*	Note			: None
*********************************************************************************/	
UINT8 StepParameterProc(UINT8 bCrossNo,CStepsDown *Data)
{
	int i;
	int j;
	UINT8 StepNo;
	UINT8 BaseCrossNo;
	int EffectDelt;
	int CycleLen;
	UINT8 byDetectorNum;
#ifdef TEST
	char disp[200];
#endif

	if(bCrossNo == 0 || bCrossNo > MAX_CROSS_NUM)
	{
		DebugWindow("StepParameterProc(),bCrossNo == 0!");
		return 30;
	}

	if(Data == NULL)
	{
		DebugWindow("StepParameterProc(),Data == NULL!");
		return 36;
	}

//deleted 04,03,2002
/*
	if(CrossData[bCrossNo - 1].Status == IDLE_STATUS || CrossData[bCrossNo - 1].Status == INITIAL_STATUS \
		|| CrossData[bCrossNo - 1].Status == FORCED_STATUS || CrossData[bCrossNo - 1].Status == OFFLINE_STATUS)
	{
		DebugWindow("StepParameterProc(),Cross Status Invalid!");
		return 35;
	}
*/
//deleted end

	StepNo = Data->byStep;
	if(StepNo < 4 || StepNo > MAX_STEP_NUM)
	{
		DebugWindow("StepParameterProc(),StepNo Out of Range!");
		return 31;
	}

	for(i = 0;i < StepNo;i++)
	{
		if(Data->iTime[i] <= 0 || Data->iTime[i] > 180)//???????
		{
#ifdef TEST
			DebugWindow("StepParameterProc(),StepValue invalid!");
#endif
			return 32;
		}
	}

	byDetectorNum = Data->byDetectorNum;
	if(byDetectorNum == 0 || byDetectorNum > MaxDetectorNum)
	{
		DebugWindow("StepParameterProc(),byDetectorNum invalid!");
		return 33;
	}

/*	for(i = 0;i < byDetectorNum;i++)
	{
		if(Data->byDetectorGreen1[i] == 0 || Data->byDetectorGreen1[i] > MAX_STEP_NUM \
			|| Data->byDetectorGreen2[i] == 0 || Data->byDetectorGreen2[i] > MAX_STEP_NUM \
			|| Data->byDetectorGreen1[i] > Data->byDetectorGreen2[i])
		{
			DebugWindow("StepParameterProc(),Green Lamp Value Invalid!");
			return 34;
		}
	}
	if(CrossData[bCrossNo - 1].Status == FORCED_STATUS || CrossData[bCrossNo - 1].Status == OFFLINE_STATUS)
	{
		DebugWindow("StepParameterProc(),Cross Status Invalid!");
		return 35;
	}
*/	

//set data now
	CycleLen = GetCycleLenEx(Data);
	if(CycleLen <= 0 || CycleLen >= 10000)
	{
		DebugWindow("StepParameterProc(),Fatal Error---Cycle Length!");
		return 60;
	}

	EffectDelt = (Data->iOffset +  CycleLen) % CycleLen;
/*	if(Data->iCoCrossNo <= 0 || Data->iCoCrossNo >= MAX_CROSS_NUM)
	{
//		BaseCrossNo = 0;
		CrossData[bCrossNo - 1].BaseCross = 0;
		CrossData[bCrossNo - 1].DeltaPhi = 0;
	}
	else
	{
*/
	if(Data->iCoCrossNo > 0)
		BaseCrossNo = FindCrossNo(Data->iCoCrossNo);
	else
		BaseCrossNo = 0;
	if( (BaseCrossNo <= 0 || BaseCrossNo >= MAX_CROSS_NUM) || BaseCrossNo == bCrossNo )
	{
#ifdef TEST
		DebugWindow("StepParameterProc(),Set BaseCrossNo=0");
#endif
//		BaseCrossNo = 0;
		CrossData[bCrossNo - 1].BaseCross = 0;
		CrossData[bCrossNo - 1].DeltaPhi = 0;
	}
	else if(CrossData[bCrossNo - 1].Type == 2 && CrossData[BaseCrossNo - 1].Type == 2)
	{
		CrossData[bCrossNo - 1].BaseCross = BaseCrossNo;
		CrossData[bCrossNo - 1].DeltaPhi = EffectDelt;
	}
	else
	{
		CrossData[bCrossNo - 1].BaseCross = 0;
		CrossData[bCrossNo - 1].DeltaPhi = 0;
	}
//	}

	CrossData[bCrossNo - 1].SynErrCount = 0;
	CrossData[bCrossNo - 1].TotalStepNo = StepNo;
	memset(CrossData[bCrossNo - 1].jcqbh,0,MAX_JCQ_NUM);
	memset(CrossData[bCrossNo - 1].jcqStartGreenStep,0,4 * MAX_JCQ_NUM);
	memset(CrossData[bCrossNo - 1].jcqEndGreenStep,0,4 * MAX_JCQ_NUM);

	for(i = 0;i < byDetectorNum;i++)
		CrossData[bCrossNo - 1].jcqbh[i] = Data->byDetectorNo[i];

	for(i = 0;i < 4;i++)
	{
		for(j = 0;j < byDetectorNum;j++)
		{
			CrossData[bCrossNo - 1].jcqStartGreenStep[i][j] = Data->byDetectorGreen1[i][j];
			CrossData[bCrossNo - 1].jcqEndGreenStep[i][j] = Data->byDetectorGreen2[i][j];
	#ifdef TEST
			if(Data->byDetectorGreen1[i][j] != 0)
			{
				memset(disp,0,200);
				sprintf(disp,"port=%3d,GreenStart[%d]=%2d,GreenEnd[%d]=%2d",bCrossNo,i + 1,					\
					Data->byDetectorGreen1[i][j],i + 1,Data->byDetectorGreen2[i][j]);
				DebugWindow(disp);
			}
	#endif
		}
	}

	if(CrossData[bCrossNo - 1].Status == NORMAL_STATUS)
	{
		//continuing run steptable,but throw it in transitsteptable
		memcpy(CrossData[bCrossNo - 1].TransitStepTable,CrossData[bCrossNo - 1].StepTable,\
													MAX_STEP_NUM * sizeof(UINT16));
		memset(CrossData[bCrossNo - 1].StepTable,0,MAX_STEP_NUM * sizeof(UINT16));
		memset(CrossData[bCrossNo - 1].MinStepTable,0,MAX_STEP_NUM * sizeof(UINT16));
		memset(CrossData[bCrossNo - 1].MaxStepTable,0,MAX_STEP_NUM * sizeof(UINT16));
		for(i = 0;i < StepNo;i++)
		{
			CrossData[bCrossNo - 1].StepTable[i] = Data->iTime[i];
			if(Data->iTime[i] >= Data->MinStepTime[i] && Data->iTime[i] <= Data->MaxStepTime[i])
			{
				CrossData[bCrossNo - 1].MinStepTable[i] = Data->MinStepTime[i];
				CrossData[bCrossNo - 1].MaxStepTable[i] = Data->MaxStepTime[i];
			}
#ifdef TEST
			memset(disp,0,200);
			sprintf(disp,"port=%3d,Step[%02d]=%d",bCrossNo,i,Data->iTime[i]);
			DebugWindow(disp);
#endif
		}

		CrossData[bCrossNo - 1].Status = TRANSIT_STATUS;
	}
	else if(CrossData[bCrossNo - 1].Status == TRANSIT_STATUS)
	{
		//let it be,only reset steptable
		memset(CrossData[bCrossNo - 1].StepTable,0,MAX_STEP_NUM * sizeof(UINT16));
		memset(CrossData[bCrossNo - 1].MinStepTable,0,MAX_STEP_NUM * sizeof(UINT16));
		memset(CrossData[bCrossNo - 1].MaxStepTable,0,MAX_STEP_NUM * sizeof(UINT16));
		for(i = 0;i < StepNo;i++)
		{
			CrossData[bCrossNo - 1].StepTable[i] = Data->iTime[i];
			if(Data->iTime[i] >= Data->MinStepTime[i] && Data->iTime[i] <= Data->MaxStepTime[i])
			{
				CrossData[bCrossNo - 1].MinStepTable[i] = Data->MinStepTime[i];
				CrossData[bCrossNo - 1].MaxStepTable[i] = Data->MaxStepTime[i];
			}
#ifdef TEST
			memset(disp,0,200);
			sprintf(disp,"port=%3d,Step[%02d]=%d",bCrossNo,i,Data->iTime[i]);
			DebugWindow(disp);
#endif
		}
	}
//ADDED 04,03,2002
	else// if(CrossData[bCrossNo - 1].Status == OTHER_STATUS)
	{
		//let it be,only reset steptable
		memset(CrossData[bCrossNo - 1].StepTable,0,MAX_STEP_NUM * sizeof(UINT16));
		memset(CrossData[bCrossNo - 1].MinStepTable,0,MAX_STEP_NUM * sizeof(UINT16));
		memset(CrossData[bCrossNo - 1].MaxStepTable,0,MAX_STEP_NUM * sizeof(UINT16));
		for(i = 0;i < StepNo;i++)
		{
			CrossData[bCrossNo - 1].StepTable[i] = Data->iTime[i];
			if(Data->iTime[i] >= Data->MinStepTime[i] && Data->iTime[i] <= Data->MaxStepTime[i])
			{
				CrossData[bCrossNo - 1].MinStepTable[i] = Data->MinStepTime[i];
				CrossData[bCrossNo - 1].MaxStepTable[i] = Data->MaxStepTime[i];
			}
#ifdef TEST
			memset(disp,0,200);
			sprintf(disp,"port=%3d,Step[%02d]=%d",bCrossNo,i,Data->iTime[i]);
			DebugWindow(disp);
#endif
		}

		if(CrossData[bCrossNo - 1].Status == IDLE_STATUS)
		{
			CrossData[bCrossNo - 1].Status = INITIAL_STATUS;

			CrossData[bCrossNo - 1].InitalWait = 3;
			CrossData[bCrossNo - 1].CurStep = MAX_STEP_NUM + 1;
			CrossData[bCrossNo - 1].NeedStep = MAX_STEP_NUM + 2;
			CrossData[bCrossNo - 1].CurCtrMode = 0xff;
			CrossData[bCrossNo - 1].CurCValue = 0xff;
			CrossData[bCrossNo - 1].Left5Min = 5 * 60;
		}
	}
//added end

	return 0;
}

/*********************************************************************************
*	Function Name	: void GetCycleLenEx(CStepsDown *pStepDown)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/04,15,2002
*   Modify		    : yincy/04,17,2002
*	Global			: None
*	Note			: None
**********************************************************************************/	
int GetCycleLenEx(CStepsDown *pStepDown)
{
	register int i;
	int len;

	if(pStepDown == NULL)
	{
		DebugWindow("GetCycleLenEx(),Ptr Error!");
		return 100000;
	}

	if(pStepDown->byStep < 4 || pStepDown->byStep > MAX_STEP_NUM)
	{
		DebugWindow("GetCycleLenEx(),TotalStepNo Out of Range!");
		return 100000;
	}

	len = 0;
	for(i = 0;i < pStepDown->byStep;i++)
	{
		len += pStepDown->iTime[i];
	}

	return len > 0 ? len : 100000;
}


/*********************************************************************************
*	Function Name	: void SetZhaoAck2TaskTable(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/02,26,2001
*	Global			: None
*	Note			: None
**********************************************************************************/	
void SetZhaoAck2TaskTable(void)
{
	CrossData_t *pDevMap = gpDevMap;
	RecBuf *pSpool = gpSpool;
	UINT32 CrossNo,CmdNo;

	CrossNo = SendingBuf.CrossNo;
	CmdNo = SendingBuf.CmdNo;

	if(CmdNo < 0x80 || CmdNo > 0x85)
		return;

	if(CrossNo == 0 || CrossNo > MAX_CROSS_NUM)
		return;

//	if(TaskTable[CrossNo - 1][CmdNo - 0x80].ReservedStatus == TASK_WAIT_ZHAO)
//		TaskTable[CrossNo - 1][CmdNo - 0x80].ReservedStatus = TASK_WAIT_HU;

	return;
}


/*********************************************************************************
*	Function Name	: void LptReSend(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/03,28,2001
*	Global			: None
*	Note			: None
**********************************************************************************/	
void LptReSend(void)
{
	CrossData_t *pDevMap = gpDevMap;
	RecBuf *pSpool = gpSpool;

//	if(SendingBuf.Status == SENDBUF_SEND_STATUS)
	{
		if(SendingBuf.ReSendTimes < MAX_RESEND_TIME)
		{
			//resend
			if(!Write2Lpt(SendingBuf.MsgData,SendingBuf.MsgLen,SendingBuf.CardNo))
			{//fatal error
				DebugWindow("LptReSend(),Write to Lpt Error!");
				
				SendingBuf.ReSendTimes++;
//				SendingBuf.TestData = SENDBUF_TIMEOUT;
				
				return;//never reached
			}
			
			SendingBuf.ReSendTimes++;
//			SendingBuf.TestData = SENDBUF_TIMEOUT;
		}
		else
		{
			//maybe never reached,sending failed to client
//			CMD_VSM_CONTENT conEvent;
//			UINT8 IsTask;
			UINT32 CrossNo,CmdNo;
			
			//if system auto send command,only clear flag
/*			if(SysSendTask == TASKSENDING)
			{
				DebugWindow("LptReSend(),System Auto Send Task Timeout!");
				SysSendTask = NOTUSE;
				SysSendTaskIndex = 0;
				memset(&SendingBuf,0,sizeof(RecBuf));
				return;
			}
*/
//			IsTask = SendingBuf.IsTask;
			CrossNo = (UINT32)(SendingBuf.CrossNo);
			CmdNo = (UINT32)(SendingBuf.CmdNo);
			
//			memset((void *)&conEvent,0x00,sizeof(CMD_VSM_CONTENT));
			
/*			if(IsTask && CmdNo >=0x80 && CmdNo <= 0x85)
				if(TaskTable[CrossNo - 1][CmdNo - 0x80].ReservedStatus != TASK_NOTUSE)
				{
					TaskTable[CrossNo - 1][CmdNo - 0x80].ReservedStatus = TASK_NOTUSE;
					TaskTable[CrossNo - 1][CmdNo - 0x80].ReservedParam = 0;
				}
*/				
//			ClearAlldata(CrossNo,CmdNo,SendingBuf.OprIndex,0);
			/*ClearTaskData(CrossNo,CmdNo,SendingBuf.OprIndex);
			*/
/*			if(SendingBuf.OprIndex == MAX_SOCKET_NUM)
			{
				//system command,only clear data 
				memset(&SendingBuf,0,sizeof(RecBuf));
				return;
			}
*/			
/*			if(SendingBuf.OprIndex == 0 || SendingBuf.OprIndex > MAX_SOCKET_NUM)
			{
				DebugWindow("LptReSend(),No Operator Or Operator Error!");
				memset(&SendingBuf,0,sizeof(RecBuf));
				return;
			}
*/			
			DebugWindow("LptReSend(),Send Result Error to Agent!");
			
//			SendDataEx((char *)&conEvent,sizeof(CMD_VSM_CONTENT),1);
			
			memset(&SendingBuf,0,sizeof(RecBuf));
			
			return;
		}
	}

	return;
}


/*********************************************************************************
*	Function Name	: void PrePareSendBuf(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/03,29,2001
*	Global			: None
*	Note			: None
**********************************************************************************/	
BOOL PrePareSendBuf(void)
{
	UINT8 CmdNo;
//	UINT32 SendParam;

//	if(SysSendTask != SYSTASK2SEND)
//		return FALSE;

//	if(SysSendTaskIndex == 0 || SysSendTaskIndex >MAX_SYSTASK_NUM)
//		return FALSE;

//	TaskTableIndex = SysSendTaskIndex - 1;

//	CmdNo = (pSysAutoTask + TaskTableIndex)->CmdNo;
//	SendParam = (pSysAutoTask + TaskTableIndex)->SendParam == 0 ? 1:(pSysAutoTask + TaskTableIndex)->SendParam;

	memset((void *)&SendingBuf,0,sizeof(RecBuf));
//	SendingBuf.CmdNo = CmdNo;
//	SendingBuf.OprIndex = MAX_SOCKET_NUM;
	SendingBuf.CrossNo = 0;
//	SendingBuf.Status = SENDBUF_SEND_STATUS;
	SendingBuf.ReSendTimes = 1;
//	SendingBuf.TestData = SENDBUF_TIMEOUT;//times

	SendingBuf.MsgData[0] = 0xaa;
	SendingBuf.MsgData[1] = 0xaa;
	SendingBuf.MsgData[3] = 0x00;
	SendingBuf.MsgData[4] = 0x00;
	SendingBuf.MsgData[5] = 0x00;
//	SendingBuf.MsgData[6] = CmdNo;
	SendingBuf.MsgData[7] = MAX_SOCKET_NUM;

	switch(CmdNo = 1)
	{
		case 0x02:
		{
			SYSTEMTIME time;
			GetLocalTime(&time);

			SendingBuf.MsgLen = ZHAO_DATA_LEN + 7;
			SendingBuf.MsgData[2] = HU_DATA_LEN + 7;
			SendingBuf.MsgData[8] = (UINT8)time.wYear;
			SendingBuf.MsgData[9] = (UINT8)time.wMonth;
			SendingBuf.MsgData[10] = (UINT8)time.wDay;
			SendingBuf.MsgData[11] = (UINT8)time.wHour;
			SendingBuf.MsgData[12] = (UINT8)time.wMinute;
			SendingBuf.MsgData[13] = (UINT8)time.wSecond;
			SendingBuf.MsgData[14] = (UINT8)time.wDayOfWeek;
			SendingBuf.MsgData[15] = 0x55;
			SendingBuf.MsgData[16] = 0x66;
		}
		break;

		case 0x81:
		case 0x82:
		case 0x83:
			SendingBuf.MsgLen = ZHAO_DATA_LEN + 1;
			SendingBuf.MsgData[2] = HU_DATA_LEN + 1;
//			SendingBuf.MsgData[8] = SendParam;
			SendingBuf.MsgData[9] = 0x55;
			SendingBuf.MsgData[10] = 0x66;
			break;

		default:
			return FALSE;
			break;
	}

	return TRUE;
}


/*********************************************************************************
*	Function Name	: UINT8 FindCrossNo(UINT8 *Crossing)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/03,01,2001
*	Global			: None
*	Note			: None
**********************************************************************************/	
UINT32 FindCrossNo(int Crossing)
{
	register int i;

	if(Crossing == 0)
	{
#ifdef TEST
		DebugWindow("FindCrossNo(),There Is No Cross No 1!!");
#endif
		return 0;
	}

	for(i = 0; i < MAX_CROSS_NUM; i++)
	{
		if(Crossing == CrossData[i].lkbh)
			break;
	}

	if(i >= MAX_CROSS_NUM)
	{
#ifdef __TEST
		DebugWindow("FindCrossNo(),There Is No Cross No 2!");
#endif
		return 0;
	}
	else 
		return ((i + 1) % MAX_CROSS_NUM);


	return 0;
}

/***************************************************************************
*	Function Name	: BOOL SanlianLcuProc(int Port,char *MsgPtr,int MsgLen)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/09,24,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
void SanlianLcuProc(int Port,char *MsgPtr,int MsgLen)
{
	UINT8 bCmdNo;
	UINT8 bReserved;
#ifdef TEST
	char disp[200];
#endif

	if(Port <= 0 || Port > MAX_CROSS_NUM)
	{
		DebugWindow("SanlianLcuProc(),Input Error!");
		return;
	}

	if(MsgPtr == NULL)
	{
		DebugWindow("SanlianLcuProc(),MsgPtr Error!");
		return;
	}

	if(MsgLen < 2 || MsgLen > 256 +  ZHAO_HEAD_LEN)
	{
		DebugWindow("SanlianLcuProc(),Msg Length Error!");
		return;
	}

	bCmdNo = *(MsgPtr + 1);
	bReserved = *(MsgPtr + 2);

	if(bCmdNo > 0)
	{
		CrossData[Port - 1].NoSignalCount = 0;

		if(CrossData[Port - 1].Status != CONNECT_STATUS)
		{
	#ifdef TEST
			memset(disp,0,200);
			sprintf(disp,"Send Connect To Port %d!",Port);
			DebugWindow(disp);
	#endif
			SendCommStatus2Wu(Port,0);
			CrossData[Port - 1].Status = CONNECT_STATUS;

			if(CrossData[Port - 1].ErrorNo > 0)
			{
				CrossData[Port - 1].ErrorNo = 0;
			}
		}
	}

	switch(bCmdNo)
	{
	case 0:
#ifdef TEST
		memset(disp,0,200);
		sprintf(disp,"Cross=%d Received Error Event!",Port);
		DebugWindow(disp);
#endif
	
		//Only send message to wu,inform error type
		if(CrossData[Port - 1].ErrorNo != bReserved)
		{
/*			if(CrossData[Port - 1].Status != DISCONNECT_STATUS)
			{
#ifdef TEST
				memset(disp,0,200);
				sprintf(disp,"Cross=%d Disconnected!",Port);
				DebugWindow(disp);
#endif
				SendCommStatus2Wu(Port,bReserved);
				CrossData[Port - 1].Status = DISCONNECT_STATUS;
			}
*/
			CrossData[Port - 1].ErrorNo = bReserved;//error number
		}

		break;

//	case 0x01:
	case 0x02:
	case 0x03:
	case 0x04:
	case 0x05:
	case 0x06:
	case 0x07:
	case 0x08:
	case 0x09:
	case 0x0a:
	case 0x17:
	case 0x18:
	case 0x19:
	case 0x29:

#ifdef FORDEBUG
	memset(disp,0,200);
	sprintf(disp,"Received Cross=%d,Command=%02xH Hu Ack Event!",Port,bCmdNo);
	DebugWindow(disp);
#endif
		break;

	case 0x80:
		{
		char *p = MsgPtr + ZHAO_HEAD_LEN;
		UINT8 tempByte;
		UNION_BYTE_BIT *pBits;
		int i;
//consider later 08,31,2001
//		BOOL sent;
		Rep_LightData HuLightData;
		Rep_DetectData HuDetectData;
		Rep_UpStatus UpStatusData;

		if(MsgLen == ZHAO_HEAD_LEN) 
		{
			DebugWindow("Received 0x80 Ack Message!");
			return;
		}
		else if(MsgLen < ZHAO_HEAD_LEN + 44) 
		{
			DebugWindow("0x80 Message Length Too Short!");
			return;
		}

		//PostMessage(hMainWnd,LPT_ZHAO_SUCC_ACK,(WPARAM)Port,(LPARAM)2345);
/*		if(IsDataAllNull((UINT8 *)p,(UINT16)13))
		{
#ifdef TEST
			memset(disp,0,200);
			sprintf(disp,"Received Cross=%d,CommandNo=%02xH Ack,Discard!",Port,bCmdNo);
			DebugWindow(disp);
#endif
			return;
		}
*/
#ifdef FORDEBUG
		memset(disp,0,200);
		sprintf(disp,"Received Cross=%d,CommandNo=%02xH Data",Port,bCmdNo);
		DebugWindow(disp);
#endif
//consider later 08,31,2001
//		sent = IsStopSent(Port,bCmdNo);

		/*CrossDataEventProc(Port,bCmdNo);*/

//following deleted 09,25,2001
/*		if(CrossData[Port - 1].NeedRealData == 0)
		{
//consider later 08,31,2001
//			if(!sent)
//				QryAllStateReqProc(0,Port,(UINT8)1);

			return;
		}
*/
		memset((void *)(&HuLightData),0,sizeof(Rep_LightData));
		HuLightData.Header.byFlag = MsgFlag;
		HuLightData.Header.MsgType = CM_LAMPSTATUS;

		strcpy(HuLightData.Header.SourceIP,HostIp);
		strcpy(HuLightData.Header.SourceID,HostId);
		strcpy(HuLightData.Header.TargetIP,SvrIp);
		strcpy(HuLightData.Header.TargetID,SvrId);

		HuLightData.Header.iCrossNo = CrossData[Port - 1].lkbh;
		HuLightData.Header.iLength = sizeof(LightData);
		HuLightData.Header.byCheckSum = FormCheckSum((char *)&(HuLightData.Header));
		
		p++;//*p = phase number
		for(i = 0;i < 10;i++)//lamp
		{
			pBits = (UNION_BYTE_BIT *)p;

			if(pBits->U_Bit.D0 == 1)
				HuLightData.Data.byStatus[2 * i] = LT_RED;
			else if(pBits->U_Bit.D1 == 1)
				HuLightData.Data.byStatus[2 * i] = LT_YELLOW;
			else if(pBits->U_Bit.D2 == 1)
				HuLightData.Data.byStatus[2 * i] = LT_GREEN;
			else
				HuLightData.Data.byStatus[2 * i] = LT_UNKNOWN;

			if(pBits->U_Bit.D3 == 1)
				HuLightData.Data.byStatus[2 * i + 1] = LT_RED;
			else if(pBits->U_Bit.D4 == 1)
				HuLightData.Data.byStatus[2 * i + 1] = LT_YELLOW;
			else if(pBits->U_Bit.D5 == 1)
				HuLightData.Data.byStatus[2 * i + 1] = LT_GREEN;
			else
				HuLightData.Data.byStatus[2 * i + 1] = LT_UNKNOWN;
			
			p++;
		}

		p = MsgPtr + ZHAO_HEAD_LEN + 1;
		pBits = (UNION_BYTE_BIT *)p;
		if(pBits->U_Bit.D6 == 1)
			HuLightData.Data.byStatus[20] = LT_RED;
		else if(pBits->U_Bit.D7 == 1)
			HuLightData.Data.byStatus[20] = LT_GREEN;
		else
			HuLightData.Data.byStatus[20] = LT_UNKNOWN;

		pBits = (UNION_BYTE_BIT *)(++p);
		if(pBits->U_Bit.D6 == 1)
			HuLightData.Data.byStatus[22] = LT_RED;
		else if(pBits->U_Bit.D7 == 1)
			HuLightData.Data.byStatus[22] = LT_GREEN;
		else
			HuLightData.Data.byStatus[22] = LT_UNKNOWN;

		pBits = (UNION_BYTE_BIT *)(++p);
		if(pBits->U_Bit.D6 == 1)
			HuLightData.Data.byStatus[21] = LT_RED;
		else if(pBits->U_Bit.D7 == 1)
			HuLightData.Data.byStatus[21] = LT_GREEN;
		else
			HuLightData.Data.byStatus[21] = LT_UNKNOWN;

		++p;
		for(i = 23;i <= 27;i++)
		{
			pBits = (UNION_BYTE_BIT *)p;
			if(pBits->U_Bit.D6 == 1)
				HuLightData.Data.byStatus[i] = LT_RED;
			else if(pBits->U_Bit.D7 == 1)
				HuLightData.Data.byStatus[i] = LT_GREEN;
			else
				HuLightData.Data.byStatus[i] = LT_UNKNOWN;

			p++;
		}
		SendDataEx((char *)&HuLightData,sizeof(Rep_LightData),(UINT8)1);

//send detector data
		memset((void *)(&HuDetectData),0,sizeof(Rep_DetectData));
		HuDetectData.Header.byFlag = MsgFlag;
		HuDetectData.Header.MsgType = CM_VEHICLESTATUS;

		strcpy(HuDetectData.Header.SourceIP,HostIp);
		strcpy(HuDetectData.Header.SourceID,HostId);
		strcpy(HuDetectData.Header.TargetIP,SvrIp);
		strcpy(HuDetectData.Header.TargetID,SvrId);

		HuDetectData.Header.iCrossNo = CrossData[Port - 1].lkbh;
		HuDetectData.Header.iLength = sizeof(DetectorData);
		HuDetectData.Header.byCheckSum = FormCheckSum((char *)&(HuDetectData.Header));
		
		p+=2;
		tempByte = (UINT8)(*p);
		for(i = 0;i < 8;i++)//detector (1--8)
		{
			HuDetectData.Data.byStatus[i] = (tempByte & 0x01);
			tempByte = tempByte >> 1;
		}

		++p;
		tempByte = (UINT8)(*p);
		for(i = 0;i < 8;i++)//detector(9--16)
		{
			HuDetectData.Data.byStatus[8 + i] = (tempByte & 0x01);
			tempByte = tempByte >> 1;
		}
		SendDataEx((char *)&HuDetectData,sizeof(Rep_DetectData),(UINT8)1);

		memset((void *)(&UpStatusData),0,sizeof(Rep_UpStatus));
		UpStatusData.Header.byFlag = MsgFlag;
		UpStatusData.Header.MsgType = CM_SLCURSTATUS;

		strcpy(UpStatusData.Header.SourceIP,HostIp);
		strcpy(UpStatusData.Header.SourceID,HostId);
		strcpy(UpStatusData.Header.TargetIP,SvrIp);
		strcpy(UpStatusData.Header.TargetID,SvrId);

		UpStatusData.Header.iCrossNo = CrossData[Port - 1].lkbh;
		UpStatusData.Header.iLength = sizeof(SLUpStatusData);
		UpStatusData.Header.byCheckSum = FormCheckSum((char *)&(UpStatusData.Header));

		p++;
		UpStatusData.Data.byCtrlMode = *p;		
		UpStatusData.Data.byFlowGroupNo = *(p + 1);
		UpStatusData.Data.byGreenTimeLeft = *(p + 2);
		UpStatusData.Data.byGreenFlashTimeLeft = *(p + 3);
		UpStatusData.Data.byYellowTimeLeft = *(p + 4);			
		UpStatusData.Data.byAllRedTimeLeft = *(p + 5);
		UpStatusData.Data.byTimePlanNo = *(p + 6);
		UpStatusData.Data.byTimeBandNo = *(p + 7);
		UpStatusData.Data.byCtrlPlanNo = *(p + 8);			
		UpStatusData.Data.IsYellowBlink = *(p + 9);
		UpStatusData.Data.IsLampOff = *(p + 10);

		SendDataEx((char *)&UpStatusData,sizeof(Rep_UpStatus),(UINT8)1);
		}
		break;

	case 0x81:
		{
		char *p = MsgPtr + ZHAO_HEAD_LEN;
		int i;
		Rep_SlLampCondition SlLampCondition;

		if(MsgLen == ZHAO_HEAD_LEN) 
		{
			DebugWindow("Received 0x81 Ack Message!");
			return;
		}
		else if(MsgLen < ZHAO_HEAD_LEN + 10) 
		{
			DebugWindow("0x81 Message Length Too Short!");
			return;
		}

		if(IsDataAllNull((UINT8 *)p,(UINT16)10))
		{
#ifdef TEST
			memset(disp,0,200);
			sprintf(disp,"Received Cross=%d,CommandNo=%02xH Ack,Discard!",Port,bCmdNo);
			DebugWindow(disp);
#endif
			return;
		}

#ifdef FORDEBUG
		memset(disp,0,200);
		sprintf(disp,"Received Cross=%d,CommandNo=%02xH Data",Port,bCmdNo);
		DebugWindow(disp);
#endif
		memset((void *)(&SlLampCondition),0,sizeof(Rep_SlLampCondition));
		SlLampCondition.Header.byFlag = MsgFlag;
		SlLampCondition.Header.MsgType = CM_SLLAMPCONDITION;

		strcpy(SlLampCondition.Header.SourceIP,HostIp);
		strcpy(SlLampCondition.Header.SourceID,HostId);
		strcpy(SlLampCondition.Header.TargetIP,SvrIp);
		strcpy(SlLampCondition.Header.TargetID,SvrId);

		SlLampCondition.Header.iCrossNo = CrossData[Port - 1].lkbh;
		SlLampCondition.Header.iLength = sizeof(SLLampStatusData);
		SlLampCondition.Header.byCheckSum = FormCheckSum((char *)&(SlLampCondition.Header));
		
		for(i = 0;i < 10;i++)//lamp
		{
			SLLTDataByte *p1 = (SLLTDataByte *)p;
			SlLampCondition.Data.LtData[i].D1 = p1->D1;
			SlLampCondition.Data.LtData[i].D2 = p1->D2;
			SlLampCondition.Data.LtData[i].D3 = p1->D3;
			p++;
		}

		SendDataEx((char *)&SlLampCondition,sizeof(Rep_SlLampCondition),(UINT8)1);
		}

		break;

	case 0x82:
		{
		char *p = MsgPtr + ZHAO_HEAD_LEN;
		int i;
		Rep_SLVehicleCount SLVehicleCount;

		if(MsgLen == ZHAO_HEAD_LEN) 
		{
			DebugWindow("Received 0x82 Ack Message!");
			return;
		}
		else if(MsgLen < ZHAO_HEAD_LEN + 32) 
		{
			DebugWindow("0x82 Message Length Too Short!");
			return;
		}

		if(IsDataAllNull((UINT8 *)p,(UINT16)32))
		{
#ifdef TEST
			memset(disp,0,200);
			sprintf(disp,"Received Cross=%d,CommandNo=%02xH Ack,Discard!",Port,bCmdNo);
			DebugWindow(disp);
#endif
			return;
		}

#ifdef FORDEBUG
		memset(disp,0,200);
		sprintf(disp,"Received Cross=%d,CommandNo=%02xH Data",Port,bCmdNo);
		DebugWindow(disp);
#endif
		memset((void *)(&SLVehicleCount),0,sizeof(Rep_SLVehicleCount));
		SLVehicleCount.Header.byFlag = MsgFlag;
		SLVehicleCount.Header.MsgType = CM_SLCOUNTDATA;

		strcpy(SLVehicleCount.Header.SourceIP,HostIp);
		strcpy(SLVehicleCount.Header.SourceID,HostId);
		strcpy(SLVehicleCount.Header.TargetIP,SvrIp);
		strcpy(SLVehicleCount.Header.TargetID,SvrId);

		SLVehicleCount.Header.iCrossNo = CrossData[Port - 1].lkbh;
		SLVehicleCount.Header.iLength = sizeof(SLVehicleCountData);
		SLVehicleCount.Header.byCheckSum = FormCheckSum((char *)&(SLVehicleCount.Header));
		
		for(i = 0;i < 16;i++)//lamp
		{
			SLVehicleCount.Data.byCount[i] = *p;
			SLVehicleCount.Data.byTime[i] = *(p + 16);
			p++;
		}

		SendDataEx((char *)&SLVehicleCount,sizeof(Rep_SLVehicleCount),(UINT8)1);
		}
		break;

	case 0x83:
		{
		char *p = MsgPtr + ZHAO_HEAD_LEN;
		Rep_SLEnvData SLNowEnvData;

		if(MsgLen == ZHAO_HEAD_LEN) 
		{
			DebugWindow("Received 0x83 Ack Message!");
			return;
		}
		else if(MsgLen == ZHAO_HEAD_LEN + 3 + 1) 
		{
			DebugWindow("Received 0x83 Ack Message!");
			return;
		}
		else if(MsgLen < ZHAO_HEAD_LEN + 4) 
		{
			DebugWindow("0x83 Message Length Too Short!");
			return;
		}

		if(IsDataAllNull((UINT8 *)p,(UINT16)4))
		{
#ifdef TEST
			memset(disp,0,200);
			sprintf(disp,"Received Cross=%d,CommandNo=%02xH Ack,Discard!",Port,bCmdNo);
			DebugWindow(disp);
#endif
			return;
		}

#ifdef FORDEBUG
		memset(disp,0,200);
		sprintf(disp,"Received Cross=%d,CommandNo=%02xH Data",Port,bCmdNo);
		DebugWindow(disp);
#endif
		memset((void *)(&SLNowEnvData),0,sizeof(Rep_SLEnvData));
		SLNowEnvData.Header.byFlag = MsgFlag;
		SLNowEnvData.Header.MsgType = CM_SLAIRDATA;

		strcpy(SLNowEnvData.Header.SourceIP,HostIp);
		strcpy(SLNowEnvData.Header.SourceID,HostId);
		strcpy(SLNowEnvData.Header.TargetIP,SvrIp);
		strcpy(SLNowEnvData.Header.TargetID,SvrId);

		SLNowEnvData.Header.iCrossNo = CrossData[Port - 1].lkbh;
		SLNowEnvData.Header.iLength = sizeof(SLEnvData);
		SLNowEnvData.Header.byCheckSum = FormCheckSum((char *)&(SLNowEnvData.Header));
		
		SLNowEnvData.Data.byNoise = *p;
		SLNowEnvData.Data.byCO = *(p + 1);
		SLNowEnvData.Data.bySO2 = *(p + 2);
		SLNowEnvData.Data.byTemp = *(p + 3);			

		SendDataEx((char *)&SLNowEnvData,sizeof(Rep_SLEnvData),(UINT8)1);
		}
		break;

	case 0x84:
		{
		char *p = MsgPtr + ZHAO_HEAD_LEN;
		int i;
		Rep_SLFlowData SLNowFlow;

		if(MsgLen == ZHAO_HEAD_LEN) 
		{
			DebugWindow("Received 0x84 Ack Message!");
			return;
		}
		else if(MsgLen < ZHAO_HEAD_LEN + 32) 
		{
			DebugWindow("0x84 Message Length Too Short!");
			return;
		}

		if(IsDataAllNull((UINT8 *)p,(UINT16)32))
		{
#ifdef TEST
			memset(disp,0,200);
			sprintf(disp,"Received Cross=%d,CommandNo=%02xH Ack,Discard!",Port,bCmdNo);
			DebugWindow(disp);
#endif
			return;
		}

#ifdef FORDEBUG
		memset(disp,0,200);
		sprintf(disp,"Received Cross=%d,CommandNo=%02xH Data",Port,bCmdNo);
		DebugWindow(disp);
#endif
		memset((void *)(&SLNowFlow),0,sizeof(Rep_SLFlowData));
		SLNowFlow.Header.byFlag = MsgFlag;
		SLNowFlow.Header.MsgType = CM_SLFLOWDATA;

		strcpy(SLNowFlow.Header.SourceIP,HostIp);
		strcpy(SLNowFlow.Header.SourceID,HostId);
		strcpy(SLNowFlow.Header.TargetIP,SvrIp);
		strcpy(SLNowFlow.Header.TargetID,SvrId);

		SLNowFlow.Header.iCrossNo = CrossData[Port - 1].lkbh;
		SLNowFlow.Header.iLength = sizeof(SLVehicleFlowData);
		SLNowFlow.Header.byCheckSum = FormCheckSum((char *)&(SLNowFlow.Header));
		
		for(i = 0;i < 16;i++)//lamp
		{
			SLNowFlow.Data.byFlow[i] = *p;
			SLNowFlow.Data.byOccupy[i] = *(p + 16);
			p++;
		}

		SendDataEx((char *)&SLNowFlow,sizeof(Rep_SLFlowData),(UINT8)1);
		}
		break;

	case 0x85:
		{
		char *p = MsgPtr + ZHAO_HEAD_LEN;
		CMsgFrame NowTestData;

		if(MsgLen == ZHAO_HEAD_LEN) 
		{
			DebugWindow("Received 0x85 Ack Message!");
			return;
		}
		else if(MsgLen < ZHAO_HEAD_LEN + 1) 
		{
			DebugWindow("0x85 Message Length Too Short!");
			return;
		}

		if(bReserved == 0xff)//just system test
		{
#ifdef TEST
			memset(disp,0,200);
			sprintf(disp,"Received Cross=%d,Auto %02xH Hu Data Event!",Port,bCmdNo);
			DebugWindow(disp);
#endif
			return;
		}

		if(IsDataAllNull((UINT8 *)p,(UINT16)1))
		{
#ifdef TEST
			memset(disp,0,200);
			sprintf(disp,"Received Cross=%d,CommandNo=%02xH Ack,Discard!",Port,bCmdNo);
			DebugWindow(disp);
#endif
			return;
		}

#ifdef FORDEBUG
		memset(disp,0,200);
		sprintf(disp,"Received Cross=%d,CommandNo=%02xH Data",Port,bCmdNo);
		DebugWindow(disp);
#endif
		memset((void *)(&NowTestData),0,sizeof(CMsgFrame));
		NowTestData.byFlag = MsgFlag;
		NowTestData.MsgType = CM_SLCOMTEST;

		strcpy(NowTestData.SourceIP,HostIp);
		strcpy(NowTestData.SourceID,HostId);
		strcpy(NowTestData.TargetIP,SvrIp);
		strcpy(NowTestData.TargetID,SvrId);

		NowTestData.iCrossNo = CrossData[Port - 1].lkbh;
		NowTestData.iLength = 0;
		NowTestData.iReserved1 = *p;
		NowTestData.byCheckSum = FormCheckSum((char *)&NowTestData);

		SendDataEx((char *)&NowTestData,sizeof(CMsgFrame),(UINT8)1);
		}
		break;

	case 0x86:
		{
		char *p = MsgPtr + ZHAO_HEAD_LEN;
		Rep_UpStatus UpStatusData;

		if(MsgLen == ZHAO_HEAD_LEN) 
		{
			DebugWindow("Received 0x86 Ack Message!");
			return;
		}
		else if(MsgLen < ZHAO_HEAD_LEN + 10) 
		{
			DebugWindow("0x86 Message Length Too Short!");
			return;
		}

		if(IsDataAllNull((UINT8 *)p,(UINT16)10))
		{
#ifdef TEST
			memset(disp,0,200);
			sprintf(disp,"Received Cross=%d,CommandNo=%02xH Ack,Discard!",Port,bCmdNo);
			DebugWindow(disp);
#endif
			return;
		}

#ifdef FORDEBUG
		memset(disp,0,200);
		sprintf(disp,"Received Cross=%d,CommandNo=%02xH Data",Port,bCmdNo);
		DebugWindow(disp);
#endif
		memset((void *)(&UpStatusData),0,sizeof(Rep_UpStatus));
		UpStatusData.Header.byFlag = MsgFlag;
		UpStatusData.Header.MsgType = CM_SLCURSTATUS;

		strcpy(UpStatusData.Header.SourceIP,HostIp);
		strcpy(UpStatusData.Header.SourceID,HostId);
		strcpy(UpStatusData.Header.TargetIP,SvrIp);
		strcpy(UpStatusData.Header.TargetID,SvrId);

		UpStatusData.Header.iCrossNo = CrossData[Port - 1].lkbh;
		UpStatusData.Header.iLength = sizeof(SLUpStatusData);
		UpStatusData.Header.byCheckSum = FormCheckSum((char *)&(UpStatusData.Header));

		UpStatusData.Data.byCtrlMode = *p;		
		UpStatusData.Data.byFlowGroupNo = *(p + 1);
		UpStatusData.Data.byGreenTimeLeft = *(p + 2);
		UpStatusData.Data.byGreenFlashTimeLeft = *(p + 3);
		UpStatusData.Data.byYellowTimeLeft = *(p + 4);			
		UpStatusData.Data.byAllRedTimeLeft = *(p + 5);
		UpStatusData.Data.byTimePlanNo = *(p + 6);
		UpStatusData.Data.byTimeBandNo = *(p + 7);
		UpStatusData.Data.byCtrlPlanNo = *(p + 8);			
		UpStatusData.Data.IsYellowBlink = *(p + 9);
		UpStatusData.Data.IsLampOff = *(p + 10);

		SendDataEx((char *)&UpStatusData,sizeof(Rep_UpStatus),(UINT8)1);
		}
		break;

	default:
		DebugWindow("SanlianLcuProc(),Received Unrecognized Command!");
		break;
	}

	return;
}

/***************************************************************************
*	Function Name	: BOOL SerialPost(int Port,char *MsgPtr,UINT16 MsgLen)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/08,06,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
void SanlianSerialProc(int Port,char *MsgPtr,int MsgLen)
{
	UINT8 bCmdNo;
	UINT8 bReserved;
#ifdef TEST
	char disp[200];
#endif

	if(Port <= 0 || Port > MAX_CROSS_NUM)
	{
		DebugWindow("SanlianSerialProc(),Input Error!");
		return;
	}

	if(MsgPtr == NULL)
	{
		DebugWindow("SanlianSerialProc(),MsgPtr Error!");
		return;
	}

	if(MsgLen < HU_HEAD_LEN + 1 || MsgLen > 256 +  HU_HEAD_LEN + 1)
	{
		DebugWindow("SanlianSerialProc(),Msg Length Error!");
		return;
	}

//	CrossData[Port - 1].ReopenLeftCount = 60;
	CrossData[Port - 1].DisconnectCount = 0;
	CrossData[Port - 1].NoSignalCount = 0;
	if(CrossData[Port - 1].Status != CONNECT_STATUS)
	{
#ifdef TEST
		memset(disp,0,200);
		sprintf(disp,"Send Connect To Port %d!",Port);
		DebugWindow(disp);
#endif
		SendCommStatus2Wu(Port,0);
		CrossData[Port - 1].Status = CONNECT_STATUS;
	}

	bCmdNo = *(MsgPtr + 2);
	bReserved = *(MsgPtr + 3);

	switch(bCmdNo)
	{
	case 0:
		//PostMessage(hMainWnd,LPT_ZHAO_ERR_ACK,(WPARAM)Port,(LPARAM)9595);
#ifdef TEST
		memset(disp,0,200);
		sprintf(disp,"SanlianSerialProc(),Cross=%d Receive Data Error!",Port);
		DebugWindow(disp);
#endif
		CrossErrEventProc(Port,bCmdNo);
		break;

	case 0x01:
	case 0x02:
	case 0x03:
	case 0x04:
	case 0x05:
	case 0x06:
	case 0x07:
	case 0x08:
	case 0x09:
	case 0x0a:
	case 0x17:
	case 0x18:
	case 0x29:
	//	PostMessage(hMainWnd,LPT_ZHAO_SUCC_ACK,(WPARAM)Port,(LPARAM)2345);
#ifdef TEST
		memset(disp,0,200);
		sprintf(disp,"Received Cross=%d,CommandNo=%02xH Ack",Port,bCmdNo);
		DebugWindow(disp);
#endif
		CrossAckEventProc(Port,bCmdNo);
		break;

	case 0x80:
		{
		char *p = MsgPtr + HU_HEAD_LEN;
		UINT8 tempByte;
		UNION_BYTE_BIT *pBits;
		int i;
//consider later 08,31,2001
//		BOOL sent;
		Rep_LightData HuLightData;
		Rep_DetectData HuDetectData;
		Rep_UpStatus UpStatusData;

		CrossDataEventProc(Port,bCmdNo);

		if(MsgLen == HU_HEAD_LEN + 1) 
		{
			DebugWindow("Received 0x80 Ack Message!");
			return;
		}
		else if(MsgLen < HU_HEAD_LEN + 44 + 1) 
		{
			DebugWindow("0x80 Message Length Too Short!");
			return;
		}

		//PostMessage(hMainWnd,LPT_ZHAO_SUCC_ACK,(WPARAM)Port,(LPARAM)2345);
//consider later 08,31,2001
//		sent = IsStopSent(Port,bCmdNo);

/*		if(IsDataAllNull((UINT8 *)p,(UINT16)13))
		{
#ifdef TEST

			memset(disp,0,200);
			sprintf(disp,"Received Cross=%d,CommandNo=%02xH Ack",Port,bCmdNo);
			DebugWindow(disp);
#endif
			return;
		}
*/

#ifdef FORDEBUG
		memset(disp,0,200);
		sprintf(disp,"Received Cross=%d,CommandNo=%02xH Data",Port,bCmdNo);
		DebugWindow(disp);
#endif

/*		if(CrossData[Port - 1].NeedRealData == 0)
		{
//consider later 08,31,2001
//			if(!sent)
//				QryAllStateReqProc(0,Port,(UINT8)1);

			return;
		}
*/
		memset((void *)(&HuLightData),0,sizeof(Rep_LightData));
		HuLightData.Header.byFlag = MsgFlag;
		HuLightData.Header.MsgType = CM_LAMPSTATUS;

		strcpy(HuLightData.Header.SourceIP,HostIp);
		strcpy(HuLightData.Header.SourceID,HostId);
		strcpy(HuLightData.Header.TargetIP,SvrIp);
		strcpy(HuLightData.Header.TargetID,SvrId);

		HuLightData.Header.iCrossNo = CrossData[Port - 1].lkbh;
		HuLightData.Header.iLength = sizeof(LightData);
		HuLightData.Header.byCheckSum = FormCheckSum((char *)&(HuLightData.Header));
		
		p++;
		for(i = 0;i < 10;i++)//lamp
		{
			pBits = (UNION_BYTE_BIT *)p;

			if(pBits->U_Bit.D0 == 1)
				HuLightData.Data.byStatus[2 * i] = LT_RED;
			else if(pBits->U_Bit.D1 == 1)
				HuLightData.Data.byStatus[2 * i] = LT_YELLOW;
			else if(pBits->U_Bit.D2 == 1)
				HuLightData.Data.byStatus[2 * i] = LT_GREEN;
			else
				HuLightData.Data.byStatus[2 * i] = LT_UNKNOWN;

			if(pBits->U_Bit.D3 == 1)
				HuLightData.Data.byStatus[2 * i + 1] = LT_RED;
			else if(pBits->U_Bit.D4 == 1)
				HuLightData.Data.byStatus[2 * i + 1] = LT_YELLOW;
			else if(pBits->U_Bit.D5 == 1)
				HuLightData.Data.byStatus[2 * i + 1] = LT_GREEN;
			else
				HuLightData.Data.byStatus[2 * i + 1] = LT_UNKNOWN;
			
			p++;
		}

		p = MsgPtr + HU_HEAD_LEN + 1;
		pBits = (UNION_BYTE_BIT *)p;
		if(pBits->U_Bit.D6 == 1)
			HuLightData.Data.byStatus[20] = LT_RED;
		else if(pBits->U_Bit.D7 == 1)
			HuLightData.Data.byStatus[20] = LT_GREEN;
		else
			HuLightData.Data.byStatus[20] = LT_UNKNOWN;

		p++;
		pBits = (UNION_BYTE_BIT *)p;
		if(pBits->U_Bit.D6 == 1)
			HuLightData.Data.byStatus[22] = LT_RED;
		else if(pBits->U_Bit.D7 == 1)
			HuLightData.Data.byStatus[22] = LT_GREEN;
		else
			HuLightData.Data.byStatus[22] = LT_UNKNOWN;

		p++;
		pBits = (UNION_BYTE_BIT *)p;
		if(pBits->U_Bit.D6 == 1)
			HuLightData.Data.byStatus[21] = LT_RED;
		else if(pBits->U_Bit.D7 == 1)
			HuLightData.Data.byStatus[21] = LT_GREEN;
		else
			HuLightData.Data.byStatus[21] = LT_UNKNOWN;

		++p;
		for(i = 23;i <= 27;i++)
		{
			pBits = (UNION_BYTE_BIT *)p;
			if(pBits->U_Bit.D6 == 1)
				HuLightData.Data.byStatus[i] = LT_RED;
			else if(pBits->U_Bit.D7 == 1)
				HuLightData.Data.byStatus[i] = LT_GREEN;
			else
				HuLightData.Data.byStatus[i] = LT_UNKNOWN;

			p++;
		}
		SendDataEx((char *)&HuLightData,sizeof(Rep_LightData),(UINT8)1);

//send detector data
		memset((void *)(&HuDetectData),0,sizeof(Rep_DetectData));
		HuDetectData.Header.byFlag = MsgFlag;
		HuDetectData.Header.MsgType = CM_VEHICLESTATUS;

		strcpy(HuDetectData.Header.SourceIP,HostIp);
		strcpy(HuDetectData.Header.SourceID,HostId);
		strcpy(HuDetectData.Header.TargetIP,SvrIp);
		strcpy(HuDetectData.Header.TargetID,SvrId);

		HuDetectData.Header.iCrossNo = CrossData[Port - 1].lkbh;
		HuDetectData.Header.iLength = sizeof(DetectorData);
		HuDetectData.Header.byCheckSum = FormCheckSum((char *)&(HuDetectData.Header));
		
		p+=2;
		tempByte = (UINT8)(*p);
		for(i = 0;i < 8;i++)//detector(1--8)
		{
			HuDetectData.Data.byStatus[i] = (tempByte & 0x01);
			tempByte = tempByte >> 1;
		}

		++p;
		tempByte = (UINT8)(*p);
		for(i = 0;i < 8;i++)//detector(9--16)
		{
			HuDetectData.Data.byStatus[8 + i] = (tempByte & 0x01);
			tempByte = tempByte >> 1;
		}
		SendDataEx((char *)&HuDetectData,sizeof(Rep_DetectData),(UINT8)1);

//send CM_SLCURSTATUS
		p++;
		memset((void *)(&UpStatusData),0,sizeof(Rep_UpStatus));
		UpStatusData.Header.byFlag = MsgFlag;
		UpStatusData.Header.MsgType = CM_SLCURSTATUS;

		strcpy(UpStatusData.Header.SourceIP,HostIp);
		strcpy(UpStatusData.Header.SourceID,HostId);
		strcpy(UpStatusData.Header.TargetIP,SvrIp);
		strcpy(UpStatusData.Header.TargetID,SvrId);

		UpStatusData.Header.iCrossNo = CrossData[Port - 1].lkbh;
		UpStatusData.Header.iLength = sizeof(SLUpStatusData);
		UpStatusData.Header.byCheckSum = FormCheckSum((char *)&(UpStatusData.Header));

		UpStatusData.Data.byCtrlMode = *p;		
		UpStatusData.Data.byFlowGroupNo = *(p + 1);
		UpStatusData.Data.byGreenTimeLeft = *(p + 2);
		UpStatusData.Data.byGreenFlashTimeLeft = *(p + 3);
		UpStatusData.Data.byYellowTimeLeft = *(p + 4);			
		UpStatusData.Data.byAllRedTimeLeft = *(p + 5);
		UpStatusData.Data.byTimePlanNo = *(p + 6);
		UpStatusData.Data.byTimeBandNo = *(p + 7);
		UpStatusData.Data.byCtrlPlanNo = *(p + 8);			
		UpStatusData.Data.IsYellowBlink = *(p + 9);
		UpStatusData.Data.IsLampOff = *(p + 10);

		SendDataEx((char *)&UpStatusData,sizeof(Rep_UpStatus),(UINT8)1);
		}
		break;

	case 0x81:
		{
		char *p = MsgPtr + HU_HEAD_LEN;
		int i;
		Rep_SlLampCondition SlLampCondition;

		CrossDataEventProc(Port,bCmdNo);

		if(MsgLen == HU_HEAD_LEN + 1) 
		{
			DebugWindow("Received 0x81 Ack Message!");
			return;
		}
		else if(MsgLen < HU_HEAD_LEN + 10 + 1) 
		{
			DebugWindow("0x81 Message Length Too Short!");
			return;
		}

		if(IsDataAllNull((UINT8 *)p,(UINT16)10))
		{
#ifdef TEST
			memset(disp,0,200);
			sprintf(disp,"Received Cross=%d,CommandNo=%02xH Ack",Port,bCmdNo);
			DebugWindow(disp);
#endif
			return;
		}

#ifdef FORDEBUG
		memset(disp,0,200);
		sprintf(disp,"Received Cross=%d,CommandNo=%02xH Data",Port,bCmdNo);
		DebugWindow(disp);
#endif

		memset((void *)(&SlLampCondition),0,sizeof(Rep_SlLampCondition));
		SlLampCondition.Header.byFlag = MsgFlag;
		SlLampCondition.Header.MsgType = CM_SLLAMPCONDITION;

		strcpy(SlLampCondition.Header.SourceIP,HostIp);
		strcpy(SlLampCondition.Header.SourceID,HostId);
		strcpy(SlLampCondition.Header.TargetIP,SvrIp);
		strcpy(SlLampCondition.Header.TargetID,SvrId);

		SlLampCondition.Header.iCrossNo = CrossData[Port - 1].lkbh;
		SlLampCondition.Header.iLength = sizeof(SLLampStatusData);
		SlLampCondition.Header.byCheckSum = FormCheckSum((char *)&(SlLampCondition.Header));
		
		for(i = 0;i < 10;i++)//lamp
		{
			SLLTDataByte *p1 = (SLLTDataByte *)p;
			SlLampCondition.Data.LtData[i].D1 = p1->D1;
			SlLampCondition.Data.LtData[i].D2 = p1->D2;
			SlLampCondition.Data.LtData[i].D3 = p1->D3;
			p++;
		}

		SendDataEx((char *)&SlLampCondition,sizeof(Rep_SlLampCondition),(UINT8)1);
		}

		break;

	case 0x82:
		{
		char *p = MsgPtr + HU_HEAD_LEN;
		int i;
		Rep_SLVehicleCount SLVehicleCount;

		CrossDataEventProc(Port,bCmdNo);

		if(MsgLen == HU_HEAD_LEN + 1) 
		{
			DebugWindow("Received 0x82 Ack Message!");
			return;
		}
		else if(MsgLen < HU_HEAD_LEN + 32 + 1) 
		{
			DebugWindow("0x82 Message Length Too Short!");
			return;
		}

		if(IsDataAllNull((UINT8 *)p,(UINT16)32))
		{
#ifdef TEST
			memset(disp,0,200);
			sprintf(disp,"Received Cross=%d,CommandNo=%02xH Ack",Port,bCmdNo);
			DebugWindow(disp);
#endif
			return;
		}

#ifdef FORDEBUG
		memset(disp,0,200);
		sprintf(disp,"Received Cross=%d,CommandNo=%02xH Data",Port,bCmdNo);
		DebugWindow(disp);
#endif

		memset((void *)(&SLVehicleCount),0,sizeof(Rep_SLVehicleCount));
		SLVehicleCount.Header.byFlag = MsgFlag;
		SLVehicleCount.Header.MsgType = CM_SLCOUNTDATA;

		strcpy(SLVehicleCount.Header.SourceIP,HostIp);
		strcpy(SLVehicleCount.Header.SourceID,HostId);
		strcpy(SLVehicleCount.Header.TargetIP,SvrIp);
		strcpy(SLVehicleCount.Header.TargetID,SvrId);

		SLVehicleCount.Header.iCrossNo = CrossData[Port - 1].lkbh;
		SLVehicleCount.Header.iLength = sizeof(SLVehicleCountData);
		SLVehicleCount.Header.byCheckSum = FormCheckSum((char *)&(SLVehicleCount.Header));
		
		for(i = 0;i < 16;i++)//lamp
		{
			SLVehicleCount.Data.byCount[i] = *p;
			SLVehicleCount.Data.byTime[i] = *(p + 16);
			p++;
		}

		SendDataEx((char *)&SLVehicleCount,sizeof(Rep_SLVehicleCount),(UINT8)1);
		}
		break;

	case 0x83:
		{
		char *p = MsgPtr + HU_HEAD_LEN;
		Rep_SLEnvData SLNowEnvData;

		CrossDataEventProc(Port,bCmdNo);

		if(MsgLen == HU_HEAD_LEN + 1) 
		{
			DebugWindow("Received 0x83 Ack Message!");
			return;
		}
		else if(MsgLen < HU_HEAD_LEN + 4 + 1) 
		{
			DebugWindow("0x83 Message Length Too Short!");
			return;
		}

		if(IsDataAllNull((UINT8 *)p,(UINT16)4))
		{
#ifdef TEST
			memset(disp,0,200);
			sprintf(disp,"Received Cross=%d,CommandNo=%02xH Ack",Port,bCmdNo);
			DebugWindow(disp);
#endif
			return;
		}

#ifdef FORDEBUG
		memset(disp,0,200);
		sprintf(disp,"Received Cross=%d,CommandNo=%02xH Data",Port,bCmdNo);
		DebugWindow(disp);
#endif

		memset((void *)(&SLNowEnvData),0,sizeof(Rep_SLEnvData));
		SLNowEnvData.Header.byFlag = MsgFlag;
		SLNowEnvData.Header.MsgType = CM_SLAIRDATA;

		strcpy(SLNowEnvData.Header.SourceIP,HostIp);
		strcpy(SLNowEnvData.Header.SourceID,HostId);
		strcpy(SLNowEnvData.Header.TargetIP,SvrIp);
		strcpy(SLNowEnvData.Header.TargetID,SvrId);

		SLNowEnvData.Header.iCrossNo = CrossData[Port - 1].lkbh;
		SLNowEnvData.Header.iLength = sizeof(SLEnvData);
		SLNowEnvData.Header.byCheckSum = FormCheckSum((char *)&(SLNowEnvData.Header));
		
		SLNowEnvData.Data.byNoise = *p;
		SLNowEnvData.Data.byCO = *(p + 1);
		SLNowEnvData.Data.bySO2 = *(p + 2);
		SLNowEnvData.Data.byTemp = *(p + 3);			

		SendDataEx((char *)&SLNowEnvData,sizeof(Rep_SLEnvData),(UINT8)1);
		}
		break;

	case 0x84:
		{
		char *p = MsgPtr + HU_HEAD_LEN;
		int i;
		Rep_SLFlowData SLNowFlow;

		CrossDataEventProc(Port,bCmdNo);

		if(MsgLen == HU_HEAD_LEN + 1) 
		{
			DebugWindow("Received 0x84 Ack Message!");
			return;
		}
		else if(MsgLen < HU_HEAD_LEN + 32 + 1) 
		{
			DebugWindow("0x84 Message Length Too Short!");
			return;
		}

		if(IsDataAllNull((UINT8 *)p,(UINT16)32))
		{
#ifdef TEST
			memset(disp,0,200);
			sprintf(disp,"Received Cross=%d,CommandNo=%02xH Ack",Port,bCmdNo);
			DebugWindow(disp);
#endif
			return;
		}

#ifdef FORDEBUG
		memset(disp,0,200);
		sprintf(disp,"Received Cross=%d,CommandNo=%02xH Data",Port,bCmdNo);
		DebugWindow(disp);
#endif

		memset((void *)(&SLNowFlow),0,sizeof(Rep_SLFlowData));
		SLNowFlow.Header.byFlag = MsgFlag;
		SLNowFlow.Header.MsgType = CM_SLFLOWDATA;

		strcpy(SLNowFlow.Header.SourceIP,HostIp);
		strcpy(SLNowFlow.Header.SourceID,HostId);
		strcpy(SLNowFlow.Header.TargetIP,SvrIp);
		strcpy(SLNowFlow.Header.TargetID,SvrId);

		SLNowFlow.Header.iCrossNo = CrossData[Port - 1].lkbh;
		SLNowFlow.Header.iLength = sizeof(SLVehicleFlowData);
		SLNowFlow.Header.byCheckSum = FormCheckSum((char *)&(SLNowFlow.Header));
		
		for(i = 0;i < 16;i++)//lamp
		{
			SLNowFlow.Data.byFlow[i] = *p;
			SLNowFlow.Data.byOccupy[i] = *(p + 16);
			p++;
		}

		SendDataEx((char *)&SLNowFlow,sizeof(Rep_SLFlowData),(UINT8)1);
		}
		break;

	case 0x85:
		{
		char *p = MsgPtr + HU_HEAD_LEN;
		CMsgFrame NowTestData;

		CrossDataEventProc(Port,bCmdNo);

		if(MsgLen == HU_HEAD_LEN + 1) 
		{
			DebugWindow("Received 0x85 Ack Message!");
			return;
		}
		else if(MsgLen < HU_HEAD_LEN + 1 + 1) 
		{
			DebugWindow("0x85 Message Length Too Short!");
			return;
		}

/*	if(IsDataAllNull((UINT8 *)p,(UINT16)1))
		{
#ifdef TEST
			memset(disp,0,200);
			sprintf(disp,"Received Cross=%d,CommandNo=%02xH Ack",Port,bCmdNo);
			DebugWindow(disp);
#endif
			return;
		}
*/
#ifdef FORDEBUG
		memset(disp,0,200);
		sprintf(disp,"Received Cross=%d,CommandNo=%02xH Data",Port,bCmdNo);
		DebugWindow(disp);
#endif

		if(bReserved == 0xff)//just the system test
			return;

		memset((void *)(&NowTestData),0,sizeof(CMsgFrame));
		NowTestData.byFlag = MsgFlag;
		NowTestData.MsgType = CM_SLCOMTEST;

		strcpy(NowTestData.SourceIP,HostIp);
		strcpy(NowTestData.SourceID,HostId);
		strcpy(NowTestData.TargetIP,SvrIp);
		strcpy(NowTestData.TargetID,SvrId);

		NowTestData.iCrossNo = CrossData[Port - 1].lkbh;
		NowTestData.iLength = 0;
		NowTestData.iReserved1 = *p;
		NowTestData.byCheckSum = FormCheckSum((char *)&NowTestData);

		SendDataEx((char *)&NowTestData,sizeof(CMsgFrame),(UINT8)1);
		}
		break;

	case 0x86:
		{
		char *p = MsgPtr + HU_HEAD_LEN;
		Rep_UpStatus UpStatusData;

		CrossDataEventProc(Port,bCmdNo);

		if(MsgLen == HU_HEAD_LEN + 1) 
		{
			DebugWindow("Received 0x86 Ack Message!");
			return;
		}
		else if(MsgLen < HU_HEAD_LEN + 11 + 1) 
		{
			DebugWindow("0x86 Message Length Too Short!");
			return;
		}

		if(IsDataAllNull((UINT8 *)p,(UINT16)10))
		{
#ifdef TEST
			memset(disp,0,200);
			sprintf(disp,"Received Cross=%d,CommandNo=%02xH Ack",Port,bCmdNo);
			DebugWindow(disp);
#endif
			return;
		}

#ifdef FORDEBUG
		memset(disp,0,200);
		sprintf(disp,"Received Cross=%d,CommandNo=%02xH Data",Port,bCmdNo);
		DebugWindow(disp);
#endif

		memset((void *)(&UpStatusData),0,sizeof(Rep_UpStatus));
		UpStatusData.Header.byFlag = MsgFlag;
		UpStatusData.Header.MsgType = CM_SLCURSTATUS;

		strcpy(UpStatusData.Header.SourceIP,HostIp);
		strcpy(UpStatusData.Header.SourceID,HostId);
		strcpy(UpStatusData.Header.TargetIP,SvrIp);
		strcpy(UpStatusData.Header.TargetID,SvrId);

		UpStatusData.Header.iCrossNo = CrossData[Port - 1].lkbh;
		UpStatusData.Header.iLength = sizeof(SLUpStatusData);
		UpStatusData.Header.byCheckSum = FormCheckSum((char *)&(UpStatusData.Header));

		UpStatusData.Data.byCtrlMode = *p;		
		UpStatusData.Data.byFlowGroupNo = *(p + 1);
		UpStatusData.Data.byGreenTimeLeft = *(p + 2);
		UpStatusData.Data.byGreenFlashTimeLeft = *(p + 3);
		UpStatusData.Data.byYellowTimeLeft = *(p + 4);			
		UpStatusData.Data.byAllRedTimeLeft = *(p + 5);
		UpStatusData.Data.byTimePlanNo = *(p + 6);
		UpStatusData.Data.byTimeBandNo = *(p + 7);
		UpStatusData.Data.byCtrlPlanNo = *(p + 8);			
		UpStatusData.Data.IsYellowBlink = *(p + 9);
		UpStatusData.Data.IsLampOff = *(p + 10);


		SendDataEx((char *)&UpStatusData,sizeof(Rep_UpStatus),(UINT8)1);
		}
		break;

	default:
		DebugWindow("SanlianSerialProc(),Received Unrecognized Command!");
		break;
	}

	return;
}

/***************************************************************************
*	Function Name	: BOOL JSSerialProc(int Port,char *MsgPtr,UINT16 MsgLen)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/02,20,2002
*	Global			: None
*	Note			: None
****************************************************************************/	
void JSSerialProc(int Port,UINT8 CmdNo,char *MsgPtr,int MsgLen)
{
#ifdef TEST
	char disp[200];
#endif

	if(Port <= 0 || Port > MAX_CROSS_NUM)
	{
		DebugWindow("JSSerialProc(),Input Error!");
		return;
	}

	if(MsgPtr == NULL)
	{
		DebugWindow("JSSerialProc(),MsgPtr Error!");
		return;
	}

	if(MsgLen >= MAX_YLF_LEN - EFFECT_YLF_HEAD_LEN)
	{
		DebugWindow("JSSerialProc(),Msg Length Error!");
		return;
	}

	CrossData[Port - 1].NoSignalCount = 0;
	CrossData[Port - 1].DisconnectCount = 0;	
	if(CrossData[Port - 1].LinkFlag != TRUE)
	{
#ifdef TEST
		memset(disp,0,200);
		sprintf(disp,"Send Connect To Port %d!",Port);
		DebugWindow(disp);
#endif
		SendCommStatus2Wu(Port,0);
		CrossData[Port - 1].LinkFlag = TRUE;
	}

	switch(CmdNo)
	{
	case 0x01:
#ifdef TEST
		memset(disp,0,200);
		sprintf(disp,"JSSerialProc(),Cross=%d Receive 0x%02X Ack Msg!",Port,CmdNo);
		DebugWindow(disp);
#endif
		JSAckEventProc(Port,CmdNo);

		break;

	case 0x11:
#ifdef TEST
		memset(disp,0,200);
		sprintf(disp,"JSSerialProc(),Cross=%d Receive 0x%02X Data Msg!",Port,CmdNo);
		DebugWindow(disp);
#endif
		JS11DataEventProc(Port,CmdNo,MsgPtr,MsgLen);
		JSAckEventProc(Port,CmdNo);

		break;

	case 0x02:
#ifdef TEST
		memset(disp,0,200);
		sprintf(disp,"JSSerialProc(),Cross=%d Receive 0x%02X Ack Msg!",Port,CmdNo);
		DebugWindow(disp);
#endif
		JSAckEventProc(Port,CmdNo);

		break;

	case 0x12:
#ifdef TEST
		memset(disp,0,200);
		sprintf(disp,"JSSerialProc(),Cross=%d Receive 0x%02X Data Msg!",Port,CmdNo);
		DebugWindow(disp);
#endif
		JS12DataEventProc(Port,CmdNo,MsgPtr,MsgLen);
		JSAckEventProc(Port,CmdNo);

		break;

	case 0x03:
#ifdef TEST
		memset(disp,0,200);
		sprintf(disp,"JSSerialProc(),Cross=%d Receive 0x%02X Ack Msg!",Port,CmdNo);
		DebugWindow(disp);
#endif
		JSAckEventProc(Port,CmdNo);

		break;

	case 0x13:
#ifdef TEST
		memset(disp,0,200);
		sprintf(disp,"JSSerialProc(),Cross=%d Receive 0x%02X Data Msg!",Port,CmdNo);
		DebugWindow(disp);
#endif
		JS13DataEventProc(Port,CmdNo,MsgPtr,MsgLen);
		JSAckEventProc(Port,CmdNo);

		break;

	case 0x04:
#ifdef TEST
		memset(disp,0,200);
		sprintf(disp,"JSSerialProc(),Cross=%d Receive 0x%02X Ack Msg!",Port,CmdNo);
		DebugWindow(disp);
#endif
		JSAckEventProc(Port,CmdNo);

		break;

	case 0x14:
#ifdef TEST
		memset(disp,0,200);
		sprintf(disp,"JSSerialProc(),Cross=%d Receive 0x%02X Data Msg!",Port,CmdNo);
		DebugWindow(disp);
#endif
		JS14DataEventProc(Port,CmdNo,MsgPtr,MsgLen);
		JSAckEventProc(Port,CmdNo);

		break;

	case 0x06:
#ifdef TEST
		memset(disp,0,200);
		sprintf(disp,"JSSerialProc(),Cross=%d Receive 0x%02X Ack Msg!",Port,CmdNo);
		DebugWindow(disp);
#endif
		JSAckEventProc(Port,CmdNo);

		break;

	case 0x16:
#ifdef TEST
		memset(disp,0,200);
		sprintf(disp,"JSSerialProc(),Cross=%d Receive 0x%02X Data Msg!",Port,CmdNo);
		DebugWindow(disp);
#endif
		JS16DataEventProc(Port,CmdNo,MsgPtr,MsgLen);
		JSAckEventProc(Port,CmdNo);

		break;

	case 0x21:
#ifdef TEST
		memset(disp,0,200);
		sprintf(disp,"JSSerialProc(),Cross=%d Receive 0x%02X Data Msg!",Port,CmdNo);
		DebugWindow(disp);
#endif
		JS21DataEventProc(Port,CmdNo,MsgPtr,MsgLen);

		break;

	case 0x22:
#ifdef TEST
		memset(disp,0,200);
		sprintf(disp,"JSSerialProc(),Cross=%d Receive 0x%02X Data Msg!",Port,CmdNo);
		DebugWindow(disp);
#endif
		JS22DataEventProc(Port,CmdNo,MsgPtr,MsgLen);
		break;

	case 0x23:
#ifdef TEST
		memset(disp,0,200);
		sprintf(disp,"JSSerialProc(),Cross=%d Receive 0x%02X Data Msg!",Port,CmdNo);
		DebugWindow(disp);
#endif
		JS23DataEventProc(Port,CmdNo,MsgPtr,MsgLen);
		break;

	case 0xff:
#ifdef TEST
		memset(disp,0,200);
		sprintf(disp,"JSSerialProc(),Cross=%d Receive Data Error!",Port);
		DebugWindow(disp);
#endif
		JSErrEventProc(Port,0);
		break;

	default:
		DebugWindow("JSSerialProc(),Received Unrecognized Command!");
		break;
	}

	return;
}

/***************************************************************************
*	Function Name	: BOOL IsDataAllNull(UINT8 *pIn,UINT16 MsgLen)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/09,28,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
BOOL IsDataAllNull(UINT8 *pIn,UINT16 MsgLen)
{
	int i;

	if(pIn == NULL)
	{
		DebugWindow("IsDataAllNull(),Fatal Error!");
		return TRUE;
	}

	if(MsgLen == 0)
	{
		DebugWindow("IsDataAllNull(),MsgLen = 0!");
		return TRUE;
	}

	for(i = 0;i < MsgLen;i++)
	{
		if( *(pIn + i) != (UINT8)0 )
			return FALSE;
	}

	return TRUE;	
}


/***************************************************************************
*	Function Name	: BOOL IsStopSent(int Port,UINT8 CmdNo)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/08,31,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
BOOL IsStopSent(int Port,UINT8 CmdNo)
{
	BYTE i;
	BYTE j;
	BYTE count;

	if(Port <= 0 || Port > MAX_CROSS_NUM)
	{
		DebugWindow("IsStopSent(),Input Port Error!");
		return FALSE;
	}
	
	i = (UINT8)(Port - 1);

	if(CrossData[i].WaitSendCount <= 0)
		return FALSE;

	if(CrossData[i].WaitHead < 0 || CrossData[i].WaitHead >= MAX_RESEND_NUM)
	{
		DebugWindow("IsStopSent(),Exceeding Max_Resend_Num!");
		ClearSanlianSendQueue((UINT8)Port);
		return FALSE;
	}

	j = CrossData[i].WaitHead;
	count = CrossData[i].WaitSendCount;
	while(j < CrossData[i].WaitEnd || count <= 0)
	{
		if(CrossData[i].DownMsgBuf[j].CmdNo == (UINT16)CmdNo)
			return TRUE;

		j = (j + 1) % MAX_RESEND_NUM;
		count--;
	}
	
	return FALSE;
}


/***************************************************************************
*	Function Name	: void CrossDataEventProc(int Port,UINT8 CmdNo)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/08,10,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
void CrossDataEventProc(int Port,UINT8 CmdNo)
{
	char *MsgHeadPtr;
	int MsgLen;	
	BYTE i;

	if(Port <= 0 || Port > MAX_CROSS_NUM)
	{
		DebugWindow("CrossDataEventProc(),Input Port Error!");
		return;
	}
	
	i = (UINT8)(Port - 1);

	if(CrossData[i].WaitHead < 0 || CrossData[i].WaitHead >= MAX_RESEND_NUM)
	{
		DebugWindow("CrossDataEventProc(),Exceeding Max_Resend_Num!");
		ClearSanlianSendQueue((UINT8)Port);
		return;
	}	

	if(CrossData[i].WaitSendCount <= 0)
		return;//no data to wait now

	if(CrossData[i].DownMsgBuf[CrossData[i].WaitHead].Status != BUF_SEND_STATUS)
		return;//Status Error,just return

	if(CrossData[i].DownMsgBuf[CrossData[i].WaitHead].CmdNo != (UINT16)CmdNo)
		return;//not the first time send data,just return

	DeleteDownBufHead((UINT8)Port);

	if(CrossData[i].WaitSendCount <= 0)
		return;//no data to send now

	//send a new command to cross
	MsgHeadPtr = (char *)( CrossData[i].DownMsgBuf[CrossData[i].WaitHead].hMsgBuf );
	MsgLen = CrossData[i].DownMsgBuf[CrossData[i].WaitHead].MsgLen;

	if((MsgLen > 260) || (MsgLen <= 0))
	{
		DebugWindow("CrossDataEventProc(),Out of Buffer Length!");
		ClearSanlianSendQueue((UINT8)Port);
		return;
	}
	
	Notify(Port,SERIAL_WRITE,(UINT8 *)MsgHeadPtr,(UINT16)MsgLen);

	CrossData[i].DownMsgBuf[CrossData[i].WaitHead].Status = BUF_SEND_STATUS;
	CrossData[i].DownMsgBuf[CrossData[i].WaitHead].SendNum = 1;
	CrossData[i].DownMsgBuf[CrossData[i].WaitHead].Timeout = MAX_HU_WAIT;//80以上的命令应立即返回值
		
	return;
}

/***************************************************************************
*	Function Name	: void ZhaoAckEventProc(int Port,UINT8 CmdNo)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/09,24,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
void ZhaoAckEventProc(int Port,UINT8 CmdNo)
{
	BYTE i;
	BYTE CrossNo;

	if(Port <= 0 || Port > MAX_CROSS_NUM)
	{
		DebugWindow("ZhaoAckEventProc(),Input Port Error!");
		return;
	}
	
	i = (UINT8)(Port - 1);

/*	if(CmdNo == (UINT8)0x17)
		CrossData[i].Status = FORCED_STATUS;
*/
	memset(&SendingBuf,0,sizeof(RecBuf));//added 10,10,2001

//	if(SendingBuf.Status == BUF_SEND_STATUS && SendingBuf.CmdNo == CmdNo)
	{
		CrossNo = GetNextLcuHuItem((UINT8)Port);
		if(CrossNo == 0)//not found
			return;
		
		i = CrossNo - 1;

		SendingBuf.Status = BUF_SEND_STATUS;
		SendingBuf.CrossNo = CrossNo;
		SendingBuf.CmdNo = (UINT8)(CrossData[i].DownMsgBuf[CrossData[i].WaitHead].CmdNo);
		SendingBuf.CardNo = CrossData[i].CardNo;
		SendingBuf.ReSendTimes = 1;
		SendingBuf.Timeout = 2;
		SendingBuf.MsgLen = CrossData[i].DownMsgBuf[CrossData[i].WaitHead].MsgLen;
		memcpy(SendingBuf.MsgData,(void *)(CrossData[i].DownMsgBuf[CrossData[i].WaitHead].hMsgBuf),\
					SendingBuf.MsgLen);

		DeleteDownBufHead(CrossNo);
		Write2Lpt(SendingBuf.MsgData,SendingBuf.MsgLen,SendingBuf.CardNo);
		return;
	}
	
	return;
}

/***************************************************************************
*	Function Name	: void CrossAckEventProc(int Port,UINT8 CmdNo)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/08,10,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
void CrossAckEventProc(int Port,UINT8 CmdNo)
{
	char *MsgHeadPtr;
	int MsgLen;	
	BYTE i;

	if(Port <= 0 || Port > MAX_CROSS_NUM)
	{
		DebugWindow("CrossAckEventProc(),Input Port Error!");
		return;
	}
	
	i = (UINT8)(Port - 1);

//	if(CmdNo == (UINT8)0x17)
//		CrossData[i].Status = FORCED_STATUS;
//	else if(CmdNo == (UINT8)0x29)
//		CrossData[i].Status = IDLE_STATUS;

	if(CrossData[i].WaitHead < 0 || CrossData[i].WaitHead >= MAX_RESEND_NUM)
	{
		DebugWindow("CrossAckEventProc(),Exceeding Max_Resend_Num!");
		ClearSanlianSendQueue((UINT8)Port);
		return;
	}	

	if(CrossData[i].WaitSendCount <= 0)
		return;//no data to wait now

	if(CrossData[i].DownMsgBuf[CrossData[i].WaitHead].Status != BUF_SEND_STATUS)
		return;//Status Error,just return

	if(CrossData[i].DownMsgBuf[CrossData[i].WaitHead].CmdNo != CmdNo)
	{
		//the cross event received so late,we have already deleted,
		//we check if have some data to send and return
		if(CrossData[i].DownMsgBuf[CrossData[i].WaitHead].Status == BUF_QUE_STATUS)
		{
		//the head not send
			MsgHeadPtr = CrossData[i].DownMsgBuf[CrossData[i].WaitHead].hMsgBuf;
			MsgLen = (UINT16)(CrossData[i].DownMsgBuf[CrossData[i].WaitHead].MsgLen);

			Notify(i + 1,SERIAL_WRITE,(UINT8 *)MsgHeadPtr,(UINT16)MsgLen);

			CrossData[i].DownMsgBuf[CrossData[i].WaitHead].Status = BUF_SEND_STATUS;
			CrossData[i].DownMsgBuf[CrossData[i].WaitHead].SendNum = 1;
			CrossData[i].DownMsgBuf[CrossData[i].WaitHead].Timeout = MAX_HU_WAIT;//80以上的命令应立即返回值
		}

		return;
	}

	DeleteDownBufHead((UINT8)Port);

	if(CrossData[i].WaitSendCount <= 0)
		return;//no data to send now

	//resend or send a new command to cross
	MsgHeadPtr = (char *)( CrossData[i].DownMsgBuf[CrossData[i].WaitHead].hMsgBuf );
	MsgLen = CrossData[i].DownMsgBuf[CrossData[i].WaitHead].MsgLen;

	if((MsgLen > 260) || (MsgLen <= 0))
	{
		DebugWindow("CrossAckEventProc(),Out of Buffer Length!");
		ClearSanlianSendQueue((UINT8)Port);
		return;
	}
	
	Notify(Port,SERIAL_WRITE,(UINT8 *)MsgHeadPtr,(UINT16)MsgLen);

	CrossData[i].DownMsgBuf[CrossData[i].WaitHead].Status = BUF_SEND_STATUS;
	CrossData[i].DownMsgBuf[CrossData[i].WaitHead].SendNum = 1;
	CrossData[i].DownMsgBuf[CrossData[i].WaitHead].Timeout = MAX_HU_WAIT;//80以上的命令应立即返回值
		
	return;
}

/***************************************************************************
*	Function Name	: BOOL JSCancelProc(int Port,UINT8 CmdNo)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/04,01,2002
*	Global			: None
*	Note			: None
****************************************************************************/	
void JSCancelLeftItems(int Port,UINT8 CmdNo,UINT8 ItemStart,UINT8 MaxItemNo)
{
	int count;

	if(ItemStart >= MaxItemNo)
		return;

	count = ItemStart;
	while(JSCancelProc(Port,CmdNo) && count <= MaxItemNo)
	{
		count++;
	}

	return;
}

/***************************************************************************
*	Function Name	: BOOL JSCancelProc(int Port,UINT8 CmdNo)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/04,01,2002
*	Global			: None
*	Note			: None
****************************************************************************/	
BOOL JSCancelProc(int Port,UINT8 CmdNo)
{
	BYTE i;

	if(Port <= 0 || Port > MAX_CROSS_NUM)
	{
		DebugWindow("JSCancelProc(),Input Port Error!");
		return FALSE;
	}
	
	i = (UINT8)(Port - 1);

	if(CrossData[i].WaitHead < 0 || CrossData[i].WaitHead >= MAX_RESEND_NUM)
	{
		DebugWindow("JSCancelProc(),Exceeding Max_Resend_Num!");
		ClearSanlianSendQueue((UINT8)Port);
		return FALSE;
	}	

	if(CrossData[i].WaitSendCount <= 0)
		return FALSE;//no data to wait now

	if(CrossData[i].DownMsgBuf[CrossData[i].WaitHead].Status != BUF_QUE_STATUS						\
				&& CrossData[i].DownMsgBuf[CrossData[i].WaitHead].Status != BUF_SEND_STATUS)
	{
		return FALSE;//Status Error,just return
	}

	if(CrossData[i].DownMsgBuf[CrossData[i].WaitHead].CmdNo != CmdNo)
	{
		return FALSE;
	}

	DeleteDownBufHead((UINT8)Port);
		return TRUE;

	return FALSE;
}

/***************************************************************************
*	Function Name	: void JSAckEventProc(int Port,UINT8 CmdNo)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/02,21,2002
*	Global			: None
*	Note			: None
****************************************************************************/	
void JSAckEventProc(int Port,UINT8 CmdNo)
{
	BYTE i;

	if(Port <= 0 || Port > MAX_CROSS_NUM)
	{
		DebugWindow("JSAckEventProc(),Input Port Error!");
		return;
	}
	
	i = (UINT8)(Port - 1);

	if(CrossData[i].WaitHead < 0 || CrossData[i].WaitHead >= MAX_RESEND_NUM)
	{
		DebugWindow("JSAckEventProc(),Exceeding Max_Resend_Num!");
		ClearSanlianSendQueue((UINT8)Port);
		return;
	}	

	if(CrossData[i].WaitSendCount <= 0)
		return;//no data to wait now

	if(CrossData[i].DownMsgBuf[CrossData[i].WaitHead].Status != BUF_SEND_STATUS)
		return;//Status Error,just return

	if(CrossData[i].DownMsgBuf[CrossData[i].WaitHead].CmdNo != CmdNo)
	{
		//the cross event received so late,we have already deleted,
		//we check if have some data to send and return
		if(CrossData[i].DownMsgBuf[CrossData[i].WaitHead].Status == BUF_QUE_STATUS)
		{
			//the head not send
			Sleep(10);
			PostMessage(hMainWnd,EXTENDMSGSEND,(WPARAM)Port,(LPARAM)0);
		}

		return;
	}

	DeleteDownBufHead((UINT8)Port);

	if(CrossData[i].WaitSendCount <= 0)
		return;//no data to send now

	//send a new command to cross
	Sleep(10);
	PostMessage(hMainWnd,EXTENDMSGSEND,(WPARAM)Port,(LPARAM)0);
		
	return;
}

/***********************************************************************************************
*	Function Name	: void JS11DataEventProc(int Port,UINT8 CmdNo,char *MsgPtr,int MsgLen)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/02,21,2002
*	Global			: None
*	Note			: None
************************************************************************************************/	
void JS11DataEventProc(int Port,UINT8 CmdNo,char *MsgPtr,int MsgLen)
{
	Cmd_StepLampColorItem RepData;
	BYTE i;
	int count;

	if(Port <= 0 || Port > MAX_CROSS_NUM)
	{
		DebugWindow("JS11DataEventProc(),Input Port Error!");
		return;
	}
	
	if(MsgLen == 0)
		MsgLen = 0;
	if(MsgPtr == NULL)
	{
		DebugWindow("JS11DataEventProc(),MsgPtr Error!");
		return;
	}

	if(MsgLen >= MAX_YLF_LEN - EFFECT_YLF_HEAD_LEN)
	{
		DebugWindow("JS11DataEventProc(),Msg Length Error!");
		return;
	}

	memset((void *)(&RepData),0,sizeof(Cmd_StepLampColorItem));
	RepData.Header.byFlag = MsgFlag;
	RepData.Header.MsgType = CM_SENDSTEPLAMPCOLORITEM;

	strcpy(RepData.Header.SourceIP,HostIp);
	strcpy(RepData.Header.SourceID,HostId);
	strcpy(RepData.Header.TargetIP,SvrIp);
	strcpy(RepData.Header.TargetID,SvrId);

	RepData.Header.iCrossNo = CrossData[Port - 1].lkbh;
	RepData.Header.iLength = sizeof(StepLampColorItem);
	RepData.Header.byCheckSum = FormCheckSum((char *)&(RepData.Header));

	if(MsgLen > 2)
	{
		RepData.StepLampColorItemData.bStepNo = (UINT8)((UINT8)(*MsgPtr) + 1);
		count = GetMinLen(MaxLampNum,MsgLen - 2,MaxLampNum);
		for(i = 0;i < MaxLampNum,count > 0;i++)
		{
			RepData.StepLampColorItemData.LampColor[i] = (UINT8)( *(MsgPtr + 2 + i) );
			count--;
		}

		//send to svr(wu)
		SendDataEx((char *)&RepData,sizeof(Cmd_StepLampColorItem),(UINT8)1);

		if( (UINT8)(*MsgPtr) + 1 == (UINT8)(*(MsgPtr + 1)) )
		{
			//BUF_CANCELED_STATUS
			JSCancelLeftItems(Port,CmdNo,(UINT8)(*(MsgPtr + 1)),MaxStepNum);
		}
	}
	else if(MsgLen == 0)
	{
		JSCancelLeftItems(Port,CmdNo,(UINT8)4,MaxStepNum);
	}

	return;
}

/*********************************************************************************************
*	Function Name	: void JS12DataEventProc(int Port,UINT8 CmdNo,char *MsgPtr,int MsgLen)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/02,21,2002
*	Global			: None
*	Note			: None
*********************************************************************************************/	
void JS12DataEventProc(int Port,UINT8 CmdNo,char *MsgPtr,int MsgLen)
{
	Cmd_StepTimePlanItem RepData;
	BYTE i;
	int count;

	if(Port <= 0 || Port > MAX_CROSS_NUM)
	{
		DebugWindow("JS12DataEventProc(),Input Port Error!");
		return;
	}
	
	if(MsgPtr == NULL)
	{
		DebugWindow("JS12DataEventProc(),MsgPtr Error!");
		return;
	}

	if(MsgLen >= MAX_YLF_LEN - EFFECT_YLF_HEAD_LEN)
	{
		DebugWindow("JS12DataEventProc(),Msg Length Error!");
		return;
	}

	memset((void *)(&RepData),0,sizeof(Cmd_StepTimePlanItem));
	RepData.Header.byFlag = MsgFlag;
	RepData.Header.MsgType = CM_SENDSTEPTIMEPLANITEM;

	strcpy(RepData.Header.SourceIP,HostIp);
	strcpy(RepData.Header.SourceID,HostId);
	strcpy(RepData.Header.TargetIP,SvrIp);
	strcpy(RepData.Header.TargetID,SvrId);

	RepData.Header.iCrossNo = CrossData[Port - 1].lkbh;
	RepData.Header.iLength = sizeof(StepTimePlanItem);
	RepData.Header.byCheckSum = FormCheckSum((char *)&(RepData.Header));

	if(MsgLen <= 2)
	{
		//this only a ack from ylf
		return;
	}

	RepData.StepTimePlanItemData.bStepTimePlanNum = (UINT8)((UINT8)(*MsgPtr) + 1);
	RepData.StepTimePlanItemData.bStepNum = (UINT8)( *(MsgPtr + 1) );

	count = GetMinLen(RepData.StepTimePlanItemData.bStepNum,MsgLen - 2,MaxStepNum);
	for(i = 0;i < RepData.StepTimePlanItemData.bStepNum,count > 0;i++)
	{
		RepData.StepTimePlanItemData.StepLen[i] = (UINT8)( *(MsgPtr + 2 + i) );
		count--;
	}

	//send to svr(wu)
	SendDataEx((char *)&RepData,sizeof(Cmd_StepTimePlanItem),(UINT8)1);

	return;
}

/***********************************************************************************************
*	Function Name	: void JS13DataEventProc(int Port,UINT8 CmdNo,char *MsgPtr,int MsgLen)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/02,21,2002
*	Global			: None
*	Note			: None
************************************************************************************************/	
void JS13DataEventProc(int Port,UINT8 CmdNo,char *MsgPtr,int MsgLen)
{
	Cmd_TimeBandDown RepData;
	BYTE i;
	int count;
	BYTE hour,minute,second;

	if(Port <= 0 || Port > MAX_CROSS_NUM)
	{
		DebugWindow("JS13DataEventProc(),Input Port Error!");
		return;
	}
	
	if(MsgPtr == NULL)
	{
		DebugWindow("JS13DataEventProc(),MsgPtr Error!");
		return;
	}

	if(MsgLen >= MAX_YLF_LEN - EFFECT_YLF_HEAD_LEN)
	{
		DebugWindow("JS13DataEventProc(),Msg Length Error!");
		return;
	}

	memset((void *)(&RepData),0,sizeof(Cmd_TimeBandDown));
	RepData.Header.byFlag = MsgFlag;
	RepData.Header.MsgType = CM_SENDTIMEBANDPLAN;

	strcpy(RepData.Header.SourceIP,HostIp);
	strcpy(RepData.Header.SourceID,HostId);
	strcpy(RepData.Header.TargetIP,SvrIp);
	strcpy(RepData.Header.TargetID,SvrId);

	RepData.Header.iCrossNo = CrossData[Port - 1].lkbh;
	RepData.Header.iLength = sizeof(TimeBandDown);
	RepData.Header.byCheckSum = FormCheckSum((char *)&(RepData.Header));

	if(MsgLen <= 2)
	{
		//this only a ack from ylf
		return;
	}

	RepData.PlanDownData.byTimeBandPlanNo = (UINT8)((UINT8)(*MsgPtr) + 1);
	RepData.PlanDownData.byTimeBandNum = (UINT8)( *(MsgPtr + 1) );

	count = GetMinLen(RepData.PlanDownData.byTimeBandNum,(MsgLen - 2) / 4,MaxTimeBandNum);
	for(i = 0;i < RepData.PlanDownData.byTimeBandNum,count > 0;i++)
	{
		hour = (UINT8)( *(MsgPtr + 2 + 4 * i) );
		minute = (UINT8)( *(MsgPtr + 2 + 4 * i + 1) );
		second = (UINT8)( *(MsgPtr + 2 + 4 * i + 2) );
		RepData.PlanDownData.StartTime[i] = (float)(hour * 60 + minute + (float)second / 60.0);
		RepData.PlanDownData.byPlanNo[i] = (UINT8)((UINT8)( *(MsgPtr + 2 + 4 * i + 3) ) + 1);

		count--;// -= 4;
	}

	//send to svr(wu)
	SendDataEx((char *)&RepData,sizeof(Cmd_TimeBandDown),(UINT8)1);

	return;
}

/***********************************************************************************************
*	Function Name	: void JS14DataEventProc(int Port,UINT8 CmdNo,char *MsgPtr,int MsgLen)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/02,21,2002
*	Global			: None
*	Note			: None
************************************************************************************************/	
void JS14DataEventProc(int Port,UINT8 CmdNo,char *MsgPtr,int MsgLen)
{
	Cmd_DatePlanItemUp RepData;
	BYTE year,month;

	if(Port <= 0 || Port > MAX_CROSS_NUM)
	{
		DebugWindow("JS14DataEventProc(),Input Port Error!");
		return;
	}
	
	if(MsgPtr == NULL)
	{
		DebugWindow("JS14DataEventProc(),MsgPtr Error!");
		return;
	}

	if(MsgLen >= MAX_YLF_LEN - EFFECT_YLF_HEAD_LEN)
	{
		DebugWindow("JS14DataEventProc(),Msg Length Error!");
		return;
	}

	memset((void *)(&RepData),0,sizeof(Cmd_DatePlanItemUp));
	RepData.Header.byFlag = MsgFlag;
	RepData.Header.MsgType = CM_SENDDATAPLAN;

	strcpy(RepData.Header.SourceIP,HostIp);
	strcpy(RepData.Header.SourceID,HostId);
	strcpy(RepData.Header.TargetIP,SvrIp);
	strcpy(RepData.Header.TargetID,SvrId);

	RepData.Header.iCrossNo = CrossData[Port - 1].lkbh;
	RepData.Header.iLength = sizeof(DatePlanItemUp);
	RepData.Header.byCheckSum = FormCheckSum((char *)&(RepData.Header));

	if(MsgLen <= 0)
	{
		//this only a ack from ylf
		return;
	}

	RepData.DatePlanItemUpData.byDateTypeNo = (UINT8)(*MsgPtr);
	year = (UINT8)( *(MsgPtr + 1) );
	month = (UINT8)( *(MsgPtr + 2) );
	RepData.DatePlanItemUpData.StartDate = (float)(year + (float)month / 100.0);
	year = (UINT8)( *(MsgPtr + 3) );
	month = (UINT8)( *(MsgPtr + 4) );
	RepData.DatePlanItemUpData.EndDate = (float)(year + (float)month / 100.0);
	RepData.DatePlanItemUpData.byPlanGroupNo = (UINT8)((UINT8)( *(MsgPtr + 5) ) + 1);//时段方案+1??

	//send to svr(wu)
	SendDataEx((char *)&RepData,sizeof(Cmd_DatePlanItemUp),(UINT8)1);

	return;
}

/***********************************************************************************************
*	Function Name	: void JS16DataEventProc(int Port,UINT8 CmdNo,char *MsgPtr,int MsgLen)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/02,21,2002
*	Global			: None
*	Note			: None
************************************************************************************************/	
void JS16DataEventProc(int Port,UINT8 CmdNo,char *MsgPtr,int MsgLen)
{
	CMD_BENCHMARK_TIME RepData;

	if(Port <= 0 || Port > MAX_CROSS_NUM)
	{
		DebugWindow("JS16DataEventProc(),Input Port Error!");
		return;
	}
	
	if(MsgPtr == NULL)
	{
		DebugWindow("JS16DataEventProc(),MsgPtr Error!");
		return;
	}

	if(MsgLen >= MAX_YLF_LEN - EFFECT_YLF_HEAD_LEN)
	{
		DebugWindow("JS16DataEventProc(),Msg Length Error!");
		return;
	}

	memset((void *)(&RepData),0,sizeof(CMD_BENCHMARK_TIME));
	RepData.Header.byFlag = MsgFlag;
	RepData.Header.MsgType = CM_SENDBENCHMARK;

	strcpy(RepData.Header.SourceIP,HostIp);
	strcpy(RepData.Header.SourceID,HostId);
	strcpy(RepData.Header.TargetIP,SvrIp);
	strcpy(RepData.Header.TargetID,SvrId);

	RepData.Header.iCrossNo = CrossData[Port - 1].lkbh;
	RepData.Header.iLength = sizeof(DATA_BENCHMARK_TIME);
	RepData.Header.byCheckSum = FormCheckSum((char *)&(RepData.Header));

	RepData.Time_Data.Year = (UINT8)(*MsgPtr);
	RepData.Time_Data.Month = (UINT8)( *(MsgPtr + 1) );
	RepData.Time_Data.Day = (UINT8)( *(MsgPtr + 2) );
	RepData.Time_Data.Hour = (UINT8)( *(MsgPtr + 3) );
	RepData.Time_Data.Minute = (UINT8)( *(MsgPtr + 4) );
	RepData.Time_Data.Second = (UINT8)( *(MsgPtr + 5) );
	RepData.Time_Data.Week = (UINT8)( *(MsgPtr + 6) );

	//send to svr(wu)
	SendDataEx((char *)&RepData,sizeof(CMD_BENCHMARK_TIME),(UINT8)1);

	return;
}

/***********************************************************************************************
*	Function Name	: void JS21DataEventProc(int Port,UINT8 CmdNo,char *MsgPtr,int MsgLen)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/02,21,2002
*	Global			: None
*	Note			: None
************************************************************************************************/	
void JS21DataEventProc(int Port,UINT8 CmdNo,char *MsgPtr,int MsgLen)
{
	Rep_LampGroupStatus RepData;
	BYTE temp;
	int count;
	int i;
	BYTE MaskRed = 0x04,MaskYellow = 0x04,MaskGreen = 0x01;

	if(Port <= 0 || Port > MAX_CROSS_NUM)
	{
		DebugWindow("JS21DataEventProc(),Input Port Error!");
		return;
	}
	
	if(MsgPtr == NULL)
	{
		DebugWindow("JS21DataEventProc(),MsgPtr Error!");
		return;
	}

	if(MsgLen >= MAX_YLF_LEN - EFFECT_YLF_HEAD_LEN)
	{
		DebugWindow("JS21DataEventProc(),Msg Length Error!");
		return;
	}

	memset((void *)(&RepData),0,sizeof(Rep_LampGroupStatus));
	RepData.Header.byFlag = MsgFlag;
	RepData.Header.MsgType = CM_LAMPCONDITION;

	strcpy(RepData.Header.SourceIP,HostIp);
	strcpy(RepData.Header.SourceID,HostId);
	strcpy(RepData.Header.TargetIP,SvrIp);
	strcpy(RepData.Header.TargetID,SvrId);

	RepData.Header.iCrossNo = CrossData[Port - 1].lkbh;
	RepData.Header.iLength = sizeof(LampGroupStatus);
	RepData.Header.byCheckSum = FormCheckSum((char *)&(RepData.Header));

	RepData.LampGroupStatusData.FaultLampNum = (UINT8)(*MsgPtr);

	count = GetMinLen(RepData.LampGroupStatusData.FaultLampNum,MsgLen - 1,MaxLampNum);
	for(i = 0;i < RepData.LampGroupStatusData.FaultLampNum,count > 0;i++)
	{
		temp = (UINT8)( *(MsgPtr + 1 + i) );

		RepData.LampGroupStatusData.FaultLampNo[i] = ((temp >> 3) & 0x1f);
		if((temp & MaskRed) > 0)
			RepData.LampGroupStatusData.LampGroupData[i].RedLamp = 1;
		if((temp & MaskYellow) > 0)
			RepData.LampGroupStatusData.LampGroupData[i].YellowLamp = 1;
		if((temp & MaskGreen) > 0)
			RepData.LampGroupStatusData.LampGroupData[i].BlueLamp = 1;

		count--;
	}

	//send to svr(wu)
	SendDataEx((char *)&RepData,sizeof(Rep_LampGroupStatus),(UINT8)1);

	return;
}

/***********************************************************************************************
*	Function Name	: void JS22DataEventProc(int Port,UINT8 CmdNo,char *MsgPtr,int MsgLen)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/02,21,2002
*	Global			: None
*	Note			: None
************************************************************************************************/	
void JS22DataEventProc(int Port,UINT8 CmdNo,char *MsgPtr,int MsgLen)
{
	int count;
	int i;

	Rep_DetectorStatus RepData;
	BYTE MaskRed = 0x04,MaskYellow = 0x04,MaskGreen = 0x01;

	if(Port <= 0 || Port > MAX_CROSS_NUM)
	{
		DebugWindow("JS22DataEventProc(),Input Port Error!");
		return;
	}
	
	if(MsgPtr == NULL)
	{
		DebugWindow("JS22DataEventProc(),MsgPtr Error!");
		return;
	}

	if(MsgLen >= MAX_YLF_LEN - EFFECT_YLF_HEAD_LEN)
	{
		DebugWindow("JS22DataEventProc(),Msg Length Error!");
		return;
	}

	memset((void *)(&RepData),0,sizeof(Rep_DetectorStatus));
	RepData.Header.byFlag = MsgFlag;
	RepData.Header.MsgType = CM_DETECTORCONDITION;

	strcpy(RepData.Header.SourceIP,HostIp);
	strcpy(RepData.Header.SourceID,HostId);
	strcpy(RepData.Header.TargetIP,SvrIp);
	strcpy(RepData.Header.TargetID,SvrId);

	RepData.Header.iCrossNo = CrossData[Port - 1].lkbh;
	RepData.Header.iLength = sizeof(DetectorStatus);
	RepData.Header.byCheckSum = FormCheckSum((char *)&(RepData.Header));

	RepData.DetectorStatusData.FaultDetectorNum = (UINT8)(*MsgPtr);

	count = GetMinLen(RepData.DetectorStatusData.FaultDetectorNum,MsgLen - 1,MaxDetectorNum);
	for(i = 0;i < RepData.DetectorStatusData.FaultDetectorNum,count > 0;i++)
	{
		RepData.DetectorStatusData.FaultDetectorNo[i] = (UINT8)( *(MsgPtr + 1 + i) );

		count--;
	}

	//send to svr(wu)
	SendDataEx((char *)&RepData,sizeof(Rep_DetectorStatus),(UINT8)1);

	return;
}

/***********************************************************************************************
*	Function Name	: void JS23DataEventProc(int Port,UINT8 CmdNo,char *MsgPtr,int MsgLen)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/02,21,2002
*	Global			: None
*	Note			: None
************************************************************************************************/	
void JS23DataEventProc(int Port,UINT8 CmdNo,char *MsgPtr,int MsgLen)
{
	Rep_YellowGateStatus RepData;
	BYTE temp;

	if(Port <= 0 || Port > MAX_CROSS_NUM)
	{
		DebugWindow("JS23DataEventProc(),Input Port Error!");
		return;
	}
	
	if(MsgPtr == NULL)
	{
		DebugWindow("JS23DataEventProc(),MsgPtr Error!");
		return;
	}

	if(MsgLen >= MAX_YLF_LEN - EFFECT_YLF_HEAD_LEN)
	{
		DebugWindow("JS23DataEventProc(),Msg Length Error!");
		return;
	}

	memset((void *)(&RepData),0,sizeof(Rep_YellowGateStatus));
	RepData.byFlag = MsgFlag;
	RepData.MsgType = CM_YELLOWGATESTATUS;

	strcpy(RepData.SourceIP,HostIp);
	strcpy(RepData.SourceID,HostId);
	strcpy(RepData.TargetIP,SvrIp);
	strcpy(RepData.TargetID,SvrId);

	RepData.iCrossNo = CrossData[Port - 1].lkbh;
	RepData.iLength = 0;

	temp = (UINT8)(*MsgPtr);
	if((temp & 0xf0) == 0xf0) 
		RepData.iReserved1 = 1;
	if((temp & 0x0f) == 0x0f) 
		RepData.iReserved2 = 1; 

	RepData.byCheckSum = FormCheckSum((char *)&RepData);

	//send to svr(wu)
	SendDataEx((char *)&RepData,sizeof(Rep_YellowGateStatus),(UINT8)1);

	return;
}

/***********************************************************************************************
*	Function Name	: void GetMinLen(int Port,UINT8 CmdNo,char *MsgPtr,int MsgLen)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/02,21,2002
*	Global			: None
*	Note			: None
************************************************************************************************/	
UINT32 GetMinLen(UINT32 ylfSet,UINT32 EffectMsgLen,UINT32 MaxBufLen)
{
	UINT32 temp;

	if(ylfSet != EffectMsgLen)
	{
		DebugWindow("GetMinLen(),ylfSet!=EffectMsgLen!");

		if(ylfSet < EffectMsgLen)
			temp = ylfSet;
		else
			temp = EffectMsgLen;

		if(MaxBufLen < temp)
			temp = MaxBufLen;

	}
	else 
	{
		if(ylfSet > MaxBufLen)
		{
			DebugWindow("GetMinLen(),ylfSet>MaxBufLen!");
			return MaxBufLen;
		}
		else
			return ylfSet;
	}

	return temp;
}

/***************************************************************************
*	Function Name	: void ZhaoErrEventProc(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/09,24,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
void ZhaoErrEventProc(void)
{
	BYTE CrossNo;
	BYTE i;

//	memset(&SendingBuf,0,sizeof(RecBuf));
	if(SendingBuf.Status != BUF_SEND_STATUS)
	{
#ifdef FORDEBUG
		DebugWindow("ZhaoErrEventProc(),Not In Send Status!");
#endif
//		return;
		CrossNo = GetNextLcuHuItem(1);
		if(CrossNo == 0)//not found
			return;
		
		i = CrossNo - 1;

		memset(&SendingBuf,0,sizeof(RecBuf));
		SendingBuf.Status = BUF_SEND_STATUS;
		SendingBuf.CrossNo = CrossNo;
		SendingBuf.CmdNo = (UINT8)(CrossData[i].DownMsgBuf[CrossData[i].WaitHead].CmdNo);
		SendingBuf.CardNo = CrossData[i].CardNo;
		SendingBuf.ReSendTimes = 1;
		SendingBuf.Timeout = 2;
		SendingBuf.MsgLen = CrossData[i].DownMsgBuf[CrossData[i].WaitHead].MsgLen;
		memcpy(SendingBuf.MsgData,(void *)(CrossData[i].DownMsgBuf[CrossData[i].WaitHead].hMsgBuf),\
					SendingBuf.MsgLen);

		DeleteDownBufHead(CrossNo);
		Write2Lpt(SendingBuf.MsgData,SendingBuf.MsgLen,SendingBuf.CardNo);
		return;
	}

	if(SendingBuf.ReSendTimes < 3)
	{
		Write2Lpt(SendingBuf.MsgData,SendingBuf.MsgLen,SendingBuf.CardNo);
		SendingBuf.ReSendTimes++;
		SendingBuf.Timeout = 2;
		DebugWindow("ZhaoErrEventProc(),Resend Data To Lcu!");
		return;
	}

	DebugWindow("ZhaoErrEventProc(),Exceed Max_Resend_Times!");

	//added 11,19,2001
	if(CrossData[SendingBuf.CrossNo - 1].ErrorNo != 10)
	{
		SendCommStatus2Wu(SendingBuf.CrossNo,10);

		CrossData[SendingBuf.CrossNo - 1].ErrorNo = 10;//10 may modify later 
	}
	//added ends

//	return;
	memset(&SendingBuf,0,sizeof(RecBuf));

	CrossNo = GetNextLcuHuItem(SendingBuf.CrossNo);
	if(CrossNo == 0)//not found
		return;
	
	i = CrossNo - 1;

	SendingBuf.Status = BUF_SEND_STATUS;
	SendingBuf.CrossNo = CrossNo;
	SendingBuf.CmdNo = (UINT8)(CrossData[i].DownMsgBuf[CrossData[i].WaitHead].CmdNo);
	SendingBuf.CardNo = CrossData[i].CardNo;
	SendingBuf.ReSendTimes = 1;
	SendingBuf.Timeout = 2;
	SendingBuf.MsgLen = CrossData[i].DownMsgBuf[CrossData[i].WaitHead].MsgLen;
	memcpy(SendingBuf.MsgData,(void *)(CrossData[i].DownMsgBuf[CrossData[i].WaitHead].hMsgBuf),\
				SendingBuf.MsgLen);

	DeleteDownBufHead(CrossNo);
	Write2Lpt(SendingBuf.MsgData,SendingBuf.MsgLen,SendingBuf.CardNo);

	return;
}
	
/***************************************************************************
*	Function Name	: UINT8 GetNextLcuHuItem(UINT8 CrossNo)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/09,24,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
UINT8 GetNextLcuHuItem(UINT8 CrossNo)
{
	int i;
	int count;
	UINT16 MsgLen;

	i = (CrossNo + MAX_CROSS_NUM - 1) % MAX_CROSS_NUM;
	count = 0;
	while(count < MAX_CROSS_NUM)
	{
		i = (i + 1) % MAX_CROSS_NUM;
		count++;

		if(CrossData[i].Type != 1)//sanlian
			continue;

		if(CrossData[i].WaitSendCount <= 0)
			continue;//no data to send

		if(CrossData[i].WaitHead < 0 || CrossData[i].WaitHead >= MAX_RESEND_NUM)
		{
			DebugWindow("GetNextLcuHuItem(),Exceeding Max_Resend_Num!");
			ClearSanlianSendQueue((UINT8)i);
			continue;
		}	

		MsgLen = CrossData[i].DownMsgBuf[CrossData[i].WaitHead].MsgLen;

		if((MsgLen > 260) || (MsgLen <= 0))
		{
			DebugWindow("GetNextLcuHuItem(),Out of Buffer Length!");
			ClearSanlianSendQueue((UINT8)i);
			continue;
		}

		return ((i + 1) % MAX_CROSS_NUM);
	}
	
	return 0;
}

/***************************************************************************
*	Function Name	: void CrossErrEventProc(int Port,UINT8 CmdNo)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/08,10,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
void CrossErrEventProc(int Port,UINT8 CmdNo)
{
	char *MsgHeadPtr;
	int MsgLen;	
	BYTE i;

	if(Port <= 0 || Port > MAX_CROSS_NUM)
	{
		DebugWindow("CrossErrEventProc(),Input Port Error!");
		return;
	}
	
	i = (UINT8)(Port - 1);

	if(CrossData[i].WaitHead < 0 || CrossData[i].WaitHead >= MAX_RESEND_NUM)
	{
		DebugWindow("CrossErrEventProc(),Exceeding Max_Resend_Num!");
		ClearSanlianSendQueue((UINT8)Port);
		return;
	}	

	if(CrossData[i].WaitSendCount <= 0)
		return;//no data to wait now

	if(CrossData[i].DownMsgBuf[CrossData[i].WaitHead].Status != BUF_SEND_STATUS)
		return;//Status Error,just return

	//notice: if cross event received so late(we have already deleted),we may send other
	//command,because we have no information to identify the command
	if(CrossData[i].DownMsgBuf[CrossData[i].WaitHead].SendNum >= 3 \
		|| CrossData[i].DownMsgBuf[CrossData[i].WaitHead].Timeout <= 0)
	{
		DeleteDownBufHead((UINT8)Port);
	}

	if(CrossData[i].WaitSendCount <= 0)
		return;//no data to send now

	//resend or send a new command to cross
	MsgHeadPtr = (char *)( CrossData[i].DownMsgBuf[CrossData[i].WaitHead].hMsgBuf );
	MsgLen = CrossData[i].DownMsgBuf[CrossData[i].WaitHead].MsgLen;

	if((MsgLen > 260) || (MsgLen <= 0))
	{
		DebugWindow("CrossErrEventProc(),Out of Buffer Length!");
		ClearSanlianSendQueue((UINT8)Port);
		return;
	}
	
	Notify(Port,SERIAL_WRITE,(UINT8 *)MsgHeadPtr,(UINT16)MsgLen);

	CrossData[i].DownMsgBuf[CrossData[i].WaitHead].Status = BUF_SEND_STATUS;
	CrossData[i].DownMsgBuf[CrossData[i].WaitHead].SendNum++;
	CrossData[i].DownMsgBuf[CrossData[i].WaitHead].Timeout = MAX_HU_WAIT;//80以上的命令应立即返回值
		
	return;
}

/***************************************************************************
*	Function Name	: void JSErrEventProc(int Port,UINT8 ErrNo)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/02,21,2002
*	Global			: None
*	Note			: None
****************************************************************************/	
void JSErrEventProc(int Port,UINT8 ErrNo)
{
	BYTE i;
	UINT32 info;

	if(Port <= 0 || Port > MAX_CROSS_NUM)
	{
		DebugWindow("CrossErrEventProc(),Input Port Error!");
		return;
	}
	
	i = (UINT8)(Port - 1);

	if(CrossData[i].WaitHead < 0 || CrossData[i].WaitHead >= MAX_RESEND_NUM)
	{
		DebugWindow("CrossErrEventProc(),Exceeding Max_Resend_Num!");
		ClearSanlianSendQueue((UINT8)Port);
		return;
	}	

	if(CrossData[i].WaitSendCount <= 0)
		return;//no data to wait now

	if(CrossData[i].DownMsgBuf[CrossData[i].WaitHead].Status != BUF_SEND_STATUS)
		return;//Status Error,just return

	//notice: if cross event received so late(we have already deleted),we may send other
	//command,because we have no information to identify the command
	if(CrossData[i].DownMsgBuf[CrossData[i].WaitHead].SendNum >= 3								\
							|| CrossData[i].DownMsgBuf[CrossData[i].WaitHead].Timeout <= 0)
	{
		DeleteDownBufHead((UINT8)Port);
		info = 0;
	}
	else
	{
		info = 1;
	}

	if(CrossData[i].WaitSendCount <= 0)
		return;//no data to send now

	if(info > 0)
	{
		info = 0x30 + CrossData[i].DownMsgBuf[CrossData[i].WaitHead].SendNum;
	}

	Sleep(10);
	//resend or send a new command to cross
	PostMessage(hMainWnd,EXTENDMSGSEND,(WPARAM)Port,(LPARAM)info);

	return;
}

/***************************************************************************
*	Function Name	: void SendMsgInQue(int Port,int Info)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/02,21,2002
*	Global			: None
*	Note			: None
****************************************************************************/	
void SendMsgInQue(int Port,int Info)
{
	char *MsgHeadPtr;
	int MsgLen;	
	BYTE i;

	if(Port <= 0 || Port > MAX_CROSS_NUM)
	{
		DebugWindow("SendMsgInQue(),Input Port Error!");
		return;
	}
	
	i = (UINT8)(Port - 1);

	if(CrossData[i].WaitHead < 0 || CrossData[i].WaitHead >= MAX_RESEND_NUM)
	{
		DebugWindow("SendMsgInQue(),Exceeding Max_Resend_Num!");
		ClearSanlianSendQueue((UINT8)Port);
		return;
	}	

	if(CrossData[i].WaitSendCount <= 0)
		return;//no data to wait now

	if(CrossData[i].DownMsgBuf[CrossData[i].WaitHead].Status == BUF_SEND_STATUS)
	{
		if(Info == 0)
			return;//already send by other code,just return;

		if(CrossData[i].DownMsgBuf[CrossData[i].WaitHead].SendNum + 0x30 != (UINT8)Info)		
			return;//already resend by other code,just return;
	}
	
	MsgHeadPtr = (char *)( CrossData[i].DownMsgBuf[CrossData[i].WaitHead].hMsgBuf );
	MsgLen = CrossData[i].DownMsgBuf[CrossData[i].WaitHead].MsgLen;

	if((MsgLen > MAX_YLF_LEN) || (MsgLen <= 0))
	{
		DebugWindow("SendMsgInQue(),Out of Buffer Length!");
		ClearSanlianSendQueue((UINT8)Port);
		return;
	}
	
	Notify(Port,SERIAL_WRITE,(UINT8 *)MsgHeadPtr,(UINT16)MsgLen);

	CrossData[i].DownMsgBuf[CrossData[i].WaitHead].Status = BUF_SEND_STATUS;
	CrossData[i].DownMsgBuf[CrossData[i].WaitHead].SendNum++;
	CrossData[i].DownMsgBuf[CrossData[i].WaitHead].Timeout = MAX_YLF_WAIT;
		
	return;
}

/***************************************************************************
*	Function Name	: BOOL  DoConnect(char *szServer)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/12,28,2000
*	Global			: None
*	Note			: None
****************************************************************************/	
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
	sprintf(szConnStrIn , "DRIVER={SQL Server};SERVER=%s;UID=sa;PWD=%s", sServer, "");

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

/***************************************************************************
*	Function Name	: void  DoDisconnect(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/12,28,2000
*	Global			: None
*	Note			: None
****************************************************************************/	
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

/***************************************************************************
*	Function Name	: BYTE  GetSQLError(HSTMT hstmt)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/12,28,2000
*	Global			: None
*	Note			: None
****************************************************************************/	
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

/***************************************************************************
*	Function Name	: BYTE DoUpdate(char *SQLString,UINT16 WaitTime)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/12,28,2000
*	Global			: None
*	Note			: None
****************************************************************************/	
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


/***************************************************************************
*	Function Name	: INT32  dbSQLExecDirect(HSTMT hstmt, char *SQLString)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/12,28,2000
*	Global			: None
*	Note			: None
****************************************************************************/	
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

/************************************************
*	Function Name	: void rtrim(char *Instr)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/12,28,2000
*	Global			: None
*	Note			: None
*************************************************/	
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

/************************************************
*	Function Name	: BOOL Initdb(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/01,05,2001
*	Global			: None
*	Note			: None
*************************************************/	
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
		//if one part >255??just ommitted here 

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

/***************************************************************************
*	Function Name	: BYTE GetDbInit(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/02,21,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
BYTE GetDbInit(void)
{

	//SynClock();

	if(GetCrossCode() == DB_SQL_SUCCESS)
	{
//		GetJcqCode();
//		GetDefaultPlan();

		return DB_SQL_SUCCESS;
	}

	return DB_SQL_GENERALLYERROR;
}

/***************************************************************************
*	Function Name	: BYTE GetDefaultPlan(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,11,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
BYTE GetDefaultPlan(void)
{
/*	BYTE   i;
	SDWORD  cbt[MAX_RET_COLNUMBER];
    char    sqlResult[3][MAX_COL_LEN];
	HSTMT   hstmt;
    RETCODE retcode;
	INT32   BindColNumber;
	UINT8	wlbh;
	UINT8	Empty[LKJ_NAME_LEN + 1];
	char	header[LKJ_NAME_LEN + 1];

   	if(hdbc == NULL)
		return DB_SQL_GENERALLYERROR;

   	retcode = SQLAllocStmt(hdbc, &hstmt);
	if(RETCODE_IS_FAILURE(retcode)) 
		return DB_SQL_GENERALLYERROR;

	BindColNumber = dbSQLExecDirect(hstmt,"select wlbh,jcqbh,lkbh from sl_jtzh.dbo.jcqms");
	if(BindColNumber == -1 || BindColNumber != 3)
		return GetSQLError(hstmt);
    
	for(i=0; i<BindColNumber; i++)
	{
		retcode = SQLBindCol(hstmt,(UWORD)(i+1), SQL_C_CHAR, 
			sqlResult[i], sizeof(sqlResult[i]), &cbt[i]);
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
		
		if(strlen(sqlResult[0]) == 0 || strlen(sqlResult[0]) >2)//1--两位？
			continue;
		if(strlen(sqlResult[1]) != JCQ_NAME_LEN)
			continue;

		if(strlen(sqlResult[2]) != JCQ_NAME_LEN)
			continue;

		wlbh = (UINT8)atoi(sqlResult[0]);
		if(wlbh == 0 || wlbh > MAX_JCQ_NUM)
			continue;//越界

		memcpy(header,sqlResult[1],LKJ_NAME_LEN);		
		if( memcmp(header,Empty,LKJ_NAME_LEN) == 0)
			continue;

		for(i = 0;i < MAX_CROSS_NUM;i++)
		{
			if( memcmp(header,CrossData[i].lkbh,LKJ_NAME_LEN) == 0)
				break;
		}

		if(i < MAX_CROSS_NUM)
		{
			if(strcmp(CrossData[i].jcqbh[wlbh -1],"") == 0)
			{
				memcpy(CrossData[i].jcqbh[wlbh -1],sqlResult[1],JCQ_NAME_LEN);
			}
		}
	}

	SQLFreeStmt(hstmt, SQL_DROP);
*/
	return DB_SQL_SUCCESS;
}

/***************************************************************************
*	Function Name	: BOOL SynClock(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/03,13,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
BYTE SynClock(void)
{
	SDWORD  cbt;
    char    sqlResult[MAX_COL_LEN];
	HSTMT   hstmt;
    RETCODE retcode;
	INT32   BindColNumber;
//	SYSTEMTIME time1,time2;
	SYSTEMTIME time;
	char *p;

   	if(hdbc == NULL)
		return DB_SQL_GENERALLYERROR;

	retcode = SQLAllocStmt(hdbc, &hstmt);
	if(RETCODE_IS_FAILURE(retcode)) 
		return DB_SQL_GENERALLYERROR;

//	GetLocalTime(&time1);
	BindColNumber = dbSQLExecDirect(hstmt,"select getdate()");
	if(BindColNumber != 1)
		return GetSQLError(hstmt);
    
	retcode = SQLBindCol(hstmt,(UWORD)1, SQL_C_CHAR, 
		sqlResult, sizeof(sqlResult), &cbt);
	if(RETCODE_IS_FAILURE(retcode))
		return GetSQLError(hstmt);
	
	retcode = SQLFetch(hstmt);
	if(RETCODE_IS_FAILURE(retcode)) 
		return GetSQLError(hstmt);
	
	if(retcode == SQL_NO_DATA_FOUND)
	{
		SQLFreeStmt(hstmt, SQL_DROP);
		return DB_SQL_GENERALLYERROR;
	}
	
	rtrim(sqlResult);

	if(strlen(sqlResult) != 23)//length must be 23
	{
		SQLFreeStmt(hstmt, SQL_DROP);
		return DB_SQL_GENERALLYERROR;
	}
	
	p = sqlResult;
	*(p + 4) = '\0';
	time.wYear = atoi(p);

	p += 5;
	*(p + 2) = '\0';
	time.wMonth = atoi(p);

	p += 3;
	*(p + 2) = '\0';
	time.wDay = atoi(p);

	p += 3;
	*(p + 2) = '\0';
	time.wHour = atoi(p);

	p += 3;
	*(p + 2) = '\0';
	time.wMinute = atoi(p);

	p += 3;
	*(p + 2) = '\0';
	time.wSecond = atoi(p);

	p += 3;
	*(p + 3) = '\0';
	time.wMilliseconds = atoi(p);

	if(time.wMilliseconds < 1000 &&
		time.wSecond < 60  && 
		time.wMinute < 60  &&
		time.wHour  < 24  &&
		time.wYear < 2100 &&  time.wYear >= 2001 &&
		time.wMonth <= 12 &&  time.wMonth >= 1   &&
		time.wDay   <= 31 &&  time.wDay  >= 1)
	{
		MidDayCount = time.wHour * 3600 + time.wMinute * 60 + time.wSecond;
		if(MidDayCount <= 12 * 3600)
			MidDayCount = 12 * 3600 - MidDayCount;
		else
			MidDayCount = 36 * 3600 - MidDayCount;//24 * 360 - (MidDayCount - 12 * 360);

		SetLocalTime(&time); 
	}

//	GetLocalTime(&time2);
		
	SQLFreeStmt(hstmt, SQL_DROP);
	return DB_SQL_SUCCESS;

}

/***************************************************************************
*	Function Name	: BYTE GetCrossCode(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/02,21,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
BYTE GetCrossCode(void)
{
//	BOOL Ret;
	char SlqStr[300];
	BYTE   i;
	SDWORD  cbt[MAX_RET_COLNUMBER];
    char    sqlResult[4][MAX_COL_LEN];
	HSTMT   hstmt;
    RETCODE retcode;
	INT32   BindColNumber;
	UINT8	wlbh;
	UINT8	Type;
	UINT8	CardNo;
	int lkbh;

   	if(hdbc == NULL)
		return DB_SQL_GENERALLYERROR;

	retcode = SQLAllocStmt(hdbc, &hstmt);
	if(RETCODE_IS_FAILURE(retcode)) 
		return DB_SQL_GENERALLYERROR;

//获得路口编号(路口信号机描述表xhjms,路口描述表lk)
/*	memset(HostId,0,10);
	Ret = GetPrivateProfileString("GENERAL","UserId","通讯机1",HostId,10, __CONFIG_FILE__);

	if(Ret <= 0 || Ret > 10)
	{
		HostId[9] = 0x00;
//		return FALSE;
	}
*/
	memset(SlqStr,0,300);
	sprintf(SlqStr,"select sl_jtzh.dbo.xhjms.wlbh,sl_jtzh.dbo.lk.lkbh,\
			sl_jtzh.dbo.xhjms.xhjlxbh,sl_jtzh.dbo.xhjms.txkh \
		 from sl_jtzh.dbo.xhjms,sl_jtzh.dbo.lk where sl_jtzh.dbo.xhjms.xhjbh = sl_jtzh.dbo.lk.xhjbh \
		 and sl_jtzh.dbo.xhjms.txjid = '%s'",HostId);
	BindColNumber = dbSQLExecDirect(hstmt,SlqStr);
	if(BindColNumber == -1 || BindColNumber != 4)
		return GetSQLError(hstmt);
    
	for(i=0; i<BindColNumber; i++)
	{
		retcode = SQLBindCol(hstmt,(UWORD)(i+1), SQL_C_CHAR, 
			sqlResult[i], sizeof(sqlResult[i]), &cbt[i]);
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
		
		if(strlen(sqlResult[0]) == 0 || strlen(sqlResult[0]) >3)//>255?
		{
			DebugWindow("GetCrossCode(),wlbh length out of range in db!");
			continue;
		}
		if(strlen(sqlResult[1]) == 0 || strlen(sqlResult[1]) > LKJ_NAME_LEN)
		{
			DebugWindow("GetCrossCode(),lkbh too large in db!");
			continue;
		}
		if(strlen(sqlResult[2]) == 0 || strlen(sqlResult[2]) > 1)
		{
			DebugWindow("GetCrossCode(),lkbh too large in db!");
			continue;
		}
		if(strlen(sqlResult[3]) > 1)
		{
			DebugWindow("GetCrossCode(),CardNo too large in db!");
			continue;
		}

		wlbh = (UINT8)atoi(sqlResult[0]);
		if(wlbh == 0 || wlbh > MAX_CROSS_NUM)
		{
			DebugWindow("GetCrossCode(),wlbh out of range in db!");
			continue;
		}
		
		Type = (UINT8)atoi(sqlResult[2]);
		if(Type == 0 || Type > 2)
		{
			DebugWindow("GetCrossCode(),Type out of range in db!");
			continue;
		}

		CardNo = (UINT8)atoi(sqlResult[3]);
		if(CardNo > 2)
			CardNo = 0;

		if(CrossData[wlbh - 1].lkbh == 0)
		{
			lkbh = atoi(sqlResult[1]);
			if(lkbh != 0)
			{
				CrossData[wlbh - 1].lkbh = lkbh;
				CrossData[wlbh - 1].Type = Type;
				CrossData[wlbh - 1].CardNo = CardNo;
			}
			else
			{
				DebugWindow("GetCrossCode(),lkbh == 0 in db!");
			}
		}
		else
		{
			DebugWindow("GetCrossCode(),Maybe one wlbh have more than one lkbh!");
		}
	}

	SQLFreeStmt(hstmt, SQL_DROP);

	return DB_SQL_SUCCESS;
}

/***************************************************************************
*	Function Name	: BYTE GetJcqCode(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/02,21,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
BYTE GetJcqCode(void)
{
/*	BYTE   i;
	SDWORD  cbt[MAX_RET_COLNUMBER];
    char    sqlResult[3][MAX_COL_LEN];
	HSTMT   hstmt;
    RETCODE retcode;
	INT32   BindColNumber;
	UINT8	wlbh;
	UINT8	Empty[LKJ_NAME_LEN + 1];
	char	header[LKJ_NAME_LEN + 1];

   	if(hdbc == NULL)
		return DB_SQL_GENERALLYERROR;

   	retcode = SQLAllocStmt(hdbc, &hstmt);
	if(RETCODE_IS_FAILURE(retcode)) 
		return DB_SQL_GENERALLYERROR;

//获得检测器编号,检测器编号和车道编号的对应关系(检测器描述表)
	BindColNumber = dbSQLExecDirect(hstmt,"select wlbh,jcqbh,lkbh from sl_jtzh.dbo.jcqms");
	if(BindColNumber == -1 || BindColNumber != 3)
		return GetSQLError(hstmt);
    
	for(i=0; i<BindColNumber; i++)
	{
		retcode = SQLBindCol(hstmt,(UWORD)(i+1), SQL_C_CHAR, 
			sqlResult[i], sizeof(sqlResult[i]), &cbt[i]);
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
		
		if(strlen(sqlResult[0]) == 0 || strlen(sqlResult[0]) >2)//1--两位？
			continue;
		if(strlen(sqlResult[1]) != JCQ_NAME_LEN)
			continue;

		if(strlen(sqlResult[2]) != JCQ_NAME_LEN)
			continue;

		wlbh = (UINT8)atoi(sqlResult[0]);
		if(wlbh == 0 || wlbh > MAX_JCQ_NUM)
			continue;//越界

		memcpy(header,sqlResult[1],LKJ_NAME_LEN);		
		if( memcmp(header,Empty,LKJ_NAME_LEN) == 0)
			continue;

		for(i = 0;i < MAX_CROSS_NUM;i++)
		{
			if( memcmp(header,CrossData[i].lkbh,LKJ_NAME_LEN) == 0)
				break;
		}

		if(i < MAX_CROSS_NUM)
		{
			if(strcmp(CrossData[i].jcqbh[wlbh -1],"") == 0)
			{
				memcpy(CrossData[i].jcqbh[wlbh -1],sqlResult[1],JCQ_NAME_LEN);
			}
		}
	}

	SQLFreeStmt(hstmt, SQL_DROP);
*/
	return DB_SQL_SUCCESS;
}

/***************************************************************************
*	Function Name	: void HuLcuResend(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/09,24,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
void HuLcuResend(void)
{
	int i;
	UINT8 CrossNo;

	if(!(UseLcu && (Card1 == 1 || Card2 == 1)))
		return;

	if(SendingBuf.Status != BUF_IDLE_STATUS)
		return;

	//find a new command to send
	CrossNo = GetNextLcuHuItem(1);
	if(CrossNo == 0)//not found
		return;
	
	i = CrossNo - 1;

	memset(&SendingBuf,0,sizeof(RecBuf));
	SendingBuf.Status = BUF_SEND_STATUS;
	SendingBuf.CrossNo = CrossNo;
	SendingBuf.CmdNo = (UINT8)(CrossData[i].DownMsgBuf[CrossData[i].WaitHead].CmdNo);
	SendingBuf.CardNo = CrossData[i].CardNo;
	SendingBuf.ReSendTimes = 1;
	SendingBuf.Timeout = 2;
	SendingBuf.MsgLen = CrossData[i].DownMsgBuf[CrossData[i].WaitHead].MsgLen;
	memcpy(SendingBuf.MsgData,(void *)(CrossData[i].DownMsgBuf[CrossData[i].WaitHead].hMsgBuf),\
				SendingBuf.MsgLen);

	DeleteDownBufHead(CrossNo);
	Write2Lpt(SendingBuf.MsgData,SendingBuf.MsgLen,SendingBuf.CardNo);

	return;
}


/***************************************************************************
*	Function Name	: void HuLcuTimeoutProc(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/09,24,2001
*   Modify		    : yincy/11,20,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
void HuLcuTimeoutProc(void)
{
	UINT8 CrossNo;
	UINT8 i;

	if(!(UseLcu && (Card1 == 1 || Card2 == 1)))
		return;

	if(SendingBuf.Status != BUF_SEND_STATUS)
		return;

	if(SendingBuf.Timeout <= 0)
	{
		if(SendingBuf.ReSendTimes < 3)
		{
			Write2Lpt(SendingBuf.MsgData,SendingBuf.MsgLen,SendingBuf.CardNo);
			SendingBuf.ReSendTimes++;
			SendingBuf.Timeout = 2;
/*			if(SendingBuf.bResendFlag == 1)
				return;

			//request to resend the command
			SendingBuf.bResendFlag = 1;
			PostMessage(hMainWnd,LCUBUF_RESEND,(WPARAM)0x01,(LPARAM)0x2345);
*/		}
		else
		{
			UINT8 tempCrossNo;

			//we send lcu error message to wu
			if(CrossData[SendingBuf.CrossNo - 1].ErrorNo != 11)
			{
				SendCommStatus2Wu(SendingBuf.CrossNo,11);
				CrossData[SendingBuf.CrossNo - 1].ErrorNo = 11;//error number,may modify later
			}

			//send a new command
			tempCrossNo = SendingBuf.CrossNo;
			memset(&SendingBuf,0,sizeof(RecBuf));
			CrossNo = GetNextLcuHuItem(tempCrossNo);
			if(CrossNo == 0)//not found
				return;
			
			i = CrossNo - 1;
			SendingBuf.Status = BUF_SEND_STATUS;
			SendingBuf.CrossNo = CrossNo;
			SendingBuf.CmdNo = (UINT8)(CrossData[i].DownMsgBuf[CrossData[i].WaitHead].CmdNo);
			SendingBuf.CardNo = CrossData[i].CardNo;
			SendingBuf.ReSendTimes = 1;
			SendingBuf.Timeout = 2;
			SendingBuf.MsgLen = CrossData[i].DownMsgBuf[CrossData[i].WaitHead].MsgLen;
			memcpy(SendingBuf.MsgData,(void *)(CrossData[i].DownMsgBuf[CrossData[i].WaitHead].hMsgBuf),\
						SendingBuf.MsgLen);

			DeleteDownBufHead(CrossNo);

			Write2Lpt(SendingBuf.MsgData,SendingBuf.MsgLen,SendingBuf.CardNo);
/*			
			if(SendingBuf.bResendFlag == 2)
				return;

			//request to send a new command
			SendingBuf.bResendFlag = 2;
			PostMessage(hMainWnd,LCUBUF_RESEND,(WPARAM)0,(LPARAM)0x2345);
*/		}
	}
	else
		SendingBuf.Timeout--;

	return;
}

/***************************************************************************
*	Function Name	: void LcuBufResendProc(UINT8 bResend)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/09,25,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
void LcuBufResendProc(UINT8 bResend)
{
	UINT8 CrossNo;
	UINT8 i;
	
	if(bResend)
	{
		if(SendingBuf.Status != BUF_SEND_STATUS)
		{
			DebugWindow("LcuBufResendProc(),SendingBuf Status Error!");
			memset(&SendingBuf,0,sizeof(RecBuf));
			return;
		}

		if(SendingBuf.bResendFlag == 1)
			SendingBuf.bResendFlag = 0;

		Write2Lpt(SendingBuf.MsgData,SendingBuf.MsgLen,SendingBuf.CardNo);
		SendingBuf.ReSendTimes++;
		SendingBuf.Timeout = 2;
	}
	else
	{
		//we send lcu error message to wu

		//send a new command
		CrossNo = GetNextLcuHuItem(SendingBuf.CrossNo);
		if(CrossNo == 0)//not found
			return;
		
		i = CrossNo - 1;

		memset(&SendingBuf,0,sizeof(RecBuf));
		SendingBuf.Status = BUF_SEND_STATUS;
		SendingBuf.CrossNo = CrossNo;
		SendingBuf.CmdNo = (UINT8)(CrossData[i].DownMsgBuf[CrossData[i].WaitHead].CmdNo);
		SendingBuf.CardNo = CrossData[i].CardNo;
		SendingBuf.ReSendTimes = 1;
		SendingBuf.Timeout = 2;
		SendingBuf.MsgLen = CrossData[i].DownMsgBuf[CrossData[i].WaitHead].MsgLen;
		memcpy(SendingBuf.MsgData,(void *)(CrossData[i].DownMsgBuf[CrossData[i].WaitHead].hMsgBuf),\
					SendingBuf.MsgLen);

		DeleteDownBufHead(CrossNo);

		Write2Lpt(SendingBuf.MsgData,SendingBuf.MsgLen,SendingBuf.CardNo);
	}

	return;
}

/***************************************************************************
*	Function Name	: void JSSerialTimeoutProc(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/02,21,2002
*	Global			: None
*	Note			: None
****************************************************************************/	
void JSSerialTimeoutProc(void)
{
	char *MsgHeadPtr;
	int i;
	UINT8 Index;
	UINT16 MsgLen;
//#ifdef TEST
//	char disp[200];
//#endif
	if(UseLcu)
		return;

	for(i = 0; i < MAX_CROSS_NUM; i++)
	{
		if(CrossData[i].Type != 2)//jingsan cross
			continue;

		if(CrossData[i].WaitSendCount == 0)
			continue;
			
		if(CrossData[i].WaitSendCount < 0 || CrossData[i].WaitSendCount > MAX_RESEND_NUM)
		{
			DebugWindow("JSSerialTimeoutProc(),WaitSendCount Error!");
			ClearSanlianSendQueue((UINT8)(i + 1));
			continue;
		}	
		
		if(CrossData[i].WaitHead < 0 || CrossData[i].WaitHead >= MAX_RESEND_NUM)
		{
			DebugWindow("JSSerialTimeoutProc(),Exceeding Max_Resend_Num!");
			ClearSanlianSendQueue((UINT8)(i + 1));
			continue;
		}	

/*		if(CrossData[i].WaitEnd < 0 || CrossData[i].WaitEnd >= MAX_RESEND_NUM)
		{
			DebugWindow("JSSerialTimeoutProc(),Exceeding Max_Resend_Num!");
			ClearSanlianSendQueue((UINT8)(i + 1));
			continue;
		}
		Index = CrossData[i].WaitHead;
		while(Index < CrossData[i].WaitEnd )
		{
			CrossData[i].DownMsgBuf[Index].Timeout--;
			Index = (++Index) % MAX_RESEND_NUM; 
		}
*/
		//now we only consider the head
		Index = CrossData[i].WaitHead;
		if(CrossData[i].DownMsgBuf[Index].SendNum >= 3)
		{
			DeleteDownBufHead((UINT8)(i + 1));

			if(CrossData[i].WaitSendCount <= 0)
				continue;

			Index = CrossData[i].WaitHead;
		}

		if(CrossData[i].DownMsgBuf[Index].Status == BUF_QUE_STATUS)
		{
		//the head not send
			MsgHeadPtr = CrossData[i].DownMsgBuf[Index].hMsgBuf;
			MsgLen = (UINT16)(CrossData[i].DownMsgBuf[Index].MsgLen);

			Notify(i + 1,SERIAL_WRITE,(UINT8 *)MsgHeadPtr,MsgLen);

			CrossData[i].DownMsgBuf[Index].Status = BUF_SEND_STATUS;
			CrossData[i].DownMsgBuf[Index].SendNum = 1;
			CrossData[i].DownMsgBuf[Index].Timeout = MAX_YLF_WAIT;

			continue;
		}

		if(CrossData[i].DownMsgBuf[Index].Timeout <= 0)
		{
		//send the next time
//			Index = CrossData[i].WaitHead;

			MsgHeadPtr = CrossData[i].DownMsgBuf[Index].hMsgBuf;
			MsgLen = (UINT16)(CrossData[i].DownMsgBuf[Index].MsgLen);

			Notify(i + 1,SERIAL_WRITE,(UINT8 *)MsgHeadPtr,MsgLen);

			CrossData[i].DownMsgBuf[Index].Status = BUF_SEND_STATUS;
			CrossData[i].DownMsgBuf[Index].SendNum++;
			CrossData[i].DownMsgBuf[Index].Timeout = MAX_YLF_WAIT;
		}
		else
			CrossData[i].DownMsgBuf[Index].Timeout--;
	}

	return;
}

/***************************************************************************
*	Function Name	: void JSSerialTimeoutProc(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/02,21,2002
*	Global			: None
*	Note			: None
****************************************************************************/	
void AckTypeCmdTimeoutProc(void)
{

	if(UseLcu)
	{
		HuLcuResend();
	}
	else
	{
		HuSerialTimeoutProc();
		JSSerialTimeoutProc();
	}

	return;
}

/***************************************************************************
*	Function Name	: void HuTimeoutProc(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/08,10,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
void HuSerialTimeoutProc(void)
{
	char *MsgHeadPtr;
	int i;
	UINT8 Index;
	UINT16 MsgLen;
//#ifdef TEST
//	char disp[200];
//#endif
	if(UseLcu)
		return;

	for(i = 0; i < MAX_CROSS_NUM; i++)
	{
//		if(CrossData[i].Type == 0)
//			continue;
		if(CrossData[i].Type != 1)//sanlian cross
			continue;

		if(CrossData[i].WaitSendCount == 0)
			continue;
			
		if(CrossData[i].WaitSendCount < 0 || CrossData[i].WaitSendCount > MAX_RESEND_NUM)
		{
			DebugWindow("HuSerialTimeoutProc(),WaitSendCount Error!");
			ClearSanlianSendQueue((UINT8)(i + 1));
			continue;
		}	
		
		if(CrossData[i].WaitHead < 0 || CrossData[i].WaitHead >= MAX_RESEND_NUM)
		{
			DebugWindow("HuSerialTimeoutProc(),Exceeding Max_Resend_Num!");
			ClearSanlianSendQueue((UINT8)(i + 1));
			continue;
		}	

/*		if(CrossData[i].WaitEnd < 0 || CrossData[i].WaitEnd >= MAX_RESEND_NUM)
		{
			DebugWindow("HuSerialTimeoutProc(),Exceeding Max_Resend_Num!");
			ClearSanlianSendQueue((UINT8)(i + 1));
			continue;
		}
		Index = CrossData[i].WaitHead;
		while(Index < CrossData[i].WaitEnd )
		{
			CrossData[i].DownMsgBuf[Index].Timeout--;
			Index = (++Index) % MAX_RESEND_NUM; 
		}
*/
		//now we only consider the head
		Index = CrossData[i].WaitHead;
		if(CrossData[i].DownMsgBuf[Index].SendNum >= 3)
		{
			DeleteDownBufHead((UINT8)(i + 1));

			if(CrossData[i].WaitSendCount <= 0)
				continue;

			Index = CrossData[i].WaitHead;
		}

		if(CrossData[i].DownMsgBuf[Index].Status == BUF_QUE_STATUS)
		{
		//the head not send
			MsgHeadPtr = CrossData[i].DownMsgBuf[Index].hMsgBuf;
			MsgLen = (UINT16)(CrossData[i].DownMsgBuf[Index].MsgLen);

			Notify(i + 1,SERIAL_WRITE,(UINT8 *)MsgHeadPtr,MsgLen);

			CrossData[i].DownMsgBuf[Index].Status = BUF_SEND_STATUS;
			CrossData[i].DownMsgBuf[Index].SendNum = 1;
			CrossData[i].DownMsgBuf[Index].Timeout = MAX_HU_WAIT;//80以上的命令应立即返回值

			continue;
		}

		if(CrossData[i].DownMsgBuf[Index].Timeout <= 0)
		{
		//send the next time
//			Index = CrossData[i].WaitHead;

			MsgHeadPtr = CrossData[i].DownMsgBuf[Index].hMsgBuf;
			MsgLen = (UINT16)(CrossData[i].DownMsgBuf[Index].MsgLen);

			Notify(i + 1,SERIAL_WRITE,(UINT8 *)MsgHeadPtr,MsgLen);

			CrossData[i].DownMsgBuf[Index].Status = BUF_SEND_STATUS;
			CrossData[i].DownMsgBuf[Index].SendNum++;
			CrossData[i].DownMsgBuf[Index].Timeout = MAX_HU_WAIT;//80以上的命令应立即返回值
		}
		else
			CrossData[i].DownMsgBuf[Index].Timeout--;
	}

	return;
}

/***************************************************************************
*	Function Name	: void CheckSignal(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,11,2001
*   Modify		    : yincy/03,07,2002
*   Author/Date     : yincy/04,08,2002
*	Global			: None
*	Note			: None
****************************************************************************/	
void CheckSignal(void)
{
	int i;
#ifdef TEST
	char disp[200];
#endif

	for(i = 0; i < MAX_CROSS_NUM; i++)
	{
		if(CrossData[i].Type == 1)//sanlian
		{
			if(UseLcu == 0)
			{
				//decrease the ReopenLeftCount
				if(CrossData[i].ReopenLeftCount > 0)
					CrossData[i].ReopenLeftCount--;

				if(CrossData[i].NoSignalCount >= 200)
				{
					if(CrossData[i].Status != DISCONNECT_STATUS)
					{
				//send disconnect to wu when disconnect
				#ifdef TEST
						memset(disp,0,200);
						sprintf(disp,"CheckSignal(),Send Port = %d Disconnected!",i + 1);
						DebugWindow(disp);
				#endif
						CrossData[i].Status = DISCONNECT_STATUS;
						SendCommStatus2Wu(i + 1,1);
						
						//send CM_TIMESYNC to Cross
						{
						SYSTEMTIME time;
						DATA_BENCHMARK_TIME Data;

						GetLocalTime(&time);
						Data.Year = (UINT8)(time.wYear - 2000);
						Data.Month = (UINT8)(time.wMonth);
						Data.Day = (UINT8)(time.wDay);
						Data.Hour = (UINT8)(time.wHour);
						Data.Minute = (UINT8)(time.wMinute);
						Data.Second = (UINT8)(time.wSecond);
						Data.Week = (UINT8)(time.wDayOfWeek);

						BenchMarkReqProc(&Data,CM_TIMESYNC,(UINT32)(i + 1),1);
						}
					}

				//following lines modified 10,24,2001,baoding
					if(CrossData[i].DisconnectCount == 1 && CrossData[i].ReopenLeftCount == 0)
					{
	#ifdef TEST
						memset(disp,0,200);
						sprintf(disp,"CheckSignal(),ReOpen Port=%d!",i + 1);
						DebugWindow(disp);
	#endif
						ClearSanlianSendQueue((UINT8)(i + 1));

						if(ClosePort(i + 1))
						{
		//following lines deleted 10,23,2001,baoding
		//					if(TerminateThread(ThreadCtr[i + 1].hThread,(DWORD)0))
		//					{
		//						ThreadCtr[i + 1].hThread = NULL;
		//					}
		//					else
		//						DebugWindow("CheckSignal(),Terminate Failed");
		//
							if(OpenPortEx(i + 1))
								Sleep(20);
							else
								DebugWindow("CheckSignal(),OpenPort Failed");
						}
						else
							DebugWindow("CheckSignal(),ClosePort Failed");

						CrossData[i].ReopenLeftCount = 60;

						//send CM_TIMESYNC to Cross
						{
						SYSTEMTIME time;
						DATA_BENCHMARK_TIME Data;

						GetLocalTime(&time);
						Data.Year = (UINT8)(time.wYear - 2000);
						Data.Month = (UINT8)(time.wMonth);
						Data.Day = (UINT8)(time.wDay);
						Data.Hour = (UINT8)(time.wHour);
						Data.Minute = (UINT8)(time.wMinute);
						Data.Second = (UINT8)(time.wSecond);
						Data.Week = (UINT8)(time.wDayOfWeek);

						BenchMarkReqProc(&Data,CM_TIMESYNC,(UINT32)(i + 1),1);
						}

					}
					else if(CrossData[i].DisconnectCount == 100)
					{
						CrossData[i].DisconnectCount = 0;
					}

					CrossData[i].NoSignalCount = 20;
					CrossData[i].DisconnectCount++;
				}
				else
					CrossData[i].NoSignalCount++;
			}
			else if(UseLcu == 1)
			{
				CrossData[i].NoSignalCount++;

				if(CrossData[i].NoSignalCount >= 60)
				{
					if(CrossData[i].Status != DISCONNECT_STATUS)
					{
				//send disconnect to wu when disconnect
				#ifdef TEST
						memset(disp,0,200);
						sprintf(disp,"CheckSignal(),Send Port=%d Disconnected!",i + 1);
						DebugWindow(disp);
				#endif
						SendCommStatus2Wu(i + 1,1);
						CrossData[i].Status = DISCONNECT_STATUS;
					}
				}
			}

			continue;
		}
		else if(CrossData[i].Type == 2)//jinsan cross
		{
	//following is jinsan
			if(!UseLcu)
			{
				if(ThreadCtr[i].dwThreadID == 0)//comm not open
					continue;
			}

			if(CrossData[i].ReopenLeftCount > 0)
				CrossData[i].ReopenLeftCount--;

			if(CrossData[i].LinkFlag == FALSE)
			{
				if(CrossData[i].DisconnectCount == 1 && CrossData[i].ReopenLeftCount == 0)
				{

#ifdef TEST
					memset(disp,0,200);
					sprintf(disp,"CheckSignal(),ReOpen Port=%d 1!",i + 1);
					DebugWindow(disp);
#endif
					ClearSanlianSendQueue((UINT8)(i + 1));

					if(ClosePort(i + 1))
					{
						if(OpenPortEx(i + 1))
							Sleep(20);
						else
							DebugWindow("CheckSignal(),OpenPort Failed");
					}
					else
						DebugWindow("CheckSignal(),ClosePort Failed");

					CrossData[i].ReopenLeftCount = 60;
				}
				else if(CrossData[i].DisconnectCount == 7200)
				{
					CrossData[i].DisconnectCount = 0;
				}

				CrossData[i].DisconnectCount++;
			}

			//former handling
			if(CrossData[i].Status == IDLE_STATUS || CrossData[i].Status == INITIAL_STATUS)
			{
				if(CrossData[i].NoSignalCount > 20 + 10)
				{
					if(CrossData[i].LinkFlag != FALSE)
					{
			#ifdef TEST
						memset(disp,0,200);
						sprintf(disp,"Port = %d Disconnected 44!",i + 1);
						DebugWindow(disp);
			#endif
						CrossData[i].LinkFlag = FALSE;

						CrossData[i].CycleTimeOffset = 0;//03,14,2002
						CrossData[i].CycleAulCount = 0;

						CrossData[i].Status = INITIAL_STATUS;
						CrossData[i].InitalWait = 3;
						CrossData[i].CurStep = MAX_STEP_NUM + 1;
						CrossData[i].NeedStep = MAX_STEP_NUM + 2;
						CrossData[i].CurCtrMode = 0xff;//D当前的控制模式
						CrossData[i].CurCValue = 0xff;//C当前的C值
						CrossData[i].Left5Min = 5 * 60;
					}

					if(CrossData[i].DisconnectCount == 0)
						SendCommStatus2Wu(i + 1,1);
				}
				else
				{
					CrossData[i].NoSignalCount++;
				}

				continue;
			}

			if(CrossData[i].NoSignalCount > 20)
			{
				if(CrossData[i].LinkFlag != FALSE)
				{
		#ifdef TEST
					memset(disp,0,200);
					sprintf(disp,"Port = %d Disconnected!",i + 1);
					DebugWindow(disp);
		#endif
					CrossData[i].LinkFlag = FALSE;

					if(CrossData[i].RealTime5Min > 0)
						ReportFlowData(i + 1);

					CrossData[i].CycleTimeOffset = 0;//03,14,2002
					CrossData[i].CycleAulCount = 0;

					CrossData[i].Status = INITIAL_STATUS;
					CrossData[i].InitalWait = 3;
					CrossData[i].CurStep = MAX_STEP_NUM + 1;
					CrossData[i].NeedStep = MAX_STEP_NUM + 2;
					CrossData[i].CurCtrMode = 0xff;//D当前的控制模式
					CrossData[i].CurCValue = 0xff;//C当前的C值
					CrossData[i].Left5Min = 5 * 60;

					//CrossData[i].NoSignalCount = 0;
				}

				if(CrossData[i].DisconnectCount == 0)
					SendCommStatus2Wu(i + 1,1);
			}
			else
				CrossData[i].NoSignalCount++;

		}
		else
		{
		//unknown cross type
			continue;
		}

	}//end for()

	return;
}

/***************************************************************************
*	Function Name	: void SerialDataProc(int port,char *MsgPtr)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,11,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
void SerialDataProc(int port,char *MsgPtr)
{
	SerialBits *pSerialBit; 
	char MsgBuf[3];
	char vdetctor;
	char Mask = (char)0xf0;
	char DataType;
	char DataValue;
	int ctrflag;

#ifdef TEST
	char disp[200];
#endif

	if(port <= 0 || port > MAX_CROSS_NUM)
	{
		DebugWindow("SerialDataProc(),Input Error!");
		return;
	}

	if(MsgPtr == NULL)
	{
		DebugWindow("SerialDataProc(),MsgPtr Error!");
		return;
	}

	memset(MsgBuf,0,3);
	memcpy(MsgBuf,MsgPtr,3);

	CrossData[port - 1].NoSignalCount = 0;
	CrossData[port - 1].DisconnectCount = 0;	
	if(CrossData[port - 1].LinkFlag != TRUE)
	{
#ifdef TEST
		memset(disp,0,200);
		sprintf(disp,"Send Connect To Port %d!",port);
		DebugWindow(disp);
#endif
		SendCommStatus2Wu(port,0);
		CrossData[port - 1].LinkFlag = TRUE;
	}

	if(CrossData[port - 1].Status == IDLE_STATUS)//discard
		return;

	if(CrossData[port - 1].Status != INITIAL_STATUS)
	{
		vdetctor = ( (MsgBuf[0] >> 4) & 0x0f ) + (MsgBuf[1] & Mask);
		DetectorProc(port,vdetctor);
	}

	if(MsgBuf[2] == 0xff || MsgBuf[2] == 0x00)//stop bits,return directly
		return;

	pSerialBit = (SerialBits *)(&MsgBuf[2]);
/*	if(pSerialBit->BitWise.StartFlag == 1)
	{
		memset(disp,0,200);
		sprintf(disp,"port = %d,Error StartFlag(1)",port);
		DebugWindow(disp);
		return;
	}
*/
	DataType = pSerialBit->BitWise.CharType;
	DataValue = (MsgBuf[2] >> 4) & 0x0f;

	switch(DataType)
	{
	case C_CHAR://c
		//memset(disp,0,200);
		//sprintf(disp,"port = %d,Receive C char!",port);
		//DebugWindow(disp);
		//if(CrossData[port - 1].CurCValue != DataValue)
		CrossData[port - 1].CurCValue = DataValue;
/*#ifdef TEST
		if(DataValue == 1)
			DataValue = 1;
#endif		
*/		break;

	case D_CHAR://d
		//memset(disp,0,200);
		//sprintf(disp,"port = %d,Receive D char!",port);
		//DebugWindow(disp);

		//if(CrossData[port - 1].CurCtrMode != DataValue)
		{
		//pSerialBit = (SerialBits *)(&MsgBuf[2]);//DataValue);

		CrossData[port - 1].CurCtrMode = pSerialBit->BitWise.D4 * 2										\
												+ pSerialBit->BitWise.D3;
	//added 03,29,2002
		if(CrossData[port - 1].CurCtrMode == 3)
		{
			//yellow blink
			if(CrossData[port - 1].InitalWait > 0)
				CrossData[port - 1].InitalWait = 0;
		}


		if(CrossData[port - 1].ForcedStep == 201)
		{
			if(CrossData[port - 1].CurCtrMode == 0 && CrossData[port - 1].InitalWait == 0)
			{
#ifdef TEST
				DebugWindow("Cross Yellow Blink Has Stopped!");
#endif
				SetYellowOffProc((UINT8)port);
			}
		}
	//added end

		//added 04,19,2002
		ctrflag = CrossData[port - 1].ManCtrFlag;
		//added end

	//07,35,2001 && 09,25,2001
		if(pSerialBit->BitWise.D2 == 1)//high
		{
			CrossData[port - 1].ManCtrFlag = 1;//pSerialBit->BitWise.D2;
		}
		else if(CrossData[port - 1].ManCtrFlag != 2)
			CrossData[port - 1].ManCtrFlag = pSerialBit->BitWise.D2;
		}
	//end

		//added 04,19,2002
		if(ctrflag != CrossData[port - 1].ManCtrFlag)
		{
			Rep_CrossCtrStatus CrossCtrStatus;

			//send to wu
			memset(&CrossCtrStatus,0,sizeof(Rep_CrossCtrStatus));
			CrossCtrStatus.byFlag = MsgFlag;
			CrossCtrStatus.MsgType = CM_CONTROLSTATUS;

			strcpy(CrossCtrStatus.SourceIP,HostIp);
			strcpy(CrossCtrStatus.SourceID,HostId);
			strcpy(CrossCtrStatus.TargetIP,SvrIp);
			strcpy(CrossCtrStatus.TargetID,SvrId);

			CrossCtrStatus.iCrossNo = CrossData[port - 1].lkbh;
			CrossCtrStatus.iLength = 0;
			CrossCtrStatus.iReserved1 = CrossData[port - 1].ManCtrFlag;

			CrossCtrStatus.byCheckSum = FormCheckSum((char *)&CrossCtrStatus);

			SendDataEx((char *)&CrossCtrStatus,sizeof(Rep_CrossCtrStatus),(UINT8)1);
		}
		//added end

		break;

	case E_CHAR://e
		{
			//memset(disp,0,200);
			//sprintf(disp,"port = %d,Receive E char!",port);
			//DebugWindow(disp);

			//pSerialBit = (SerialBits *)(&MsgBuf[2]);//DataValue);

		//added 08,17,2002
			if(CrossData[port - 1].CurCValue > 0x0f)
			{
				DebugWindow("At least lost one c char!");
				return;
			}
		//end

			CurStepEventProc(port,(UINT8)(pSerialBit->BitWise.D1) );
		}
		break;

	case F_CHAR:
	case G_CHAR://fg
		//memset(disp,0,200);
		//sprintf(disp,"port = %d,Receive FG char!",port);
		//DebugWindow(disp);
		//added 08,17,2002
		CrossData[port - 1].CurCValue = 0xff;

		break;

	default:
		//added 08,17,2002
		CrossData[port - 1].CurCValue = 0xff;

		DebugWindow("SerialDataProc(),Unknown Char!");
		break;
	}


	return;
}

/***************************************************************************
*	Function Name	: void CurStepEventProc(int port,UINT8 D5Value)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,11,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
void CurStepEventProc(int port,UINT8 D5Value)
{
	UINT8 CurCValue;
	UINT8 CurStep;
	int i;
#ifdef TEST
	char disp[200];
#endif

	if(port <= 0 || port > MAX_CROSS_NUM)
	{
		DebugWindow("CurStepEventProc(),Input port Error!");
		return;
	}

	CurCValue = CrossData[port - 1].CurCValue;
	if(CurCValue > 0x0f)
	{
		//lost c char ,so this e char is of no use
#ifdef TEST
		DebugWindow("CurStepEventProc(),Lost One C Char or Cross Initializing!");
#endif
		return;
	}

	if(D5Value)
		CurStep = CurCValue + 16;
	else
		CurStep = CurCValue;

	//set CValue to impossible value
	CrossData[port - 1].CurCValue = 0xff;

#ifdef TEST
	memset(disp,0,200);
	sprintf(disp,"CurStep = %d",CurStep);
//	DebugWindow(disp);
#endif

/*	if(CurStep == 1 && CrossData[port - 1].NeedStep == 0)
		CrossData[port - 1].NeedStep = 0;
*/
	//
	if(CrossData[port - 1].Status == INITIAL_STATUS)
	{
		if(CurStep == 0 && CurStep != CrossData[port - 1].CurStep)// \
//					&& CrossData[port - 1].CurStep < MAX_STEP_NUM)
		{
			InitialPrepare(port);

			if(CrossData[port - 1].Status == NORMAL_STATUS)
				SynCrossPhase(port);
		}
		else if(CrossData[port - 1].InitalWait > 0)
		{
			if(CurStep < CrossData[port - 1].TotalStepNo)//added 02,29,2002
			{
				CrossData[port - 1].CurStep = CurStep;
				CrossData[port - 1].NeedStep = CurStep;
			}
			else
			{
				DebugWindow("CurStepEventProc(),CurStep>TotalStepNo 1!");
				return;
			}
		}

		return;
	}
	
//following added 03,20,2002 for adjust time length
	if(CrossData[port - 1].Status == FORCED_STATUS)
	{
		if(CrossData[port - 1].ForcedStep == CrossData[port - 1].CurStep)
		{
			if(CrossData[port - 1].InitalWait > 0)
			{
				CrossData[port - 1].ForecedLeftTime = CrossData[port - 1].ForecedLeftTime		\
							- (1000 - CrossData[port - 1].InitalWait - 15);
				CrossData[port - 1].InitalWait = 0;
			}
		}
	}
//added end

	if(CurStep == CrossData[port - 1].CurStep)
	{
		//step not changed,only return;
		return;
	}

	//following added 03,20,2002
/*	if(CrossData[port - 1].Status == FORCED_STATUS && CrossData[port - 1].InitalWait == 0)
	{
		if(CurStepGreatTheStep(port,CurStep) && CrossData[port - 1].ForcedStep == CrossData[port - 1].CurStep)
		{
#ifdef TEST
			DebugWindow("Cross Has Stepped Out of Forced Step!");
#endif
			SetStepoffAdjust((UINT8)port);
			CrossData[port - 1].Status = TRANSIT_STATUS;
		}
	}
	//added end
*/
//added 08,18,2002
/*	if(CrossData[port - 1].Status == FORCED_STATUS && CrossData[port - 1].InitalWait == 0)
	{
		DebugWindow("Received error step number in FORCED_STATUS!");
		return;
	}
*///added end
//curStep changed
	if(CurStep < CrossData[port - 1].TotalStepNo)//added 02,29,2002
	{
		CrossData[port - 1].CurStep = CurStep;
	}
	else
	{
		DebugWindow("CurStepEventProc(),CurStep>TotalStepNo 2!");

		CrossData[port - 1].Status = IDLE_STATUS;
		CrossData[port - 1].InitalWait = 3;
		CrossData[port - 1].CurStep = MAX_STEP_NUM + 1;
		CrossData[port - 1].NeedStep = MAX_STEP_NUM + 2;
		CrossData[port - 1].CurCtrMode = 0xff;//D当前的控制模式
		CrossData[port - 1].CurCValue = 0xff;//C当前的C值
		CrossData[port - 1].Left5Min = 5 * 60;

		CrossData[port - 1].CycleTimeOffset = 0;
		CrossData[port - 1].CycleAulCount = 0;

		return;
	}

	//accumulated red lamp data at the point of entering green lamp
	RedLampProc(port,CurStep);
	//notify wu that cross status has changed
//	if(CurStep == 4)
//		CurStep = 4;
	SendCrossStatus2Wu(port);

	if(CurStep != 0)
	{
		//if not the first step,only return
		return;
	}

//following is the new round of a cycle
	//accumulate cycle data and clear cycle data
#ifdef TEST
	memset(disp,0,200);
	sprintf(disp,"Port=%d,In Past Cycle,Cycle=%d,CycleResend=%d",port,								\
						CrossData[port - 1].CycleTimeOffset,CrossData[port - 1].CycleAulCount);
	DebugWindow(disp);
#endif

	//added 07,30,2001
//	if(CrossData[port - 1].CycleVeCount[i] > 0)
	CrossData[port - 1].RedCycleCount5m++;
	//added end
	for(i = 0;i < MAX_JCQ_NUM; i++)
	{
		if(CrossData[port - 1].CycleVeCount[i] <= 0)
			continue;

		CrossData[port - 1].VeCountp5m[i] += CrossData[port - 1].CycleVeCount[i];
		CrossData[port - 1].HiBitsp5m[i] += CrossData[port - 1].CycleHiBits[i];
//added 07,30,2001
//		CrossData[port - 1].RedVeCountp5m[i] += CrossData[port - 1].tempRedVeCountp5m[i];
//		CrossData[port - 1].RedVeWaitTime5m[i] += CrossData[port - 1].tempRedVeWaitTime5m[i];
//end

	//added 07,31,2001
		CrossData[port - 1].RedVeCountp5m[i] += CrossData[port - 1].CycleRedVeCount[i];
		CrossData[port - 1].RedVeWaitTime5m[i] += CrossData[port - 1].CycleRedVeWait[i];
	//end
/*
#ifdef TEST
		memset(disp,0,200);
		sprintf(disp,"port=%d,RedVeWait[%d]=%d",port,i+1,CrossData[port - 1].CycleRedVeWait[i]/20);
		DebugWindow(disp);
#endif
*/
		CrossData[port - 1].CycleVeCount[i] = 0;
		CrossData[port - 1].CycleHiBits[i] = 0;
//added 07,30,2001
//		CrossData[port - 1].tempRedVeCountp5m[i] = 0;
//		CrossData[port - 1].tempRedVeWaitTime5m[i] = 0;
//end
	//added 07,31,2001
		CrossData[port - 1].CycleRedVeCount[i] = 0;
		CrossData[port - 1].CycleRedVeWait[i] = 0;
	//end
	}
	CrossData[port - 1].RealTime5Min +=	(CrossData[port - 1].CycleTimeOffset +						\
											CrossData[port - 1].CycleAulCount);
	CrossData[port - 1].CycleTimeOffset = 0;
	CrossData[port - 1].CycleAulCount = 0;

	switch(CrossData[port - 1].Status)
	{
	case TRANSIT_STATUS:

		TransitPrepare(port);
		break;

	case NORMAL_STATUS:

		NormalPrepare(port);
		break;

	case FORCED_STATUS:

	//here,there is no so_called period now,but may not reach the forced(required) step
		NormalPrepare(port);
		CrossData[port - 1].Status = FORCED_STATUS;
		break;

	case OFFLINE_STATUS:
	//only break
		break;

	default:
		break;
	}
	
	if(CrossData[port - 1].Status == NORMAL_STATUS)
		SynCrossPhase(port);

	return;
}

/***************************************************************************
*	Function Name	: void SynCrossPhase(int port)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/08,15,2001
*   Modify		    : yincy/04,17,2002
*	Global			: None
*	Note			: None
****************************************************************************/	
void SynCrossPhase(int port)
{
	int BaseCross;
	int Phi;
	int adjustLen,absLen;
	int cyclelen;
	int halfCycle;
#ifdef TEST
	char disp[200];
#endif

	if(port <= 0 || port > MAX_CROSS_NUM)
	{
		DebugWindow("SynCrossPhase(),Input port Error!");
		return;
	}

	if(CrossData[port - 1].Type != 2)//jinsan
		return;

	if(CrossData[port - 1].BaseCross == 0 || CrossData[port - 1].BaseCross >= MAX_CROSS_NUM)
		return;

	BaseCross = CrossData[port - 1].BaseCross;
	Phi = CrossData[port - 1].DeltaPhi;
//added 04,15,2002
	if(CrossData[BaseCross - 1].Type != 2)
	{
		DebugWindow("SynCrossPhase(),BaseCross Error!");
		CrossData[port - 1].SynErrCount = 0;
		CrossData[port - 1].BaseCross = 0;
		CrossData[port - 1].DeltaPhi = 0;

		return;
	}
//added end

	if(CrossData[BaseCross - 1].Status != NORMAL_STATUS)
	{
#ifdef TEST
		DebugWindow("SynCrossPhase(),BaseCross Status Invalid!");
#endif
		CrossData[port - 1].SynErrCount++;
		return;//can not syn so far
	}
/*
	if(CrossData[BaseCross - 1].ManCtrFlag > 0)
	{
#ifdef TEST
		DebugWindow("SynCrossPhase(),BaseCross ManCtl Status!");
#endif
		CrossData[port - 1].SynErrCount++;
		return;
	}
*/
	if(!CycleIsEqual(port,BaseCross))
	{
		DebugWindow("SynCrossPhase(),BaseCross and port cycle not equal!");
		CrossData[port - 1].SynErrCount = 0;
		CrossData[port - 1].BaseCross = 0;
		CrossData[port - 1].DeltaPhi = 0;

		return;
	}

	if(CrossData[port - 1].SynErrCount > MAX_SYNERR_COUNT)
	{
		DebugWindow("SynCrossPhase(),SynErrCount Exceed MAX_SYNERR_COUNT!");
		CrossData[port - 1].SynErrCount = 0;
		CrossData[port - 1].BaseCross = 0;
		CrossData[port - 1].DeltaPhi = 0;

		return;
	}
/*		
#ifdef TEST
	memset(disp,0,200);
	sprintf(disp,"Port=%03d,Start Syn!",port);
	DebugWindow(disp);
#endif
	if(BaseCross <= 0 || BaseCross > MAX_CROSS_NUM)
	{
		DebugWindow("SynCrossPhase(),BaseCross Error!");
		CrossData[port - 1].SynErrCount = 0;
		CrossData[port - 1].BaseCross = 0;
		CrossData[port - 1].DeltaPhi = 0;
		return;
	}

	if(CrossData[BaseCross - 1].Type != 2)
	{
		DebugWindow("SynCrossPhase(),BaseCross Type Error!");
		CrossData[port - 1].SynErrCount = 0;
		CrossData[port - 1].BaseCross = 0;
		CrossData[port - 1].DeltaPhi = 0;
		return;
	}
*/
#ifdef TEST
	memset(disp,0,200);
	sprintf(disp,"Port=%d,BasePort=%d,Required Phi=%d,Real Phi=%d",port,CrossData[port - 1].BaseCross,							\
			CrossData[port - 1].DeltaPhi,CrossData[CrossData[port - 1].BaseCross - 1].CycleTimeOffset);
	DebugWindow(disp);
#endif

	cyclelen = GetCycleLen(port);
	halfCycle = cyclelen / 2;
	if(cyclelen <= 0)
	{
//#ifdef TEST
		DebugWindow("cyclelen-Fatal error!");
//#endif
		return;//fatal error
	}
	adjustLen =  CrossData[port - 1].DeltaPhi - CrossData[BaseCross - 1].CycleTimeOffset;
/*	if(adjustLen < 0)
		absLen = - adjustLen;
	else
		absLen = adjustLen;
*/
	absLen = adjustLen > 0 ? adjustLen : -adjustLen;
	if(absLen <= MIN_SYN_LEN)
	{
#ifdef TEST
		DebugWindow("---Syn Not Needed---");
#endif
		return;//need not to syn so far
	}

	if(adjustLen > 0)
	{
		if(absLen <= halfCycle)
		{
			if(TimeCanDelay(port))
			{
				StartSyn(port,adjustLen);
			}
			else if(TimeCanShort(port))
			{
				StartSyn(port,absLen - cyclelen);
			}
			else
			{
				DebugWindow("Syn Failed 1---No Syn Space!");
			}
		}
		else
		{
			if(TimeCanShort(port))
			{
				StartSyn(port,absLen - cyclelen);
			}
			else if(TimeCanDelay(port))
			{
				StartSyn(port,adjustLen);
			}
			else
			{
				DebugWindow("Syn Failed 2---No Syn Space!");
			}
		}
	}
	else
	{
		if(absLen <= halfCycle)
		{
			if(TimeCanShort(port))
			{
				StartSyn(port,adjustLen);
			}
			else if(TimeCanDelay(port))
			{
				StartSyn(port,cyclelen - absLen);
			}
			else
			{
				DebugWindow("Syn Failed 3---No Syn Space!");
			}
		}
		else
		{
			if(TimeCanDelay(port))
			{
				StartSyn(port,cyclelen - absLen);
			}
			else if(TimeCanShort(port))
			{
				StartSyn(port,adjustLen);
			}
			else
			{
				DebugWindow("Syn Failed 4---No Syn Space!");
			}
		}
	}

	return;
}


/***************************************************************************
*	Function Name	: void InSynCross(int port,int AddLen)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/04,18,2002
*	Global			: None
*	Note			: None
****************************************************************************/	
void InSynCross(int port,int AddLen)
{
	int i;
	UINT8 StepNo,EffectStepNo;
	int j;
	int EffectTotalLen;
	int EffectStepNumber[MAX_STEP_NUM / 4];
	int EffectStepLen[MAX_STEP_NUM / 4];
	int temp1;
	int absLen;

	if(port <= 0 || port > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("InSynCross(),Input port Error!");
#endif
		return;
	}

	absLen = AddLen > 0 ? AddLen : - AddLen;

	StepNo = CrossData[port - 1].TotalStepNo;
	if(StepNo <= 0 || StepNo > MAX_STEP_NUM)
		return;

	EffectStepNo = 0;
	EffectTotalLen = 0;
	j = 0;
	for(i = 0;i < StepNo;i++)
	{
		if(CrossData[port - 1].StepTable[i] >= MIN_GREEN_LEN)
		{
			EffectStepNo++;
			EffectTotalLen += CrossData[port - 1].StepTable[i];
			EffectStepNumber[j] = i;
			EffectStepLen[j] = CrossData[port - 1].StepTable[i];

			j++;
		}
	}

	if(EffectStepNo <= 0)
	{
		DebugWindow("InSynCross(),No Step Length > MIN_GREEN_LEN,Only Return!");
		CrossData[port - 1].SynErrCount++;

		return;
	}

/*	UINT8 EffectStepNo;
	int EffectTotalLen;
	int EffectStepNumber[MAX_STEP_NUM / 4];
	int EffectStepLen[MAX_STEP_NUM / 4];
	absLen = Addlen > 0 ? AddLen : -AddLen;
*/
	temp1 = absLen * 100 / EffectTotalLen;
	if(temp1 > 30)
	{
		if(AddLen > 0)
			AddLen = 30 * EffectTotalLen / 100;
		else
			AddLen = - 30 * EffectTotalLen / 100;
	}
	

	for(j = 0; j < EffectStepNo;j++)
	{
		EffectStepLen[j] = EffectStepLen[j] * AddLen / EffectTotalLen;
	}


	return;
}
/***************************************************************************
*	Function Name	: void StartSyn(int port,int AddLen)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/08,15,2001
*   Modify		    : yincy/04,01,2002
*   Modify		    : yincy/04,15,2002
*	Global			: None
*	Note			: None
****************************************************************************/	
void StartSyn(int port,int AddLen)
{
	int i;
	UINT8 StepNo,EffectStepNo;
	int temp1,temp2,temp3;
	int absLen;
	int cyclelen;

	if(port <= 0 || port > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("StartSyn(),Input port Error!");
#endif
		return;
	}

	if(AddLen == 0)
	{
		DebugWindow("StartSyn(),AddLen=0!");
		return;	
	}

	if(AddLen < 0)
		absLen = - AddLen;
	else
		absLen = AddLen;

	cyclelen = GetCycleLen(port);
	if(cyclelen < absLen * 5)
		absLen = cyclelen / 5;

	StepNo = CrossData[port - 1].TotalStepNo;
	if(StepNo <= 0 || StepNo > MAX_STEP_NUM)
		return;

	EffectStepNo = 0;
	for(i = 0;i < StepNo;i++)
	{
		if(CrossData[port - 1].StepTable[i] >= MIN_GREEN_LEN)
			EffectStepNo++;
	}

	if(EffectStepNo <= 0)
	{
		DebugWindow("StartSyn(),No Step Length > MIN_GREEN_LEN,Only Return!");
		CrossData[port - 1].SynErrCount++;
		return;
	}

	temp1 = absLen / EffectStepNo;
	temp2 = absLen % EffectStepNo;

//Main adjust
	memcpy(CrossData[port - 1].TransitStepTable,CrossData[port - 1].StepTable,MAX_STEP_NUM * sizeof(UINT16));
	if(temp1 > 0)
	{
		for(i = 0;i < StepNo;i++)
		{
			if(CrossData[port - 1].StepTable[i] < MIN_GREEN_LEN)
			{
				CrossData[port - 1].TransitStepTable[i] = CrossData[port - 1].StepTable[i];
				continue;
			}

			if(AddLen > 0)
			{
				if(CrossData[port - 1].MaxStepTable[i] == 0)
					CrossData[port - 1].TransitStepTable[i] = CrossData[port - 1].StepTable[i] + temp1;
				else
				{
					if(CrossData[port - 1].StepTable[i] + temp1 > CrossData[port - 1].MaxStepTable[i])
					{
						CrossData[port - 1].TransitStepTable[i] = CrossData[port - 1].MaxStepTable[i];
					//??????
						temp2 += (CrossData[port - 1].StepTable[i] + temp1 - CrossData[port - 1].MaxStepTable[i]);
					//	temp2 += (temp1 - CrossData[port - 1].MaxStepTable[i]);
					}
					else
						CrossData[port - 1].TransitStepTable[i] = CrossData[port - 1].StepTable[i] + temp1;
				}

			}
			else
			{
			//addlen<0

				if(CrossData[port - 1].MinStepTable[i] == 0)
				{
					if(CrossData[port - 1].StepTable[i] <= temp1 + MIN_GREEN_LEN)
					{
						temp3 = CrossData[port - 1].StepTable[i] / 2;
						if(temp3 < MIN_GREEN_LEN)
							temp3 = MIN_GREEN_LEN;
						CrossData[port - 1].TransitStepTable[i] = temp3;

					
						temp2 += ( temp3 - (CrossData[port - 1].StepTable[i] - temp1) );
						//temp2 += (temp1 - temp3);
					}
					else
						CrossData[port - 1].TransitStepTable[i] = CrossData[port - 1].StepTable[i] - temp1;
				}
				else
				{
					//CrossData[port - 1].MinStepTable[i] > 0
					if(CrossData[port - 1].StepTable[i] < temp1 + CrossData[port - 1].MinStepTable[i])
					{
						CrossData[port - 1].TransitStepTable[i] = CrossData[port - 1].MinStepTable[i];
						temp2 += ( CrossData[port - 1].MinStepTable[i]											\
													- (CrossData[port - 1].StepTable[i] - temp1) );
					}
					else
						CrossData[port - 1].TransitStepTable[i] = CrossData[port - 1].StepTable[i] - temp1;
				}
			}
		}
	}

//min adjust
	if(temp2 > 0)
	{
		if(AddLen > 0)
		{
			for(i = 0;i < StepNo;i++)
			{
				if(CrossData[port - 1].MaxStepTable[i] == 0)
				{
					if( CrossData[port - 1].TransitStepTable[i] >= MIN_GREEN_LEN)
					{
						CrossData[port - 1].TransitStepTable[i] += temp2;
						goto exit_adjust;
					}
				}
				else
				{
					//MaxStepTable[i] > 0
					if(CrossData[port - 1].TransitStepTable[i] < CrossData[port - 1].MaxStepTable[i])
					{
						temp3 = CrossData[port - 1].MaxStepTable[i] - CrossData[port - 1].TransitStepTable[i];
						if(temp3 >= temp2)
						{
							CrossData[port - 1].TransitStepTable[i] += temp2;
							goto exit_adjust;
						}
						else
						{
							CrossData[port - 1].TransitStepTable[i] = CrossData[port - 1].MaxStepTable[i];
							temp2 -= temp3;
						}
					}
				}
			}
		}
		else
		{
			//addlen < 0
			for(i = 0;i < StepNo;i++)
			{
				if(CrossData[port - 1].MinStepTable[i] == 0)
				{
					if( CrossData[port - 1].TransitStepTable[i] > MIN_GREEN_LEN)
					{
						if(CrossData[port - 1].TransitStepTable[i] - temp2 > MIN_GREEN_LEN)
						{
							CrossData[port - 1].TransitStepTable[i] -= temp2;
							goto exit_adjust;
						}
						else
						{
							temp3 = CrossData[port - 1].TransitStepTable[i];
							CrossData[port - 1].TransitStepTable[i] = MIN_GREEN_LEN;
							temp2 -= (temp3 - MIN_GREEN_LEN);
						}
					}
				}
				else
				{
					//MinStepTable[i] > 0
					if(CrossData[port - 1].TransitStepTable[i] > CrossData[port - 1].MinStepTable[i])
					{
						temp3 = CrossData[port - 1].TransitStepTable[i] - CrossData[port - 1].MinStepTable[i];
						if(temp3 >= temp2)
						{
							CrossData[port - 1].TransitStepTable[i] -= temp2;
							goto exit_adjust;
						}
						else
						{
							CrossData[port - 1].TransitStepTable[i] = CrossData[port - 1].MinStepTable[i];
							temp2 -= temp3;
						}
					}
				}
			}
		}
	}

exit_adjust:
	for(i = 0;i < StepNo;i++)
	{
		if(CrossData[port - 1].TransitStepTable[i] != CrossData[port - 1].StepTable[i])
		{
			CrossData[port - 1].Status = TRANSIT_STATUS;
			return;
		}
	}

#ifdef TEST
	DebugWindow("StartSyn(),NORMAL_STATUS!");
#endif
	CrossData[port - 1].Status = NORMAL_STATUS;
		
	return;
}

/***************************************************************************
*	Function Name	: BOOL TimeCanDelay(int port)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/04,16,2002
*	Global			: None
*	Note			: None
****************************************************************************/	
BOOL TimeCanDelay(int port)
{
	int i;

	if(port <= 0 || port > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("TimeCanDelay(),Input port Error!");
#endif
		return FALSE;
	}

	if(CrossData[port - 1].TotalStepNo < 4 || CrossData[port - 1].TotalStepNo > MAX_STEP_NUM)
	{
#ifdef TEST
		DebugWindow("TimeCanDelay(),Input TotalStepNo Error!");
#endif
		return FALSE;
	}

	for(i = 0;i < CrossData[port - 1].TotalStepNo;i++)
	{
		if(CrossData[port - 1].StepTable[i] < MIN_GREEN_LEN)
			continue;

		if(CrossData[port - 1].MaxStepTable[i] > CrossData[port - 1].StepTable[i]						\
											|| CrossData[port - 1].MaxStepTable[i] == 0)
		{
			return TRUE;
		}
	}

	return FALSE;
}

/***************************************************************************
*	Function Name	: BOOL TimeCanShort(int port)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/04,16,2002
*	Global			: None
*	Note			: None
****************************************************************************/	
BOOL TimeCanShort(int port)
{
	int i;

	if(port <= 0 || port > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("TimeCanShort(),Input port Error!");
#endif
		return FALSE;
	}

	if(CrossData[port - 1].TotalStepNo < 4 || CrossData[port - 1].TotalStepNo > MAX_STEP_NUM)
	{
#ifdef TEST
		DebugWindow("TimeCanShort(),Input TotalStepNo Error!");
#endif
		return FALSE;
	}

	for(i = 0;i < CrossData[port - 1].TotalStepNo;i++)
	{
		if(CrossData[port - 1].StepTable[i] < MIN_GREEN_LEN)
			continue;

		if(CrossData[port - 1].MinStepTable[i] < CrossData[port - 1].StepTable[i]						\
											|| CrossData[port - 1].MinStepTable[i] == 0)
		{
			return TRUE;
		}
	}

	return FALSE;
}


/***************************************************************************
*	Function Name	: int GetAdjustLen(int port,int BaseCross)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/08,15,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
int GetAdjustLen(int port,int BaseCross)
{
	int NeedPhi;
	int CurPhi;

	if(port <= 0 || port > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("GetAdjustLen(),Input port Error!");
#endif
		return 30000;
	}

	if(BaseCross <= 0 || BaseCross > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("GetAdjustLen(),Input BaseCross Error!");
#endif
		return 30000;
	}

	NeedPhi = CrossData[port - 1].DeltaPhi;
	CurPhi = CrossData[BaseCross - 1].CycleTimeOffset;

	return (CurPhi - NeedPhi);
}


/***************************************************************************
*	Function Name	: BOOL CycleIsEqual(int port,int BaseCross)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/08,15,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
BOOL CycleIsEqual(int port,int BaseCross)
{
	int templen1,templen2;

	if(port <= 0 || port > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("CycleIsEqual(),Input port Error!");
#endif
		return FALSE;
	}

	if(BaseCross <= 0 || BaseCross > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("CycleIsEqual(),Input BaseCross Error!");
#endif
		return FALSE;
	}

	if(CrossData[port - 1].TotalStepNo <= 0 || CrossData[port - 1].TotalStepNo > MAX_STEP_NUM)
		return FALSE;

	if(CrossData[port - 1].TotalStepNo != CrossData[BaseCross - 1].TotalStepNo)
		return FALSE;

	templen1 = GetCycleLen(BaseCross);
	templen2 = GetCycleLen(port);

	if( (templen1 == templen2) && (templen1 != 0) )
		return TRUE;
	
	return FALSE;
}

/***************************************************************************
*	Function Name	: int GetCycleLen(int port)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/08,15,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
int GetCycleLen(int port)
{
	int TotalStepNo;
	int i;
	int len;

	if(port <= 0 || port > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("GetCycleLen(),Input port Error!");
#endif
		return 0;
	}

	TotalStepNo = CrossData[port - 1].TotalStepNo;
	if(TotalStepNo < 4 || TotalStepNo > MAX_STEP_NUM)
	{
#ifdef TEST
		DebugWindow("GetCycleLen(),TotalStepNo Error!");
#endif
		CrossData[port - 1].Status = IDLE_STATUS;
		CrossData[port - 1].InitalWait = 3;
		CrossData[port - 1].CurStep = MAX_STEP_NUM + 1;
		CrossData[port - 1].NeedStep = MAX_STEP_NUM + 2;
		CrossData[port - 1].CurCtrMode = 0xff;//D当前的控制模式
		CrossData[port - 1].CurCValue = 0xff;//C当前的C值
		CrossData[port - 1].Left5Min = 5 * 60;

		CrossData[port - 1].CycleTimeOffset = 0;
		CrossData[port - 1].CycleAulCount = 0;

		return 0;
	}

	len = 0;
	for(i = 0;i < TotalStepNo;i++)
		len += (CrossData[port - 1].StepTable[i]);

	return len;
}

/***************************************************************************
*	Function Name	: void StartSynEx(int port,int AddLen)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,28,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
#ifdef __OLD__
void StartSynEx(int port,int AddLen)
{
	int i;
	UINT8 StepNo,EffectStepNo;
	int temp1,temp2,temp3;
	int absLen;

	if(port <= 0 || port > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("StartSynEx(),Input port Error!");
#endif
		return;
	}

	if(AddLen < 0)
		absLen = - AddLen;
	else
		absLen = AddLen;

	StepNo = CrossData[port - 1].TotalStepNo;
	if(StepNo <= 0 || StepNo > MAX_STEP_NUM)
		return;

	EffectStepNo = 0;
	for(i = 0;i < StepNo;i++)
	{
		if(CrossData[port - 1].StepTable[i] > 5)
			EffectStepNo++;
	}

	if(EffectStepNo <= 0)
	{
		DebugWindow("StartSynEx(),No Step Length > 5,Only Return!");
		CrossData[port - 1].SynErrCount++;
		return;
	}

	temp1 = absLen / EffectStepNo;
	temp2 = absLen % EffectStepNo;


//Main adjust
	for(i = 0;i < StepNo;i++)
	{
		if(CrossData[port - 1].StepTable[i] <= 5)
		{
			CrossData[port - 1].TransitStepTable[i] = CrossData[port - 1].StepTable[i];
			continue;
		}

		if(AddLen > 0)
		{
			if(CrossData[port - 1].MaxStepTable[i] == 0)
				CrossData[port - 1].TransitStepTable[i] = CrossData[port - 1].StepTable[i] + temp1;
			else
			{
				if(CrossData[port - 1].StepTable[i] + temp1 > CrossData[port - 1].MaxStepTable[i])
				{
					CrossData[port - 1].TransitStepTable[i] = CrossData[port - 1].MaxStepTable[i];
					temp2 += (CrossData[port - 1].StepTable[i] + temp1 - CrossData[port - 1].MaxStepTable[i]);
				}
				else
					CrossData[port - 1].TransitStepTable[i] = CrossData[port - 1].StepTable[i] + temp1;
			}
		}
		else
		{
		//addlen<0

			if(CrossData[port - 1].MinStepTable[i] == 0)
			{
				if(CrossData[port - 1].StepTable[i] <= temp1)
				{
					temp3 = CrossData[port - 1].StepTable[i] / 2;
					if(temp3 < 5)
						temp3 = 5;
					CrossData[port - 1].TransitStepTable[i] = temp3;

					//?????
					temp2 += ( temp3 - (CrossData[port - 1].StepTable[i] - temp1) );
				}
				else
					CrossData[port - 1].TransitStepTable[i] = CrossData[port - 1].StepTable[i] - temp1;
			}
			else
			{
				//CrossData[port - 1].MinStepTable[i] > 0
				if(CrossData[port - 1].StepTable[i] - temp1 < CrossData[port - 1].MinStepTable[i])
				{
					CrossData[port - 1].TransitStepTable[i] = CrossData[port - 1].MinStepTable[i];
					temp2 += ( CrossData[port - 1].MinStepTable[i] \
										- (CrossData[port - 1].StepTable[i] - temp1) );
				}
				else
					CrossData[port - 1].TransitStepTable[i] = CrossData[port - 1].StepTable[i] - temp1;
			}//end if(CrossData[port - 1].MinStepTable[i] == 0)
		}//end if(AddLen > 0)
	}//end for	

	CrossData[port - 1].Status = TRANSIT_STATUS;

	if(temp2 == 0)
		return;

//min adjust
	if(AddLen > 0)
	{
		for(i = 0;i < StepNo;i++)
		{
			if(CrossData[port - 1].MaxStepTable[i] == 0)
			{
				if( CrossData[port - 1].TransitStepTable[i] > 5)
				{
					CrossData[port - 1].TransitStepTable[i] += temp2;
					return;
				}
			}
			else
			{
				//MaxStepTable[i] > 0
				if(CrossData[port - 1].TransitStepTable[i] < CrossData[port - 1].MaxStepTable[i])
				{
					temp3 = CrossData[port - 1].MaxStepTable[i] - CrossData[port - 1].TransitStepTable[i];
					if(temp3 > temp2)
					{
						CrossData[port - 1].TransitStepTable[i] += temp2;
						return;
					}
					else
					{
						CrossData[port - 1].TransitStepTable[i] = CrossData[port - 1].MaxStepTable[i];
						temp2 -= temp3;
					}
				}
			}
		}
	}
	else
	{
		//addlen < 0
		for(i = 0;i < StepNo;i++)
		{
			if(CrossData[port - 1].MinStepTable[i] == 0)
			{
				if( CrossData[port - 1].TransitStepTable[i] > 5)
				{
					if(CrossData[port - 1].TransitStepTable[i] - temp2 > 5)
					{
						CrossData[port - 1].TransitStepTable[i] -= temp2;
						return;
					}
					else
					{
						temp3 = CrossData[port - 1].TransitStepTable[i];
						CrossData[port - 1].TransitStepTable[i] = 5;
						temp2 -= (temp3 - 5);
					}
				}
			}
			else
			{
				//MinStepTable[i] > 0
				if(CrossData[port - 1].TransitStepTable[i] > CrossData[port - 1].MinStepTable[i])
				{
					temp3 = CrossData[port - 1].TransitStepTable[i] - CrossData[port - 1].MinStepTable[i];
					if(temp3 > temp2)
					{
						CrossData[port - 1].TransitStepTable[i] -= temp2;
						return;
					}
					else
					{
						CrossData[port - 1].TransitStepTable[i] = CrossData[port - 1].MinStepTable[i];
						temp2 -= temp3;
					}
				}
			}//end if(MinStepTable[i] 
		}//end for
	}

	return;
}
#endif

/***************************************************************************
*	Function Name	: void GetComStatusChange(int port,int Status)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/11,01,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
void GetComStatusChange(int port,int Status)
{
	SYSTEMTIME time;

	if(port <= 0 || port > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("GetComStatusChange(),Input port Error!");
#endif
		return;
	}

	GetLocalTime(&time);

//	if(Status == 1)
	if(Status > 0)
	{
		CrossData[port - 1].LastDisconnectTime.wYear = time.wYear;
		CrossData[port - 1].LastDisconnectTime.wMonth = time.wMonth;
		CrossData[port - 1].LastDisconnectTime.wDay = time.wDay;
		CrossData[port - 1].LastDisconnectTime.wHour = time.wHour;
		CrossData[port - 1].LastDisconnectTime.wMinute = time.wMinute;
		CrossData[port - 1].LastDisconnectTime.wSecond = time.wSecond;
		CrossData[port - 1].LastDisconnectTime.wDayOfWeek = time.wDayOfWeek;
	}
	else if(Status == 0)
	{
		CrossData[port - 1].LastConnectTime.wYear = time.wYear;
		CrossData[port - 1].LastConnectTime.wMonth = time.wMonth;
		CrossData[port - 1].LastConnectTime.wDay = time.wDay;
		CrossData[port - 1].LastConnectTime.wHour = time.wHour;
		CrossData[port - 1].LastConnectTime.wMinute = time.wMinute;
		CrossData[port - 1].LastConnectTime.wSecond = time.wSecond;
		CrossData[port - 1].LastConnectTime.wDayOfWeek = time.wDayOfWeek;
	}
	else
	{
#ifdef TEST
		DebugWindow("GetComStatusChange(),Status Value Error!");
#endif
	}

	return;
}

/***************************************************************************
*	Function Name	: void SendCommStatus2Wu(int port,int Status)
*	Description		: 0 is ok otherwise error code
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/10,23,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
void SendCommStatus2Wu(int port,int Status)
{
	Rep_UpCommStatus UpCommStatus;

	if(port <= 0 || port > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("SendCommStatus2Wu(),Input port Error!");
#endif
		return;
	}

	GetComStatusChange(port,Status);

	memset(&UpCommStatus,0,sizeof(Rep_UpCommStatus));
	UpCommStatus.byFlag = MsgFlag;
	UpCommStatus.MsgType = CM_SENDCOMMSTATUS;

	strcpy(UpCommStatus.SourceIP,HostIp);
	strcpy(UpCommStatus.SourceID,HostId);
	strcpy(UpCommStatus.TargetIP,SvrIp);
	strcpy(UpCommStatus.TargetID,SvrId);

	UpCommStatus.iCrossNo = CrossData[port - 1].lkbh;
	UpCommStatus.iLength = 0;
	UpCommStatus.iReserved1 = Status;

	UpCommStatus.byCheckSum = FormCheckSum((char *)&UpCommStatus);

	SendDataEx((char *)&UpCommStatus,sizeof(Rep_UpCommStatus),(UINT8)1);
	
	return;
}



/***************************************************************************
*	Function Name	: void SendCrossStatus2Wu(int port)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,24,2001
*	Global			: None
*	Note			: None
*	Modify			: yincy/08,02,2001
****************************************************************************/	
void SendCrossStatus2Wu(int port)
{
	Rep_CrossNowStatus CrossStatus;
//	CrossNowStatus realstatus;
//#ifdef TEST
//	int j;
//	char disp[200];
//#endif

	if(port <= 0 || port > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("SendCrossStatus2Wu(),Input port Error!");
#endif
		return;
	}
/*
	memset(&realstatus,0,sizeof(CrossNowStatus));
	realstatus.NowStep = CrossData[port - 1].CurStep + 1;
	realstatus.NowCtrMode = CrossData[port - 1].CurCtrMode;
*/
	memset(&CrossStatus,0,sizeof(Rep_CrossNowStatus));
	CrossStatus.byFlag = MsgFlag;
	CrossStatus.MsgType = CM_LAMPSTATUS;

	strcpy(CrossStatus.SourceIP,HostIp);
	strcpy(CrossStatus.SourceID,HostId);
	strcpy(CrossStatus.TargetIP,SvrIp);
	strcpy(CrossStatus.TargetID,SvrId);

	CrossStatus.iCrossNo = CrossData[port - 1].lkbh;
	//CrossStatus.iLength = sizeof(CrossNowStatus);
//	CrossStatus.iLength = *((WORD *)&realstatus);
	CrossStatus.iLength = 0;
	CrossStatus.iReserved1 = CrossData[port - 1].CurCtrMode;
	CrossStatus.iReserved2 = CrossData[port - 1].CurStep + 1;

	CrossStatus.byCheckSum = FormCheckSum((char *)&CrossStatus);

//#ifdef TEST
//	memset(disp,0,200);
//	sprintf(disp,"Port=%3d,CurStep=%2d",port,CrossData[port - 1].CurStep + 1);
//	DebugWindow(disp);
//#endif

	//send to svr
//#ifdef TEST
//	for(j = 0;j<2;j++)
//#endif
	SendDataEx((char *)&CrossStatus,sizeof(Rep_CrossNowStatus),(UINT8)1);
	
	return;
}


/***************************************************************************
*	Function Name	: void RedLampProc(int port)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,18,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
void RedLampProc(int port,UINT8 CurStep)
{
	int i;
	int j;

	if(port <= 0 || port > MAX_CROSS_NUM)
	{
		DebugWindow("RedLampProc(),Input port Error!");
		return;
	}

/*	if(CurStep > MAX_STEP_NUM)
	{
		DebugWindow("RedLampProc(),Input CurStep Error!");
		return;
	}
*/
	switch(CrossData[port - 1].Status)
	{
	case TRANSIT_STATUS:
	case NORMAL_STATUS:
	case FORCED_STATUS:
	case OFFLINE_STATUS:

		for(i = 0;i < 4;i++)
		{
			for(j = 0;j < MAX_JCQ_NUM; j++)
			{

				if(CrossData[port - 1].jcqStartGreenStep[i][j] == 0  \
					|| CrossData[port - 1].jcqStartGreenStep[i][j] > CrossData[port - 1].TotalStepNo \
					|| CrossData[port - 1].jcqEndGreenStep[i][j] == 0 \
					|| CrossData[port - 1].jcqEndGreenStep[i][j] > CrossData[port - 1].TotalStepNo \
					|| CrossData[port - 1].jcqStartGreenStep[i][j] > CrossData[port - 1].jcqEndGreenStep[i][j] )
				{
		//				DebugWindow("RedLampProc(),Warning: Maybe No GreenStep!");
					continue;//return;
				}

		//modify 07,25,2001
		//			if(CrossData[port - 1].CurStep >= CrossData[port - 1].jcqStartGreenStep[i] - 1 \
		//				|| CrossData[port - 1].CurStep <= CrossData[port - 1].jcqEndGreenStep[i] - 1)
				if(CrossData[port - 1].CurStep == CrossData[port - 1].jcqStartGreenStep[i][j] - 1)
				{
				//Entering Green lamp status,accumulate(dump) red time data to 5-Min data 
					if(CrossData[port - 1].AsyCycRedVeCount[j] > 0)
					{
					//deleted 07,31,2001
						//CrossData[port - 1].tempRedVeCountp5m[i] += CrossData[port - 1].CycleRedVeCount[i];
						//CrossData[port - 1].tempRedVeWaitTime5m[i] += CrossData[port - 1].CycleRedVeWait[i];
						//CrossData[port - 1].RedVeCountp5m[i]++;
						CrossData[port - 1].AsyCycRedVeCount[j] = 0;
/*						if(i == 0)
							i = 0;
						if(i == 2)
							i = 2;
*/						//CrossData[port - 1].CycleRedVeWait[i] = 0;
					}
				}
			}
		}
		break;

	default:
		break;
	}


	return;
}

/***************************************************************************
*	Function Name	: void NewRoundPrepare(int port)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,11,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
void NewRoundPrepare(int port)
{
//	int i;

	if(port <= 0 || port > MAX_CROSS_NUM)
	{
#ifdef TEST	
		DebugWindow("NewRoundPrepare(),Input port Error!");
#endif
		return;
	}

	//prepare for tranist status
	if(CrossData[port - 1].Status == TRANSIT_STATUS)
		TransitPrepare(port);
	else if(CrossData[port - 1].Status == INITIAL_STATUS)
		InitialPrepare(port);
	else if(CrossData[port - 1].Status == NORMAL_STATUS)
		NormalPrepare(port);
		
	return;
}

/***************************************************************************
*	Function Name	: void NormalPrepare(int port)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,11,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
void NormalPrepare(int port)
{
#ifdef TEST	
	char disp[200];
#endif

	if(port <= 0 || port > MAX_CROSS_NUM)
	{
#ifdef TEST	
		DebugWindow("InitialPrepare(),Input port Error!");
#endif
		return;
	}

	//this is a opportunity to syn cross,a remedy
	if(CrossData[port - 1].NeedStep != 0)
	{
#ifdef TEST	
		memset(disp,0,200);
		sprintf(disp,"NormalPrepare(),Port=%d,CurStep=0,NeedStep=%d",port,						\
											CrossData[port - 1].NeedStep);
		DebugWindow(disp);
#endif
		if(CrossData[port - 1].InitalWait != 1)//added 02,29,2002
			CrossData[port - 1].InitalWait = 0;
		CrossData[port - 1].NeedStep = 0;
		CrossData[port - 1].Status = NORMAL_STATUS;
	}

	CrossData[port - 1].CurStep = 0;
//	CrossData[port - 1].CurStepCount = 0;
	CrossData[port - 1].StepErrCount = 0;

	return;
}

/***************************************************************************
*	Function Name	: void InitialPrepare(int port)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,11,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
void InitialPrepare(int port)
{
	if(port <= 0 || port > MAX_CROSS_NUM)
	{
		DebugWindow("InitialPrepare(),Input port Error!");
		return;
	}

	//this is a good chance to syn cross
//	if(CrossData[port - 1].InitalWait == 0)
//	{
		CrossData[port - 1].InitalWait = 0;
		CrossData[port - 1].CurStep = 0;
//		CrossData[port - 1].CurStepCount = 0;
		CrossData[port - 1].NeedStep = 0;
		CrossData[port - 1].StepErrCount = 0;
		CrossData[port - 1].Status = NORMAL_STATUS;
//	}

	return;
}

/***************************************************************************
*	Function Name	: void TransitPrepare(int port)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,11,2001
*	Global			: None
*	Note			: None
*   Modify			: yincy/07,25,2001
****************************************************************************/	
void TransitPrepare(int port)
{
	UINT8 BaseCross;
	UINT16 DeltT;
#ifdef TEST
	char disp[200];
#endif

	if(port <= 0 || port > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("TransitPrepare(),Input port Error!");
#endif
		return;
	}

	if(CrossData[port - 1].SynFlag == 0)
	{
	//we must run here so far!

		if(CrossData[port - 1].Status == TRANSIT_STATUS)
			CrossData[port - 1].Status = NORMAL_STATUS;

		if(CrossData[port - 1].NeedStep != 0)
		{
#ifdef TEST	
			memset(disp,0,200);
			sprintf(disp,"TransitPrepare(),Port = %d,CurStep = 0,NeedStep = %d",port,			\
												CrossData[port - 1].NeedStep);
			DebugWindow(disp);
#endif
			CrossData[port - 1].InitalWait = 0;
			CrossData[port - 1].NeedStep = 0;
			CrossData[port - 1].Status = NORMAL_STATUS;
		}

		CrossData[port - 1].CurStep = 0;
		//	CrossData[port - 1].CurStepCount = 0;
		CrossData[port - 1].StepErrCount = 0;

	}
	else if(CrossData[port - 1].SynFlag == 1)
	{
		if(PeriodEqual(port))
		{
			if(CrossData[port - 1].BaseCross == 0 || CrossData[port - 1].BaseCross == port)
			{
				CrossData[port - 1].Status = NORMAL_STATUS;
				CrossData[port - 1].SynFlag = 0;
				CrossData[port - 1].BaseCross = 0;//????
				CrossData[port - 1].DeltaPhi = 0;//????
			}
			else
			{
				BaseCross = CrossData[port - 1].BaseCross;
				if(CrossData[BaseCross - 1].Status == NORMAL_STATUS)
				{
					DeltT = CrossData[BaseCross - 1].CycleTimeOffset;
					if(DeltT == CrossData[port - 1].DeltaPhi)
					{
						CrossData[port - 1].Status = NORMAL_STATUS;
						CrossData[port - 1].SynFlag = 0;
						CrossData[port - 1].BaseCross = 0;//????
						CrossData[port - 1].DeltaPhi = 0;//????
					}
					else
					{
						SynPhase(port,DeltT);
					}
				}
			}
		}//if(PeriodEqual(port))
		else
		{
			SynPeriod(port);
		}
	}//if(CrossData[port - 1].SynFlag == 1)
	else if(CrossData[port - 1].SynFlag == 2)
	{
		BaseCross = CrossData[port - 1].BaseCross;
		if(BaseCross <= 0)
		{
			CrossData[port - 1].Status = NORMAL_STATUS;
			CrossData[port - 1].SynFlag = 0;
			CrossData[port - 1].BaseCross = 0;//????
			CrossData[port - 1].DeltaPhi = 0;//????
			DebugWindow("TransitPrepare(),Fatal Error,Internal data error!");
		}

		if(CrossData[BaseCross - 1].Status == NORMAL_STATUS)
		{
			DeltT = CrossData[BaseCross - 1].CycleTimeOffset;
			if(DeltT == CrossData[port - 1].DeltaPhi)
			{
				CrossData[port - 1].Status = NORMAL_STATUS;
				CrossData[port - 1].SynFlag = 0;
				CrossData[port - 1].BaseCross = 0;//????
				CrossData[port - 1].DeltaPhi = 0;//????
			}
			else
			{
				SynPhase(port,DeltT);
			}
		}
	}

	return;
}

/***************************************************************************
*	Function Name	: void SynPhase(int port,UINT16 DeltT)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,11,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
void SynPhase(int port,UINT16 DeltT)
{
	//this version is simple,we may revise later according to conditions 
	int i;
	UINT8 TotalStepNo;
	UINT16 DeltPhi;
	UINT16 tempphase;
	UINT8 tempflag;
	BOOL AddFlag[MAX_STEP_NUM];
	int AddNum = 0;
	UINT16 PhiPerStep; 
	UINT16 LeftPhi;
	BOOL First;

	if(port <= 0 || port > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("SynPhase(),Input port Error!");
#endif
		return;
	}

	TotalStepNo = CrossData[port - 1].TotalStepNo;
	DeltPhi = CrossData[port - 1].DeltaPhi;

	if(DeltT >= DeltPhi)
	{
		tempphase = DeltT - DeltPhi;
		tempflag = 0;
	}
	else
	{
		tempphase = DeltPhi - DeltT;
		tempflag = 1;
	}

	for(i = 0;i < TotalStepNo;i++)
	{
		CrossData[port - 1].TransitStepTable[i] = CrossData[port - 1].StepTable[i];
	}

	for(i = 0;i < TotalStepNo;i++)
	{
		if(CrossData[port - 1].TransitStepTable[i] < 10)//flash can not add
			AddFlag[i] = FALSE;
		else
		{
			AddFlag[i] = TRUE;
			AddNum ++; 
		}
	}

	if(AddNum == 0)
	{
		CrossData[port - 1].Status = NORMAL_STATUS;
		CrossData[port - 1].SynFlag = 0;
		CrossData[port - 1].BaseCross = 0;//????
		CrossData[port - 1].DeltaPhi = 0;//????
		DebugWindow("SynPhase(),Fatal Error,Internal data error!");
		return;
	}

	PhiPerStep = tempphase / AddNum;
	LeftPhi = tempphase - PhiPerStep * AddNum;

	First = TRUE;
	for(i = 0;i < TotalStepNo;i++)
	{
		if(AddFlag[i])
		{
			if(First)
			{
				CrossData[port - 1].TransitStepTable[i] += (PhiPerStep + LeftPhi);
				First = FALSE;
			}
			else
				CrossData[port - 1].TransitStepTable[i] += PhiPerStep;
		}
	}		
	
	return;
}

/***************************************************************************
*	Function Name	: void SynPeriod(int port)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,11,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
void SynPeriod(int port)
{
	//this version is simple,we may revise later according to conditions 
	int i;
	UINT8 TotalStepNo;

	if(port <= 0 || port > MAX_CROSS_NUM)
	{
		DebugWindow("SynPeriod(),Input port Error!");
		return;
	}

	TotalStepNo = CrossData[port - 1].TotalStepNo;
	if(TotalStepNo < 4 || TotalStepNo > MAX_STEP_NUM)
	{
#ifdef TEST
		DebugWindow("SynPeriod(),TotalStepNo Error!");
#endif
		CrossData[port - 1].Status = IDLE_STATUS;
		CrossData[port - 1].InitalWait = 3;
		CrossData[port - 1].CurStep = MAX_STEP_NUM + 1;
		CrossData[port - 1].NeedStep = MAX_STEP_NUM + 2;
		CrossData[port - 1].CurCtrMode = 0xff;//D当前的控制模式
		CrossData[port - 1].CurCValue = 0xff;//C当前的C值
		CrossData[port - 1].Left5Min = 5 * 60;

		CrossData[port - 1].CycleTimeOffset = 0;
		CrossData[port - 1].CycleAulCount = 0;

		return;
	}

	for(i = 0;i < TotalStepNo;i++)
		CrossData[port - 1].TransitStepTable[i] = CrossData[port - 1].StepTable[i];

	return;
}

/***************************************************************************
*	Function Name	: BOOL PeriodEqual(int port)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,11,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
BOOL PeriodEqual(int port)
{
	int i;
	UINT8 TotalStepNo;

	if(port <= 0 || port > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("PeriodEqual(),Input port Error!");
#endif
		return FALSE;
	}

	TotalStepNo = CrossData[port - 1].TotalStepNo;
	if(TotalStepNo < 4 || TotalStepNo > MAX_STEP_NUM)
	{
#ifdef TEST
		DebugWindow("PeriodEqual(),TotalStepNo Error!");
#endif
		CrossData[port - 1].Status = IDLE_STATUS;
		CrossData[port - 1].InitalWait = 3;
		CrossData[port - 1].CurStep = MAX_STEP_NUM + 1;
		CrossData[port - 1].NeedStep = MAX_STEP_NUM + 2;
		CrossData[port - 1].CurCtrMode = 0xff;//D当前的控制模式
		CrossData[port - 1].CurCValue = 0xff;//C当前的C值
		CrossData[port - 1].Left5Min = 5 * 60;

		CrossData[port - 1].CycleTimeOffset = 0;
		CrossData[port - 1].CycleAulCount = 0;

		return FALSE;
	}

	for(i = 0;i < TotalStepNo;i++)
	{
		if(CrossData[port - 1].StepTable[i] != CrossData[port - 1].TransitStepTable[i])
			return FALSE;
	}

	return TRUE;
}

/***************************************************************************
*	Function Name	: void DetectorProc(int port,char vdetctor)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,11,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
void DetectorProc(int port,char vdetctor)
{
	char Mask = (char)0x01;
	char detector;
	int i;

	if(port <= 0 || port > MAX_CROSS_NUM)
	{
		DebugWindow("DetectorProc(),Input port Error!");
		return;
	}

	detector = vdetctor;

	for(i = 0; i < 8 /*MAX_JCQ_NUM*/;i++)
	{

		IncRedlampWaitTime(port,(UINT8)i);

		if(CrossData[port - 1].LastReceBit[i])//last time receive bit is high
		{
			if(detector & Mask)//this time high again
			{
				CrossData[port - 1].HiCount[i]++;
			}
			else//this time low
			{
				//high to low
				SendVehiclePassby(port,(UINT8)i);

				if(CrossData[port - 1].HiCount[i] >= 3)
				{
					//a ve passed by
					CrossData[port - 1].CycleVeCount[i]++;
					CrossData[port - 1].CycleHiBits[i] += CrossData[port - 1].HiCount[i];
	
					IncRedlampVehicleCount(port,(UINT8)i);

					CrossData[port - 1].LowCount[i] = 1;
				}
				else
				{
					//omitted,AS LOW 07,18,2001
					CrossData[port - 1].LowCount[i] = CrossData[port - 1].HiCount[i] + 1;
					CrossData[port - 1].HiCount[i] = 0;	
				}

				CrossData[port - 1].LastReceBit[i] = 0;
			}
		}
		else//last time receive bit is low
		{
			if(detector & Mask)//this time high
			{
				//low to high
				SendVehiclePassing(port,(UINT8)i);

				if(CrossData[port - 1].LowCount[i] == 1)
				{
					if(CrossData[port - 1].HiCount[i] > 2 && CrossData[port - 1].CycleVeCount[i] > 0)
					{//if < 0,what to do
						CrossData[port - 1].CycleVeCount[i]--;
						CrossData[port - 1].CycleHiBits[i] -= CrossData[port - 1].HiCount[i];
					}

					DecRedlampVehicleCount(port,(UINT8)i);

					CrossData[port - 1].HiCount[i] += 2;
					CrossData[port - 1].LowCount[i] = 0;
				}
				else
				{
					CrossData[port - 1].HiCount[i] = 1;
				}

				CrossData[port - 1].LastReceBit[i] = 1;
			}
			else//this time low
			{
				CrossData[port - 1].LowCount[i]++;
			}
		}

		detector = detector >> 1;//sign
	}

	return;
}

/***************************************************************************
*	Function Name	: void DecRedlampVehicleCount(int Port,UINT8 jcqwlbh)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,20,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
void DecRedlampVehicleCount(int port,UINT8 jcqwlbh)
{
	int i;

	if(port <= 0 || port > MAX_CROSS_NUM)
	{
		DebugWindow("DecRedlampVehicleCount(),port Error!");
		return;
	}

	if(jcqwlbh >= MAX_JCQ_NUM)
	{
		DebugWindow("DecRedlampVehicleCount(),jcqwlbh Error!");
		return;
	}

	for(i = 0;i < 4;i++)
	{
		if(CrossData[port - 1].jcqStartGreenStep[i][jcqwlbh] == 0 \
			|| CrossData[port - 1].jcqStartGreenStep[i][jcqwlbh] > CrossData[port - 1].TotalStepNo \
			|| CrossData[port - 1].jcqEndGreenStep[i][jcqwlbh] == 0 \
			|| CrossData[port - 1].jcqEndGreenStep[i][jcqwlbh] > CrossData[port - 1].TotalStepNo \
			|| CrossData[port - 1].jcqStartGreenStep[i][jcqwlbh] > CrossData[port - 1].jcqEndGreenStep[i][jcqwlbh] )
		{
//			DebugWindow("DecRedlampVehicleCount(),Maybe No GreenStep!");
			continue;
			//return;
		}

		if(CrossData[port - 1].CurStep < CrossData[port - 1].jcqStartGreenStep[i][jcqwlbh] - 1 \
			|| CrossData[port - 1].CurStep > CrossData[port - 1].jcqEndGreenStep[i][jcqwlbh] - 1)
		{
		//in red lamp status
			if(CrossData[port - 1].CycleRedVeCount[jcqwlbh] > 0)
			{
				CrossData[port - 1].CycleRedVeCount[jcqwlbh]--;
				CrossData[port - 1].CycleRedVeWait[jcqwlbh]--;//only increase this 50ms interval,decrease it
			//added 07,31,2001
	//			CrossData[port - 1].CycleRedVeWait[jcqwlbh] -= CrossData[port - 1].HiCount[jcqwlbh];
			//end
			}

	//added 07,31,2001
			if(CrossData[port - 1].AsyCycRedVeCount[jcqwlbh] > 0)
				CrossData[port - 1].AsyCycRedVeCount[jcqwlbh]--;
	//end

		}
	}

	return;
}

/***************************************************************************
*	Function Name	: void IncRedlampVehicleCount(int Port,UINT8 jcqwlbh)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,20,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
void IncRedlampVehicleCount(int port,UINT8 jcqwlbh)
{
	int i;
/*
#ifdef TEST
	char disp[200];
#endif
*/
	if(port <= 0 || port > MAX_CROSS_NUM)
	{
		DebugWindow("IncRedlampVehicleCount(),port Error!");
		return;
	}

	if(jcqwlbh >= MAX_JCQ_NUM)
	{
		DebugWindow("IncRedlampVehicleCount(),jcqwlbh Error!");
		return;
	}

	for(i = 0;i < 4;i++)
	{
		if(CrossData[port - 1].jcqStartGreenStep[i][jcqwlbh] == 0 \
			|| CrossData[port - 1].jcqStartGreenStep[i][jcqwlbh] > CrossData[port - 1].TotalStepNo \
			|| CrossData[port - 1].jcqEndGreenStep[i][jcqwlbh] == 0 \
			|| CrossData[port - 1].jcqEndGreenStep[i][jcqwlbh] > CrossData[port - 1].TotalStepNo \
			|| CrossData[port - 1].jcqStartGreenStep[i][jcqwlbh] > CrossData[port - 1].jcqEndGreenStep[i][jcqwlbh] )
		{
	//		DebugWindow("IncRedlampVehicleCount(),Warning Maybe No GreenStep!");
			continue;
			//return;
		}

		if(CrossData[port - 1].CurStep < CrossData[port - 1].jcqStartGreenStep[i][jcqwlbh] - 1 \
			|| CrossData[port - 1].CurStep > CrossData[port - 1].jcqEndGreenStep[i][jcqwlbh] - 1)
		{
	/*
	#ifdef TEST
			if(jcqwlbh == 0)
			{
			memset(disp,0,200);
			sprintf(disp,"Total=%d,Red=%d,Green1=%d,Green2=%d,CurStep=%d",
				CrossData[port - 1].CycleVeCount[jcqwlbh], \
				CrossData[port - 1].CycleRedVeCount[jcqwlbh],\
				CrossData[port - 1].jcqStartGreenStep[jcqwlbh] - 1,\
				CrossData[port - 1].jcqEndGreenStep[jcqwlbh] - 1 ,\
				CrossData[port - 1].CurStep);
			DebugWindow(disp);
			}
	#endif
	*/		//in red lamp status
			CrossData[port - 1].CycleRedVeCount[jcqwlbh]++;

			//added 07,31,2001
			CrossData[port - 1].AsyCycRedVeCount[jcqwlbh]++;
			//end
			
			//added 07,31,2001
	//		CrossData[port - 1].CycleRedVeWait[jcqwlbh] += CrossData[port - 1].HiCount[jcqwlbh];
		}
	}

	return;
}

/***************************************************************************
*	Function Name	: void IncRedlampWaitTime(int Port,UINT8 jcqwlbh)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,20,2001
*	Global			: None
*	Note			: None
*	Modify			: yincy/07,31,2001
****************************************************************************/	
void IncRedlampWaitTime(int port,UINT8 jcqwlbh)
{
	int i;
/*
#ifdef TEST
	char disp[200];
#endif
*/
	if(port <= 0 || port > MAX_CROSS_NUM)
	{
		DebugWindow("IncRedlampWaitTime(),port Error!");
		return;
	}

	if(jcqwlbh >= MAX_JCQ_NUM)
	{
		DebugWindow("IncRedlampWaitTime(),jcqwlbh Error!");
		return;
	}

	for(i = 0;i < 4;i++)
	{
		if(CrossData[port - 1].jcqStartGreenStep[i][jcqwlbh] == 0 \
			|| CrossData[port - 1].jcqStartGreenStep[i][jcqwlbh] > CrossData[port - 1].TotalStepNo \
			|| CrossData[port - 1].jcqEndGreenStep[i][jcqwlbh] == 0 \
			|| CrossData[port - 1].jcqEndGreenStep[i][jcqwlbh] > CrossData[port - 1].TotalStepNo \
			|| CrossData[port - 1].jcqStartGreenStep[i][jcqwlbh] > CrossData[port - 1].jcqEndGreenStep[i][jcqwlbh] )
		{
	//		DebugWindow("IncRedlampWaitTime(),Warning Maybe No GreenStep!");
			continue;
			//return;
		}

		if(CrossData[port - 1].CurStep < CrossData[port - 1].jcqStartGreenStep[i][jcqwlbh] - 1 \
			|| CrossData[port - 1].CurStep > CrossData[port - 1].jcqEndGreenStep[i][jcqwlbh] - 1)
		{
			if(CrossData[port - 1].AsyCycRedVeCount[jcqwlbh] > 0)
			{
	/*
	#ifdef TEST
				if(jcqwlbh == 0 || jcqwlbh == 2)
				{
					memset(disp,0,200);
					sprintf(disp,"wlbh=%d,RedVehcle=%d,RedWait=%d",jcqwlbh,CrossData[port - 1].AsyCycRedVeCount[jcqwlbh],\
					CrossData[port - 1].CycleRedVeWait[jcqwlbh]);
					DebugWindow(disp);
				}
	#endif
	*/
				CrossData[port - 1].CycleRedVeWait[jcqwlbh] += CrossData[port - 1].AsyCycRedVeCount[jcqwlbh];
			}
		}
		//every 50ms increase by AsyCycRedVeCount[jcqwlbh] times

	}

	return;
}


/***************************************************************************
*	Function Name	: void SendVehiclePassby(int Port,UINT8 jcqwlbh)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,20,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
void SendVehiclePassby(int port,UINT8 jcqwlbh)
{
	CMsgFrame RealtimeData;
//	RealTimeData realtimedata;
	UINT8 jcqbh;

	if(port <= 0 || port > MAX_CROSS_NUM)
	{
		DebugWindow("SendVehiclePassby(),port Error!");
		return;
	}

	if(jcqwlbh >= MAX_JCQ_NUM)
	{
		DebugWindow("SendVehiclePassby(),jcqwlbh Error!");
		return;
	}

	if(CrossData[port - 1].NeedRealData == 0)
		return;

//	memset(&realtimedata,0,sizeof(RealTimeData));
	jcqbh = (CrossData[port - 1].jcqbh[jcqwlbh]);
/*	realtimedata.DetectorBh = jcqbh;
	realtimedata.Status = 0;
*/
	memset(&RealtimeData,0,sizeof(CMsgFrame));
	RealtimeData.byFlag = MsgFlag;
	RealtimeData.MsgType = CM_VEHICLESTATUS;

	strcpy(RealtimeData.SourceIP,HostIp);
	strcpy(RealtimeData.SourceID,HostId);
	strcpy(RealtimeData.TargetIP,SvrIp);
	strcpy(RealtimeData.TargetID,SvrId);

	RealtimeData.iCrossNo = CrossData[port - 1].lkbh;
//modify 08,02,2001
/*
	jcqbh = (CrossData[port - 1].jcqbh[jcqwlbh]);
	RealtimeData.iLength = 0;
	RealtimeData.iLength = jcqbh << 8;//pass by
*/
//	RealtimeData.iLength = *((WORD *)&realtimedata);
	RealtimeData.iLength = 0;
	RealtimeData.iReserved1 = jcqbh;
	RealtimeData.iReserved2 = 0;
	RealtimeData.byCheckSum = FormCheckSum((char *)&RealtimeData);

	SendDataEx((char *)&RealtimeData,sizeof(CMsgFrame),(UINT8)1);
	
	return;
}

/***************************************************************************
*	Function Name	: void SendVehiclePassing(int port,UINT8 jcqwlbh)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,20,2001
*	Modify			: yincy/09,21,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
void SendVehiclePassing(int port,UINT8 jcqwlbh)
{
//	RealTimeData realtimedata;
	CMsgFrame RealtimeData;
	UINT8 jcqbh;

	if(port <= 0 || port > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("SendVehiclePassing(),port Error!");
#endif
		return;
	}

	if(jcqwlbh >= MAX_JCQ_NUM)
	{
#ifdef TEST
		DebugWindow("SendVehiclePassing(),jcqwlbh Error!");
#endif
		return;
	}

	if(CrossData[port - 1].NeedRealData == 0)
		return;

//	memset(&realtimedata,0,sizeof(RealTimeData));
	jcqbh = (CrossData[port - 1].jcqbh[jcqwlbh]);
/*	realtimedata.DetectorBh = jcqbh;
	realtimedata.Status = 1;
*/
	memset(&RealtimeData,0,sizeof(CMsgFrame));
	RealtimeData.byFlag = MsgFlag;
	RealtimeData.MsgType = CM_VEHICLESTATUS;

	strcpy(RealtimeData.SourceIP,HostIp);
	strcpy(RealtimeData.SourceID,HostId);
	strcpy(RealtimeData.TargetIP,SvrIp);
	strcpy(RealtimeData.TargetID,SvrId);

	RealtimeData.iCrossNo = CrossData[port - 1].lkbh;
//modify 08,02,2001
/*	jcqbh = (CrossData[port - 1].jcqbh[jcqwlbh]);
	RealtimeData.iLength = 0;
	RealtimeData.iLength = jcqbh << 8 + 1;//have vehicle
*/
//	RealtimeData.iLength = *((WORD *)&realtimedata);
	RealtimeData.iLength = 0;
	RealtimeData.iReserved1 = jcqbh;
	RealtimeData.iReserved2 = 1;

	RealtimeData.byCheckSum = FormCheckSum((char *)&RealtimeData);

	SendDataEx((char *)&RealtimeData,sizeof(CMsgFrame),(UINT8)1);
	
	return;
}

/***************************************************************************
*	Function Name	: BOOL Send2AllCross(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,11,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
BOOL Send2AllCross(void)
{
	int i;

	if(MidDayCount > 0)
		MidDayCount--;

	if(!UseLcu)
	{
		for(i = 0;i < SerialCommNum; i++)
		{
			//added 08,06,2001
			if(CrossData[i].Type != 2)//jinsan
				continue;

			if(CrossData[i].Status == IDLE_STATUS)
				continue;

			if(CrossData[i].LinkFlag == FALSE)//3,6,02
				continue;

			SendCtr2Cross(i + 1,NULL);
			SendParam2Sys(i + 1);
		}
	}
	else
		Send2Lcu();			

	if(MidDayCount == 0 || MidDayCount == 5 * 3600 || MidDayCount == 3600 \
								|| MidDayCount == 1200 || MidDayCount == 500)
	{
		SYSTEMTIME time;

		GetLocalTime(&time);

		MidDayCount = time.wHour * 3600 + time.wMinute * 60 + time.wSecond;
/*		if(MidDayCount <= 12 * 3600)
			MidDayCount = 12 * 3600 - MidDayCount + 24 * 3600;
		else
			MidDayCount = 36 * 3600 - MidDayCount;//24 * 360 - (MidDayCount - 12 * 360);
*/
		MidDayCount = 36 * 3600 - MidDayCount;
	}

	return TRUE;
}

/***************************************************************************
*	Function Name	: BOOL Send2AllCrossEx(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,11,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
/*
BOOL Send2AllCrossEx(void)
{
	char info[3] = {0,0,0};
	SerialBits *pSerialBit; 

	pSerialBit = (SerialBits *)(&info[0]);
	pSerialBit->BitWise.CharType = A_CHAR;
	pSerialBit->BitWise.D1 = 1;
	pSerialBit->BitWise.D2 = 0;
	pSerialBit->BitWise.D3 = 0;
	pSerialBit->BitWise.D4 = 1;
	pSerialBit = (SerialBits *)(&info[1]);
	pSerialBit->BitWise.CharType = B_CHAR;
	pSerialBit->BitWise.D1 = 1;
	pSerialBit->BitWise.D2 = 1;
	pSerialBit->BitWise.D3 = 0;
	pSerialBit->BitWise.D4 = 1;
	pSerialBit = (SerialBits *)(&info[2]);
	pSerialBit->BitWise.CharType = C_CHAR;
	pSerialBit->BitWise.D1 = 0;
	pSerialBit->BitWise.D2 = 1;
	pSerialBit->BitWise.D3 = 1;
	pSerialBit->BitWise.D4 = 0;

	SerialPost(1,info,3);

	return TRUE;
}
*/

/***************************************************************************
*	Function Name	: void Send2Lcu(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,11,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
void Send2Lcu(void)
{
	char CtrCode[9];
	UINT8 MsgData[MAX_LPT_LEN];
	int MsgLen;
	int i;
	int CrossCount;
	UINT8 CardNo = 0;

	CrossCount = 0;
	memset(MsgData,0,MAX_LPT_LEN);
	for(i = 0;i < MAX_CROSS_NUM; i++)
	{
		//added 08,06,2001
		if(CrossData[i].Type != 2)//=2jinsan
			continue;

		if(CrossData[i].Status == IDLE_STATUS)
			continue;

		memset(CtrCode,0,9);
		SendCtr2Cross(i + 1,CtrCode);
		if(CtrCode[1] == 0)//CtrCode not set
			continue;

		if(CardNo == 0)
			CardNo = CrossData[i].CardNo;

		CrossCount++;
		if(CrossCount * 5 + 6 >= MAX_LPT_LEN)
		{
			DebugWindow("Send2Lcu(),Can not send to Cross!");
			return;
		}

		MsgData[(CrossCount - 1) * 5 + 4] = (UINT8)(i + 1);
		MsgData[(CrossCount - 1) * 5 + 4 + 1] = CtrCode[0];
		MsgData[(CrossCount - 1) * 5 + 4 + 2] = CtrCode[2];
		MsgData[(CrossCount - 1) * 5 + 4 + 3] = CtrCode[4];
		MsgData[(CrossCount - 1) * 5 + 4 + 4] = CtrCode[6];

		SendParam2Sys(i + 1);
	}

	if(CrossCount <= 0)
		return;

	MsgLen = CrossCount * 5 + 6;

	MsgData[0] = 0xaa;
	MsgData[1] = 0xbb;
	MsgData[2] = MsgLen & 0x00ff;
	MsgData[3] = ((MsgLen & 0xff00) >> 8) & 0x00ff;

	MsgData[CrossCount * 5 + 4] = 0x55;
	MsgData[CrossCount * 5 + 4 + 1] = 0x66;

	if(MsgLen > 0 && MsgLen < MAX_LPT_LEN)
		Write2Lpt(MsgData,(UINT16)MsgLen,CardNo);
	else
	{
#ifdef TEST
		DebugWindow("Send2Lcu(),Fatal Error!");
#endif
	}

	return;
}


/***************************************************************************
*	Function Name	: void SendCtr2Cross(int port,char *InPtr)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,11,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
void SendCtr2Cross(int port,char *InPtr)
{
	char CtrCode[9] = {(char)0x08,(char)0,(char)0x0a,(char)0,(char)0x0c,(char)0xf3,\
						(char)0x0e,(char)0xf1,(char)0xff};
	SerialBits *pSerialBit; 

	if(port <= 0 || port > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("SendCtr2Cross(),Input port Error!");
#endif
		return;
	}
/*
	if(InPtr == NULL)
	{
#ifdef TEST
		DebugWindow("SendCtr2Cross(),Input InPtr Error!");
#endif
		return;
	}
*/
	if(CrossData[port - 1].ManCtrFlag == 1)
	{
		//when in Manul control mode,we only increase CycleAulCount
		//we handle it later
#ifdef TEST
		//bugWindow("SendCtr2Cross(),In Manul Control Mode-Hold!");
#endif
		CrossData[port - 1].CycleAulCount++;
		
		//added 04,17,2002
		ClearSynFlag(port);
		//add end

		return;
	}
	else if(CrossData[port - 1].ManCtrFlag == 2)
	{
		Rep_CrossCtrStatus CrossCtrStatus;
		//Manul Step on
#ifdef TEST
		//bugWindow("SendCtr2Cross(),In Manul Control Mode-Step On!");
#endif
		pSerialBit = (SerialBits *)(&CtrCode[0]); 
		pSerialBit->BitWise.D1 = 1;
		pSerialBit->BitWise.D2 = 1;
		CtrCode[1] = ~(CtrCode[0]);
		CtrCode[3] = ~(CtrCode[2]);

		CrossData[port - 1].CycleAulCount++;
		CrossData[port - 1].ManCtrFlag = 0;

		//send to wu
		memset(&CrossCtrStatus,0,sizeof(Rep_CrossCtrStatus));
		CrossCtrStatus.byFlag = MsgFlag;
		CrossCtrStatus.MsgType = CM_CONTROLSTATUS;

		strcpy(CrossCtrStatus.SourceIP,HostIp);
		strcpy(CrossCtrStatus.SourceID,HostId);
		strcpy(CrossCtrStatus.TargetIP,SvrIp);
		strcpy(CrossCtrStatus.TargetID,SvrId);

		CrossCtrStatus.iCrossNo = CrossData[port - 1].lkbh;
		CrossCtrStatus.iLength = 0;
		CrossCtrStatus.iReserved1 = CrossData[port - 1].ManCtrFlag;

		CrossCtrStatus.byCheckSum = FormCheckSum((char *)&CrossCtrStatus);

		SendDataEx((char *)&CrossCtrStatus,sizeof(Rep_CrossCtrStatus),(UINT8)1);

	}
	else
	{
	
		switch(CrossData[port - 1].Status)
		{
		case OFFLINE_STATUS:
			SendOffline(port,CtrCode);
			break;

		case FORCED_STATUS:
			ForcedProc(port,CtrCode);
			break;

		case NORMAL_STATUS:
			NormalProc(port,CtrCode);
			break;

		case TRANSIT_STATUS:
			TransitProc(port,CtrCode);
			break;

		case INITIAL_STATUS:
			InitialProc(port,CtrCode);
			break;

		case IDLE_STATUS:
			return;
			break;

		default:
			return;
			break;
		}
	}

	if(CtrCode[1] == 0)//CtrCode not set
		return;

	//if midday syn clock
	if(MidDayCount <= 0)
	{
#ifdef TEST
		DebugWindow("Send Time to Cross!");
#endif
		pSerialBit = (SerialBits *)(&CtrCode[2]); 
		pSerialBit->BitWise.D3 = 1;
		CtrCode[3] = ~(CtrCode[2]);
	}

	if(!UseLcu)
		SerialPost(port,CtrCode,9);
	else if(InPtr != NULL)
		memcpy(InPtr,CtrCode,9);
	else
	{
#ifdef TEST
		DebugWindow("SendCtr2Cross(),Fatal Error!");
#endif
	}
		
	return;
}

/***************************************************************************
*	Function Name	: void SendOffline(int port,char *CtrCodePtr)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,24,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
void SendOffline(int port,char *CtrCodePtr)
{

	if(port <= 0 || port > MAX_CROSS_NUM)
	{
		DebugWindow("SendOffline(),Input port Error!");
		return;
	}

	if(CtrCodePtr == NULL)
	{
		DebugWindow("SendOffline(),Input Ptr Error!");
		return;
	}

	if(CrossData[port - 1].InitalWait > 0)
		CrossData[port - 1].InitalWait--;

//	CrossData[port - 1].CycleTimeOffset++;//increase
	CrossData[port - 1].CycleAulCount++;//modify 03,20,2002

	if(CrossData[port - 1].InitalWait == 0)
		return;

	//send offline for 30 times
	*(CtrCodePtr + 1) = ~(*CtrCodePtr);
	*(CtrCodePtr + 3) = ~(*(CtrCodePtr + 2));

	return;
}

/***************************************************************************
*	Function Name	: void ForcedProc(int port,char *CtrCodePtr)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,21,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
void ForcedProc(int port,char *CtrCodePtr)
{
	UINT8 ForcedStep;
	SerialBits *pSerialBit;
#ifdef TEST
	char disp[200];
#endif

	if(port <= 0 || port > MAX_CROSS_NUM)
	{
		DebugWindow("ForcedProc(),Input port Error!");
		return;
	}
 
	ForcedStep = CrossData[port - 1].ForcedStep;

	//added 09,19,2001
	if(ForcedStep == 201)//yellow blink
	{
		//CrossData[port - 1].CycleTimeOffset++;//increase?????????????????????
		CrossData[port - 1].CycleAulCount++;//modify 03,20,2002

		//send yellow blink
		pSerialBit = (SerialBits *)CtrCodePtr; 
		pSerialBit->BitWise.D2 = 1;
		pSerialBit->BitWise.D3 = 1;
		*(CtrCodePtr + 1) = ~(*CtrCodePtr);

		pSerialBit = (SerialBits *)(CtrCodePtr + 2);
		pSerialBit->BitWise.D1 = 1;
		pSerialBit->BitWise.D2 = 1;
		*(CtrCodePtr + 3) = ~(*(CtrCodePtr + 2));

		return;
	}
	else if(ForcedStep == 211)//blackout
	{
		//CrossData[port - 1].CycleTimeOffset++;//increase??
		CrossData[port - 1].CycleAulCount++;//modify 03,20,2002

		//send blackout
		pSerialBit = (SerialBits *)CtrCodePtr;
		pSerialBit->BitWise.D2 = 1;
		pSerialBit->BitWise.D3 = 1;
		pSerialBit->BitWise.D4 = 1;
		*(CtrCodePtr + 1) = ~(*CtrCodePtr);

		*(CtrCodePtr + 3) = ~(*(CtrCodePtr + 2));

		return;
	}
	//added end

	if(ForcedStep == CrossData[port - 1].CurStep)
	{
#ifdef TEST
		memset(disp,0,200);
		sprintf(disp,"port=%3d SetStepOn=%3d",port,ForcedStep + 1);
		DebugWindow(disp);
#endif

		//CrossData[port - 1].CycleTimeOffset++;//increase??
		CrossData[port - 1].CycleAulCount++;//modify 03,20,2002
//		CrossData[port - 1].InitalWait = 0;//added 03,20,2002

		//hold on
		pSerialBit = (SerialBits *)CtrCodePtr; 
		pSerialBit->BitWise.D2 = 1;
		pSerialBit->BitWise.D3 = 1;
		*(CtrCodePtr + 1) = ~(*CtrCodePtr);
		*(CtrCodePtr + 3) = ~(*(CtrCodePtr + 2));
	}
	else
	{
		if(CrossData[port - 1].AulStatus == TRANSIT_STATUS)
		{
			ForcedAdjust(port);
			CrossData[port - 1].AulStatus = IDLE_STATUS;
		}
		else if(CrossData[port - 1].AulStatus == NORMAL_STATUS)
		{
			CrossData[port - 1].AulStatus = IDLE_STATUS;
		}

		CrossData[port - 1].InitalWait++;//added 03,20,2002

		NormalProc(port,CtrCodePtr);
	}

	return;
}

/***************************************************************************
*	Function Name	: void InitialProc(int port,char *CtrCodePtr)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,11,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
void InitialProc(int port,char *CtrCodePtr)
{
	int i;
#ifdef TEST
	char disp[200];
#endif

	if(port <= 0 || port > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("InitialProc(),Input port Error!");
#endif
		return;
	}

#ifdef TEST
	memset(disp,0,200);
	sprintf(disp,"Port = %d,InitialProc(),CurStep = %d,NeedStep = %d,InitalWait = %d!",port,\
					CrossData[port - 1].CurStep,CrossData[port - 1].NeedStep,CrossData[port - 1].InitalWait);
//	DebugWindow(disp);
#endif

	if(CrossData[port - 1].InitalWait > 0)
	{
		CrossData[port - 1].InitalWait--;
		return;
	}

	if(CrossData[port - 1].CurStep > MAX_STEP_NUM)
	{
		CrossData[port - 1].InitalWait = 3;
		return;
	}

	if(CrossData[port - 1].CurStep != CrossData[port - 1].NeedStep)
	{
		CrossData[port - 1].CurStep = MAX_STEP_NUM + 1;
		CrossData[port - 1].NeedStep = MAX_STEP_NUM + 2;

		CrossData[port - 1].CycleTimeOffset = 0;
		CrossData[port - 1].CycleAulCount = 0;

		CrossData[port - 1].InitalWait = 3;
		return;
	}

	//this is the first time send ctrcode to cross
	memset(CrossData[port - 1].TransitStepTable,0,MAX_STEP_NUM * sizeof(UINT16));
	for(i = 0;i < CrossData[port - 1].CurStep;i++)
		CrossData[port - 1].TransitStepTable[i] = 0;

	for(i = CrossData[port - 1].CurStep;i < CrossData[port - 1].TotalStepNo;i++)
		CrossData[port - 1].TransitStepTable [i] = CrossData[port - 1].StepTable[i];

	CrossData[port - 1].CycleTimeOffset = 0;
	//CrossData[port - 1].CycleAulCount = 0;

	CrossData[port - 1].Status = TRANSIT_STATUS;

	TransitProc(port,CtrCodePtr);

	return;
}

/***************************************************************************
*	Function Name	: void NormalProc(int port,char *CtrCodePtr)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,11,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
void NormalProc(int port,char *CtrCodePtr)
{
	SerialBits *pSerialBit; 
	int i;
	UINT16 TimeElaspe,temptime;
	int count;
#ifdef TEST
	char disp[200];
#endif

	if(port <= 0 || port > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("NormalProc(),Input port Error!");
#endif
		return;
	}

#ifdef TESTz		
	memset(disp,0,200);
	sprintf(disp,"Port=%d,NormalProc(),CurStep=%d,NeedStep=%d!",port,							\
						CrossData[port - 1].CurStep,CrossData[port - 1].NeedStep);
	//DebugWindow(disp);
#endif

	if(CrossData[port - 1].NeedStep == CrossData[port - 1].CurStep)
	{
		CrossData[port - 1].StepErrCount = 0;
		CrossData[port - 1].CycleTimeOffset++;

		TimeElaspe = 0;
		for(i = 0;i <= CrossData[port - 1].CurStep;i++)
			TimeElaspe += CrossData[port - 1].StepTable[i];

		if(CrossData[port - 1].CycleTimeOffset >= TimeElaspe)
		{
		//step on
			pSerialBit = (SerialBits *)CtrCodePtr; 
			pSerialBit->BitWise.D1 = 1;
			pSerialBit->BitWise.D2 = 1;
			*(CtrCodePtr + 1) = ~(*CtrCodePtr);
			*(CtrCodePtr + 3) = ~(*(CtrCodePtr + 2));
			
			if(CrossData[port - 1].TotalStepNo != 0)
				CrossData[port - 1].NeedStep = (CrossData[port - 1].CurStep + 1)									\
													% (CrossData[port - 1].TotalStepNo);
			else
			{
				CrossData[port - 1].Status = IDLE_STATUS;
				DebugWindow("NormalProc(),Fatal Error,TotalStepNo = 0!");
			}

			//DebugWindow("NormalProc(),Send Forward");
 		}
		else//hold
		{
			pSerialBit = (SerialBits *)CtrCodePtr; 
			pSerialBit->BitWise.D2 = 1;
			pSerialBit->BitWise.D3 = 1;
			*(CtrCodePtr + 1) = ~(*CtrCodePtr);
			*(CtrCodePtr + 3) = ~(*(CtrCodePtr + 2));

			//DebugWindow("NormalProc(),Send Hold");
		}
	}
	else//further consideration later
	{
		if(CurStepLessNeedStep(port))
		{
		//curStep < needstep
			CrossData[port - 1].StepErrCount++;
			CrossData[port - 1].CycleAulCount++;
 			if(CrossData[port - 1].StepErrCount > MAX_ERR_INCYCLE)
			{
			//too many error times,may cross comm error
#ifdef TEST
				memset(disp,0,200);
				sprintf(disp,"Port = %d,NormalProc(),Too Many Not_Matched_Step Times!",port);
				DebugWindow(disp);
#endif
				//added 04,11,2002
				if(CrossData[port - 1].RealTime5Min > 0)
					ReportFlowData(port);

				CrossData[port - 1].Status = INITIAL_STATUS;
				CrossData[port - 1].InitalWait = 3;
				CrossData[port - 1].CurStep = MAX_STEP_NUM + 1;
				CrossData[port - 1].NeedStep = MAX_STEP_NUM + 2;
				CrossData[port - 1].CurCtrMode = 0xff;//D当前的控制模式
				CrossData[port - 1].CurCValue = 0xff;//C当前的C值
				CrossData[port - 1].Left5Min = 5 * 60;

				CrossData[port - 1].CycleTimeOffset = 0;
				CrossData[port - 1].CycleAulCount = 0;

				return;
			}

			if(CrossData[port - 1].StepErrCount < MAX_ERRRESEND_NUM)
			{
			//wait cross step event for 2s
			//	DebugWindow("Wait Cross for 2s!");
				pSerialBit = (SerialBits *)CtrCodePtr; 
				pSerialBit->BitWise.D1 = 0;
				pSerialBit->BitWise.D2 = 1;
				pSerialBit->BitWise.D3 = 1;
				*(CtrCodePtr + 1) = ~(*CtrCodePtr);
				*(CtrCodePtr + 3) = ~(*(CtrCodePtr + 2));

				//DebugWindow("NormalProc(),Send Hold 2");
			}
			else
			{
			//step on
				pSerialBit = (SerialBits *)CtrCodePtr; 
				pSerialBit->BitWise.D1 = 1;
				pSerialBit->BitWise.D2 = 1;
				pSerialBit->BitWise.D3 = 0;
				*(CtrCodePtr + 1) = ~(*CtrCodePtr);
				*(CtrCodePtr + 3) = ~(*(CtrCodePtr + 2));

				//DebugWindow("NormalProc(),Send Forward 2");
			}
		}
		else
		{
		//curStep > needstep
			//1.maybe lost some upsend step value;
			//2.maybe cross receive step_on signal too later,leads center send more than
			//	one step_on signals.
			i = 0;
			count = 0;
			TimeElaspe = 0;
			while((i < CrossData[port - 1].CurStep) && (count < CrossData[port - 1].TotalStepNo)) 
			{
				TimeElaspe += CrossData[port - 1].StepTable[i];
				i++;
				count++;
			}

			if(count >= CrossData[port - 1].TotalStepNo)
			{
#ifdef TEST
				memset(disp,0,200);
				sprintf(disp,"CurStepLessNeedStep(),Port=%03d,Fatal Error!",port);
				DebugWindow(disp);
#endif

				CrossData[port - 1].Status = INITIAL_STATUS;
				CrossData[port - 1].InitalWait = 3;
				CrossData[port - 1].CurStep = MAX_STEP_NUM + 1;
				CrossData[port - 1].NeedStep = MAX_STEP_NUM + 2;
				CrossData[port - 1].CurCtrMode = 0xff;//D当前的控制模式
				CrossData[port - 1].CurCValue = 0xff;//C当前的C值
				CrossData[port - 1].Left5Min = 5 * 60;

				CrossData[port - 1].CycleTimeOffset = 0;
				CrossData[port - 1].CycleAulCount = 0;

				return;
			}

			CrossData[port - 1].CycleTimeOffset++;//07,23,2001
			if(TimeElaspe >= CrossData[port - 1].CycleTimeOffset)
			{
			//always like this
				temptime = CrossData[port - 1].CycleTimeOffset;
				CrossData[port - 1].CycleTimeOffset = TimeElaspe;
				CrossData[port - 1].CycleAulCount -= (TimeElaspe - temptime);

				CrossData[port - 1].NeedStep = CrossData[port - 1].CurStep;//note
#ifdef TEST
				memset(disp,0,200);
				sprintf(disp,"Port=%d,NormalProc(),Adjust Step!",port);
				DebugWindow(disp);
#endif
			//hold
				pSerialBit = (SerialBits *)CtrCodePtr;
				pSerialBit->BitWise.D1 = 0;
				pSerialBit->BitWise.D2 = 1;
				pSerialBit->BitWise.D3 = 1;
				*(CtrCodePtr + 1) = ~(*CtrCodePtr);
				*(CtrCodePtr + 3) = ~(*(CtrCodePtr + 2));

#ifdef TEST
				//DebugWindow("NormalProc(),Send Hold 3");
#endif
			}
			else
			{
#ifdef TEST
				memset(disp,0,200);
				sprintf(disp,"Port=%03d,NormalProc(),May Step Length Not Equal!",port);
				DebugWindow(disp);
#endif
				temptime = CrossData[port - 1].CycleTimeOffset;
				CrossData[port - 1].CycleTimeOffset = TimeElaspe;
				CrossData[port - 1].CycleAulCount += (temptime - TimeElaspe);

				CrossData[port - 1].NeedStep = CrossData[port - 1].CurStep;//note
#ifdef TEST
				memset(disp,0,200);
				sprintf(disp,"Port=%d,NormalProc(),Adjust Step!",port);
				DebugWindow(disp);
#endif
			//hold
				pSerialBit = (SerialBits *)CtrCodePtr; 
				pSerialBit->BitWise.D1 = 0;
				pSerialBit->BitWise.D2 = 1;
				pSerialBit->BitWise.D3 = 1;
				*(CtrCodePtr + 1) = ~(*CtrCodePtr);
				*(CtrCodePtr + 3) = ~(*(CtrCodePtr + 2));

				//DebugWindow("NormalProc(),Send Hold 4");
			}
		}//endif CurStepLessNeedStep(port)
	}//endif curstep != needstep

	return;
}

/***************************************************************************
*	Function Name	: void TransitProc(int port,char *CtrCodePtr)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,11,2001
*	Global			: None
*	Note			: None
*   Modify			: yincy/07,23,2001
****************************************************************************/	
void TransitProc(int port,char *CtrCodePtr)
{
	SerialBits *pSerialBit; 
	int i;
	UINT16 TimeElaspe,temptime;
#ifdef TEST
	char disp[200];
#endif

	if(port <= 0 || port > MAX_CROSS_NUM)
	{
		DebugWindow("TransitProc(),Input port Error!");
		return;
	}

#ifdef TEST
	memset(disp,0,200);
	sprintf(disp,"Port=%d,TransitProc(),CurStep=%d,NeedStep=%d!",port,													\
						CrossData[port - 1].CurStep,CrossData[port - 1].NeedStep);
	//DebugWindow(disp);
#endif

	if(CrossData[port - 1].AulStatus == OFFLINE_STATUS)//offline status to transit status
	{
		//adjust once
		NewTransitAdjust(port);
		CrossData[port - 1].AulStatus = IDLE_STATUS;
	}

	if(CrossData[port - 1].NeedStep == CrossData[port - 1].CurStep)
	{
		CrossData[port - 1].StepErrCount = 0;
		CrossData[port - 1].CycleTimeOffset++;

		TimeElaspe = 0;
		for(i = 0;i <= CrossData[port - 1].CurStep;i++)
			TimeElaspe += CrossData[port - 1].TransitStepTable[i];

		if(CrossData[port - 1].CycleTimeOffset >= TimeElaspe)
		{
		//step on
			pSerialBit = (SerialBits *)CtrCodePtr; 
			pSerialBit->BitWise.D1 = 1;
			pSerialBit->BitWise.D2 = 1;
			*(CtrCodePtr + 1) = ~(*CtrCodePtr);
			*(CtrCodePtr + 3) = ~(*(CtrCodePtr + 2));

			//DebugWindow("TransitProc(),Send Forward 1");
			
			if(CrossData[port - 1].TotalStepNo != 0)
				CrossData[port - 1].NeedStep = (CrossData[port - 1].CurStep + 1) \
														% (CrossData[port - 1].TotalStepNo);
			else
			{
				CrossData[port - 1].Status = INITIAL_STATUS;
				CrossData[port - 1].InitalWait = 3;
				CrossData[port - 1].CurStep = MAX_STEP_NUM + 1;
				CrossData[port - 1].NeedStep = MAX_STEP_NUM + 2;
				CrossData[port - 1].CurCtrMode = 0xff;//D当前的控制模式
				CrossData[port - 1].CurCValue = 0xff;//C当前的C值
				CrossData[port - 1].Left5Min = 5 * 60;

				CrossData[port - 1].CycleTimeOffset = 0;
				CrossData[port - 1].CycleAulCount = 0;

				DebugWindow("TransitProc(),Fatal Error,TotalStepNo = 0!");
			}
 		}
		else//hold
		{
			pSerialBit = (SerialBits *)CtrCodePtr; 
			pSerialBit->BitWise.D2 = 1;
			pSerialBit->BitWise.D3 = 1;
			*(CtrCodePtr + 1) = ~(*CtrCodePtr);
			*(CtrCodePtr + 3) = ~(*(CtrCodePtr + 2));

			//DebugWindow("TransitProc(),Send Hold 1");
		}
	}
	else//further consideration later
	{
		if(CurStepLessNeedStep(port))
		{
		//curStep < needstep
			CrossData[port - 1].StepErrCount++;
			CrossData[port - 1].CycleAulCount++;
 			if(CrossData[port - 1].StepErrCount > MAX_ERR_INCYCLE)
			{
				//too many error times,may cross comm error
#ifdef TEST
				memset(disp,0,200);
				sprintf(disp,"Port=%d,TransitProc(),Too Many Not_Matched_Step Times!",port);
				DebugWindow(disp);
#endif
				//added 04,11,2002
				if(CrossData[port - 1].RealTime5Min > 0)
					ReportFlowData(port);

				CrossData[port - 1].Status = INITIAL_STATUS;
				CrossData[port - 1].InitalWait = 3;
				CrossData[port - 1].CurStep = MAX_STEP_NUM + 1;
				CrossData[port - 1].NeedStep = MAX_STEP_NUM + 2;
				CrossData[port - 1].CurCtrMode = 0xff;//D当前的控制模式
				CrossData[port - 1].CurCValue = 0xff;//C当前的C值
				CrossData[port - 1].Left5Min = 5 * 60;

				CrossData[port - 1].CycleTimeOffset = 0;
				CrossData[port - 1].CycleAulCount = 0;

				return;
			}

			if(CrossData[port - 1].StepErrCount < MAX_ERRRESEND_NUM)
			{
			//wait cross step event for 2s
			//	DebugWindow("Wait Cross for 2s!");
				pSerialBit = (SerialBits *)CtrCodePtr; 
				pSerialBit->BitWise.D1 = 0;
				pSerialBit->BitWise.D2 = 1;
				pSerialBit->BitWise.D3 = 1;
				*(CtrCodePtr + 1) = ~(*CtrCodePtr);
				*(CtrCodePtr + 3) = ~(*(CtrCodePtr + 2));

				//DebugWindow("TransitProc(),Send Hold");
			}
			else
			{
			//step on
				pSerialBit = (SerialBits *)CtrCodePtr; 
				pSerialBit->BitWise.D1 = 1;
				pSerialBit->BitWise.D2 = 1;
				pSerialBit->BitWise.D3 = 0;
				*(CtrCodePtr + 1) = ~(*CtrCodePtr);
				*(CtrCodePtr + 3) = ~(*(CtrCodePtr + 2));

				//DebugWindow("TransitProc(),Send Forward");
			}
		}
		else
		{
		//curStep > needstep
			//1.maybe lost some upsend step value;
			//2.maybe cross receive step_on signal too later,leads center send more than
			//	one step_on signals.
			i = 0;
			TimeElaspe = 0;
			while(i < CrossData[port - 1].CurStep) 
			{
				TimeElaspe += CrossData[port - 1].TransitStepTable[i];
				i++;
			}

			//CrossData[port - 1].CycleAulCount++;//added 03,29,2002
			//CrossData[port - 1].CycleTimeOffset++;//deleted 03,29,2002
			if(TimeElaspe >= CrossData[port - 1].CycleTimeOffset)
			{
			//always like this
				temptime = CrossData[port - 1].CycleTimeOffset;
				CrossData[port - 1].CycleTimeOffset = TimeElaspe;
				CrossData[port - 1].CycleAulCount -= (TimeElaspe - temptime);

				CrossData[port - 1].NeedStep = CrossData[port - 1].CurStep;//note
#ifdef TEST
				memset(disp,0,200);
				sprintf(disp,"Port=%d,TransitProc(),Adjust Step!",port);
				DebugWindow(disp);
#endif
			//hold step
				pSerialBit = (SerialBits *)CtrCodePtr; 
				pSerialBit->BitWise.D1 = 0;
				pSerialBit->BitWise.D2 = 1;
				pSerialBit->BitWise.D3 = 1;
				*(CtrCodePtr + 1) = ~(*CtrCodePtr);
				*(CtrCodePtr + 3) = ~(*(CtrCodePtr + 2));

				DebugWindow("TransitProc(),Hold 2");
			}
			else
			{
#ifdef TEST
				memset(disp,0,200);
				sprintf(disp,"Port=%3d,TransitProc(),May Step Length not Equal!",port);
				DebugWindow(disp);
#endif
				temptime = CrossData[port - 1].CycleTimeOffset;
				CrossData[port - 1].CycleTimeOffset = TimeElaspe;
				CrossData[port - 1].CycleAulCount += (temptime - TimeElaspe);

				CrossData[port - 1].NeedStep = CrossData[port - 1].CurStep;//note
#ifdef TEST
				memset(disp,0,200);
				sprintf(disp,"Port=%d,TransitProc(),Adjust Step!",port);
				DebugWindow(disp);
#endif
			//hold step
				pSerialBit = (SerialBits *)CtrCodePtr; 
				pSerialBit->BitWise.D1 = 0;
				pSerialBit->BitWise.D2 = 1;
				pSerialBit->BitWise.D3 = 1;
				*(CtrCodePtr + 1) = ~(*CtrCodePtr);
				*(CtrCodePtr + 3) = ~(*(CtrCodePtr + 2));

				DebugWindow("TransitProc(),Hold 3");
			}
		}//endif CurStepLessNeedStep
	}//endif curstep != needstep

	return;
}

/***************************************************************************
*	Function Name	: void NewTransitAdjust(int port)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,24,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
void NewTransitAdjust(int port)
{
	int i = 0;
	int TimeElaspe = 0;
	int temptime;
	int count;
#ifdef TEST
	char disp[200];
#endif

	if(port <= 0 || port > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("NewTransitAdjust(), Fatal Error Input port Error!");
#endif
		return;
	}

//	memset(CrossData[port - 1].TransitStepTable,0,MAX_STEP_NUM * sizeof(UINT16));
	memcpy(CrossData[port - 1].TransitStepTable,CrossData[port - 1].StepTable,MAX_STEP_NUM * sizeof(UINT16));

	count = 0;
	while(i < CrossData[port - 1].CurStep && count < CrossData[port - 1].TotalStepNo) 
	{
		TimeElaspe += CrossData[port - 1].TransitStepTable[i];
		i++;
		count++;
	}

	if(count >= CrossData[port - 1].TotalStepNo)
	{
	//Step Num in comm computer is not match that in cross 
	//send error to cp
		DebugWindow("Maybe Step Num in Cross is not match in Comm Computer!");

		CrossData[port - 1].Status = INITIAL_STATUS;
		CrossData[port - 1].InitalWait = 3;
		CrossData[port - 1].CurStep = MAX_STEP_NUM + 1;
		CrossData[port - 1].NeedStep = MAX_STEP_NUM + 2;
		CrossData[port - 1].CurCtrMode = 0xff;//D当前的控制模式
		CrossData[port - 1].CurCValue = 0xff;//C当前的C值
		CrossData[port - 1].Left5Min = 5 * 60;

		CrossData[port - 1].CycleTimeOffset = 0;
		CrossData[port - 1].CycleAulCount = 0;

		return;
	}

	//added 08,02,2001
	CrossData[port - 1].CycleTimeOffset++;

	temptime = CrossData[port - 1].CycleTimeOffset;
	CrossData[port - 1].CycleTimeOffset = TimeElaspe;
	CrossData[port - 1].NeedStep = CrossData[port - 1].CurStep;
	CrossData[port - 1].CycleAulCount += (temptime - TimeElaspe);

#ifdef TEST
	memset(disp,0,200);
	sprintf(disp,"Port=%d,NewTransitAdjust(),Adjust Step And Offset Here!",port);
	DebugWindow(disp);
#endif

	return;
}


/***************************************************************************
*	Function Name	: void SetStepoffAdjust(int port)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,24,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
void SetStepoffAdjust(int port)
{
	int i = 0;
	int TimeElaspe = 0;
	int temptime;
#ifdef TEST
	char disp[200];
#endif

	if(port <= 0 || port > MAX_CROSS_NUM)
	{
		DebugWindow("SetStepoffAdjust(), Fatal Error Input port Error!");
		return;
	}

//	memset(CrossData[port - 1].TransitStepTable,0,MAX_STEP_NUM * sizeof(UINT16));
	memcpy(CrossData[port - 1].TransitStepTable,CrossData[port - 1].StepTable,MAX_STEP_NUM * sizeof(UINT16));

	while(i <= CrossData[port - 1].CurStep) //only this line diffrent,let it step on
	{
		TimeElaspe += CrossData[port - 1].TransitStepTable[i];
		i++;
	}

//modified 03,20,2002
	temptime = CrossData[port - 1].CycleTimeOffset;
	if(temptime >= TimeElaspe)
	{
		CrossData[port - 1].CycleTimeOffset = TimeElaspe;
		CrossData[port - 1].NeedStep = CrossData[port - 1].CurStep;
		CrossData[port - 1].CycleAulCount += (temptime - TimeElaspe);
	}
	else
	{
		CrossData[port - 1].CycleTimeOffset = TimeElaspe;
		CrossData[port - 1].NeedStep = CrossData[port - 1].CurStep;
		CrossData[port - 1].CycleAulCount -= (TimeElaspe - temptime);
	}
//modified end

#ifdef TEST
	memset(disp,0,200);
	sprintf(disp,"Port=%d,SetStepoffAdjust(),Adjust Step And Offset Here!",port);
	DebugWindow(disp);
#endif

	return;
}

/***************************************************************************
*	Function Name	: void ForcedAdjust(int port)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,24,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
void ForcedAdjust(int port)
{
	int i = 0;
	int TimeElaspe = 0;
	int temptime;
#ifdef TEST
	char disp[200];
#endif

	if(port <= 0 || port > MAX_CROSS_NUM)
	{
		DebugWindow("ForcedAdjust(), Fatal Error Input port Error!");
		return;
	}

	while(i < CrossData[port - 1].CurStep)
	{
		TimeElaspe += CrossData[port - 1].StepTable[i];
		i++;
	}

//following modified 03,20,2002
	temptime = CrossData[port - 1].CycleTimeOffset;
	if(temptime >= TimeElaspe)
	{
		CrossData[port - 1].CycleTimeOffset = TimeElaspe;
		CrossData[port - 1].NeedStep = CrossData[port - 1].CurStep;
		CrossData[port - 1].CycleAulCount += (temptime - TimeElaspe);
	}
	else
	{
		CrossData[port - 1].CycleTimeOffset = TimeElaspe;
		CrossData[port - 1].NeedStep = CrossData[port - 1].CurStep;
		CrossData[port - 1].CycleAulCount -= (TimeElaspe - temptime);
	}
//modified end

#ifdef TEST
	memset(disp,0,200);
	sprintf(disp,"Port=%d,ForcedAdjust(),Adjust Step And Offset Here!",port);
	DebugWindow(disp);
#endif

	return;
}

/***************************************************************************
*	Function Name	: BOOL CurStepLessNeedStep(int port);
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,23,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
BOOL CurStepLessNeedStep(int port)
{
	int disparity;
	int halfphase;
	UINT8 CurStep;
#ifdef TEST
	char disp[200];
#endif

	if(port <= 0 || port > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("CurStepLessNeedStep(), Fatal Error Input port Error!");
#endif
		return FALSE;
	}

	halfphase = CrossData[port - 1].TotalStepNo / 2;
	if(halfphase < 2)
	{
#ifdef TEST
		memset(disp,0,200);
		sprintf(disp,"CurStepLessNeedStep(),Port = %d Fatal Error,Step Num too little!",port);
		DebugWindow(disp);
#endif
		return FALSE;		
	}

	if(CrossData[port - 1].CurStep == CrossData[port - 1].NeedStep)
	{
#ifdef TEST
		memset(disp,0,200);
		sprintf(disp,"CurStepLessNeedStep(),Port = %d,CurStep == NeedStep",port);
//		DebugWindow(disp);
#endif
		return FALSE;
	}

#ifdef TEST
	memset(disp,0,200);
	sprintf(disp,"CurStepLessNeedStep(),Port=%d,CurStep=%d,NeedStep=%d",						\
					port,CrossData[port - 1].CurStep,CrossData[port - 1].NeedStep);
	DebugWindow(disp);
#endif

//	if(CrossData[port - 1].CurStep == 1 && CrossData[port - 1].NeedStep == 0)
//		CrossData[port - 1].NeedStep = 0;//deleted 03,14,2002

	CurStep = CrossData[port - 1].CurStep;
	disparity = 0;
	while((CurStep != CrossData[port - 1].NeedStep) \
									&& disparity < CrossData[port - 1].TotalStepNo)
	{
		disparity++;
		CurStep = (CurStep + 1) % (CrossData[port - 1].TotalStepNo);
	}

	if(disparity >= CrossData[port - 1].TotalStepNo)
	{
	//Step Num in comm computer is not match that in cross 
	//send error to cp
		DebugWindow("Maybe Step Num in Cross is not match in Comm Computer!");

		CrossData[port - 1].Status = INITIAL_STATUS;
		CrossData[port - 1].InitalWait = 3;
		CrossData[port - 1].CurStep = MAX_STEP_NUM + 1;
		CrossData[port - 1].NeedStep = MAX_STEP_NUM + 2;
		CrossData[port - 1].CurCtrMode = 0xff;//D当前的控制模式
		CrossData[port - 1].CurCValue = 0xff;//C当前的C值
		CrossData[port - 1].Left5Min = 5 * 60;

		CrossData[port - 1].TotalStepNo = 0;

		CrossData[port - 1].CycleTimeOffset = 0;
		CrossData[port - 1].CycleAulCount = 0;
	//further consideration later
		//if(
		//CrossData[port - 1]
		return FALSE;
	}

	if(disparity <= halfphase)
		return TRUE;
	else
		return FALSE;

	return TRUE;
}

/***************************************************************************
*	Function Name	: BOOL CurStepGreatTheStep(int port,UINT8 TheStep)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/03,20,2002
*	Global			: None
*	Note			: None
****************************************************************************/	
BOOL CurStepGreatTheStep(int port,UINT8 TheStep)
{
	int disparity;
	int halfphase;
	UINT8 CurStep;
#ifdef TEST
	char disp[200];
#endif

	if(port <= 0 || port > MAX_CROSS_NUM)
	{
		DebugWindow("CurStepGreatTheStep(), Fatal Error Input port Error!");
		return FALSE;
	}

	halfphase = CrossData[port - 1].TotalStepNo / 2;
	if(halfphase < 2)
	{
#ifdef TEST
		memset(disp,0,200);
		sprintf(disp,"CurStepGreatTheStep(),Port = %d Fatal Error,Step Num too little!",port);
		DebugWindow(disp);
#endif
		return FALSE;		
	}

	if(CrossData[port - 1].CurStep == TheStep)
	{
#ifdef TEST
		memset(disp,0,200);
		sprintf(disp,"CurStepGreatTheStep(),Port = %d,CurStep == NeedStep",port);
//		DebugWindow(disp);
#endif
		return FALSE;
	}

#ifdef TEST
	memset(disp,0,200);
	sprintf(disp,"CurStepGreatTheStep(),Port=%d,CurStep=%d,NeedStep=%d",						\
					port,CrossData[port - 1].CurStep + 1,CrossData[port - 1].NeedStep + 1);
	DebugWindow(disp);
#endif

	CurStep = CrossData[port - 1].CurStep;
	disparity = 0;
	while((CurStep != TheStep) && disparity < CrossData[port - 1].TotalStepNo)
	{
		disparity++;
		CurStep = (CurStep + 1) % (CrossData[port - 1].TotalStepNo);
	}

	if(disparity >= CrossData[port - 1].TotalStepNo)
	{
	//Step Num in comm computer is not match that in cross 
	//send error to cp
#ifdef TEST
		DebugWindow("Maybe Step Num in Cross is not match in Comm Computer!");
#endif

		CrossData[port - 1].Status = INITIAL_STATUS;
		CrossData[port - 1].InitalWait = 3;
		CrossData[port - 1].CurStep = MAX_STEP_NUM + 1;
		CrossData[port - 1].NeedStep = MAX_STEP_NUM + 2;
		CrossData[port - 1].CurCtrMode = 0xff;//D当前的控制模式
		CrossData[port - 1].CurCValue = 0xff;//C当前的C值
		CrossData[port - 1].Left5Min = 5 * 60;

		CrossData[port - 1].TotalStepNo = 0;

		CrossData[port - 1].CycleTimeOffset = 0;
		CrossData[port - 1].CycleAulCount = 0;
	//further consideration later
		//if(
		//CrossData[port - 1]
		return FALSE;
	}

	if(disparity < halfphase)
		return TRUE;
	else
		return FALSE;

	return TRUE;
}


/***************************************************************************
*	Function Name	: void SendParam2Sys(int port)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,11,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
void SendParam2Sys(int port)
{

	if(port <= 0 || port > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("SendParam2Sys(),Input port Error!");
#endif
		return;
	}

	if(CrossData[port - 1].Status == IDLE_STATUS)
		return;
	
	//CROSS not controling(maybe can not receive cross information),return
	if(CrossData[port - 1].Status == INITIAL_STATUS/*&& CrossData[port - 1].CurStep > MAX_STEP_NUM*/)
	{
		return;
	}

	if(CrossData[port - 1].Left5Min > 0)
		CrossData[port - 1].Left5Min--;

	if(CrossData[port - 1].Left5Min == 0)
	{
		if(CrossData[port - 1].RealTime5Min > 0)
			ReportFlowData(port);
/*		else if(CrossData[port - 1].Status == FORCED_STATUS)//added 09,18,2001,not tested
			ReportForcedFlowData(port);
*/
		CrossData[port - 1].Left5Min = 5 * 60;
	}

	return;
}

/*********************************************************************************
*	Function Name	: void ReportForcedFlowData(int port)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/09,18,2001
*	Global			: None
*	Note			: None
**********************************************************************************/	
void ReportForcedFlowData(int port)
{
	int i;
	Rep_FlowData FlowData5Min;
#ifdef TEST
	char disp[200];
#endif

	if(port <= 0 || port > MAX_CROSS_NUM)
	{
#ifdef TEST
		DebugWindow("ReportFlowData(),Input port Error!");
#endif
		return;
	}

	memset(&FlowData5Min,0,sizeof(Rep_FlowData));

	FlowData5Min.Header.byFlag = MsgFlag;
	FlowData5Min.Header.MsgType = CM_FLOWDATA;

	strcpy(FlowData5Min.Header.SourceIP,HostIp);
	strcpy(FlowData5Min.Header.SourceID,HostId);
	strcpy(FlowData5Min.Header.TargetIP,SvrIp);
	strcpy(FlowData5Min.Header.TargetID,SvrId);

	FlowData5Min.Header.iCrossNo = CrossData[port - 1].lkbh;
	FlowData5Min.Header.iLength = sizeof(FlowData);
	FlowData5Min.Header.byCheckSum = FormCheckSum((char *)&(FlowData5Min.Header));

	if(CrossData[port - 1].CycleTimeOffset + CrossData[port - 1].CycleAulCount < 5 * 60)
		return;//exit directly
	
	FlowData5Min.Data.iCycleTime = CrossData[port - 1].CycleTimeOffset + CrossData[port - 1].CycleAulCount;
	FlowData5Min.Data.iTotalTime = 5 * 60;
	FlowData5Min.Data.iCycleNo = (int)CrossData[port - 1].RedCycleCount5m;

#ifdef TEST
	memset(disp,0,200);
	sprintf(disp,"Port=%3d,time=%3d,CycleCount=%2d",port,\
			CrossData[port - 1].CycleTimeOffset + CrossData[port - 1].CycleAulCount,CrossData[port - 1].RedCycleCount5m);
	DebugWindow(disp);
#endif


	for(i = 0;i < 8/*MAX_JCQ_NUM*/;i++)
	{
		FlowData5Min.Data.iVehicleNo[i] = CrossData[port - 1].CycleVeCount[i];
		FlowData5Min.Data.iOccupyTime[i] = CrossData[port - 1].CycleHiBits[i] / 20;
		FlowData5Min.Data.iVehNoAtRed[i] = CrossData[port - 1].CycleRedVeCount[i];
		FlowData5Min.Data.iWaitTime[i] = CrossData[port - 1].CycleRedVeWait[i] / 20;
#ifdef TEST
		memset(disp,0,200);
		sprintf(disp,"Port=%3d,VeCount=%3d,RedCount=%3d,RedWait=%5d",port,\
				CrossData[port - 1].CycleVeCount[i],CrossData[port - 1].CycleRedVeCount[i],\
				CrossData[port - 1].CycleRedVeWait[i] / 20);
		DebugWindow(disp);
#endif
		CrossData[port - 1].CycleVeCount[i] = 0;
		CrossData[port - 1].CycleHiBits[i] = 0;
		CrossData[port - 1].CycleRedVeCount[i] = 0;
		CrossData[port - 1].CycleRedVeWait[i] = 0;
	}

	CrossData[port - 1].RealTime5Min = 0;
	CrossData[port - 1].Left5Min = 5 * 60;
	CrossData[port - 1].RedCycleCount5m = 0;

	//send to svr(wu)
	SendDataEx((char *)&FlowData5Min,sizeof(Rep_FlowData),(UINT8)1);

	return;
}

/*********************************************************************************
*	Function Name	: void ReportFlowData(int port)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,21,2001
*	Global			: None
*	Note			: None
**********************************************************************************/	
void ReportFlowData(int port)
{
	int i;
	Rep_FlowData FlowData5Min;
#ifdef TEST
	char disp[200];
#endif

	if(port <= 0 || port > MAX_CROSS_NUM)
	{
		DebugWindow("ReportFlowData(),Input port Error!");
		return;
	}

	if(CrossData[port - 1].RealTime5Min == 0)
	{
		DebugWindow("ReportFlowData(),RealTimeMin=0,only return Directly");
		return;
	}

	memset(&FlowData5Min,0,sizeof(Rep_FlowData));

	FlowData5Min.Header.byFlag = MsgFlag;
	FlowData5Min.Header.MsgType = CM_FLOWDATA;

	strcpy(FlowData5Min.Header.SourceIP,HostIp);
	strcpy(FlowData5Min.Header.SourceID,HostId);
	strcpy(FlowData5Min.Header.TargetIP,SvrIp);
	strcpy(FlowData5Min.Header.TargetID,SvrId);

	FlowData5Min.Header.iCrossNo = CrossData[port - 1].lkbh;
	FlowData5Min.Header.iLength = sizeof(FlowData);
	FlowData5Min.Header.byCheckSum = FormCheckSum((char *)&(FlowData5Min.Header));

	FlowData5Min.Data.iCycleTime = CrossData[port - 1].RealTime5Min;
	FlowData5Min.Data.iTotalTime = 5 * 60;
	FlowData5Min.Data.iCycleNo = (int)CrossData[port - 1].RedCycleCount5m;

#ifdef TEST
	memset(disp,0,200);
/*	sprintf(disp,"Port=%03d,time = %3d,CycleCount = %02d",port,\*/
	sprintf(disp,"Port=%3d,time=%3d,CycleCount=%2d",port,\
			CrossData[port - 1].RealTime5Min,CrossData[port - 1].RedCycleCount5m);
	DebugWindow(disp);
#endif


/*	if(CrossData[port - 1].Status == FORCED_STATUS)
	{


	}
	else
	{
*/		for(i = 0;i < 8/*MAX_JCQ_NUM*/;i++)
		{
			FlowData5Min.Data.iVehicleNo[i] = CrossData[port - 1].VeCountp5m[i];
			FlowData5Min.Data.iOccupyTime[i] = CrossData[port - 1].HiBitsp5m[i] / 20;
			FlowData5Min.Data.iVehNoAtRed[i] = CrossData[port - 1].RedVeCountp5m[i];
			FlowData5Min.Data.iWaitTime[i] = CrossData[port - 1].RedVeWaitTime5m[i] / 20;
	#ifdef TEST
			memset(disp,0,200);
			sprintf(disp,"Port=%3d,VeCount=%3d,RedCount=%3d,RedWait=%5d",port,\
					CrossData[port - 1].VeCountp5m[i],CrossData[port - 1].RedVeCountp5m[i],\
					CrossData[port - 1].RedVeWaitTime5m[i] / 20);
			DebugWindow(disp);
	#endif
			CrossData[port - 1].VeCountp5m[i] = 0;
			CrossData[port - 1].HiBitsp5m[i] = 0;
			CrossData[port - 1].RedVeWaitTime5m[i] = 0;
			CrossData[port - 1].RedVeCountp5m[i] = 0;
		}

		CrossData[port - 1].RealTime5Min = 0;
		CrossData[port - 1].Left5Min = 5 * 60;
		CrossData[port - 1].RedCycleCount5m = 0;
//	}
//	if(port==5)
//		CrossData[port - 1].RedCycleCount5m = 0;
/*
#ifdef TEST	
	for(i = 0;i < 8;i++)
	{
		if(FlowData5Min.Data.iVehicleNo[i] < FlowData5Min.Data.iVehNoAtRed[i])
			break;
	}
#endif
*/
	//send to svr(wu)
	SendDataEx((char *)&FlowData5Min,sizeof(Rep_FlowData),(UINT8)1);

	return;
}

/*********************************************************************************
*	Function Name	: void ClearCommTxkh(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/11,01,2001
*	Global			: None
*	Note			: None
**********************************************************************************/	
void ClearCommTxkh(void)
{
	int i;

	for(i = 0;i < MAX_CROSS_NUM;i++)
	{
		if(CrossData[i].lkbh != 0/* && CrossData[i].Type == 1*/)
		{
			if(CrossData[i].CardNo != 0)
			{
		#ifdef TEST
				DebugWindow("Clear Cardno for cross");
		#endif
				CrossData[i].CardNo = 0;
			}
		}
	}

	return;
}

/*********************************************************************************
*	Function Name	: void AdjustTxkh(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/11,19,2001
*	Global			: None
*	Note			: None
**********************************************************************************/	
void AdjustTxkh(void)
{
	int i;
	int Txkh;

	if(UseLcu == 0)
		return;

	if(Card1 > 0 && Card2 > 0)
		return;//cannot adjust

	if(Card1 > 0)
		Txkh = 1;
	else if(Card2 > 0)
		Txkh = 2;
	else
		Txkh = 0;

	for(i = 0;i < MAX_CROSS_NUM;i++)
	{
		if(CrossData[i].lkbh != 0/* && CrossData[i].Type == 1*/)
		{
			if(CrossData[i].CardNo != Txkh)
			{
		#ifdef TEST
				DebugWindow("Adjust Cardno for cross");
		#endif
				CrossData[i].CardNo = Txkh;
			}
		}
	}

	return;
}

/*********************************************************************************
*	Function Name	: void TsctlInit(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/01,19,2001
*	Global			: None
*	Note			: None
**********************************************************************************/	
void TsctlInit(void)
{
	int i;

	for(i = 1; i <= MAX_CROSS_NUM;i++)
		InitialCross(i);

//	memset(OprTable,0,(MAX_SOCKET_NUM + 1)*sizeof(COprData));	
//	memset(SendSpool,0,MAX_SPOOL_NUM*sizeof(RecBuf));
//	memset(&SendingBuf,0,sizeof(RecBuf));
//	memset(TaskTable,0,MAX_CROSS_NUM*(0x85 - 0x80 + 1)*sizeof(TaskData));

	GetDbInit();

	if(UseLcu == 0)
		ClearCommTxkh();
	else
	{
		AdjustTxkh();
	}

	ClearSignal4Hu();

 	memset(&SendingBuf,0,sizeof(RecBuf));
/*	for(i = 0; i<16;i++)
	{
//		SetCrossInitStatus(i + 2);//?????????

		CrossData[i + 2].Status = NORMAL_STATUS;
		CrossData[i + 2].StepTable[0] = 30;
		CrossData[i + 2].StepTable[1] = 6;
		CrossData[i + 2].StepTable[2] = 30;
		CrossData[i + 2].StepTable[3] = 6;

		CrossData[i + 2].TotalStepNo = 4;
	}
*/
/*	CrossData[7].Status = INITIAL_STATUS;
	CrossData[7].StepTable[0] = 30;
	CrossData[7].StepTable[1] = 6;
	CrossData[7].StepTable[2] = 30;
	CrossData[7].StepTable[3] = 6;
	CrossData[7].TotalStepNo = 4;


	CrossData[4].Status = INITIAL_STATUS;
	CrossData[4].StepTable[0] = 30;
	CrossData[4].StepTable[1] = 6;
	CrossData[4].StepTable[2] = 30;
	CrossData[4].StepTable[3] = 6;
	CrossData[4].TotalStepNo = 4;

	CrossData[5].Status = INITIAL_STATUS;
	CrossData[5].StepTable[0] = 30;
	CrossData[5].StepTable[1] = 6;
	CrossData[5].StepTable[2] = 30;
	CrossData[5].StepTable[3] = 6;
	CrossData[5].TotalStepNo = 4;

	CrossData[9].Status = INITIAL_STATUS;
	CrossData[9].StepTable[0] = 30;
	CrossData[9].StepTable[1] = 6;
	CrossData[9].StepTable[2] = 30;
	CrossData[9].StepTable[3] = 6;
	CrossData[9].TotalStepNo = 4;
*/
	return;
}

/***************************************************************************
*	Function Name	: void SetCrossInitStatus(int port)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,11,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
void SetCrossInitStatus(int port)
{
	if(port <= 0 || port > MAX_CROSS_NUM)
	{
		DebugWindow("SetCrossInitStatus(),port error!");
		return;
	}
	
	if(CrossData[port - 1].lkbh == 0)
	{
		CrossData[port - 1].Status = IDLE_STATUS;
		return;
	}
	else if(CrossData[port - 1].TotalStepNo < 4 || CrossData[port - 1].TotalStepNo > MAX_STEP_NUM)
	{
		CrossData[port - 1].Status = IDLE_STATUS;
		return;
	}
	else
	{
		CrossData[port - 1].Status = INITIAL_STATUS;
		CrossData[port - 1].InitalWait = 3;
		CrossData[port - 1].CurStep = MAX_STEP_NUM + 1;
		CrossData[port - 1].NeedStep = MAX_STEP_NUM + 2;
		CrossData[port - 1].CurCtrMode = 0xff;//D当前的控制模式
		CrossData[port - 1].CurCValue = 0xff;//C当前的C值
		CrossData[port - 1].Left5Min = 5 * 60;

		CrossData[port - 1].CycleTimeOffset = 0;
		CrossData[port - 1].CycleAulCount = 0;
	}

	return;
}

/***************************************************************************
*	Function Name	: void InitialCross(int port)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,11,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
void InitialCross(int port)
{
	if(port <= 0 || port > MAX_CROSS_NUM)
	{
		DebugWindow("InitialCross(),port error!");
		return;
	}
	
	memset(&CrossData[port - 1],0,sizeof(CrossData_t));

	CrossData[port - 1].InitalWait = 3;
	CrossData[port - 1].CurStep = MAX_STEP_NUM + 1;
	CrossData[port - 1].NeedStep = MAX_STEP_NUM + 2;
	CrossData[port - 1].CurCtrMode = 0xff;
	CrossData[port - 1].CurCValue = 0xff;
	CrossData[port - 1].Left5Min = 5 * 60;

	CrossData[port - 1].CycleTimeOffset = 0;
	CrossData[port - 1].CycleAulCount = 0;

	CrossData[port - 1].NoSignalCount = 100;//15;

//	CrossData[port - 1].LinkFlag = TRUE;

	return;
}

/***************************************************************************
*	Function Name	: void ClearSignal4Hu(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/11,20,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
void ClearSignal4Hu(void)
{
	int i;

	if(UseLcu == 0)
		return;

	for(i = 0;i < MAX_CROSS_NUM;i++)
	{
		if(CrossData[i].Type == 1)
			CrossData[i].NoSignalCount = 0;
	}

	return;
}

/***************************************************************************
*	Function Name	: void GetLcuFlag(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/09,07,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
void GetLcuFlag(void)
{
	UseLcu = GetPrivateProfileInt("GENERAL","UseLcu",0, __CONFIG_FILE__);

	if(UseLcu == 0)
	{
		//card not insert
		Card1 = 0;
		Card2 = 0;
		IntMode = 0;
		IntNumber = 0;
	}
	else
	{
		Card1 = GetPrivateProfileInt("GENERAL","Card1",0, __CONFIG_FILE__);
		Card2 = GetPrivateProfileInt("GENERAL","Card2",0, __CONFIG_FILE__);

		if(Card1 == 0 && Card2 == 0)
		{
			//card not insert
			UseLcu = 0;
			IntMode = 0;
			IntNumber = 0;
		}
		else
		{
			IntMode = GetPrivateProfileInt("GENERAL","IntMode",0, __CONFIG_FILE__);

			if(IntMode == 1)
				IntNumber = GetPrivateProfileInt("GENERAL","IntNumber",10, __CONFIG_FILE__);
			else
				IntNumber = 0;
		}
	}

/*	
	if(Card1)
		CardCount++; 
	if(Card2)
		CardCount++; 
*/
	return;
}

/***************************************************************************
*	Function Name	: void InitCrossComm(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,11,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
void InitCrossComm(void)
{
	if(UseLcu)
	{
		if(!LptInit())	
		{
			DebugWindow("InitCrossEnv(),Initilize Lpt Drive Failed!");
			return;
		}
	}
	else
		MultiSerialInit();

	return;
}

/***************************************************************************
*	Function Name	: void ServiceInit(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,11,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
void ServiceInit(void)
{
/*	BOOL Ret;

	Ret = GetSvrIp(SvrIp);
	if(!Ret)
	{
		DebugWindow("ServiceInit(),Get Svr Ip Failed!");
		return;
	}

	GetLcuFlag();
*/
	TsctlInit();
	SetRealTimer();

	return;
}

/***************************************************************************
*	Function Name	: void GetAppConfig(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/11,21,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
void GetAppConfig(void)
{
	BOOL Ret;

	GetLcuFlag();

	SerialStart = GetPrivateProfileInt("GENERAL","SerStart",1, __CONFIG_FILE__);
	if(SerialStart < 1)
		SerialStart = 1;

	SerialCommNum = GetPrivateProfileInt("GENERAL","SerCommNum",0, __CONFIG_FILE__);
	if(SerialCommNum == 0 || SerialCommNum >= MAX_CROSS_NUM)
	{
		DebugWindow("GetAppConfig(),SerialCommNum Out Of Range!");
		SerialCommNum = 1;
	}

	memset(HostId,0,10);
	Ret = GetPrivateProfileString("GENERAL","UserId","通讯机1",HostId,10, __CONFIG_FILE__);
	if(Ret <= 0 || Ret > 10)
	{
		HostId[9] = 0x00;
	}

	memset(SvrIp,0,sizeof(SvrIp));
	Ret = GetSvrIp(SvrIp);
	if(!Ret)
	{
		DebugWindow("GetAppConfig(),Get Svr Ip Failed!");
		return;
	}

	TcpPort = GetPrivateProfileInt("GENERAL","SockPort",SLJK_TCP_PORT, __CONFIG_FILE__);

	return;
}

/***************************************************************************
*	Function Name	: void GetSvrIp(char *InIpAddr)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,16,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
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

/***************************************************************************
*	Function Name	: void SendTestMsg(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/09,20,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
void SendTestMsg(void)
{
//	BOOL Ret;
	CMsgFrame CheckinEvent;

	if(strcmp(SvrIp,"") == 0)
		return;

/*
	memset(HostId,0,10);
	Ret = GetPrivateProfileString("GENERAL","UserId","通讯机1",HostId,10, __CONFIG_FILE__);

	if(Ret <= 0 || Ret > 10)
	{
		HostId[9] = 0x00;
//		return FALSE;
	}
*/
	memset(&CheckinEvent,0,sizeof(CMsgFrame));
	CheckinEvent.byFlag = MsgFlag;
	CheckinEvent.MsgType = CM_MESSAGE;

	strcpy(CheckinEvent.SourceIP,HostIp);
	strcpy(CheckinEvent.SourceID,HostId);
	strcpy(CheckinEvent.TargetIP,SvrIp);
	strcpy(CheckinEvent.TargetID,SvrId);

	CheckinEvent.iCrossNo = 0;//indicate this is comm computer
	CheckinEvent.iLength = 0;
	CheckinEvent.byCheckSum = FormCheckSum((char *)&CheckinEvent);

	if(SocketData[1].Socket == INVALID_SOCKET)
		return;

	SendDataEx((char *)&CheckinEvent,sizeof(CMsgFrame),(UINT8)1);
	
	return;
}

/***************************************************************************
*	Function Name	: void SendCheckin(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,19,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
void SendCheckin(void)
{
	BOOL Ret;
	CMsgFrame CheckinEvent;

	memset(SvrIp,0,16);
	Ret = GetSvrIp(SvrIp);
	if(!Ret)
	{
		DebugWindow("SendCheckin(),Get Svr Ip Failed!");
		return;
	}
/*
	memset(HostId,0,10);
	Ret = GetPrivateProfileString("GENERAL","UserId","通讯机1",HostId,10, __CONFIG_FILE__);

	if(Ret <= 0 || Ret > 10)
	{
		HostId[9] = 0x00;
//		return FALSE;
	}
*/
	memset(&CheckinEvent,0,sizeof(CMsgFrame));
	CheckinEvent.byFlag = MsgFlag;
	CheckinEvent.MsgType = CM_CHECKIN;

	strcpy(CheckinEvent.SourceIP,HostIp);
	strcpy(CheckinEvent.SourceID,HostId);
	strcpy(CheckinEvent.TargetIP,SvrIp);
	strcpy(CheckinEvent.TargetID,SvrId);

	CheckinEvent.iCrossNo = 1;//indicate this is comm computer
	CheckinEvent.iLength = 0;
	CheckinEvent.byCheckSum = FormCheckSum((char *)&CheckinEvent);

	SendDataEx((char *)&CheckinEvent,sizeof(CMsgFrame),(UINT8)1);
	
	return;
}

/***************************************************************************
*	Function Name	: BYTE FormCheckSum(char *InPtr)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,19,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
BYTE FormCheckSum(char *InPtr)
{
	register int i;
	int sum = 0;
	int len = sizeof(CMsgFrame);

	if(InPtr == NULL)
	{
		DebugWindow("FormCheckSum(),Input Ptr == NULL!");
		return 0;
	}

	for(i = 0;i < len;i++)
		sum += *(InPtr + i);

	sum = sum & 0xff;

	return (BYTE)sum;
}

/***************************************************************************
*	Function Name	: BOOL IsCheckSumOk(char *InPtr)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/07,20,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
BOOL IsCheckSumOk(char *InPtr)
{
	register int i;
	int sum = 0;
	CMsgFrame *p;
	UINT8 tempsum;
	int len = sizeof(CMsgFrame);

	if(InPtr == NULL)
	{
		DebugWindow("FormCheckSum(),Input Ptr == NULL!");
		return FALSE;
	}

	for(i = 0;i < len;i++)
		sum += *(InPtr + i);

	p = (CMsgFrame *)InPtr;
	sum -= p->byCheckSum;

	tempsum = (char)(sum & 0xff);

	if(tempsum == (p->byCheckSum))
		return TRUE;
	else
		return FALSE;

}

/***************************************************************************
*	Function Name	: void CheckForecedTime(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/08,13,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
void CheckForecedTime(void)
{
	int i;

	for(i = 0; i < MAX_CROSS_NUM; i++)
	{
		if(CrossData[i].Status != FORCED_STATUS)
			continue;

		if(CrossData[i].ForecedLeftTime == 0)
		{
			DebugWindow("System Auto Stop Invoking!");
/*			if(CrossData[i].Type == 1)//hu
				EndColorReqProc(NULL,CM_ENDCOLOR,(UINT8)(i + 1),1);
			else
*/			if(CrossData[i].Type == 2)//jinsan cross
			{
				if(CrossData[i].ForcedStep == 201)
				{
					SetYellowOffProc((UINT8)(i + 1));
				}
				else if(CrossData[i].ForcedStep == 211)
				{
					SetBlackOffProc((UINT8)(i + 1));
				}
				else
				{
					SetStepoffAdjust((UINT8)(i + 1));
					CrossData[i].Status = TRANSIT_STATUS;
				}
			}
		}
		else
			CrossData[i].ForecedLeftTime--;	
	}

	return;
}

/***************************************************************************
*	Function Name	: void SendTest2Hu(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/10,11,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
void SendTest2Hu(void)
{
	int i;
#ifdef FORDEBUG
	char disp[200];
#endif

	for(i = 0; i < MAX_CROSS_NUM; i++)
	{
		if(CrossData[i].Type != 1)//sanlian
			continue;

		if(!UseLcu)
		{
			if(ThreadCtr[i].dwThreadID == 0)//comm not open
				continue;
		}

		if(NowCanSend((UINT8)(i + 1)) && CrossData[i].NoSignalCount > 20)
		{
			if(UseLcu)
			{
#ifdef FORDEBUG
				memset(disp,0,200);
				sprintf(disp,"Auto Send 0x85 To Port %d!",i + 1);
				DebugWindow(disp);
#endif
				StartTestReqProc(0,(UINT8)(i + 1),0xff);

				continue;
			}

			//following use serial comm
			if(CrossData[i].Status != DISCONNECT_STATUS)
			{
#ifdef FORDEBUG
				memset(disp,0,200);
				sprintf(disp,"Auto Send 0x85 To Port %d!",i + 1);
				DebugWindow(disp);
#endif
				StartTestReqProc(0,(UINT8)(i + 1),0xff);
			}
			else
			{
				SYSTEMTIME time;
				DATA_BENCHMARK_TIME Data;

				GetLocalTime(&time);
				Data.Year = (UINT8)(time.wYear - 2000);
				Data.Month = (UINT8)(time.wMonth);
				Data.Day = (UINT8)(time.wDay);
				Data.Hour = (UINT8)(time.wHour);
				Data.Minute = (UINT8)(time.wMinute);
				Data.Second = (UINT8)(time.wSecond);
				Data.Week = (UINT8)(time.wDayOfWeek);

#ifdef FORDEBUG
				memset(disp,0,200);
				sprintf(disp,"Auto Send 0x02 To Port %d!",i + 1);
				DebugWindow(disp);
#endif
				BenchMarkReqProc(&Data,CM_TIMESYNC,(UINT32)(i + 1),1);
			}
		}


	}//end for

	return;
}

/***************************************************************************
*	Function Name	: void SendCurTime(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/08,14,2001
*   Modify		    : yincy/04,03,2002
*	Global			: None
*	Note			: None
****************************************************************************/	
void SendCurTime(void)
{
	int i;
	SYSTEMTIME time;
	DATA_BENCHMARK_TIME Data;

	GetLocalTime(&time);
	Data.Year = (UINT8)(time.wYear - 2000);
	Data.Month = (UINT8)(time.wMonth);
	Data.Day = (UINT8)(time.wDay);
	Data.Hour = (UINT8)(time.wHour);
	Data.Minute = (UINT8)(time.wMinute);
	Data.Second = (UINT8)(time.wSecond);
	Data.Week = (UINT8)(time.wDayOfWeek);

	for(i = 0; i < MAX_CROSS_NUM; i++)
	{
		if(CrossData[i].Type == 1)
		{
		//sanlian
			if(!UseLcu)
			{
				if(ThreadCtr[i].dwThreadID == 0)
				{
				//comm not open	
					continue;
				}
			}

			if(NowCanSend((UINT8)(i + 1)))
			{
	#ifdef TEST
				DebugWindow("Auto Send 0x02!");
	#endif
				BenchMarkReqProc(&Data,CM_TIMESYNC,(UINT8)(i + 1),1);
			}
		}
		else if(CrossData[i].Type == 2)
		{
			//jingsan
			if(!UseLcu)
			{
				if(ThreadCtr[i].dwThreadID == 0)
				{
					//comm not open
					continue;
				}
			}

			//CommStatusCheck(i + 1);
			if(NowCanSend((UINT8)(i + 1)))
			{
	#ifdef TEST
				DebugWindow("Auto Send 0x06!");
	#endif
				JSBenchMarkReqProc(&Data,0x06,(UINT32)(i + 1),1);
			}
		}
		else
		{
		//unknwon type
			if(!UseLcu)
			{
				if(ThreadCtr[i].dwThreadID == 0)
				{
				//comm not open	
					continue;
				}

				continue;
			}

			continue;
		}
	}

	return;
}

/***************************************************************************
*	Function Name	: BYTE FormHuCheckSum(char *InPtr,int Len)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/08,08,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
BYTE FormHuChecksum(char *InPtr,int Len)
{
	register int i;
	int sum = 0;
	int len = Len;

	if(InPtr == NULL)
	{
#ifdef TEST
		DebugWindow("FormCheckSum(),Input Ptr == NULL!");
#endif
		return 0;
	}

	if(Len > 300)
	{
#ifdef TEST
		DebugWindow("FormHuChecksum(),Input Len Error!");
#endif
		return FALSE;
	}

	for(i = 0;i < len;i++)
		sum += *(InPtr + i);

	sum = sum & 0xff;

	return (BYTE)sum;
}

/***************************************************************************
*	Function Name	: BYTE FormYlfChecksum(char *InPtr,int Len)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/02,20,2002
*	Global			: None
*	Note			: None
****************************************************************************/	
BYTE FormYlfChecksum(BYTE *InPtr,int Len)
{
	register int i;
	UINT32 sum = 0;
	int len = Len;

	if(InPtr == NULL)
	{
#ifdef TEST
		DebugWindow("FormCheckSum(),Input Ptr == NULL!");
#endif
		return 0;
	}

	if(Len > 300)
	{
#ifdef TEST
		DebugWindow("FormCheckSum(),Input Len Error!");
#endif
		return FALSE;
	}

	for(i = 0;i < len;i++)
		sum += *(InPtr + i);

	sum = sum & 0x000000ff;

	return (BYTE)sum;
}

/***************************************************************************
*	Function Name	: BOOL IsYlfChecksumOk(char *InPtr,int Len)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/02,20,2002
*	Global			: None
*	Note			: None
****************************************************************************/	
BOOL IsYlfChecksumOk(char *InPtr,int Len)
{
	register int i;
	UINT32 sum = 0;
	UINT8 tempsum;

	if(InPtr == NULL)
	{
#ifdef TEST
		DebugWindow("IsYlfChecksumOk(),Input Ptr == NULL!");
#endif
		return FALSE;
	}

	if(Len > 300)
	{
#ifdef TEST
		DebugWindow("IsYlfChecksumOk(),Input Len Error!");
#endif
		return FALSE;
	}

	for(i = 0;i < Len;i++)
		sum += (UINT8)(*(InPtr + i));

	tempsum = (UINT8)(sum & 0xff);

	if(tempsum == (UINT8)(*(InPtr + Len)))
		return TRUE;
	else
		return FALSE;

	return TRUE;
}

/***************************************************************************
*	Function Name	: BOOL IsCheckSumOk(char *InPtr,int Len)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/08,08,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
BOOL IsHuChecksumOk(char *InPtr,int Len)
{
	register int i;
	UINT32 sum = 0;
	UINT8 tempsum;

	if(InPtr == NULL)
	{
#ifdef TEST
		DebugWindow("FormCheckSum(),Input Ptr == NULL!");
#endif
		return FALSE;
	}

	if(Len > 300)
	{
#ifdef TEST
		DebugWindow("FormCheckSum(),Input Len Error!");
#endif
		return FALSE;
	}

	for(i = 0;i < Len;i++)
		sum += *(InPtr + i);

//	sum -= *(InPtr + Len);

	tempsum = (UINT8)(sum & 0xff);

	if(tempsum == (UINT8)(*(InPtr + Len)))
		return TRUE;
	else
		return FALSE;
}
