#pragma once

#include <Windows.h>
#include <Psapi.h>

#include <vector>
#include <tchar.h>

#include "stringpool.h"

static constexpr DWORD HANDLE_LIST_CAPACITY = 1024;
static constexpr DWORD PID_LIST_CAPACITY = 8192;

typedef struct _ProcessHandle
{
	DWORD pid;
	DWORD iNamePool;
} ProcessHandle;

class ProcessListManager
{
public:
	ProcessListManager();
	~ProcessListManager();

	DWORD Refresh();
	bool TryOpen(DWORD index);
	bool TryClose();

	bool TryGetOpenProcess(HANDLE* hProcessPtr) const;
	DWORD GetPid(DWORD index) const;
	DWORD GetProcessCount() const;
	LPCWSTR GetProcessName(DWORD index) const;

private:
	std::vector<ProcessHandle> _handleList;
	StringPool _pNamePool;
	HANDLE _openProcess;

	DWORD _pidListCapacity;
	DWORD* _pidListBuffer;
};