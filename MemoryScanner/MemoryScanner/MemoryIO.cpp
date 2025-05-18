#include "MemoryIO.h"

int ReadInt32(HANDLE hProcess, LPCVOID address)
{
    union _buffer
    {
        char buffer[4];
        int value;
    } uBuffer;

    ReadProcessMemory(hProcess, address, uBuffer.buffer, 4, nullptr);

    return uBuffer.value;
}

void WriteInt32(HANDLE hProcess, LPVOID address, int value)
{
    union _buffer
    {
        char buffer[4];
        int value;
    } uBuffer;

    uBuffer.value = value;

    WriteProcessMemory(hProcess, address, uBuffer.buffer, 4, nullptr);
}
