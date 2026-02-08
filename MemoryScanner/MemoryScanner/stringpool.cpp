#include "stringpool.h"

StringPool::StringPool()
{
	_rawDataCount = 0;
	_rawDataCapacity = 65536;
	_rawDataPool = (TCHAR*)malloc(sizeof(TCHAR) * _rawDataCapacity);

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

	free(_rawDataPool);
	free(_offsetPool);
}

DWORD StringPool::Push(const TCHAR* src, DWORD size)
{
	DWORD index = _rawDataCount;

	if (_rawDataCount + size + 1 >= _rawDataCapacity)
	{
		_rawDataPool = Append(_rawDataPool, _rawDataCapacity, _rawDataCapacity * 2);
	}

	_rawDataPool[index] = (TCHAR)size;
	memcpy(_rawDataPool + 1, src, size);
	_rawDataCount = size + 1;

	if (_offsetCount >= _offsetCapacity)
	{
		_offsetPool = Append(_offsetPool, _offsetCapacity, _offsetCapacity * 2);
	}

	_offsetPool[_offsetCount] = index;
	++_offsetCount;

	return index;
}

DWORD StringPool::TrySearch(TCHAR* dst, DWORD index)
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
		DWORD length = (DWORD)_rawDataPool[index];
		memcpy(dst, &_rawDataPool[index + 1], length);
	}

	return r;
}

template<typename T>
T* StringPool::Append(const T* array, DWORD capacityCurrent, DWORD capacityNext)
{
	DWORD s = sizeof(T);
	DWORD sc = s * capacityCurrent;
	DWORD sn = s * capacityNext;

	T* newArray = (T*)malloc(sc);

	memcpy(newArray, array, sc);
	free(array);

	return newArray;
}