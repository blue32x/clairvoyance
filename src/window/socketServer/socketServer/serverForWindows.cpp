#include <winsock.h> 
#include <stdio.h> 
#include <conio.h> 
 
#define BUF_SIZE 1024
#define PORT 6061
#define IP "192.168.0.82"
 
int main() 
{ 
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

	while((ch = getche()) != ']')
	{
		// Read socket
		/*
        Readn = recv( ClientSocket, ReadBuffer, BUF_SIZE,0 ); 
		*/

		// Write socket
		buffer[0] = ch;
		buffer[1] = '\0';
		printf("%d : %s\n", strlen(buffer), buffer);
        send(ClientSocket, buffer, strlen(buffer) + 1, 0); 
    }

	closesocket(ClientSocket);  
    closesocket(EndpointSocket);  
    WSACleanup(); 
    return 0; 
} 