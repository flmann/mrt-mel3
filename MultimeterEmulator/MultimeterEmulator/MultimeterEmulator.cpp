// MultimeterEmulator.cpp: Definiert den Einstiegspunkt für die Konsolenanwendung.
//


#include "stdafx.h"
#include "MultimeterEmulator.h"

int main()
{
	printf("Started\n");
	//https://stackoverflow.com/questions/2230758/what-does-lpcwstr-stand-for-and-how-should-it-be-handled-with
	LPCWSTR port = L"COM3";

	BOOL success = FALSE;

	// Opening
	HANDLE hComm;
	hComm = CreateFile(port,
		GENERIC_READ | GENERIC_WRITE,
		NULL,
		NULL,
		OPEN_EXISTING,
		NULL,
		NULL);

	if (hComm == INVALID_HANDLE_VALUE) {
		success = FALSE;
		wprintf(L"Error while opening port: %ls, ERRORCODE: %d\n", port, GetLastError());
		return 1;
	}
	wprintf(L"Successfully opened port %ls, Handle is: %d\n", port, hComm);

	// Checking Status
	DCB dcb;
	SecureZeroMemory(&dcb, sizeof(DCB));

	success = GetCommState(hComm, &dcb);

	if (!success)
	{
		//  Handle the error.
		printf("GetCommState failed with error %d.\n", GetLastError());
		return (2);
	}
	// TODO do move checking

	char rb[1];
	DWORD recvSize = 0;
	BYTE D = 0x44; // 44 in Hex is the char 'D'

	// wait for keyboard hit
	while (!_kbhit()) {
		success = ReadFile(hComm, rb, 5, &recvSize, NULL);
		if (!success) {
			wprintf(L"Error while reading, ERRORCODE: %d\n", GetLastError());
			return 3;
		}
		printf("received: %s\n", (char*)rb);

		if (recvSize == 1) {
			if (*(char*)rb == D) {

			}
		}
		Sleep(1000);
	}
	return 0;

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
	//char rb[14];
	success = ReadFile(hComm, rb, 5, &actSize, NULL);

	printf("Read: %s\n", rb);
	// Closing
	success = CloseHandle(hComm);

	printf("Done\n");
	return 0;
}


