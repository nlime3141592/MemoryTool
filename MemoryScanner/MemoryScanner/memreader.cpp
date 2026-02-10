#include "memreader.h"

#include <assert.h>

static constexpr SIZE_T DEFAULT_SYSTEM_PAGE_SIZE = 4096;

MemoryReader::MemoryReader() :
	_systemPageSize(DEFAULT_SYSTEM_PAGE_SIZE),
	_maxPatternLength(DEFAULT_SYSTEM_PAGE_SIZE),
	_pageCount(0),
	_mbi(),
	_mbiSize(sizeof(_mbi)),
	_pageSequenceLength(0),
	_pageSequence(nullptr),
	_pageIndex(nullptr),
	_pageData(nullptr),
	_baseAddress(0),
	_limitAddress(0)
{
	SetMaxPatternLength(DEFAULT_SYSTEM_PAGE_SIZE);
}

MemoryReader::~MemoryReader()
{
	FreePages();
}

void MemoryReader::SetMaxPatternLength(SIZE_T length)
{
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);

	assert(length > 0);

	DWORD pageCount = (length - 1) / sysinfo.dwPageSize + 1;

	// Memory allocation failed.
	if ((_systemPageSize != sysinfo.dwPageSize || _pageCount != pageCount) &&
		!TryReallocPages(sysinfo.dwPageSize, pageCount))
	{
		FreePages();

		_systemPageSize = DEFAULT_SYSTEM_PAGE_SIZE;
		_maxPatternLength = DEFAULT_SYSTEM_PAGE_SIZE;
		_pageCount = 0;

		return;
	}

	assert(_pageSequence != nullptr);
	assert(_pageIndex != nullptr);
	assert(_pageData != nullptr);

	_systemPageSize = sysinfo.dwPageSize;
	_maxPatternLength = length;
	_pageCount = pageCount;
}

void MemoryReader::SetProcessHandle(HANDLE hProcess)
{
	_hProcess = hProcess;
}

void MemoryReader::Reset()
{
	_pageSequenceOffset = 0;
	_pageSequenceLength = 0;
	_baseAddress = (PBYTE)0;
	_limitAddress = (PBYTE)0;
}

bool MemoryReader::TryGetLatestPage(BYTE** ptrPage) const
{
	if (_pageSequenceLength > 0)
	{
		*ptrPage = _pageSequence[_pageSequenceLength - 1];
		return true;
	}
	else
	{
		*ptrPage = nullptr;
		return false;
	}
}

bool MemoryReader::Compare(BYTE* source, DWORD length) const
{
	assert(source != nullptr);
	assert(length <= _maxPatternLength);

	DWORD readAvailableCount = _pageSequenceLength * _systemPageSize - _pageSequenceOffset;

	if (readAvailableCount < length)
	{
		return false;
	}

	for (DWORD i = 0; i < length; ++i)
	{
		DWORD j = i + _pageSequenceOffset;
		DWORD p = j / _systemPageSize;
		DWORD o = j % _systemPageSize;

		BYTE b0 = source[i];
		BYTE b1 = _pageSequence[p][o];

		if (b0 != b1)
		{
			return false;
		}
	}

	return true;
}

DWORD MemoryReader::TryRead(DWORD byteAmount)
{
	if (!CanRead())
	{
		return 0;
	}
	if (_limitAddress == 0)
	{
		TryQueryNextPage(_hProcess);
	}

	DWORD offset = _pageSequenceOffset + byteAmount;
	DWORD rdLength = 0;

	while (offset >= _systemPageSize)
	{
		if (TryQueryNextPage(_hProcess))
		{
			rdLength += _systemPageSize;
			offset -= _systemPageSize;
			continue;
		}

		break;
	}

	if (offset < _systemPageSize)
	{
		_pageSequenceOffset = offset;
		return byteAmount;
	}

	while (offset >= _systemPageSize && _pageSequenceLength > 0)
	{
		rdLength += _systemPageSize;
		offset -= _systemPageSize;
		_pageSequenceLength -= 1;
	}

	if (_pageSequenceLength > 0)
	{
		rdLength += offset;
		_pageSequenceOffset = offset;
		return rdLength;
	}
	else
	{
		rdLength -= (offset) % _systemPageSize;
		_pageSequenceOffset = -1;
		return rdLength;
	}
}

//DWORD MemoryReader::TryRead(DWORD byteAmount)
//{
//	DWORD offset = _pageSequenceOffset + byteAmount;
//	DWORD rdLength = 0;
//	
//	while (offset >= _systemPageSize)
//	{
//		offset -= _systemPageSize;
//		rdLength += _systemPageSize;
//		
//		if (TryQueryNextPage(_hProcess))
//			continue;
//		if (_pageSequenceLength > 0)
//			--_pageSequenceLength;
//		if (_pageSequenceLength <= 0)
//			break;
//	}
//
//	if (_pageSequenceLength <= 0 || offset < 0 || offset >= _systemPageSize)
//	{
//		_pageSequenceOffset = -1;
//		//return false;
//
//		rdLength += (_systemPageSize - _pageSequenceOffset);
//	}
//	else
//	{
//		_pageSequenceOffset = offset;
//		//return true;
//
//		return byteAmount;
//	}
//}

DWORD64 MemoryReader::GetRegionBaseAddress() const
{
	return (DWORD64)(_mbi.BaseAddress);
}

DWORD64 MemoryReader::GetRegionSize() const
{
	return (DWORD64)(_mbi.RegionSize);
}

DWORD64 MemoryReader::GetPageBaseAddress() const
{
	return (DWORD64)_baseAddress;
}

DWORD64 MemoryReader::GetPageSize() const
{
	return (DWORD64)_systemPageSize;
}

bool MemoryReader::CanRead() const
{
	return _pageSequenceOffset >= 0;
}

bool MemoryReader::TryReallocPages(DWORD pageSize, DWORD pageCount)
{
	BYTE* pData = (BYTE*)realloc(_pageData, sizeof(BYTE) * pageSize * pageCount);
	BYTE** pIndex = (BYTE**)realloc(_pageIndex, sizeof(BYTE*) * pageSize);
	BYTE** pSequence = (BYTE**)realloc(_pageSequence, sizeof(BYTE*) * pageSize);

	_pageData = pData;
	_pageIndex = pIndex;
	_pageSequence = pSequence;

	if (pData == nullptr || pIndex == nullptr || pSequence == nullptr)
	{
		FreePages();

		return false;
	}

	for (DWORD i = 0; i < pageCount; ++i)
	{
		BYTE* address = pData + i * pageSize;

		pIndex[i] = address;
		pSequence[i] = address;
	}

	return true;
}

void MemoryReader::FreePages()
{
	free(_pageSequence);
	free(_pageIndex);
	free(_pageData);

	_pageSequence = nullptr;
	_pageIndex = nullptr;
	_pageData = nullptr;
}

bool MemoryReader::TryQueryNextPage(HANDLE hProcess)
{
	PBYTE baseAddress = _baseAddress;
	PBYTE limitAddress = _limitAddress;
	bool foundNewRegion = false;

	// Query memory region from process memory space.
	while (baseAddress >= limitAddress && VirtualQueryEx(hProcess, baseAddress, &_mbi, _mbiSize) == _mbiSize)
	{
		if (_mbi.State != MEM_COMMIT ||
			(_mbi.Protect & PAGE_GUARD) ||
			(_mbi.Protect & PAGE_NOACCESS))
		{
			limitAddress = (PBYTE)_mbi.BaseAddress + _mbi.RegionSize;
			baseAddress = limitAddress;
			continue;
		}
		else
		{
			limitAddress = (PBYTE)_mbi.BaseAddress + _mbi.RegionSize;
			baseAddress = (PBYTE)_mbi.BaseAddress;
			foundNewRegion = true;
			break;
		}
	}

	// End of process memory space.
	if (!foundNewRegion && baseAddress >= limitAddress)
	{
		_baseAddress = limitAddress;
		_limitAddress = limitAddress;
		return false;
	}

	// Read next page from memory region.
	BYTE* pageData = _pageSequence[_pageSequenceLength];

	for (int i = _pageCount - 2; i >= 0; --i)
	{
		_pageSequence[i + 1] = _pageSequence[i];
	}

	_pageSequence[0] = pageData;

	if (_pageSequenceLength < _pageCount)
	{
		++_pageSequenceLength;
	}

	SIZE_T cbRead;
	BOOL rdResult = ReadProcessMemory(hProcess, baseAddress, pageData, _systemPageSize, &cbRead);

	//assert(rdResult && (cbRead == _systemPageSize));
	//assert(rdResult);

	_baseAddress = baseAddress + _systemPageSize;
	_limitAddress = limitAddress;

	return true;
}

void MemoryReader::ShiftSequenceCircularLeft(DWORD length)
{
	BYTE* tmp = _pageSequence[0];
	int j = length - 1;

	for (int i = 0; i < j; ++i)
	{
		_pageSequence[i] = _pageSequence[i + 1];
	}

	_pageSequence[j] = 0;
}