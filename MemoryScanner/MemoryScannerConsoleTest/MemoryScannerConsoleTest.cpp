// MemoryScannerConsoleTest.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <psapi.h>
#include <iostream>

#define ALLOC_SIZE 0x40000000
#define TEST_LPCNT 15

PVOID64* QUERY_ADDRESS;
PVOID64* RD_BUFFER;

void VQueryEX(int pid);

typedef union _ScanData
{
	uint8_t bytes[8];

	int8_t data8;
	int16_t data16;
	int32_t data32;
	int64_t data64;
	uint8_t udata8;
	uint16_t udata16;
	uint32_t udata32;
	uint64_t udata64;
} ScanData;

void toBigEndian(ScanData* data, int size)
{
	int half = size / 2;

	for (int i = 0; i < half; ++i)
	{
		int j = size - i - 1;

		int t = data->bytes[i];
		data->bytes[i] = data->bytes[j];
		data->bytes[j] = t;
	}
}

int input(void* data, int size, int sign)
{
	int usize = size;

	if (sign != 0)
		size = -size;

	ScanData* pdat = (ScanData*)data;

	switch (size)
	{
	case 1:
		return scanf("%hhu", &pdat->udata8);
	case 2:
		return scanf("%hu", &pdat->udata16);
	case 4:
		return scanf("%u", &pdat->udata32);
	case 8:
		return scanf("%llu", &pdat->udata64);
	case -1:
		return scanf("%hhd", &pdat->data8);
	case -2:
		return scanf("%hd", &pdat->data16);
	case -4:
		return scanf("%d", &pdat->data32);
	case -8:
		return scanf("%lld", &pdat->data64);
	default:
		int wrLength = 0;
		PBYTE* hexdat = (PBYTE*)data;
		for (int i = 0; i < size; ++i)
		{
			wrLength += scanf("%hhx", hexdat + i);
		}
		return wrLength;
	}

	return 0;
}

SIZE_T queryNew(HANDLE hProcess, ScanData* data, int size)
{
	MEMORY_BASIC_INFORMATION mbi;
	LPCVOID address = 0;
	SIZE_T length = 0;

	while (VirtualQueryEx(hProcess, address, &mbi, sizeof(mbi)) == sizeof(mbi))
	{
		if (mbi.State != MEM_COMMIT)
		{
			address = (PBYTE)mbi.BaseAddress + mbi.RegionSize;
			continue;
		}

		ScanData source;

		printf("Query for Base Address #%p (Region Size == %llu)\n", mbi.BaseAddress, mbi.RegionSize);
		
		for (SIZE_T i = 0; i < mbi.RegionSize; i += size)
		{
			LPCVOID addrSource = (PVOID64)((SIZE_T)mbi.BaseAddress + i);
			ReadProcessMemory(hProcess, addrSource, source.bytes, size, NULL);

			switch (size)
			{
			case 1:
				if (source.udata8 == data->udata8)
					QUERY_ADDRESS[length++] = (PVOID64)addrSource;
				break;
			case 2:
				if (source.udata16 == data->udata16)
					QUERY_ADDRESS[length++] = (PVOID64)addrSource;
				break;
			case 4:
				if (source.udata32 == data->udata32)
					QUERY_ADDRESS[length++] = (PVOID64)addrSource;
				break;
			case 8:
				if (source.udata64 == data->udata64)
					QUERY_ADDRESS[length++] = (PVOID64)addrSource;
				break;
			default:
				break;
			}
		}

		address = (PBYTE)mbi.BaseAddress + mbi.RegionSize;
	}

	return length;
}

SIZE_T queryContinue(HANDLE hProcess, ScanData* data, int size, SIZE_T length)
{
	MEMORY_BASIC_INFORMATION mbi;
	LPCVOID address = 0;
	SIZE_T newLength = 0;
	ScanData source;

	for (SIZE_T i = 0; i < length; ++i)
	{
		ReadProcessMemory(hProcess, QUERY_ADDRESS[i], source.bytes, size, NULL);

		switch (size)
		{
		case 1:
			if (source.udata8 == data->udata8)
				QUERY_ADDRESS[newLength++] = QUERY_ADDRESS[i];
			break;
		case 2:
			if (source.udata16 == data->udata16)
				QUERY_ADDRESS[newLength++] = QUERY_ADDRESS[i];
			break;
		case 4:
			if (source.udata32 == data->udata32)
				QUERY_ADDRESS[newLength++] = QUERY_ADDRESS[i];
			break;
		case 8:
			if (source.udata64 == data->udata64)
				QUERY_ADDRESS[newLength++] = QUERY_ADDRESS[i];
			break;
		default:
			break;
		}
	}

	return newLength;
}

void printQueryAddresses(SIZE_T length)
{
	for (SIZE_T i = 0; i < length; ++i)
	{
		printf("ADDR == %016p\n", QUERY_ADDRESS[i]);
	}
}

void ProcMemInfo(int pid);

#include "query.h"
#include "scan.h"

int main()
{
	int pid = 16300;
	int size = 0;
	int sign = 0;
	int endian = 0;
	char cmd = '\0';

	printf("Enter the PID: ");
	scanf("%d", &pid);

	printf("Enter the Data Size (1, 2, 4, 8): ");
	scanf("%d", &size);

	printf("Enter the Sign (0 == unsigned, 1 == signed): ");
	scanf("%d", &sign);

	printf("Enter the Endian (0 == little, 1 == big): ");
	scanf("%d", &endian);

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

	PrimitiveData minData;
	PrimitiveData maxData;

	PrimitiveData* ptrMin;
	PrimitiveData* ptrMax;

	LPCVOID address;
	SIZE_T length;

	while (cmd != 'q' && cmd != 'Q')
	{
		printf("Enter the Commands::\n");
		printf("  S : New Scan\n");
		printf("  s : Scan Continue\n");
		printf("  m : Modify Memory\n");
		printf("  q, Q: Quit\n");

		scanf("%c", &cmd);

		switch (cmd)
		{
		case 'S':
			ptrMin = NULL;
			ptrMax = NULL;
			printf("Enter the Min Value: ");
			if (input((ScanData*)(&minData), size, sign))
				ptrMin = &minData;
			printf("Enter the Max Value: ");
			if (input((ScanData*)(&maxData), size, sign))
				ptrMax = &maxData;
			length = ScanIntegerNewly(hProcess, ptrMin, ptrMax, size, endian, sign, TRUE, TRUE, TRUE);
			PrintAddresses();
			printf("Qnery New: %llu\n", length);
			break;
		case 's':
			ptrMin = NULL;
			ptrMax = NULL;
			printf("Enter the Min Value: ");
			if (input((ScanData*)(&minData), size, sign))
				ptrMin = &minData;
			printf("Enter the Max Value: ");
			if (input((ScanData*)(&maxData), size, sign))
				ptrMax = &maxData;
			length = ScanIntegerContinuously(hProcess, ptrMin, ptrMax, size, endian, sign, TRUE, TRUE, TRUE);
			PrintAddresses();
			printf("Qnery Continue: %llu\n", length);
			break;
		case 'm':
			printf("Enter the Memory Address: ");
			scanf("%llx", &address);
			printf("Enter the Value: ");
			input((ScanData*)(&minData), size, sign);
			WriteProcessMemory(hProcess, (LPVOID)address, (LPCVOID)minData.bytes, size, NULL);
			printf("Modifying OK\n");
		default:
			break;
		}
	}

	free(memBuffer);
	free(adrBuffer);
	return 0;
}

int main2()
{
	//int pid = 24024;

	//SYSTEM_INFO sysinfo;
	//GetSystemInfo(&sysinfo);
	//printf("dwPageSize == %d\n", sysinfo.dwPageSize);

	/*QUERY_ADDRESS = (PVOID64*)malloc(sizeof(PVOID64) * ALLOC_SIZE);
	RD_BUFFER = (PVOID64*)malloc(sizeof(PVOID64) * ALLOC_SIZE);*/

	int pid = 0;
	int size = 0;
	int sign = 0;
	int endian = 0;
	char cmd = '\0';

	printf("Enter the PID: ");
	scanf("%d", &pid);

	printf("Enter the Data Size (1, 2, 4, 8): ");
	scanf("%d", &size);

	printf("Enter the Sign (0 == positive, 1 == negative): ");
	scanf("%d", &endian);

	printf("Enter the Endian (0 == little, 1 == big): ");
	scanf("%d", &endian);

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

	PrimitiveData data;

	LPCVOID address;
	SIZE_T length;

	while (cmd != 'q' && cmd != 'Q')
	{
		printf("Enter the Commands::\n");
		printf("  S : New Scan\n");
		printf("  s : Scan Continue\n");
		printf("  m : Modify Memory\n");
		printf("  q, Q: Quit\n");

		scanf("%c", &cmd);

		switch (cmd)
		{
		case 'S':
			printf("Enter the Value: ");
			input((ScanData*)(&data), size, sign);
			length = QueryNew(hProcess, (ScanData*)(&data), size);
			//length = queryNew(hProcess, (ScanData*)(&data), size);
			printQueryAddresses(length);
			printf("Qnery New: %llu\n", length);
			break;
		case 's':
			printf("Enter the Value: ");
			input((ScanData*)(&data), size, sign);
			length = QueryContinue(hProcess, (ScanData*)(&data), size);
			//length = queryContinue(hProcess, (ScanData*)(&data), size, length);
			printQueryAddresses(length);
			printf("Qnery Continue: %llu\n", length);
			break;
		case 'm':
			printf("Enter the Memory Address: ");
			scanf("%llx", &address);
			printf("Enter the Value: ");
			input((ScanData*)(&data), size, sign);
			WriteProcessMemory(hProcess, (LPVOID)address, (LPCVOID)data.bytes, size, NULL);
			printf("Modifying OK\n");
		default:
			break;
		}
	}

	free(QUERY_ADDRESS);
	free(RD_BUFFER);
	return 0;
}

int main1()
{
	QUERY_ADDRESS = (PVOID64*)malloc(sizeof(PVOID64) * ALLOC_SIZE);

	int pid = 0;
	int size = 0;
	int sign = 0;
	int endian = 0;
	char cmd = '\0';

	printf("Enter the PID: ");
	scanf("%d", &pid);

	printf("Enter the Data Size (1, 2, 4, 8): ");
	scanf("%d", &size);

	printf("Enter the Sign (0 == positive, 1 == negative): ");
	scanf("%d", &endian);

	printf("Enter the Endian (0 == little, 1 == big): ");
	scanf("%d", &endian);

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	MEMORY_BASIC_INFORMATION mbi;
	
	ScanData data;

	LPCVOID address;
	SIZE_T length;

	while (cmd != 'q' && cmd != 'Q')
	{
		printf("Enter the Commands::\n");
		printf("  S : New Scan\n");
		printf("  s : Scan Continue\n");
		printf("  m : Modify Memory\n");
		printf("  q, Q: Quit\n");

		scanf("%c", &cmd);

		switch (cmd)
		{
		case 'S':
			printf("Enter the Value: ");
			input(&data, size, sign);
			length = queryNew(hProcess, &data, size);
			printQueryAddresses(length);
			printf("Qnery New: %llu\n", length);
			break;
		case 's':
			printf("Enter the Value: ");
			input(&data, size, sign);
			length = queryContinue(hProcess, &data, size, length);
			printQueryAddresses(length);
			printf("Qnery Continue: %llu\n", length);
			break;
		case 'm':
			printf("Enter the Memory Address: ");
			scanf("%llx", &address);
			printf("Enter the Value: ");
			input(&data, size, sign);
			WriteProcessMemory(hProcess, (LPVOID)address, (LPCVOID)data.bytes, size, NULL);
			printf("Modifying OK\n");
		default:
			break;
		}
	}

	return 0;
}

// 프로그램 실행: <Ctrl+F5> 또는 [디버그] > [디버깅하지 않고 시작] 메뉴
// 프로그램 디버그: <F5> 키 또는 [디버그] > [디버깅 시작] 메뉴

// 시작을 위한 팁: 
//   1. [솔루션 탐색기] 창을 사용하여 파일을 추가/관리합니다.
//   2. [팀 탐색기] 창을 사용하여 소스 제어에 연결합니다.
//   3. [출력] 창을 사용하여 빌드 출력 및 기타 메시지를 확인합니다.
//   4. [오류 목록] 창을 사용하여 오류를 봅니다.
//   5. [프로젝트] > [새 항목 추가]로 이동하여 새 코드 파일을 만들거나, [프로젝트] > [기존 항목 추가]로 이동하여 기존 코드 파일을 프로젝트에 추가합니다.
//   6. 나중에 이 프로젝트를 다시 열려면 [파일] > [열기] > [프로젝트]로 이동하고 .sln 파일을 선택합니다.
