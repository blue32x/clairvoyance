#include "stdafx.h"
#include <stdio.h>

#include <initguid.h>
#include <winsock2.h>
#include <ws2bth.h>

#include <Windows.h>
#include <tchar.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <hidsdi.h>

DEFINE_GUID(SAMPLE_UUID, 0x31b44148, 0x041f, 0x42f5, 0x8e, 0x73, 0x18, 0x6d, 0x5a, 0x47, 0x9f, 0xc9);

#pragma comment(lib, "hid.lib")
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "irprops.lib")

#define ARRAY_SIZE(x)	(sizeof(x) / sizeof((x)[0]))
#define WC_MAINFRAME	TEXT("MainFrame")
#define MAX_BUTTONS		128
#define MAX_BUFFER_SIZE	1024
#define CHECK(exp)		{ if(!(exp)) goto Error; }
#define SAFE_FREE(p)	{ if(p) { HeapFree(hHeap, 0, p); (p) = NULL; } }

//
// Global variables
//
BOOL bButtonStates[MAX_BUTTONS];
LONG lAxisX;
LONG lAxisY;
LONG lAxisZ;
LONG lAxisRz;
LONG lHat;
INT  g_NumberOfButtons;
char m_cText[100];

SOCKET client;
char buf[1024] = { 0 };

void SendJoystickValues()
{
	char buffer[MAX_BUFFER_SIZE];
	sprintf_s(buffer,"%ld/%ld/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d",
		lAxisX,lAxisY,bButtonStates[0],bButtonStates[1],
		bButtonStates[2],bButtonStates[3],bButtonStates[4],
		bButtonStates[5],bButtonStates[6],bButtonStates[7],
		bButtonStates[8],bButtonStates[9],bButtonStates[10],bButtonStates[11]);

	//Send buffer to rasp-pi on bluetooth network
	/*
	int sended = 0;
	sended = send( client, buffer, sizeof(buffer), 0 );
	if( sended > 0 ) {
		printf("sended: %s(len : %d)\n", buffer, sended);
	}
	*/
}

void ParseRawInput(PRAWINPUT pRawInput)
{
	PHIDP_PREPARSED_DATA pPreparsedData;
	HIDP_CAPS            Caps;
	PHIDP_BUTTON_CAPS    pButtonCaps;
	PHIDP_VALUE_CAPS     pValueCaps;
	USHORT               capsLength;
	UINT                 bufferSize;
	HANDLE               hHeap;
	USAGE                usage[MAX_BUTTONS];
	ULONG                i, usageLength, value=0;

	pPreparsedData = NULL;
	pButtonCaps    = NULL;
	pValueCaps     = NULL;
	hHeap          = GetProcessHeap();

	//
	// Get the preparsed data block
	//

	CHECK( GetRawInputDeviceInfo(pRawInput->header.hDevice, RIDI_PREPARSEDDATA, NULL, &bufferSize) == 0 );
	CHECK( pPreparsedData = (PHIDP_PREPARSED_DATA)HeapAlloc(hHeap, 0, bufferSize) );
	CHECK( (int)GetRawInputDeviceInfo(pRawInput->header.hDevice, RIDI_PREPARSEDDATA, pPreparsedData, &bufferSize) >= 0 );

	//
	// Get the joystick's capabilities
	//

	// Button caps
	CHECK( HidP_GetCaps(pPreparsedData, &Caps) == HIDP_STATUS_SUCCESS )
	CHECK( pButtonCaps = (PHIDP_BUTTON_CAPS)HeapAlloc(hHeap, 0, sizeof(HIDP_BUTTON_CAPS) * Caps.NumberInputButtonCaps) );

	capsLength = Caps.NumberInputButtonCaps;
	CHECK( HidP_GetButtonCaps(HidP_Input, pButtonCaps, &capsLength, pPreparsedData) == HIDP_STATUS_SUCCESS )
		g_NumberOfButtons = pButtonCaps->Range.UsageMax - pButtonCaps->Range.UsageMin + 1;

	// Value caps
	CHECK( pValueCaps = (PHIDP_VALUE_CAPS)HeapAlloc(hHeap, 0, sizeof(HIDP_VALUE_CAPS) * Caps.NumberInputValueCaps) );
	capsLength = Caps.NumberInputValueCaps;
	CHECK( HidP_GetValueCaps(HidP_Input, pValueCaps, &capsLength, pPreparsedData) == HIDP_STATUS_SUCCESS )

	//
	// Get the pressed buttons
	//

	usageLength = g_NumberOfButtons;
	CHECK(
		HidP_GetUsages(
		HidP_Input, pButtonCaps->UsagePage, 0, usage, &usageLength, pPreparsedData,
		(PCHAR)pRawInput->data.hid.bRawData, pRawInput->data.hid.dwSizeHid
		) == HIDP_STATUS_SUCCESS );

	ZeroMemory(bButtonStates, sizeof(bButtonStates));
	for(i = 0; i < usageLength; i++)
		bButtonStates[usage[i] - pButtonCaps->Range.UsageMin] = TRUE;

	//
	// Get the state of discrete-valued-controls
	//

	for(i = 0; i < Caps.NumberInputValueCaps; i++)
	{
		CHECK(
			HidP_GetUsageValue(
			HidP_Input, pValueCaps[i].UsagePage, 0, pValueCaps[i].Range.UsageMin, &value, pPreparsedData,
			(PCHAR)pRawInput->data.hid.bRawData, pRawInput->data.hid.dwSizeHid
			) == HIDP_STATUS_SUCCESS );

		switch(pValueCaps[i].Range.UsageMin)
		{
		case 0x30:	// X-axis
			lAxisX = (LONG)value - 128;
			break;

		case 0x31:	// Y-axis
			lAxisY = (LONG)value - 128;
			break;

		case 0x32: // Z-axis
			lAxisZ = (LONG)value - 128;
			break;

		case 0x35: // Rotate-Z
			lAxisRz = (LONG)value - 128;
			break;

		case 0x39:	// Hat Switch
			lHat = value;
			break;
		}
	}

	//
	// Clean up
	//

Error:
	SAFE_FREE(pPreparsedData);
	SAFE_FREE(pButtonCaps);
	SAFE_FREE(pValueCaps);
}

void DrawButton(HDC hDC, int i, int x, int y, BOOL bPressed)
{
	HBRUSH hOldBrush, hBr;
	TCHAR  sz[4];
	RECT   rc;

	if(bPressed)
	{
		hBr       = CreateSolidBrush(RGB(192, 0, 0));
		hOldBrush = (HBRUSH)SelectObject(hDC, hBr);
	}

	rc.left   = x;
	rc.top    = y;
	rc.right  = x + 30;
	rc.bottom = y + 30;
	Ellipse(hDC, rc.left, rc.top, rc.right, rc.bottom);
	_stprintf_s(sz, ARRAY_SIZE(sz), TEXT("%d"), i);
	DrawText(hDC, sz, -1, &rc, DT_SINGLELINE | DT_CENTER | DT_VCENTER);

	if(bPressed)
	{
		SelectObject(hDC, hOldBrush);
		DeleteObject(hBr);
	}
}

void DrawCrosshair(HDC hDC, int x, int y, LONG xVal, LONG yVal)
{
	Rectangle(hDC, x, y, x + 256, y + 256);
	MoveToEx(hDC, x + xVal - 5 + 128, y + yVal + 128, NULL);
	LineTo(hDC, x + xVal + 5 + 128, y + yVal + 128);
	MoveToEx(hDC, x + xVal + 128, y + yVal - 5 + 128, NULL);
	LineTo(hDC, x + xVal + 128, y + yVal + 5 + 128);
}

void DrawDPad(HDC hDC, int x, int y, LONG value)
{
	LONG i;

	for(i = 0; i < 8; i++)
	{
		HBRUSH hOldBrush = 0;
		HBRUSH hBr = 0;
		int xPos = (int)(sin(-2 * M_PI * i / 8 + M_PI) * 80.0) + 80;
		int yPos = (int)(cos(2 * M_PI * i / 8 + M_PI) * 80.0) + 80;

		if(value == i)
		{
			hBr       = CreateSolidBrush(RGB(192, 0, 0));
			hOldBrush = (HBRUSH)SelectObject(hDC, hBr);
		}

		Ellipse(hDC, x + xPos, y + yPos, x + xPos + 20, y + yPos + 20);

		if(value == i)
		{
			SelectObject(hDC, hOldBrush);
			DeleteObject(hBr);
		}
	}
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_CREATE:
		{
			//
			// Register for joystick devices
			//

			RAWINPUTDEVICE rid;

			rid.usUsagePage = 1;
			rid.usUsage     = 4;	// Joystick
			rid.dwFlags     = 0;
			rid.hwndTarget  = hWnd;

			if(!RegisterRawInputDevices(&rid, 1, sizeof(RAWINPUTDEVICE)))
				return -1;
		}
		return 0;

	case WM_INPUT:
		{
			//
			// Get the pointer to the raw device data, process it and update the window
			//

			PRAWINPUT pRawInput;
			UINT      bufferSize;
			HANDLE    hHeap;

			GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &bufferSize, sizeof(RAWINPUTHEADER));

			hHeap     = GetProcessHeap();
			pRawInput = (PRAWINPUT)HeapAlloc(hHeap, 0, bufferSize);
			if(!pRawInput)
				return 0;

			GetRawInputData((HRAWINPUT)lParam, RID_INPUT, pRawInput, &bufferSize, sizeof(RAWINPUTHEADER));
			ParseRawInput(pRawInput);

			HeapFree(hHeap, 0, pRawInput);

			InvalidateRect(hWnd, NULL, TRUE);
			UpdateWindow(hWnd);
		}
		return 0;

	case WM_PAINT:
		{
			//
			// Draw the buttons and axis-values
			//

			PAINTSTRUCT ps;
			HDC         hDC;
			int         i;

			hDC = BeginPaint(hWnd, &ps);
			SetBkMode(hDC, TRANSPARENT);

			for(i = 0; i < g_NumberOfButtons; i++)
				DrawButton(hDC, i+1, 20 + i * 40, 20, bButtonStates[i]);
			DrawCrosshair(hDC, 20, 100, lAxisX, lAxisY);
			DrawCrosshair(hDC, 296, 100, lAxisZ, lAxisRz);
			DrawDPad(hDC, 600, 140, lHat);

			EndPaint(hWnd, &ps);
		}
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	/**************************
	  rfcommServer.cpp source
	**************************/
	/* Socket Variables */
	SOCKET server;
	SOCKADDR_BTH sa;
	int sa_len = sizeof(sa);

	WORD wVersionRequested = MAKEWORD( 2, 0 );
	WSADATA wsaData;

	CSADDR_INFO sockInfo;
	WSAQUERYSET serviceInfo = { 0 };

	SOCKADDR_BTH ca;
	int ca_len = sizeof(ca);
	DWORD buf_len = sizeof(buf);

	/* Window Variables */
	HWND hWnd;
	MSG msg;
	WNDCLASSEX wcex;
	FILE * newstream;

	AllocConsole();

	// initialize windows sockets
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
	if( SOCKET_ERROR == bind( server, (const sockaddr*)&sa, sizeof(SOCKADDR_BTH)) ) {
		int error = WSAGetLastError();
		printf("%d\n", error);
		ExitProcess(2);
	}
	listen( server, 1 );
	// check which port we��re listening on
	if( SOCKET_ERROR == getsockname( server, (SOCKADDR*) &sa, &sa_len ) ) {
		ExitProcess(2);
	}
	printf("listening on RFCOMM port: %d\n", sa.port);

	// advertise the service
	sockInfo.iProtocol = BTHPROTO_RFCOMM;
	sockInfo.iSocketType = SOCK_STREAM;
	sockInfo.LocalAddr.lpSockaddr = (LPSOCKADDR) &sa;
	sockInfo.LocalAddr.iSockaddrLength = sizeof(sa);
	sockInfo.RemoteAddr.lpSockaddr = (LPSOCKADDR) &sa;
	sockInfo.RemoteAddr.iSockaddrLength = sizeof(sa);
	
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
	/*
	client = accept( server, (LPSOCKADDR) &ca, &ca_len );
	if( SOCKET_ERROR == client ) {
		ExitProcess(2);
	}
	printf("Accepted connection \n");
	*/

	/*
	// receive
	int received = 0;
	received = recv( client, buf, sizeof(buf), 0 );
	if( received > 0 ) {
	printf("received: %s(len : %d)\n", buf, received);
	}

	strcpy_s(buf, "bye!");

	// send
	int sended = 0;
	sended = send( client, buf, sizeof(buf), 0 );
	if( sended > 0 ) {
	printf("sended: %s(len : %d)\n", buf, sended);
	}
	*/

	/**************************
	    RawInput.c source
	**************************/
	//freopen("CONOUT$", "wt", stdout);
	freopen_s(&newstream, "CONOUT$","wt", stdout);
	//
	// Register window class
	//

	wcex.cbSize        = sizeof(WNDCLASSEX);
	wcex.cbClsExtra    = 0;
	wcex.cbWndExtra    = 0;
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wcex.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
	wcex.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);
	wcex.hInstance     = hInstance;
	wcex.lpfnWndProc   = WindowProc;
	wcex.lpszClassName = WC_MAINFRAME;
	wcex.lpszMenuName  = NULL;
	wcex.style         = CS_HREDRAW | CS_VREDRAW;

	if(!RegisterClassEx(&wcex))
		return -1;

	//
	// Create window
	//

	hWnd = CreateWindow(WC_MAINFRAME, TEXT("Joystick using Raw Input API"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);
	ShowWindow(hWnd, nShowCmd);
	UpdateWindow(hWnd);

	//
	// Message loop
	//

	while(GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		SendJoystickValues();
	}

	FreeConsole();
	closesocket(client);
	closesocket(server);
	return (int)msg.wParam;
}