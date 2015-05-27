// Raw Input API sample showing joystick support
#include <stdio.h>
#include <windows.h>
#include <tchar.h>
#include <conio.h>

#define _USE_MATH_DEFINES
#include <math.h>
#include <hidsdi.h>

#define ARRAY_SIZE(x)	(sizeof(x) / sizeof((x)[0]))
#define WC_MAINFRAME	TEXT("MainFrame")
#define MAX_BUTTONS		128
#define BUF_SIZE	64
#define CHECK(exp)		{ if(!(exp)) goto Error; }
#define SAFE_FREE(p)	{ if(p) { HeapFree(hHeap, 0, p); (p) = NULL; } }

#pragma comment(lib, "hid.lib")

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

HANDLE hMapFile;
HANDLE hMapFile2;
HANDLE hMapFile3;
LPCTSTR pBuf;
LPCTSTR pBuf2;
LPCTSTR pBuf3;
TCHAR szMsg[BUF_SIZE];
TCHAR szMsg2[BUF_SIZE];
TCHAR szMsg3[BUF_SIZE];

void SendJoystickValues()
{
	char buffer[BUF_SIZE];
	char stopSig;
	///// Receive stop signal from kinect process
	pBuf3 = (LPTSTR) MapViewOfFile(hMapFile3, // handle to map object
		FILE_MAP_ALL_ACCESS,  // read/write permission
		0,
		0,
		BUF_SIZE);

	if (pBuf3 == NULL)
	{
		_tprintf(TEXT("Could not map view of hMapFile3 (%d).\n"),
			GetLastError());

		CloseHandle(hMapFile3);
		return ;
	}

	// convert LPCTSTR pBuf to char [] buf
	WideCharToMultiByte(CP_ACP, 0, pBuf3, BUF_SIZE, buffer, BUF_SIZE, NULL, NULL);
	stopSig = buffer[0];
	//printf("stop signal : %c\n", stopSig);


	///// Send raw data to Server process
	sprintf(buffer,"%ld/%ld/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%c",
		lAxisX,lAxisY,bButtonStates[0],bButtonStates[1],
		bButtonStates[2],bButtonStates[3],bButtonStates[4],
		bButtonStates[5],bButtonStates[6],bButtonStates[7],
		bButtonStates[8],bButtonStates[9],bButtonStates[10],bButtonStates[11], stopSig);
	/*
	sprintf(buffer,"%ld/%ld/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d",
		lAxisX,lAxisY,bButtonStates[0],bButtonStates[1],
		bButtonStates[2],bButtonStates[3],bButtonStates[4],
		bButtonStates[5],bButtonStates[6],bButtonStates[7],
		bButtonStates[8],bButtonStates[9],bButtonStates[10],bButtonStates[11]);
	*/
	printf("raw data : %s\n",buffer);

	// convert LPCTSTR pBuf to char [] buf
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, buffer, strlen(buffer), szMsg, BUF_SIZE);

	pBuf = (LPTSTR) MapViewOfFile(hMapFile,   // handle to map object
		FILE_MAP_ALL_ACCESS, // read/write permission
		0,
		0,
		BUF_SIZE);

	if (pBuf == NULL)
	{
		_tprintf(TEXT("Could not map view of hMapFile (%d).\n"),
			GetLastError());

		CloseHandle(hMapFile);
		return ;
	}
	//Write buffer for server process
	CopyMemory((PVOID)pBuf, szMsg, (_tcslen(szMsg) * sizeof(TCHAR)));


	///// Send mode signal to kinect process
	sprintf(buffer,"%ld", lHat);
	printf("mode selection : %s\n",buffer);

	// convert LPCTSTR pBuf to char [] buf
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, buffer, strlen(buffer), szMsg2, BUF_SIZE);

	pBuf2 = (LPTSTR) MapViewOfFile(hMapFile2,   // handle to map object
		FILE_MAP_ALL_ACCESS, // read/write permission
		0,
		0,
		BUF_SIZE);

	if (pBuf2 == NULL)
	{
		_tprintf(TEXT("Could not map view of hMapFile2 (%d).\n"),
			GetLastError());

		CloseHandle(hMapFile2);
		return ;
	}
	//Write buffer for kinect process
	CopyMemory((PVOID)pBuf2, szMsg2, (_tcslen(szMsg2) * sizeof(TCHAR)));

	UnmapViewOfFile(pBuf3);
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
	ULONG                i, usageLength, value;

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
		HBRUSH hOldBrush;
		HBRUSH hBr;
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
	HWND hWnd;
	MSG msg;
	WNDCLASSEX wcex;
	int count = 0;

	//
	// Create Named Shared Memory
	//
	///// For Raw data
	hMapFile = CreateFileMapping(
		INVALID_HANDLE_VALUE,    // use paging file
		NULL,                    // default security
		PAGE_READWRITE,          // read/write access
		0,                       // maximum object size (high-order DWORD)
		BUF_SIZE,				 // maximum object size (low-order DWORD)
		TEXT("RawData"));        // name of mapping object

	if (hMapFile == NULL)
	{
		_tprintf(TEXT("Could not create hMapFile mapping object (%d).\n"),
			GetLastError());
		return 1;
	}

	///// For Mode select signal
	hMapFile2 = CreateFileMapping(
		INVALID_HANDLE_VALUE,    
		NULL,                    
		PAGE_READWRITE,          
		0,                       
		BUF_SIZE,				 
		TEXT("ModeSig"));                

	if (hMapFile2 == NULL)
	{
		_tprintf(TEXT("Could not create hMapFile2 mapping object (%d).\n"),
			GetLastError());
		return 1;
	}

	///// For Stop signal
	hMapFile3 = CreateFileMapping(
		INVALID_HANDLE_VALUE,    
		NULL,                    
		PAGE_READWRITE,          
		0,                       
		BUF_SIZE,				 
		TEXT("StopSig"));                

	if (hMapFile3 == NULL)
	{
		_tprintf(TEXT("Could not create hMapFile3 mapping object (%d).\n"),
			GetLastError());
		return 1;
	}

	AllocConsole();

	freopen("CONOUT$", "wt", stdout);

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
		if(count == 4)
		{
			count = 0;
			SendJoystickValues();
		}
		count++;
	}

	FreeConsole();
	UnmapViewOfFile(pBuf);
	UnmapViewOfFile(pBuf2);
	UnmapViewOfFile(pBuf3);
	CloseHandle(hMapFile);
	CloseHandle(hMapFile2);
	CloseHandle(hMapFile3);

	return (int)msg.wParam;
}