#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <iostream>
#include <fstream>

int _tmain(int argc, _TCHAR* argv[])
{
	///-----------------------------Open port-----------------------------------------------------------------

	// Name of port where device is found
	LPCWSTR port = L"COM5";

	// Open port for reading
	HANDLE hComm = ::CreateFile(port, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);

	// Check if port has been opened succesfully
	if (hComm == INVALID_HANDLE_VALUE) std::cout << "Failed to open " << port << " error: " << GetLastError() << std::endl;
	else std::cout << port << " has been opened succesfully\n";

	///-----------------------------Configure port------------------------------------------------------------

	// Create DCB structure 
	DCB dcb = { 0 };

	// Get Comm state
	if (!::GetCommState(hComm, &dcb)) std::cout << "Failed to get Comm state, error: " << GetLastError() << std::endl;

	// Configure strcutre
	dcb.DCBlength = sizeof(DCB);

	// Set Baud rate
	dcb.BaudRate = CBR_256000;
	// Set number of bytes in bits that are recieved through the port
	dcb.ByteSize = 8;
	dcb.StopBits = ONESTOPBIT;

	// Check if port has been configured correctly
	if (!::SetCommState(hComm, &dcb)) std::cout << "\nFailed to set Comm State, error: " << GetLastError();
	else std::cout << "Comm state has been set succesfully\n";

	///-----------------------------Read data-------------------------------------------------------------------

	char buffer;
	DWORD maxBytes = 1;
	if (!::ReadFile(hComm, &buffer, maxBytes, NULL, NULL)) std::cout << "\nFailed to read from " << port << " error: " << GetLastError() << std::endl;
	else std::cout << "File has been read succesfully\n";

	///-----------------------------Write to text file----------------------------------------------------------

	std::fstream file;
	int counter = 0;
	// Writing to text file will be done later
	while (ReadFile(hComm, &buffer, maxBytes, NULL, NULL))
	{
		std::cout << buffer;
	}

	///-----------------------------Close port------------------------------------------------------------------

	CloseHandle(hComm);
	file.close();

	std::cout << "\nCOM40 has been closed!\n";
	return 0; 
}