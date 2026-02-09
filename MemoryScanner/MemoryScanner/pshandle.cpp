#include "pshandle.h"

ProcessListManager::ProcessListManager() :
	_handleList(HANDLE_LIST_CAPACITY),
	_openProcess(nullptr),
	_pidListCapacity(PID_LIST_CAPACITY)
{
	_pidListBuffer = (DWORD*)malloc(sizeof(DWORD) * PID_LIST_CAPACITY);
}

ProcessListManager::~ProcessListManager()
{
	free((void*)_pidListBuffer);
}

DWORD ProcessListManager::Refresh()
{
	_handleList.clear();

	DWORD bytes;
	DWORD count;

	if (!EnumProcesses(_pidListBuffer, sizeof(DWORD) * PID_LIST_CAPACITY, &bytes))
	{
		return 0;
	}

	count = bytes / sizeof(DWORD);

	for (DWORD i = 0; i < count; ++i)
	{
		if (_pidListBuffer[i] == 0)
			continue;

		ProcessHandle handle;

		handle.pid = _pidListBuffer[i];
		handle.iNamePool = -1;

		HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, handle.pid);

		if (hProcess)
		{
			HMODULE hModule;
			DWORD cbNeeded;
			//DWORD size = MAX_PATH;
			TCHAR szProcessName[MAX_PATH] = L"<unknown>";

			if (EnumProcessModules(hProcess, &hModule, sizeof(hModule), &cbNeeded))
			{
				GetModuleBaseName(hProcess, hModule, szProcessName, sizeof(szProcessName) / sizeof(TCHAR));
			}

			//QueryFullProcessImageName(hProcess, 0, szProcessName, &size);

			handle.iNamePool = _pNamePool.Push(szProcessName, _tcslen(szProcessName));
			CloseHandle(hProcess);
		}

		_handleList.push_back(handle);
	}

	return _handleList.size();
}

bool ProcessListManager::TryOpen(DWORD index)
{
	if (_openProcess != nullptr)
	{
		return false;
	}

	_openProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, _handleList[index].pid);

	return (_openProcess != nullptr);
}

bool ProcessListManager::TryClose()
{
	if (!CloseHandle(_openProcess))
	{
		return false;
	}

	_openProcess = nullptr;
	return true;
}

bool ProcessListManager::TryGetOpenProcess(HANDLE* hProcessPtr) const
{
	if (_openProcess == nullptr)
	{
		return false;
	}

	*hProcessPtr = _openProcess;
	return true;
}

DWORD ProcessListManager::GetPid(DWORD index) const
{
	return _handleList[index].pid;
}

DWORD ProcessListManager::GetProcessCount() const
{
	return _handleList.size();
}

LPCWSTR ProcessListManager::GetProcessName(DWORD index) const
{
	if (_handleList[index].iNamePool == -1)
	{
		return L"<unknown>";
	}

	LPCWSTR name;
	DWORD i = _handleList[index].iNamePool;
	DWORD length = _pNamePool.TrySearch(&name, i);

	return name;
}