#include "iocard2c.h"

CIOCard1 m_IOCard;

DWORD dwIntCount;
HWND hMainWnd;

void IntHandler(CIOCard1 * pDrv)
{
	static DWORD a = 0;
	
	dwIntCount = pDrv->GetIntCount();
	TRACE("IntCount =%d\n",dwIntCount);

	PostMessage(hMainWnd,LPT_READ,
		pDrv->ReadPortW(0x360),
		pDrv->ReadPortW(0x362));	 
//      pDrv->GetIntCount());	 
	if(dwIntCount != a+1) {
		int b=0;
	}

	a = dwIntCount;

}

BOOL Open(void)
{
	return m_IOCard.Open(0x360, 0x367,11);
}

BOOL Open(WORD IOPortAddrStart,WORD IOPortAddrStart,BYTE IRQ)
{
	return m_IOCard.Open(IOPortAddrStart,IOPortAddrStart,IRQ);
}

void Close(void)
{
	m_IOCard.Close();
}

BOOL EnableIntrrupt(pInterruptHandlerRoutine pIntHndlr)
{
	return m_IOCard.EnableIntrrupt(pIntHndlr);
}

void DisableIntrrupt(void)
{
	m_IOCard.DisableIntrrupt();
}

DWORD GetIntCount(void)
{
	return m_IOCard.GetIntCount();
}

BYTE ReadPortB(WORD wPortAddr)
{
	return m_IOCard.ReadPortB(wPortAddr);
}

WORD ReadPortW(WORD wPortAddr)
{
	return m_IOCard.ReadPortW(wPortAddr);
}

DWORD ReadPortDW(WORD wPortAddr)
{
	return m_IOCard.ReadPortDW(wPortAddr);
}

void WritePort(WORD wPortAddr,BYTE bValue)
{
	m_IOCard.WritePort(wPortAddr,bValue);
}

void WritePort(WORD wPortAddr,WORD wValue)
{
	m_IOCard.WritePort(wPortAddr,wValue);
}

void WritePort(WORD wPortAddr,DWORD dwValue)
{
	m_IOCard.WritePort(wPortAddr,dwValue);
}





