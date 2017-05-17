#include "IOCard1.h"

#ifdef cplusplus
extern "C"{
#endif

BOOL Open(void);
BOOL Open(WORD IOPortAddrStart,WORD IOPortAddrStart,BYTE IRQ);
void Close(void);
BOOL EnableIntrrupt(pInterruptHandlerRoutine pIntHndlr);
void DisableIntrrupt(void);
DWORD GetIntCount(void);
BYTE ReadPortB(WORD wPortAddr);
WORD ReadPortW(WORD wPortAddr);
DWORD ReadPortDW(WORD wPortAddr);
void WritePort(WORD wPortAddr,BYTE bValue);
void WritePort(WORD wPortAddr,WORD wValue);
void WritePort(WORD wPortAddr,DWORD dwValue);

#ifdef cplusplus
}
#endif

