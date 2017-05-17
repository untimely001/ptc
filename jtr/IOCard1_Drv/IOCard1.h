// IOCard1.h: interface for the CIOCard1 class.
//
//////////////////////////////////////////////////////////////////////
#ifndef ISA_IO_DLL
#define ISA_IO_DLL

#include <windows.h>
#include "e:/program/windriver/include/windrvr.h"

#define CARD1_TOTAL_ITEMS	2

#ifdef __cplusplus
extern "C" {
#endif

//internal data type
enum { INTRRUPT_OPTIONS = 0 };
enum { IO_ADDR_RANGE1 = 0, INTRRUPT1 = 1};

typedef void(*pInterruptHandlerRoutine)(PVOID);

typedef struct CIOCard1
{
	BYTE IRQ;						//IRQ number
	WORD IO_Addr_Start;			//start of I/O port addr  
	WORD IO_Addr_End;				//end of I/O port addr

	HANDLE				hDriver;	//handle of this device driver
    WD_CARD_REGISTER	CardReg;
	WD_INTERRUPT		Intrp;

	pInterruptHandlerRoutine IntHndlr;
}CIOCard1;

// interrupt-waitting thread routine prototype:
DWORD WINAPI wait_interrupt(PVOID pData);

//operations
void InitIOCard(void);
BOOL DefaultOpen(void);
BOOL RegisterCard(void);
void SetIRQ(BYTE Irq);
void SetIOAddr(WORD wStartAddr,WORD wEndAddr);


#ifdef __cplusplus
}
#endif

#endif

