#include <windows.h>
#include <psapi.h>
#include <iostream>

int ProcMemSpace()
{
	int pid = 14760;

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

	CHAR lpImageFileName[512] = { };

	if (GetProcessImageFileNameA(hProcess, lpImageFileName, 511) == 0)
	{
		// error
		return -1;
	}

	HMODULE lphModule[1024];
	DWORD lpcbNeeded;
	DWORD loopCount = 0;

	if (EnumProcessModules(hProcess, lphModule, sizeof(lphModule), &lpcbNeeded) == 0)
	{
		// error
		return -2;
	}

	lpcbNeeded /= 8;

	for (DWORD i = 0; i < lpcbNeeded; ++i)
	{
		CHAR lpFileName[512] = {};
		DWORD result = 0;

		if ((result = GetModuleFileNameExA(hProcess, lphModule[i], lpFileName, 511)) == 0)
		{
			// error
			//printf("GetLaseError == %d\n", GetLastError());
			return -3;
		}

		MODULEINFO modInfo;

		if (GetModuleInformation(hProcess, lphModule[i], &modInfo, sizeof(modInfo)) == 0)
		{
			// error
			return -4;
		}

		LPVOID ptr = modInfo.EntryPoint;

		printf("%s\n", lpFileName);
		printf("\tEntryPoint == %p\n", ptr);
		printf("\tlpBaseOfDll == %p\n", modInfo.lpBaseOfDll);
		printf("\tSizeOfImage == %d\n", modInfo.SizeOfImage);

		++loopCount;
	}

	printf("lpcbNeeded == %d\n", lpcbNeeded);
	printf("loopCount == %d\n", loopCount);

	return 0;
}
