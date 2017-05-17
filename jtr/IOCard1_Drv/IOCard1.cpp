/*****************************************************************
*ModuleName: IOCard1_Drv            FileName:IOCard1_Drv.c	     *
*CreateDate: 11/12/2001             Author:  yincy               *
*Version:    1.0                                                 *
*History:                                                        *
* Date       Version       Modifier           Activies           *
*11/12/2001  1.0             yincy             create            *
*****************************************************************/

#include "IOCard1.h"
#include "IOCard1_Drv.h"

CIOCard1 CardData;

/***************************************************************************
*	Function Name	: void SetIRQ(BYTE Irq)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/11,14,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
void SetIRQ(BYTE Irq)
{
	CardData.IRQ = Irq;

	return;
}

/***************************************************************************
*	Function Name	: void SetIOAddr(WORD wStartAddr,WORD wEndAddr)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/11,14,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
void SetIOAddr(WORD wStartAddr,WORD wEndAddr)
{
	CardData.IO_Addr_Start = wStartAddr;
	CardData.IO_Addr_End = wEndAddr;

	return;
}

/***************************************************************************
*	Function Name	: DWORD WINAPI wait_interrupt (PVOID pData)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/11,14,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
DWORD WINAPI wait_interrupt (PVOID pData)
{
	CIOCard1 *p = (CIOCard1 *)pData;

	while(1)
	{
		WD_IntWait (p->hDriver, &p->Intrp);	//wait for a intrrupt occur

		if (p->Intrp.fStopped)
			break; // WD_IntDisable called by parent

		p->IntHndlr((PVOID)(p->IO_Addr_Start));	//call user defined function;
	}

	return 0;
}

/***************************************************************************
*	Function Name	: void InitIOCard(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/11,14,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
void InitIOCard(void)
{
	BZERO(CardData);
/*
	CardData.hDriver = NULL;
	CardData.IO_Addr_Start = 0x360;
	CardData.IO_Addr_End = 0x367;
	CardData.IRQ = 11;
*/
	return;
}

/***************************************************************************
*	Function Name	: BOOL DefaultOpen(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/11,14,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
BOOL DefaultOpen(void)
{
	WD_VERSION ver;
    WD_LICENSE lic;

    CardData.hDriver = WD_Open();
    if (CardData.hDriver==INVALID_HANDLE_VALUE)
    {
		MessageBox(NULL,
				"DefaultOpen(),Can not open WinDriver device!",
				"Warning",
				MB_OK);

		return FALSE;
	}
    
	BZERO(ver);
	WD_Version(CardData.hDriver,&ver);
	if(ver.dwVer <WD_VER)
	{
		MessageBox(NULL,
				"DefaultOpen(),WinDriver Version Error!",
				"Warning",
				MB_OK);

		return FALSE;
	}

	strcpy(lic.cLicense, "6C3C3225C73EFB96D73EADCFE321F554FB60D65C.A9AB070E");
    WD_License(CardData.hDriver, &lic);

	if(!RegisterCard())
	{
		return FALSE;
	}

	return TRUE;
}

/***************************************************************************
*	Function Name	: BOOL Open(WORD IOPortAddrStart,WORD IOPortAddrEnd,BYTE IRQ)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/11,14,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
BOOL Open(WORD IOPortAddrStart,WORD IOPortAddrEnd,BYTE IRQ)
{
	SetIOAddr(IOPortAddrStart,IOPortAddrEnd);
	SetIRQ(IRQ);

	return DefaultOpen();
}


/***************************************************************************
*	Function Name	: BOOL RegisterCard(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/11,14,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
BOOL RegisterCard(void)
{
    int i = 0;
    WD_ITEMS* pItem;

	BZERO(CardData.CardReg);
    CardData.CardReg.Card.dwItems = CARD1_TOTAL_ITEMS;
    pItem = &CardData.CardReg.Card.Item[0];

    pItem[IO_ADDR_RANGE1].item = ITEM_IO;
    pItem[IO_ADDR_RANGE1].fNotSharable = TRUE;
    pItem[IO_ADDR_RANGE1].I.IO.dwAddr = CardData.IO_Addr_Start;// beginning of io address
    pItem[IO_ADDR_RANGE1].I.IO.dwBytes = CardData.IO_Addr_End - CardData.IO_Addr_Start + 1;//io range

	if(CardData.IRQ > 0 )//??????????
	{
		pItem[INTRRUPT1].item = ITEM_INTERRUPT;
		pItem[INTRRUPT1].fNotSharable = FALSE;
		pItem[INTRRUPT1].I.Int.dwInterrupt = CardData.IRQ;
		pItem[INTRRUPT1].I.Int.dwOptions = INTRRUPT_OPTIONS;
	}

	WD_CardRegister(CardData.hDriver,&CardData.CardReg);
	if(CardData.CardReg.hCard == NULL)
	{
		MessageBox(NULL,
				"DefaultOpen(),WinDriver Initilization Error,Device May In Use!",
				"Warning",
				MB_OK);

		return FALSE;
	}

	return TRUE;
}

/***************************************************************************
*	Function Name	: void Close(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/11,14,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
void Close(void)
{
	WD_CardUnregister(CardData.hDriver,&CardData.CardReg);
	WD_Close(CardData.hDriver);
	CardData.hDriver = NULL;

	return;
}

/***************************************************************************
*	Function Name	: BOOL EnableIntrrupt(pInterruptHandlerRoutine pIntHndlr)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/11,14,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
BOOL EnableIntrrupt(pInterruptHandlerRoutine pIntHndlr)
{
	DWORD ThreadId;

	BZERO(CardData.Intrp);
	CardData.Intrp.hInterrupt = CardData.CardReg.Card.Item[INTRRUPT1].I.Int.hInterrupt; 
	CardData.Intrp.Cmd = NULL;
	CardData.Intrp.dwCmds = 0;
	WD_IntEnable(CardData.hDriver, &CardData.Intrp);
	if (!CardData.Intrp.fEnableOk)
	{
		return FALSE;
	}

//	CardData.hWD = CardData.hDriver;
//	CardData.pIntrp = &CardData.Intrp;
	CardData.IntHndlr = pIntHndlr;
//	CardData.pDriver = this;

	if(CreateThread(NULL,NULL,wait_interrupt,&CardData,0,&ThreadId) == NULL)
	{
		return FALSE;
	}

	return TRUE;
}


/***************************************************************************
*	Function Name	: BOOL DisableIntrrupt(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/11,14,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
BOOL DisableIntrrupt(void)
{
	if(WD_IntDisable(CardData.hDriver,&CardData.Intrp) == 0)
		return FALSE;

	return TRUE;
}


/***************************************************************************
*	Function Name	: DWORD GetIntCount(void)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/11,14,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
DWORD GetIntCount(void)
{
	WD_IntCount(CardData.hDriver,&CardData.Intrp);

	return CardData.Intrp.dwCounter;
}

/***************************************************************************
*	Function Name	: BYTE ReadPortB(WORD wPortAddr)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/11,14,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
BYTE ReadPortB(WORD wPortAddr)
{
	WD_TRANSFER Trns;
	BYTE read_data;

	if(wPortAddr < CardData.IO_Addr_Start \
		|| wPortAddr > CardData.IO_Addr_End)
	{
		MessageBox(NULL,
				"ReadPortB(),Port No Out Of Range!",
				"Warning",
				MB_OK);

		return 0;
	}

	BZERO(Trns);
	Trns.cmdTrans = RP_BYTE; // Read Port BYTE
	Trns.dwPort = wPortAddr;
	WD_Transfer (CardData.hDriver, &Trns);
	read_data = Trns.Data.Byte;

	return read_data;
}

/***************************************************************************
*	Function Name	: WORD ReadPortW(WORD wPortAddr)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/11,14,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
WORD ReadPortW(WORD wPortAddr)
{
	WD_TRANSFER Trns;
	WORD read_data;

	if(wPortAddr < CardData.IO_Addr_Start \
		|| wPortAddr > CardData.IO_Addr_End)
	{
		MessageBox(NULL,
				"ReadPortW(),Port No Out Of Range!",
				"Warning",
				MB_OK);

		return 0;
	}

	BZERO(Trns);
	Trns.cmdTrans = RP_WORD; 
	Trns.dwPort = wPortAddr;
	WD_Transfer (CardData.hDriver, &Trns);
	read_data = Trns.Data.Word;

	return read_data;
}


/***************************************************************************
*	Function Name	: DWORD ReadPortDW(WORD wPortAddr)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/11,14,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
DWORD ReadPortDW(WORD wPortAddr)
{
	WD_TRANSFER Trns;
	DWORD read_data;

	if(wPortAddr < CardData.IO_Addr_Start \
		|| wPortAddr > CardData.IO_Addr_End)
	{
		MessageBox(NULL,
				"ReadPortDW(),Port No Out Of Range!",
				"Warning",
				MB_OK);

		return 0;
	}

	BZERO(Trns);
	Trns.cmdTrans = RP_DWORD; 
	Trns.dwPort = wPortAddr;
	WD_Transfer (CardData.hDriver, &Trns);
	read_data = Trns.Data.Dword;

	return read_data;
}

/***************************************************************************
*	Function Name	:  ReadPortS(WORD wPortAddr,char *pBuf,WORD dwBytes)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/11,23,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
BOOL ReadPortS(WORD wPortAddr,char *pBuf,WORD dwBytes)
{
	WD_TRANSFER Trns;

	if(wPortAddr < CardData.IO_Addr_Start \
		|| wPortAddr > CardData.IO_Addr_End)
	{
		MessageBox(NULL,
				"ReadPortS(),Port No Out Of Range!",
				"Warning",
				MB_OK);

		return 0;
	}
	if(pBuf == NULL)
	{
		MessageBox(NULL,
				"ReadPortS(),Input Data Address Is NULL!",
				"Warning",
				MB_OK);

		return 0;
	}
	if(dwBytes <= 0)
	{
		MessageBox(NULL,
				"ReadPortS(),Byte Lenghth Error!",
				"Warning",
				MB_OK);

		return 0;
	}

	BZERO(Trns);
	Trns.cmdTrans = RP_SBYTE; // Read Port SBYTE
	Trns.dwPort = wPortAddr;
	Trns.fAutoinc = FALSE;
	Trns.dwBytes = dwBytes;
	Trns.dwOptions = 0;
	Trns.Data.pBuffer = (PVOID)pBuf;

	if(WD_Transfer(CardData.hDriver, &Trns) == 0)
		return FALSE;
	else
		return TRUE;
}

/***************************************************************************
*	Function Name	: BOOL WritePortB(WORD wPortAddr,BYTE bValue)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/11,14,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
BOOL WritePortB(WORD wPortAddr,BYTE bValue)
{
	WD_TRANSFER Trns;

	if(wPortAddr < CardData.IO_Addr_Start \
		|| wPortAddr > CardData.IO_Addr_End)
	{
		MessageBox(NULL,
				"WritePortB(),Port No Out Of Range!",
				"Warning",
				MB_OK);

		return FALSE;
	}

	BZERO(Trns);
	Trns.cmdTrans = WP_BYTE; 
	Trns.dwPort = wPortAddr;
	Trns.Data.Byte = bValue;

	if(WD_Transfer (CardData.hDriver, &Trns) == 0)
		return FALSE;
	else
		return TRUE;
}

/***************************************************************************
*	Function Name	: BOOL WritePortW(WORD wPortAddr,WORD wValue)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/11,14,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
BOOL WritePortW(WORD wPortAddr,WORD wValue)
{
	WD_TRANSFER Trns;

	if(wPortAddr < CardData.IO_Addr_Start \
		|| wPortAddr > CardData.IO_Addr_End)
	{
		MessageBox(NULL,
				"WritePortW(),Port No Out Of Range!",
				"Warning",
				MB_OK);

		return FALSE;
	}

	BZERO(Trns);
	Trns.cmdTrans = WP_WORD; 
	Trns.dwPort = wPortAddr;
	Trns.Data.Word = wValue;

	if(WD_Transfer (CardData.hDriver, &Trns) == 0)
		return FALSE;
	else
		return TRUE;

}

/***************************************************************************
*	Function Name	: BOOL WritePortDW(WORD wPortAddr,DWORD dwValue)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/11,14,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
BOOL WritePortDW(WORD wPortAddr,DWORD dwValue)
{
	WD_TRANSFER Trns;

	if(wPortAddr < CardData.IO_Addr_Start \
		|| wPortAddr > CardData.IO_Addr_End)
	{
		MessageBox(NULL,
				"WritePortDW(),Port No Out Of Range!",
				"Warning",
				MB_OK);

		return FALSE;
	}

	BZERO(Trns);
	Trns.cmdTrans = WP_DWORD; 
	Trns.dwPort = wPortAddr;
	Trns.Data.Dword = dwValue;

	if(WD_Transfer (CardData.hDriver, &Trns) == 0)
		return FALSE;
	else
		return TRUE;

}

/***************************************************************************
*	Function Name	:  BOOL WritePortS(WORD wPortAddr,char *pBuf,WORD dwBytes)
*	Description		: 
*	INPUT	        :	
*	OUTPUT		    :	
*   RETURN          : 
*   Author/Date     : yincy/11,23,2001
*	Global			: None
*	Note			: None
****************************************************************************/	
BOOL WritePortS(WORD wPortAddr,char *pBuf,WORD dwBytes)
{
	WD_TRANSFER Trns;

	if(wPortAddr < CardData.IO_Addr_Start \
		|| wPortAddr > CardData.IO_Addr_End)
	{
		MessageBox(NULL,
				"WritePortS(),Port No Out Of Range!",
				"Warning",
				MB_OK);

		return 0;
	}
	if(pBuf == NULL)
	{
		MessageBox(NULL,
				"WritePortS(),Input Data Address Is NULL!",
				"Warning",
				MB_OK);

		return 0;
	}
	if(dwBytes <= 0)
	{
		MessageBox(NULL,
				"WritePortS(),Byte Lenghth Error!",
				"Warning",
				MB_OK);

		return 0;
	}

	BZERO(Trns);
	Trns.cmdTrans = WP_SBYTE;
	Trns.dwPort = wPortAddr;
	Trns.fAutoinc = FALSE;
	Trns.dwBytes = dwBytes;
	Trns.dwOptions = 0;
	Trns.Data.pBuffer = (PVOID)pBuf;

	if(WD_Transfer(CardData.hDriver, &Trns) == 0)
		return FALSE;
	else
		return TRUE;
}


