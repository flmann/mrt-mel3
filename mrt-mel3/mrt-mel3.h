
#include <windows.h>
#include <tchar.h>
#include <stdio.h>

const DCB dcbProtek506 = { .BaudRate = CBR_1200,.Parity = NOPARITY,.StopBits = TWOSTOPBITS,.ByteSize = 7 };
const DCB dcbPeakTech4390 = { .BaudRate = CBR_9600,.Parity = NOPARITY,.StopBits = TWOSTOPBITS,.ByteSize = 7,.fRtsControl = RTS_CONTROL_DISABLE };

const BYTE D = 0x44; // 44 in Hex is the char 'D'

void PrintCommState(const DCB dcb);
float getResistanceFromString(const char* string, int len);
float convertResistanceToCelcius(const int res);
int main();