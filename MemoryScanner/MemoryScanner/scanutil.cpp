#include "scanutil.h"

int ByteStreamToWideString(WCHAR* buffer, LPCVOID data, SIZE_T dataLength)
{
	int offset = 0;
	PBYTE pdat = (PBYTE)data;

	for (SIZE_T i = 0; i < dataLength; ++i)
	{
		offset += swprintf(buffer + offset, 4, L"%02X", pdat[i]);
	}

	return offset + 1;
}