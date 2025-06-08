#include "query.h"
#include <iostream>

static PBYTE kmp_pi;

static SIZE_T ADR_BUFFER_SIZE = 0;
static SIZE_T ADR_BUFFER_COUNT = 0;
static LPVOID* ADR_BUFFER = NULL;

static SIZE_T MEM_BUFFER_SIZE = 0;
static LPVOID* MEM_BUFFER = NULL;

static SIZE_T AppendBuffer(LPVOID** src, SIZE_T* prevSize, SIZE_T nextSize)
{
	if (nextSize == 0)
		return *prevSize;

	LPVOID* resized = (LPVOID*)realloc(*src, sizeof(LPVOID) * nextSize);
	if (!resized)
		return *prevSize;

	*src = resized;
	*prevSize = nextSize;
	return nextSize;
}

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

SIZE_T QueryNew(HANDLE hProcess, LPCVOID data, SIZE_T dataLength)
{
	MEMORY_BASIC_INFORMATION mbi;
	LPCVOID baseAddr = 0;
	SIZE_T rdBufferLength;
	SIZE_T i;

	ADR_BUFFER_COUNT = 0;
	
	while (VirtualQueryEx(hProcess, baseAddr, &mbi, sizeof(mbi)) == sizeof(mbi))
	{
		if (mbi.State != MEM_COMMIT ||
			mbi.RegionSize < dataLength)
		{
			baseAddr = (PBYTE)mbi.BaseAddress + mbi.RegionSize;
			continue;
		}

		if (MEM_BUFFER_SIZE < mbi.RegionSize)
			AppendBuffer(&MEM_BUFFER, &MEM_BUFFER_SIZE, mbi.RegionSize);

		printf("Query for Base Address #%p (Region Size == %llu)\n", mbi.BaseAddress, mbi.RegionSize);
		ReadProcessMemory(hProcess, mbi.BaseAddress, MEM_BUFFER, mbi.RegionSize, &rdBufferLength);
		
		for (i = 0; i < rdBufferLength; i += dataLength)
		{
			if (!Compare((PrimitiveData*)((PBYTE)MEM_BUFFER + i), (PrimitiveData*)data, dataLength))
				continue;

			if (ADR_BUFFER_COUNT == ADR_BUFFER_SIZE)
				AppendBuffer(&ADR_BUFFER, &ADR_BUFFER_SIZE, ADR_BUFFER_COUNT * 2);

			ADR_BUFFER[ADR_BUFFER_COUNT++] = ((PBYTE)mbi.BaseAddress + i);
		}

		baseAddr = (PBYTE)mbi.BaseAddress + mbi.RegionSize;
	}

	return ADR_BUFFER_COUNT;
}

SIZE_T QueryContinue(HANDLE hProcess, LPCVOID data, SIZE_T dataLength)
{
	MEMORY_BASIC_INFORMATION mbi;
	LPCVOID baseAddr = 0;
	SIZE_T shouldRead = 0;
	SIZE_T rdBufferLength;
	SIZE_T i;
	SIZE_T j;
	SIZE_T n = ADR_BUFFER_COUNT;

	ADR_BUFFER_COUNT = 0;

	VirtualQueryEx(hProcess, baseAddr, &mbi, sizeof(mbi));

	for (SIZE_T i = 0; i < n; ++i)
	{
		while ((PBYTE)mbi.BaseAddress + mbi.RegionSize < ADR_BUFFER[i])
		{
			baseAddr = (PBYTE)mbi.BaseAddress + mbi.RegionSize;
			VirtualQueryEx(hProcess, baseAddr, &mbi, sizeof(mbi));
			shouldRead = 1;
		}

		if (shouldRead != 0)
		{
			ReadProcessMemory(hProcess, mbi.BaseAddress, MEM_BUFFER, mbi.RegionSize, &rdBufferLength);
			shouldRead = 0;
		}

		j = (PBYTE)ADR_BUFFER[i] - mbi.BaseAddress;

		printf("Query for Address #%p\n", ADR_BUFFER[i]);
		if (!Compare((PrimitiveData*)((PBYTE)MEM_BUFFER + j), (PrimitiveData*)data, dataLength))
			continue;

		ADR_BUFFER[ADR_BUFFER_COUNT++] = ADR_BUFFER[i];
	}

	return ADR_BUFFER_COUNT;
}