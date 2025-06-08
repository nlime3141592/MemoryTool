#pragma once

#include <windows.h>

#include "query.h"

#define ENDIAN_LITTLE 0
#define ENDIAN_BIG 1

#define SIGN_UNSIGNED 0
#define SIGN_SIGNED 1

static SIZE_T adrBufferSize = 0;
static SIZE_T adrBufferCount = 0;
static LPVOID* adrBuffer = NULL;

static SIZE_T memBufferSize = 0;
static LPVOID* memBuffer = NULL;

//typedef union _PrimitiveData
//{
//	UINT8 bytes[8];
//	INT8 data8;
//	INT16 data16;
//	INT32 data32;
//	INT64 data64;
//	UINT8 udata8;
//	UINT16 udata16;
//	UINT32 udata32;
//	UINT64 udata64;
//	FLOAT float32;
//	DOUBLE float64;
//} PrimitiveData;

//static SIZE_T AppendBuffer(void** src, SIZE_T prevSize, SIZE_T nextSize);
static SIZE_T AppendBuffer(LPVOID** src, SIZE_T* prevSize, SIZE_T nextSize);
static int CompareInteger(void* data1, void* data2, SIZE_T dataLength, UINT8 endianness, UINT8 sign);
static int CompareFloat(PrimitiveData* data1, PrimitiveData* data2, SIZE_T dataLength);
static void ChangeEndian(void* data, SIZE_T dataLength);

void PrintAddresses();

SIZE_T ScanIntegerNewly(
	HANDLE hProcess,
	LPCVOID minData, LPCVOID maxData,
	SIZE_T dataLength,
	UINT8 endianness,
	UINT8 sign,
	BOOL minInclusive, BOOL maxInclusive,
	BOOL useMemoryAlignment
);

SIZE_T ScanIntegerContinuously(
	HANDLE hProcess,
	LPCVOID minData, LPCVOID maxData,
	SIZE_T dataLength,
	UINT8 endianness,
	UINT8 sign,
	BOOL minInclusive, BOOL maxInclusive,
	BOOL useMemoryAlignment
);

SIZE_T ScanFloatNewly(
	HANDLE hProcess,
	LPCVOID minData, LPCVOID maxData,
	SIZE_T dataLength,
	UINT8 endianness,
	UINT8 sign,
	BOOL minInclusive, BOOL maxInclusive
);

SIZE_T ScanFloatContinuously(
	HANDLE hProcess,
	LPCVOID minData, LPCVOID maxData,
	SIZE_T dataLength,
	UINT8 endianness,
	UINT8 sign,
	BOOL minInclusive, BOOL maxInclusive
);