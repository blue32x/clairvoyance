#include "stdafx.h"
#include <initguid.h>
#include <winsock2.h>
#include <ws2bth.h>

#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>

DEFINE_GUID(SAMPLE_UUID, 0x31b44148, 0x041f, 0x42f5, 0x8e, 0x73, 0x18, 0x6d, 0x5a, 0x47, 0x9f, 0xc9);

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "irprops.lib")

// Max Data Length = 33(include null charater)
#define BUF_SIZE 64

TCHAR szName[]=TEXT("GlobalMyFileMappingObject");
HANDLE hMapFile;
LPCTSTR pBuf;

int _tmain(int argc, _TCHAR* argv[])
{
	//
	// Open Named Shared Memory
	//
	hMapFile = OpenFileMapping(
		FILE_MAP_ALL_ACCESS,   // read/write access
		FALSE,                 // do not inherit the name
		szName);               // name of mapping object

	if (hMapFile == NULL)
	{
		_tprintf(TEXT("Could not open file mapping object (%d).\n"),
			GetLastError());
		return 1;
	}

	//
	// Creating rfcomm socket server
	//
	SOCKET server;
	SOCKADDR_BTH sa;
	int sa_len = sizeof(sa);

	// initialize windows sockets
	WORD wVersionRequested;
	WSADATA wsaData;
	wVersionRequested = MAKEWORD( 2, 0 );

	if( WSAStartup( wVersionRequested, &wsaData ) != 0 ) {
		ExitProcess(2);
	}
	// create the server socket
	server = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
	if( SOCKET_ERROR == server ) {
		ExitProcess(2);
	}
	// bind the server socket to an arbitrary RFCOMM port
	sa.addressFamily = AF_BTH;
	sa.btAddr = 0;
	sa.port = BT_PORT_ANY;
	if( SOCKET_ERROR == bind( server, (const sockaddr*) &sa, sizeof(SOCKADDR_BTH)) ) {
		ExitProcess(2);
	}
	listen( server, 1 );
	// check which port we¡¯re listening on
	if( SOCKET_ERROR == getsockname( server, (SOCKADDR*) &sa, &sa_len ) ) {
		ExitProcess(2);
	}
	printf("listening on RFCOMM port: %d\n", sa.port);

	// advertise the service
	CSADDR_INFO sockInfo;
	sockInfo.iProtocol = BTHPROTO_RFCOMM;
	sockInfo.iSocketType = SOCK_STREAM;
	sockInfo.LocalAddr.lpSockaddr = (LPSOCKADDR) &sa;
	sockInfo.LocalAddr.iSockaddrLength = sizeof(sa);
	sockInfo.RemoteAddr.lpSockaddr = (LPSOCKADDR) &sa;
	sockInfo.RemoteAddr.iSockaddrLength = sizeof(sa);
	WSAQUERYSET serviceInfo = { 0 };
	serviceInfo.dwSize = sizeof(serviceInfo);
	serviceInfo.dwNameSpace = NS_BTH;
	serviceInfo.lpszServiceInstanceName = (LPWSTR)"Win32 Sample Bluetooth Service";
	serviceInfo.lpszComment = (LPWSTR)"Description of service...";
	serviceInfo.lpServiceClassId = (LPGUID) &SAMPLE_UUID;
	serviceInfo.dwNumberOfCsAddrs = 1;
	serviceInfo.lpcsaBuffer = &sockInfo;
	if( SOCKET_ERROR ==	WSASetService( &serviceInfo, RNRSERVICE_REGISTER, 0 ) ) {
		ExitProcess(2);
	}
	SOCKADDR_BTH ca;
	int ca_len = sizeof(ca);
	SOCKET client;
	char buf[BUF_SIZE] = { 0 };
	DWORD buf_len = sizeof(buf);
	client = accept( server, (LPSOCKADDR) &ca, &ca_len );
	if( SOCKET_ERROR == client ) {
		ExitProcess(2);
	}
	printf("Accepted connection \n");

	// receive
	/*
	int received = 0;
	received = recv( client, buf, sizeof(buf), 0 );
	if( received > 0 ) {
		printf("received: %s(len : %d)\n", buf, received);
	}

	strcpy_s(buf, "bye!");
	*/

	// send
	LARGE_INTEGER liCounter1, liCounter2, liFrequency;
	QueryPerformanceFrequency(&liFrequency);
	QueryPerformanceCounter(&liCounter1);         // Start
	int sended = 0;
	int count = 0;
	double elapsedTime = 0;
	while(1)
	{
		QueryPerformanceCounter(&liCounter2);         // End
		if((elapsedTime = (double)(liCounter2.QuadPart - liCounter1.QuadPart) / (double)liFrequency.QuadPart) > (double)1)
		{
			printf("Time : %f, Count : %d\n", elapsedTime, count);
			return 1;
		}
		count++;
		pBuf = (LPTSTR) MapViewOfFile(hMapFile, // handle to map object
			FILE_MAP_ALL_ACCESS,  // read/write permission
			0,
			0,
			BUF_SIZE);

		if (pBuf == NULL)
		{
			continue;
			/*
			_tprintf(TEXT("Could not map view of file (%d).\n"),
				GetLastError());

			CloseHandle(hMapFile);

			return 1;
			*/
		}

		// convert LPCTSTR pBuf to char [] buf
		WideCharToMultiByte(CP_ACP, 0, pBuf, BUF_SIZE, buf, BUF_SIZE, NULL, NULL);
		buf[strlen(buf)] = '\0';

		sended = send( client, buf, sizeof(buf), 0 );
		if( sended > 0 ) {
			printf("sended: %s(len : %d)\n", buf, sended);
		}
	}

	closesocket(client);
	closesocket(server);
	UnmapViewOfFile(pBuf);
	CloseHandle(hMapFile);

	return 0;
}