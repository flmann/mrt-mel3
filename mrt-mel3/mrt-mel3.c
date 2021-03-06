// MEL3.c: Definiert den Einstiegspunkt für die Konsolenanwendung.
//

#include "stdafx.h"
#include "mrt-mel3.h"
#include <time.h>

int main()
{
	printf("Started\n");

	int dvm = 0;
	LPCWSTR port = L"COM4";

	BOOL success = FALSE;
	HANDLE hComm = NULL;
	DCB dcb;
	DCB dcbInit;

	printf("\nWhich multimeter do you want to use? \n1) DVM Protek 506 \n2) DVM PeakTech 4390 \n\nChoose by pressing 1 or 2!\n");
	scanf("%d", &dvm);
	if (dvm != 1 && dvm != 2) {
		printf("Error: No correct input!");
		return 1;
	}
	if (dvm == 1) {
		dcbInit = dcbProtek506;
	}
	if (dvm == 2) {
		dcbInit = dcbPeakTech4390;
	}

	//https://stackoverflow.com/questions/2230758/what-does-lpcwstr-stand-for-and-how-should-it-be-handled-with
	// Opening
	hComm = CreateFile(port,
		GENERIC_READ | GENERIC_WRITE,
		NULL,
		NULL,
		OPEN_EXISTING,
		NULL,
		NULL);

	if (hComm == INVALID_HANDLE_VALUE || hComm == NULL) {
		success = FALSE;
		wprintf(L"Error while opening port: %ls, ERRORCODE: %d\n", port, GetLastError());
		return 1;
	}
	wprintf(L"Successfully opened port %ls, Handle is: %d\n", port, hComm);

	// Set CommState
	success = SetCommState(hComm, &dcbInit);
	if (!success) {
		printf("Error while setting commState, Error code: %d\n", GetLastError());
	}
	// Checking Status
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

	// wait for keyboard hit
	while (!_kbhit()) {
		char rb[15];
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
		DWORD errors;
		COMSTAT stat;

		Sleep(500);

		ClearCommError(hComm, &errors, &stat);

		success = ReadFile(hComm, rb, stat.cbInQue, &recvSize, NULL);
		if (!success) {
			printf("Error while reading, ERRORCODE: %d\n", GetLastError());
			break;
		}
		if (stat.cbInQue != recvSize || recvSize == 0) {
			printf("Error while reading, sizes received (%d) and expected (%d) dont match\n", recvSize, stat.cbInQue);
			continue;
		}
		//printf("received %.*s, inQ: %d\n", recvSize, rb, stat.cbInQue);

		float res = getResistanceFromString(rb, recvSize);
		float celcius = convertResistanceToCelcius(res);
		printf("Temperatur = %.1f Grad\n", celcius);

		//substract process time
		long double time = clock() - start_clk;
		//move to const
		const int well = 1000 - time;
		if (well > 0) {
			Sleep(well);
		}
	}

	success = CloseHandle(hComm);
	if (!success) {
		printf("Error while trying to close handle");
		return 3;		//return error?
	}
	// Closing

	printf("Done\n");
	return 0;
}

void PrintCommState(const DCB dcb)
{
	//  Print some of the DCB structure values
	printf("\nBaudRate = %d, ByteSize = %d, Parity = %d, StopBits = %d\n",
		dcb.BaudRate,
		dcb.ByteSize,
		dcb.Parity,
		dcb.StopBits);
}

float getResistanceFromString(const char* string, int len) {
	int i = 0;

	int start = -1;
	char substr[5];
	float f;
	while (i < len) {
		// c is the current char
		char c = *(string + i);
		if (isdigit(c)) {
			if (start == -1) {
				start = i;
				strncpy(substr, string + start, 5);
				//printf("--- found: %s\n", substr);
				sscanf(substr, "%f", &f);				//f = resistance in k/M/ohm
				//printf("--- found: %f\n", f);
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
	return 142.633f + 2.66196f * (res - 50) - 0.00109441f * (res - 50) * (res - 50);
}
