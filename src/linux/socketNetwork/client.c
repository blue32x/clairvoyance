#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <wiringPi.h>
#include <wiringSerial.h>

#define BUF_SIZE 1024

int main(int argc, char** argv)
{
	int client_socket;
	struct sockaddr_in server_addr;
	char buff[BUF_SIZE];

	// Initailize and connect socket
	client_socket = socket(PF_INET, SOCK_STREAM, 0);
	if(client_socket == -1)
	{
		printf("socket create failed\n");
		exit(1);
	}

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(6061);			//server port
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");	//server ip
	printf("client socket initialize success\n");

	if(connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
	{
		printf("socket connect failed\n");
		exit(1);
	}
	printf("client socket connect success\n");

	// Write socket
	/*
	write(client_socket, argv[1], strlen(argv[1] + 1));
	*/

	while(1)
	{
		// Read socket
		read(client_socket, buff, BUF_SIZE);
		printf("%s\n", buff);
	}

	close(client_socket);
	return 0;
}
