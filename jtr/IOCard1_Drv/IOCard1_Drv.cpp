#include <windows.h>
#include "IOCard1.h"

BOOL APIENTRY 
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	// Remove this if you use lpReserved
	UNREFERENCED_PARAMETER(lpReserved);


	if (dwReason == DLL_PROCESS_ATTACH)
	{
		InitIOCard();
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
	}

	return TRUE;
}

