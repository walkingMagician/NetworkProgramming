#include "FormatLastError.h"


LPSTR FormatLastError(DWORD dwMessageID)
{
	LPSTR szBuffer = NULL;
	FormatMessage
	(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dwMessageID,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&szBuffer,
		1024,
		NULL
	);
	return szBuffer;
}

void PrintLastEroor(DWORD dwMessageID)
{
	LPSTR szMessage = FormatLastError(dwMessageID);
	printf("Error %i: %s", dwMessageID, szMessage);
	LocalFree(szMessage);
}