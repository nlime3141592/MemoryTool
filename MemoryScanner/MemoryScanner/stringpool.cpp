#include "stringpool.h"

StringPool::StringPool()
{
	_rawDataCount = 0;
	_rawDataCapacity = 65536;
	_rawDataPool = (LPWSTR)malloc(sizeof(WCHAR) * _rawDataCapacity);

	_offsetCount = 0;
	_offsetCapacity = 1024;
	_offsetPool = (DWORD*)malloc(sizeof(DWORD) * _offsetCapacity);
}

StringPool::~StringPool()
{
	_rawDataCount = 0;
	_rawDataCapacity = 0;

	_offsetCount = 0;
	_offsetCapacity = 0;

	free((void*)_rawDataPool);
	free((void*)_offsetPool);
}

void StringPool::Clear()
{
	_rawDataCount = 0;
	_offsetCount = 0;
}

DWORD StringPool::Push(LPCWSTR src, DWORD size)
{
	DWORD index = _rawDataCount;

	if (_rawDataCount + size + 1 >= _rawDataCapacity)
	{
		DWORD nextCapacity = _rawDataCapacity * 2;

		_rawDataPool = Append(_rawDataPool, _rawDataCapacity, nextCapacity);
		_rawDataCapacity = nextCapacity;
	}

	_rawDataPool[index] = (WCHAR)size;
	memcpy(_rawDataPool + index + 1, src, size * sizeof(WCHAR));
	_rawDataCount += size + 1;

	if (_offsetCount >= _offsetCapacity)
	{
		DWORD nextCapacity = _offsetCapacity * 2;

		_offsetPool = Append(_offsetPool, _offsetCapacity, nextCapacity);
		_offsetCapacity = nextCapacity;
	}

	_offsetPool[_offsetCount] = index;
	++_offsetCount;

	return index;
}

DWORD StringPool::TrySearch(LPWSTR dst, DWORD index)
{
	DWORD i = 0;
	DWORD j = _offsetCount - 1;
	DWORD m = 0;
	DWORD r = 0;

	while (i <= j && r == 0)
	{
		DWORD m = (i + j) / 2;

		if (index < _offsetPool[m])
			j = m - 1;
		else if (index > _offsetPool[m])
			i = m + 1;
		else
			r = 1;
	}

	if (r != 0)
	{
		r = (DWORD)_rawDataPool[index];
		memcpy(dst, &_rawDataPool[index + 1], r * sizeof(WCHAR));
	}

	dst[r] = 0;

	return r;
}

template<typename T>
T* StringPool::Append(const T* array, DWORD capacityCurrent, DWORD capacityNext)
{
	DWORD s = sizeof(T);
	DWORD sc = s * capacityCurrent;
	DWORD sn = s * capacityNext;

	T* newArray = (T*)malloc(sn);

	if (newArray != 0)
	{
		memcpy(newArray, array, sc);
		free((void*)array);
	}

	return newArray;
}