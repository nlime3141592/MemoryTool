#pragma once

#include <Windows.h>
#include <stdlib.h>

class StringPool
{
public:
	StringPool();
	~StringPool();

	DWORD Push(const TCHAR* src, DWORD size);
	DWORD TrySearch(TCHAR* dst, DWORD index);

private:
	DWORD _rawDataCount;
	DWORD _rawDataCapacity;
	TCHAR* _rawDataPool;

	DWORD _offsetCount;
	DWORD _offsetCapacity;
	DWORD* _offsetPool;

	template<typename T>
	T* Append(const T* array, DWORD capacityCurrent, DWORD capacityNext);
};