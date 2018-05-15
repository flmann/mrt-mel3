// MEL3.c: Definiert den Einstiegspunkt für die Konsolenanwendung.
//

#include "stdafx.h"
#include "mrt-mel3.h"
#include <time.h>

int main()
{
	printf("Started\n");
	
	int dvm = 0;
	printf("\nWhich multimeter do you want to use? \n1) DVM Protek 506 \n2) DVM PeakTech 4390 \n\nChoose by pressing 1 or 2!");
	scanf("%d", &dvm);
	if (dvm != 1 && dvm != 2) {
		printf("Error: No correct input!");
		return 1;
	}
	
	
	//https://stackoverflow.com/questions/2230758/what-does-lpcwstr-stand-for-and-how-should-it-be-handled-with
	LPCWSTR port = L"COM4";

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

	// Set CommState
	// Protek 506
	if (dvm == 1) {
		DCB dcbInit = { .BaudRate = CBR_1200,.Parity = NOPARITY,.StopBits = TWOSTOPBITS,.ByteSize = 7, .fRtsControl = RTS_CONTROL_ENABLE};
		success = SetCommState(hComm, &dcbInit);
	}
	
	// PeakTech 4390
	if (dvm == 2) {
		DCB dcbInit = { .BaudRate = CBR_9600,.Parity = NOPARITY,.StopBits = TWOSTOPBITS,.ByteSize = 7, .fRtsControl = RTS_CONTROL_DISABLE };
		success = SetCommState(hComm, &dcbInit);
	}

	if (!success) {
		printf("Error while setting commState, Error code: %d\n", GetLastError());
	}
	// Checking Status
	DCB dcb;
	SecureZeroMemory(&dcb, sizeof(DCB));

	success = GetCommState(hComm, &dcb);
	if (!success) {
		printf("Error while getting commState, Error code: %d\n", GetLastError());
	}
	PrintCommState(dcb);

	if (!success)
	{
		printf("GetCommState failed with error %d.\n", GetLastError());
		return (2);
	}
	// TODO do move checking

	//TODO move to const + generic send string or byte
	char rb[15];
	BYTE D = 0x44; // 44 in Hex is the char 'D'

				   // wait for keyboard hit
	while (!_kbhit()) {
		// Send D to Multimeter
		DWORD sendSize = 0;
		DWORD recvSize = -1;
		clock_t start_clk = clock();

		success = WriteFile(hComm, &D, 1, &sendSize, NULL);
		if (!success) {
			printf("Error while writing, ERRORCODE: %d\n", GetLastError());
			break; //TODO should we always break ?
		}
		if (sendSize != 1) {
			printf("Error not same size after send\n");
			break;
		}

		success = ReadFile(hComm, rb, 15, &recvSize, NULL);
		if (!success) {
			printf("Error while reading, ERRORCODE: %d\n", GetLastError());
			break;
		}
		// TODO move to const
		printf("received >%s< with len (%d)\n", rb, strlen(rb));
		if (recvSize != 15) {
			printf("Error after receiving: received is not of size 15, size is %d\n", recvSize);
			break;
		}


		float res = getResistanceFromString((char*)rb);
		float celcius = convertResistanceToCelcius(res);
		printf("received res: %f \n", celcius);

		//substract process time
		long double time = clock() - start_clk;
		//move to const
		const int well = 1000 - time;
		Sleep(well);
	}

	success = CloseHandle(hComm);
	if (!success) {
		printf("Error while trying to close handle");
		return 0;		//return error?
	}
	// Closing

	printf("Done\n");
	return 0;
}

void PrintCommState(const DCB dcb)
{
	//  Print some of the DCB structure values
	_tprintf(TEXT("\nBaudRate = %d, ByteSize = %d, Parity = %d, StopBits = %d\n"),
		dcb.BaudRate,
		dcb.ByteSize,
		dcb.Parity,
		dcb.StopBits);
}

float getResistanceFromString(const char* string) {
	int i = 0;
	//int res = 0;
	//while (*(string + i++) != 13);

	//while (i-- > 0) {
	//	if (*(string + i) == '.') {
	//		continue;
	//	}
	//	
	//}
	int start = -1;
	char substr[5];
	float f;
	while (i < 15) {
		// c is the current char
		char c = *(string + i);
		if (isdigit(c)) {
			if (start == -1) {
				start = i;
				strncpy(substr, string + start, 5);
				//printf("--- found: %s", substr);
				sscanf(substr, "%f", &f);				//f = resistance in k/M/ohm
				printf("--- found: %f", f);
			}
			strncpy(substr, string + start + 5, 2);
			// conv MOhm to Ohm
			if (strstr(substr, "M") != NULL) {
				f = f * 1000000;
			}
			if (strstr(substr, "k") != NULL) {
				f = f * 1000;
			}
			if (strstr(substr, "m") != NULL) {
				f = f / 1000;
			}
			return f;					//return resistance value
		}

		// Check for CR
		if (c == 13) {

			printf("Warning: No resistance detected or OL!");
			break;
		}
		i++;
	}

	return f;
}

float convertResistanceToCelcius(const int res) {
	//142.633 + 2.66196 (x - 50) - 0.00109441 (x - 50)^2
	return 142.633f + 2.66196f *(res - 50) - 0.00109441f * (res - 50) * (res - 50);
}
