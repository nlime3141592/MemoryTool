#include "pshandle.h"

#include <Windows.h>
#include <Psapi.h>

#include <vector>

int RefreshProcessList(std::vector<DWORD>& pidList)
{
	pidList.clear();

	DWORD bytes;
	DWORD count;

	if (!EnumProcesses(pidList.data(), sizeof(DWORD) * pidList.capacity(), &bytes))
	{
		return 0;
	}

	count = bytes / sizeof(DWORD);

	for (DWORD i = 0; i < count; ++i)
	{
		DWORD pid = pidList[i];

		if (pid == 0)
			continue;

		HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);

		if (hProcess)
		{
			HMODULE hMod;
			DWORD cbNeeded;
			TCHAR szProcessName[256] = TEXT("<unknown>");

			if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded))
			{
				GetModuleBaseName(hProcess, hMod, szProcessName, sizeof(szProcessName) / sizeof(TCHAR));
			}


		}
	}
}