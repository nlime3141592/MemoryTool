#pragma once

#include "framework.h"

int ReadInt32(HANDLE hProcess, LPCVOID address);
void WriteInt32(HANDLE hProcess, LPVOID address, int value);