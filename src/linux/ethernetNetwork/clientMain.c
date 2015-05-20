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

#include "util.h"
#include "carControl.h"

#define BUF_SIZE 1024
#define PORT 6061
#define IP "192.168.0.82"

int main(int argc, char** argv)
{
	int client_socket;
	struct sockaddr_in server_addr;
	char buff[BUF_SIZE];

	int fd;
	char **joystickData;
	size_t numData;

	long Data2, Data4, Data6, Data7, Data13;
	long rf_count, lf_count, fl_count, bl_count, back_gear;
	Data2=Data4=Data6=Data7=Data13=0;
	rf_count=lf_count=fl_count=bl_count=back_gear=0;

	/* Initailize socket network and connect socket */
	client_socket = socket(PF_INET, SOCK_STREAM, 0);
	if(client_socket == -1)
	{
		printf("socket create failed\n");
		exit(1);
	}

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);			//server port
	server_addr.sin_addr.s_addr = inet_addr(IP);		//server ip
	printf("client socket initialize success\n");

	if(connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
	{
		printf("socket connect failed\n");
		exit(1);
	}
	printf("client socket connect success\n");


	/* Initailize serial network */
	if((fd = serialOpen ("/dev/ttyAMA0", 19200)) < 0)
	{
		fprintf (stderr, "Unable to open serial device: %s\n", strerror (errno)) ;
		return 1 ;
	}
	if(wiringPiSetup () == -1)
	{
		fprintf (stdout, "Unable to start wiringPi: %s\n", strerror (errno)) ;
		return 1 ;
	}

	while((input = getche()) != 'i')
	{
		if(input == 'w')
			speedControl(fd, 1);
		if(input == 's')
			speedControl(fd, -1);
		/* Read socket */
		//read(client_socket, buff, BUF_SIZE);
		//printf("%s\n", buff);
	}

	close(client_socket);
	close(fd);
	return 0;
}
