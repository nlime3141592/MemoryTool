#include "scan.h"

static PBYTE kmp_pi;

static PBYTE KMP_PI(LPCVOID data, SIZE_T dataLength)
{
	PBYTE p = (PBYTE)data;
	PBYTE pi = (PBYTE)malloc(sizeof(BYTE) * dataLength);

	SIZE_T i = 0;
	SIZE_T j = 0;
	pi[0] = 0;

	for (i = 1; i < dataLength; ++i)
	{
		while (j > 0 && p[i] != p[j])
			j = kmp_pi[j - 1];
		if (p[i] == p[j])
			kmp_pi[i] = ++j;
	}

	return pi;
}

static void KMP_PI_Free(PBYTE pi)
{
	free(pi);
}

static void ChangeEndian(PrimitiveData* data, SIZE_T dataLength)
{
	SIZE_T half = dataLength >> 2;

	for (SIZE_T i = 0; i < half; ++i)
	{
		int j = dataLength - i - 1;
		int t = data->bytes[i];
		data->bytes[i] = data->bytes[j];
		data->bytes[j] = t;
	}
}

static int Compare(PrimitiveData* a, PrimitiveData* b, SIZE_T dataLength)
{
	switch (dataLength)
	{
	case 1:
		return a->udata8 == b->udata8;
	case 2:
		return a->udata16 == b->udata16;
	case 4:
		return a->udata32 == b->udata32;
	case 8:
		return a->udata64 == b->udata64;
	default:
		return 0;
	}
}

static SIZE_T KMP(LPCVOID data, SIZE_T dataLength, SIZE_T idxRdBuffer, LPCVOID rdBuffer, SIZE_T rdBufferLength, LPCVOID pi)
{
	if (rdBufferLength - idxRdBuffer < dataLength)
		return -1;

	PBYTE p = (PBYTE)data;
	PBYTE rd = (PBYTE)rdBuffer;

	SIZE_T i = 0;

	for (i = 0; i < dataLength; ++i)
	{
		if (p[i] != rd[idxRdBuffer + i])
			return idxRdBuffer + i;
	}

	return idxRdBuffer + i;
}

SIZE_T QueryNew(HANDLE hProcess, LPCVOID data, SIZE_T dataLength, LPCVOID addrBuffer, LPCVOID rdBuffer, SIZE_T pageSize)
{
	MEMORY_BASIC_INFORMATION mbi;
	LPCVOID baseAddr = 0;
	SIZE_T matchedCount = 0;

	while (VirtualQueryEx(hProcess, baseAddr, &mbi, sizeof(mbi)) == sizeof(mbi))
	{
		if (mbi.State != MEM_COMMIT ||
			mbi.RegionSize < dataLength)
		{
			baseAddr = (PBYTE)mbi.BaseAddress + mbi.RegionSize;
			continue;
		}

		SIZE_T rdBufferLength;
		ReadProcessMemory(hProcess, mbi.BaseAddress, (LPVOID)rdBuffer, mbi.RegionSize, &rdBufferLength);
		
		SIZE_T i = 0;

		for (i = 0; i < rdBufferLength; i += dataLength)
		{
			if (!Compare((PrimitiveData*)((PBYTE)rdBuffer + i), (PrimitiveData*)data, dataLength))
				continue;

			((PVOID64*)addrBuffer)[matchedCount++] = ((PBYTE)rdBuffer + i);
		}
	}

	return matchedCount;
}

SIZE_T QueryContinue(HANDLE hProcess, LPCVOID data, SIZE_T dataLength, LPCVOID addrBuffer, SIZE_T addrCount, LPCVOID rdBuffer, SIZE_T pageSize)
{
	MEMORY_BASIC_INFORMATION mbi;
	LPCVOID baseAddr = 0;
	SIZE_T matchedCount = 0;

	VirtualQueryEx(hProcess, baseAddr, &mbi, sizeof(mbi));

	for (SIZE_T i = 0; i < addrCount; ++i)
	{
		while ((PBYTE)mbi.BaseAddress + mbi.RegionSize < ((LPCVOID*)addrBuffer)[i])
			VirtualQueryEx(hProcess, ((LPCVOID*)addrBuffer)[i], &mbi, sizeof(mbi));

		SIZE_T rdBufferLength;
		ReadProcessMemory(hProcess, mbi.BaseAddress, (LPVOID)rdBuffer, mbi.RegionSize, &rdBufferLength);

		SIZE_T j = (SIZE_T)((PVOID64*)addrBuffer)[i] - (SIZE_T)mbi.BaseAddress;

		if (!Compare((PrimitiveData*)((PVOID64*)addrBuffer + j), (PrimitiveData*)data, dataLength))
			continue;

		((PVOID64*)addrBuffer)[matchedCount++] = ((PVOID64*)addrBuffer)[i];
	}

	return matchedCount;
}