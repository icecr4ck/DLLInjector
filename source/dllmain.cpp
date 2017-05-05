// dllmain.cpp : Defines the entry point for the DLL application.
#define _WIN32_WINNT _WIN32_WINNT_WINXP

#include "stdafx.h"

BOOL APIENTRY DllMain( HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		MessageBox(NULL, (LPCWSTR)L"Congrats! The DLL has been successfully injected!", (LPCWSTR)L"You are in DLLMain!", MB_OK);
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

