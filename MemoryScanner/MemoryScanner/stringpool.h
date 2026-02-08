#pragma once

#include <Windows.h>
#include <stdlib.h>

class StringPool
{
public:
	StringPool();
	~StringPool();

	void Clear();
	DWORD Push(LPCWSTR src, DWORD size);
	DWORD TrySearch(LPWSTR dst, DWORD index);

private:
	DWORD _rawDataCount;
	DWORD _rawDataCapacity;
	LPWSTR _rawDataPool;

	DWORD _offsetCount;
	DWORD _offsetCapacity;
	DWORD* _offsetPool;

	template<typename T>
	T* Append(const T* array, DWORD capacityCurrent, DWORD capacityNext);
};