#include <windows.h>
#include <psapi.h>
#include <iostream>

void BasicMemInfo()
{
	MEMORY_BASIC_INFORMATION mbi;
	LPCVOID address = 0;

	while (VirtualQuery(address, &mbi, sizeof(mbi)) == sizeof(mbi))
	{
		printf("Base address == %p\n", mbi.BaseAddress);
		printf("Region size == %lld\n", mbi.RegionSize);
		printf("State == %s\n", ((mbi.State == MEM_COMMIT) ? "Committed" : "Reserved/Free"));

		address = (PBYTE)mbi.BaseAddress + mbi.RegionSize;
	}
}