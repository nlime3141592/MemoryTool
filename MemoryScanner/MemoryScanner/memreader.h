#pragma once

#include <Windows.h>

class MemoryReader
{
public:
	MemoryReader();
	~MemoryReader();

	void SetMaxPatternLength(SIZE_T length);
	void SetProcessHandle(HANDLE hProcess);
	void Reset();

	bool TryGetLatestPage(BYTE** ptrPage) const;
	bool Compare(BYTE* source, DWORD length) const;
	DWORD TryRead(DWORD byteAmount);

	DWORD64 GetRegionBaseAddress() const;
	DWORD64 GetRegionSize() const;
	DWORD64 GetPageBaseAddress() const;
	DWORD64 GetPageSize() const;

	bool CanRead() const;

private:
	DWORD _systemPageSize;

	SIZE_T _maxPatternLength;
	DWORD _pageCount;

	HANDLE _hProcess;

	MEMORY_BASIC_INFORMATION _mbi;
	DWORD _mbiSize;
	INT32 _pageSequenceLength;
	INT32 _pageSequenceOffset;
	BYTE** _pageSequence;
	BYTE** _pageIndex;
	BYTE* _pageData;
	PBYTE _baseAddress;
	PBYTE _limitAddress;

	bool TryReallocPages(DWORD pageSize, DWORD pageCount);
	void FreePages();

	bool TryQueryNextPage(HANDLE hProcess);

	void ShiftSequenceCircularLeft(DWORD length);
};