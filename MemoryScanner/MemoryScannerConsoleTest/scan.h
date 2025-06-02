#pragma once

#include <windows.h>

typedef union _PrimitiveData
{
	UINT8 bytes[8];
	INT8 data8;
	INT16 data16;
	INT32 data32;
	INT64 data64;
	UINT8 udata8;
	UINT16 udata16;
	UINT32 udata32;
	UINT64 udata64;
	FLOAT float32;
	DOUBLE float64;
} PrimitiveData;

SIZE_T QueryNew(HANDLE hProcess, LPCVOID data, SIZE_T dataLength, LPCVOID addrBuffer, LPCVOID rdBuffer, SIZE_T pageSize);
SIZE_T QueryContinue(HANDLE hProcess, LPCVOID data, SIZE_T dataLength, LPCVOID addrBuffer, SIZE_T addrCount, LPCVOID rdBuffer, SIZE_T pageSize);