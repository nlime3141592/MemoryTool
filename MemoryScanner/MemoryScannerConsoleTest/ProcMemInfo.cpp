#include <windows.h>
#include <psapi.h>
#include <iostream>

void ProcMemInfo(int pid)
{
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

	PROCESS_MEMORY_COUNTERS pmc;

	if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)))
	{
		SIZE_T sizeB = pmc.WorkingSetSize;
		SIZE_T sizeK = sizeB / 1024;
		SIZE_T sizeM = sizeK / 1024;

		printf("Working Set Size (Physical Memory): %lld B == %lld KB == %lld MB\n", sizeB, sizeK, sizeM);
	}
	else
	{
		printf("Failed to get memory info.\n");
	}
}