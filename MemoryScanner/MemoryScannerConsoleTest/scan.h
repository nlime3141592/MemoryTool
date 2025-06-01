#include <windows.h>

typedef union _PrimitiveData
{
	UINT8 bytes[8];
	INT8 data;
	INT16 data;
	INT32 data;
	INT64 data;
	UINT8 data;
	UINT16 data;
	UINT32 data;
	UINT64 data;
} PrimitiveData;

SIZE_T ScanNew(HANDLE hProcess, LPVOID lpBuffer, int n)
{
	MEMORY_BASIC_INFORMATION mbi;

	LPCVOID baseAddress = 0;
	SIZE_T length = 0;
}

SIZE_T ScanContinue(HANDLE hProcess, LPVOID lpBuffer, int n, SIZE_T length)
{
	MEMORY_BASIC_INFORMATION mbi;
	LPCVOID baseAddress = 0;
	SIZE_T newLength = 0;

	VirtualQueryEx(hProcess, baseAddress, &mbi, sizeof(mbi));

	for (SIZE_T i = 0; i < length; ++i)
	{
		while (((LPVOID*)lpBuffer)[i] < mbi.BaseAddress)
		{
			baseAddress = (PBYTE)mbi.BaseAddress + mbi.RegionSize;
			VirtualQueryEx(hProcess, baseAddress, &mbi, sizeof(mbi));
		}

		if (baseAddress != mbi.BaseAddress)
		{
			ReadProcessMemory(hProcess, baseAddress, lpBuffer, mbi.RegionSize, NULL);
		}
	}

	while (VirtualQueryEx(hProcess, baseAddress, &mbi, sizeof(mbi)) == sizeof(mbi))
	{

	}
}