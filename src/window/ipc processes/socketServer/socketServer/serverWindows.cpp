#include <winsock.h> 
#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>

#define PORT 6061
#define IP "192.168.0.82"

#define BUF_SIZE 64
#define NUM_DATA 14

HANDLE hMapFile;
LPCTSTR pBuf;

int main() 
{
	//
	// Open Named Shared Memory
	//
	hMapFile = OpenFileMapping(
		FILE_MAP_ALL_ACCESS,   // read/write access
		FALSE,                 // do not inherit the name
		TEXT("RawData"));      // name of mapping object

	if (hMapFile == NULL)
	{
		_tprintf(TEXT("Could not open file mapping object (%d).\n"),
			GetLastError());
		return 1;
	}

	//
	// Create socket server
	//
	WSADATA wsaData; 
	SOCKET EndpointSocket, ClientSocket; 
	struct sockaddr_in SockInfo, ClientSockInfo; 

	int status; 
	int SockLen;  
	char buffer[BUF_SIZE]; 
	char ch;

	// winsock start
	if(WSAStartup(MAKEWORD(2,2),&wsaData)!= 0) 
	{ 
		printf("error\r\n"); 
		return 0; 
	} 

	// Create socket
	EndpointSocket = socket(PF_INET, SOCK_STREAM, 0); 
	if(EndpointSocket == INVALID_SOCKET)
	{
		printf("server socket create failed.\r\n"); 
		return 1; 
	}
	printf("server socket create success\r\n"); 

	ZeroMemory(&SockInfo, sizeof(struct sockaddr_in)); 
	SockInfo.sin_family = PF_INET; 
	SockInfo.sin_port = htons(PORT) ;						// server port
	//SockInfo.sin_addr.S_un.S_addr = htonl(INADDR_ANY);	// server ip
	SockInfo.sin_addr.S_un.S_addr = inet_addr(IP);			// server ip

	// Bind
	status = bind(EndpointSocket, (struct sockaddr*)&SockInfo, sizeof(struct sockaddr_in)); 
	if(status == SOCKET_ERROR)  
	{ 
		printf("bind failed.\n"); 
		return 0; 
	} 
	printf("server socket bind success\n"); 

	// Listen
	if(SOCKET_ERROR == listen(EndpointSocket, 5)) 
	{ 
		printf("listen Error\n"); 
		return 0; 
	} 
	printf("server socket listen success\n"); 

	do
	{ 
		// Accept clinet socket
		ZeroMemory(&ClientSockInfo, sizeof(struct sockaddr_in)); 
		SockLen = sizeof(struct sockaddr_in); 
		ClientSocket = accept(EndpointSocket, (struct sockaddr*)&ClientSockInfo, &SockLen); 
		if(ClientSocket == INVALID_SOCKET) 
		{ 
			printf("Accept Error\n"); 
			closesocket(EndpointSocket); 
			WSACleanup(); 
			return 1; 
		} 
		printf("accept client socket success\n"); 
	}while(0);

	char buf[BUF_SIZE] = {0};
	int count = 0;
	while(1)
	{
		if(count == 4)
		{
			count = 0;
			// read from shared memory
			pBuf = (LPTSTR) MapViewOfFile(hMapFile, // handle to map object
				FILE_MAP_ALL_ACCESS,  // read/write permission
				0,
				0,
				BUF_SIZE);

			if (pBuf == NULL)
			{
				_tprintf(TEXT("Could not map view of file (%d).\n"),
					GetLastError());

				CloseHandle(hMapFile);
				return 1;
			}

			// convert LPCTSTR pBuf to char [] buf
			WideCharToMultiByte(CP_ACP, 0, pBuf, BUF_SIZE, buf, BUF_SIZE, NULL, NULL);
			buf[strlen(buf)] = '\0';

			// send
			int sended = 0;
			sended = send( ClientSocket, buf, sizeof(buf), 0 );
			if( sended > 0 ) {
				printf("%s (sended count: %d, sizeofbuf: %d)\n", buf, sended, sizeof(buf));
			}
			UnmapViewOfFile(pBuf);
		}
		Sleep(10);
		count++;
	}
	closesocket(ClientSocket);  
	closesocket(EndpointSocket);
	UnmapViewOfFile(pBuf);
	CloseHandle(hMapFile);
	WSACleanup(); 
	return 0; 
}


/*
if (hmapfile == null)
{ _tprintf(text("could not open file mapping object (%d).\n"), getlasterror()); 
return 1;
}
pbuf = mapviewoffile(hmapfile,
// handle to map object file_map_write,
// read/write permission 0, 0, sizeof(mmfstruct));
if (pbuf == null)
{ _tprintf(text("could not map view of file (%d).\n"), getlasterror());
closehandle(hmapfile);
return 1
; }
mmfstruct test_data; 
// define variables here 
copymemory(pbuf, &test_data, sizeof(mmfstruct));
unmapviewoffile(pbuf);
closehandle(hmapfile);
*/