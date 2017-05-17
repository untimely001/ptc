#include <windows.h>

#ifdef ISA_IO_DLL
	#define ISACARD_API __declspec(dllexport)
#else
	#define ISACARD_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef ISA_IO_DLL
typedef void(*pInterruptHandlerRoutine)(PVOID);
#endif

// These function is exported from the isacard.dll
ISACARD_API BOOL Open(WORD IOPortAddrStart,WORD IOPortAddrEnd,BYTE IRQ);
ISACARD_API void Close(void);
ISACARD_API BOOL EnableIntrrupt(pInterruptHandlerRoutine pIntHndlr);
ISACARD_API BOOL DisableIntrrupt(void);
ISACARD_API DWORD GetIntCount(void);
ISACARD_API BYTE ReadPortB(WORD wPortAddr);
ISACARD_API WORD ReadPortW(WORD wPortAddr);
ISACARD_API DWORD ReadPortDW(WORD wPortAddr);
ISACARD_API BOOL ReadPortS(WORD wPortAddr,char *pBuf,WORD dwBytes);
ISACARD_API BOOL WritePortB(WORD wPortAddr,BYTE bValue);
ISACARD_API BOOL WritePortW(WORD wPortAddr,WORD wValue);
ISACARD_API BOOL WritePortDW(WORD wPortAddr,DWORD dwValue);
ISACARD_API BOOL WritePortS(WORD wPortAddr,char *pBuf,WORD dwBytes);

#ifdef __cplusplus
}
#endif

