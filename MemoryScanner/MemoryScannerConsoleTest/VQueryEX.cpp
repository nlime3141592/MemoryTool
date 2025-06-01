#include <Windows.h>
#include <iostream>

void VQueryEX(int pid)
{
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	MEMORY_BASIC_INFORMATION lpBuffer;
	LPCVOID address = 0;

	while (VirtualQueryEx(hProcess, address, &lpBuffer, sizeof(lpBuffer)) == sizeof(lpBuffer))
	{
		printf("Base address = %p\n", lpBuffer.BaseAddress);
		printf("Region size == %lld\n", lpBuffer.RegionSize);
		printf("State == %s\n", ((lpBuffer.State == MEM_COMMIT) ? "Committed" : "Reserved/Free"));

		address = (PBYTE)lpBuffer.BaseAddress + lpBuffer.RegionSize;
	}
}