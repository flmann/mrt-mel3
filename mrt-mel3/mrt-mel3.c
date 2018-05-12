// MEL3.c: Definiert den Einstiegspunkt für die Konsolenanwendung.
//

#include "stdafx.h"
#include "mrt-mel3.h"

int main()
{
	printf("Started\n");
	//https://stackoverflow.com/questions/2230758/what-does-lpcwstr-stand-for-and-how-should-it-be-handled-with?utm_medium=organic&utm_source=google_rich_qa&utm_campaign=google_rich_qa
	LPCWSTR gszPort = L"COM5";

	BOOL success = FALSE;

	// Opening
	HANDLE hComm;
	hComm = CreateFile(gszPort,
		GENERIC_READ | GENERIC_WRITE,
		NULL,
		NULL,
		OPEN_EXISTING,
		NULL,
		NULL);
	if (hComm == INVALID_HANDLE_VALUE) {
		printf("ERROR OPENING\n"); //TODO handle the error better + error message
	}

	// Checking Status
	printf("Handle is: %d\n", hComm);

	DCB dcb;
	SecureZeroMemory(&dcb, sizeof(DCB));

	success = GetCommState(hComm, &dcb);

	if (!success)
	{
		//  Handle the error.
		printf("GetCommState failed with error %d.\n", GetLastError());
		return (2);
	}

	PrintCommState(dcb);

	// Sending

	BYTE data = 0x44; // 44 in Hex is the char 'D'
	DWORD actSize = 1;

	success = WriteFile(hComm, &data, 1, &actSize, NULL);

	if (!success) {
		printf("ERROR WRITING, %d\n", GetLastError());
	}

	if (sizeof(data) != actSize) {
		printf("SIZE IS NOT EQUAL: Sent %d, instead of %d\n", actSize, sizeof(data)); //TODO handle the error better + error message
	}
	// Receiving
	char rb[14];
	DWORD 
	success = ReadFile(hComm, rb, 5, &actSize, NULL);

	printf("Read: %s\n", rb);
	// Closing
	success = CloseHandle(hComm);
	
	printf("Done\n");
	return 0;
}

void PrintCommState(DCB dcb)
{
	//  Print some of the DCB structure values
	_tprintf(TEXT("\nBaudRate = %d, ByteSize = %d, Parity = %d, StopBits = %d\n"),
		dcb.BaudRate,
		dcb.ByteSize,
		dcb.Parity,
		dcb.StopBits);
}


