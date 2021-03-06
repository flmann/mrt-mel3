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
	
	// Setting Multimeter Commstate
	DCB dcbInit = { .BaudRate = CBR_1200,.Parity = NOPARITY,.StopBits = TWOSTOPBITS,.ByteSize = 7 };
	success = SetCommState(hComm, &dcbInit);
	if (!success) {
		printf("Error while setting commState, Error code: %d\n", GetLastError());
	}
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
		success = ReadFile(hComm, rb, 1, &recvSize, NULL);
		if (!success) {
			wprintf(L"Error while reading, ERRORCODE: %d\n", GetLastError());
			break;
		}
		printf("received: %s with size %d\n", rb, recvSize);
		if (*(char*)rb != D) {
			printf("WARNING. skipping loop");
			continue;
		}
	
		DWORD actSize = 0;
		DWORD size = 0;
		void *s = malloc(sizeof(char) * (15 + 1));

		LPVOID pData = s;
		buildDVMString(pData, &size);

		success = WriteFile(hComm, pData, size, &actSize, NULL);
		if (!success) {
			printf("Error while writing, ERRORCODE: %d\n", GetLastError());
		}
		printf("sent: %.*s\n", size, pData);
		free(s);
		Sleep(1000);
	}
	
	success = CloseHandle(hComm);

	// Closing

	printf("Done\n");
	return 0;
}


int buildDVMString(LPVOID buffer, LPDWORD len) {
	
	char s[LENGTH];
	s[0] = 'R';
	s[1] = 'E';
	s[2] = 'S';
	s[3] = ' ';
	s[4] = rand() % 10 + 48;
	s[5] = rand() % 10 + 48;
	s[6] = '.';
	s[7] = rand() % 10 + 48;
	s[8] = rand() % 10 + 48;
	s[9] = ' ';
	s[10] = ' ';
	s[11] = 'o';
	s[12] = 'h';
	s[13] = 'm';
	s[13] = 13;
	memcpy(buffer, s, LENGTH);
	*len = LENGTH;
	return 0;
}
