#include <assert.h>
#include <stdio.h>

#include "scan.h"

static SIZE_T adrBufferSize = 0;
static SIZE_T adrBufferCount = 0;
LPVOID* adrBuffer = NULL;

static SIZE_T memBufferSize = 0;
static LPVOID* memBuffer = NULL;

static SIZE_T AppendBuffer(void** src, SIZE_T prevSize, SIZE_T nextSize)
{
	if (nextSize == 0)
		nextSize = (prevSize == 0) ? 1 : prevSize * 2;

	void* newAddr;

	if ((newAddr = realloc(*src, sizeof(LPVOID) * nextSize)) == NULL)
	{
		return prevSize;
	}

	*src = newAddr;
	return nextSize;
}

static int CompareInteger(void* data1, void* data2, SIZE_T dataLength, UINT8 endianness, UINT8 sign)
{
	assert(data1 != NULL && data2 != NULL);
	assert(dataLength > 0);

	int iEnd, iBeg, iDelta;

	if (endianness != ENDIAN_LITTLE)
	{
		iBeg = 0;
		iEnd = dataLength;
		iDelta = 1;
	}
	else
	{
		iBeg = dataLength - 1;
		iEnd = -1;
		iDelta = -1;
	}

	UINT8* d1 = (UINT8*)data1;
	UINT8* d2 = (UINT8*)data2;

	if (sign != SIGN_UNSIGNED)
	{
		UINT8 b1 = d1[iBeg] & 0x80;
		UINT8 b2 = d2[iBeg] & 0x80;

		if (b1 > b2)
			return -1;
		else if (b1 < b2)
			return 1;
	}

	for (int i = iBeg; i != iEnd; i += iDelta)
	{
		if (d1[i] < d2[i])
			return -1;
		else if (d1[i] > d2[i])
			return 1;
	}

	return 0;
}

static int CompareFloat(PrimitiveData* data1, PrimitiveData* data2, SIZE_T dataLength)
{
	assert(data1 != NULL && data2 != NULL);
	assert(dataLength == 4 || dataLength == 8);

	switch (dataLength)
	{
	case 4:
		if (data1->float32 < data2->float32)
			return -1;
		else if (data1->float32 > data2->float32)
			return 1;
		return 0;
	case 8:
		if (data1->float64 < data2->float64)
			return -1;
		else if (data1->float64 > data2->float64)
			return 1;
		return 0;
	default:
		assert(FALSE);
		return 0;
	}
}

static void ChangeEndian(void* data, SIZE_T dataLength)
{
	UINT8* d = (UINT8*)data;
	SIZE_T i = 0;
	SIZE_T j = dataLength - 1;
	UINT8 t;

	while (i < j)
	{
		t = d[i];
		d[i] = d[j];
		d[j] = t;
		++i;
		--j;
	}
}

void PrintAddresses()
{
	for (SIZE_T i = 0; i < adrBufferCount; ++i)
	{
		printf("Address Found == %016p\n", adrBuffer[i]);
	}
}

SIZE_T ScanIntegerNewly(
	HANDLE hProcess,
	LPCVOID minData, LPCVOID maxData,
	SIZE_T dataLength,
	UINT8 endianness,
	UINT8 sign,
	BOOL minInclusive, BOOL maxInclusive,
	BOOL useMemoryAlignment
)
{
	assert(hProcess != NULL);
	assert(minData != NULL || maxData != NULL);
	assert(dataLength > 0);

	MEMORY_BASIC_INFORMATION mbi;
	LPCVOID baseAddr = 0;
	SIZE_T rdLength;

	adrBufferCount = 0;

	while (VirtualQueryEx(hProcess, baseAddr, &mbi, sizeof(mbi)) == sizeof(mbi))
	{
		if (mbi.State != MEM_COMMIT ||
			mbi.RegionSize < dataLength ||
			(mbi.Protect & PAGE_GUARD) ||
			(mbi.Protect & PAGE_NOACCESS))
		{
			baseAddr = (PBYTE)mbi.BaseAddress + mbi.RegionSize;
			continue;
		}

		if (memBufferSize < mbi.RegionSize)
			memBufferSize = AppendBuffer((void**)(&memBuffer), memBufferSize, mbi.RegionSize);

		printf("[scan.cpp] Query for Base Address #%p (Region Size == %llu)\n", mbi.BaseAddress, mbi.RegionSize);

		ReadProcessMemory(hProcess, mbi.BaseAddress, memBuffer, mbi.RegionSize, &rdLength);

		// Memory Alignment를 고려한 방식으로 데이터 비교
		// TODO: 기회가 된다면 KMP 알고리즘을 적용해 Memory Alignment를 고려하지 않는 방식의 읽기를 추가할 것.
		for (SIZE_T i = 0; i < rdLength - dataLength; i += dataLength)
		{
			if (minData != NULL && CompareInteger((void*)minData, (void*)((PBYTE)memBuffer + i), dataLength, endianness, sign) >= (minInclusive != FALSE))
				continue;
			if (maxData != NULL && CompareInteger((void*)((PBYTE)memBuffer + i), (void*)maxData, dataLength, endianness, sign) >= (maxInclusive != FALSE))
				continue;
			if (adrBufferCount == adrBufferSize)
				adrBufferSize = AppendBuffer((void**)(&adrBuffer), adrBufferSize, adrBufferSize * 2);

			adrBuffer[adrBufferCount++] = ((PBYTE)mbi.BaseAddress + i);
		}

		baseAddr = (PBYTE)mbi.BaseAddress + mbi.RegionSize;
	}

	return adrBufferCount;
}

SIZE_T ScanIntegerContinuously(
	HANDLE hProcess,
	LPCVOID minData, LPCVOID maxData,
	SIZE_T dataLength,
	UINT8 endianness,
	UINT8 sign,
	BOOL minInclusive, BOOL maxInclusive,
	BOOL useMemoryAlignment
)
{
	MEMORY_BASIC_INFORMATION mbi;
	LPCVOID baseAddr = 0;
	SIZE_T shouldRead = 0;
	SIZE_T rdLength;
	SIZE_T i;
	SIZE_T j;
	SIZE_T n = adrBufferCount;

	adrBufferCount = 0;

	VirtualQueryEx(hProcess, baseAddr, &mbi, sizeof(mbi));

	for (i = 0; i < n; ++i)
	{
		while ((PBYTE)mbi.BaseAddress + mbi.RegionSize < adrBuffer[i])
		{
			baseAddr = (PBYTE)mbi.BaseAddress + mbi.RegionSize;
			VirtualQueryEx(hProcess, baseAddr, &mbi, sizeof(mbi));
			shouldRead = 1;
		}

		if (shouldRead != 0)
		{
			if (memBufferSize < mbi.RegionSize)
				memBufferSize = AppendBuffer((void**)(&memBuffer), memBufferSize, mbi.RegionSize);

			ReadProcessMemory(hProcess, mbi.BaseAddress, memBuffer, mbi.RegionSize, &rdLength);
			shouldRead = 0;
		}

		j = (PBYTE)adrBuffer[i] - mbi.BaseAddress;

		printf("[scan.cpp] Query for Address #%p\n", adrBuffer[i]);

		if (minData != NULL && CompareInteger((void*)minData, (void*)((PBYTE)memBuffer + j), dataLength, endianness, sign) >= (minInclusive != FALSE))
			continue;
		if (maxData != NULL && CompareInteger((void*)((PBYTE)memBuffer + j), (void*)maxData, dataLength, endianness, sign) >= (maxInclusive != FALSE))
			continue;

		adrBuffer[adrBufferCount++] = adrBuffer[i];
	}

	return adrBufferCount;
}

SIZE_T ScanFloatNewly(
	HANDLE hProcess,
	LPCVOID minData, LPCVOID maxData,
	SIZE_T dataLength,
	UINT8 endianness,
	UINT8 sign,
	BOOL minInclusive, BOOL maxInclusive
)
{
	assert(hProcess != NULL);
	assert(minData != NULL || maxData != NULL);
	assert(dataLength == 4 || dataLength == 8);

	MEMORY_BASIC_INFORMATION mbi;
	LPCVOID baseAddr = 0;
	SIZE_T rdLength;

	PrimitiveData* pMin = (PrimitiveData*)minData;
	PrimitiveData* pMax = (PrimitiveData*)maxData;
	PrimitiveData targetData;

	adrBufferCount = 0;

	while (VirtualQueryEx(hProcess, baseAddr, &mbi, sizeof(mbi)) == sizeof(mbi))
	{
		if (mbi.State != MEM_COMMIT)
		{
			baseAddr = (PBYTE)mbi.BaseAddress + mbi.RegionSize;
			continue;
		}

		if (memBufferSize < mbi.RegionSize)
			memBufferSize = AppendBuffer((void**)(&memBuffer), memBufferSize, mbi.RegionSize);

		ReadProcessMemory(hProcess, mbi.BaseAddress, memBuffer, mbi.RegionSize, &rdLength);

		// Memory Alignment를 고려한 방식으로 데이터 비교
		// TODO: 기회가 된다면 KMP 알고리즘을 적용해 Memory Alignment를 고려하지 않는 방식의 읽기를 추가할 것.
		for (SIZE_T i = 0; i < rdLength - dataLength; i += dataLength)
		{
			memcpy(&targetData, (PBYTE*)(memBuffer + i), dataLength);

			if (endianness != ENDIAN_LITTLE)
			{
				ChangeEndian((void*)(&targetData), dataLength);
			}

			if (minData != NULL && CompareFloat((PrimitiveData*)minData, (PrimitiveData*)((PBYTE)memBuffer + i), dataLength) >= (minInclusive != FALSE))
				continue;
			if (maxData != NULL && CompareFloat((PrimitiveData*)((PBYTE)memBuffer + i), (PrimitiveData*)maxData, dataLength) >= (maxInclusive != FALSE))
				continue;
			if (adrBufferCount == adrBufferSize)
				adrBufferSize = AppendBuffer((void**)(&adrBuffer), adrBufferSize, adrBufferSize * 2);

			adrBuffer[adrBufferCount++] = ((PBYTE)mbi.BaseAddress + i);
		}

		baseAddr = (PBYTE)mbi.BaseAddress + mbi.RegionSize;
	}

	return adrBufferCount;
}

int Modify(HANDLE hProcess, LPVOID address, LPCVOID data, SIZE_T dataLength)
{
	return WriteProcessMemory(hProcess, address, data, dataLength, NULL);
}