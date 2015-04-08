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

#include "carControl.h"

#define BUF_SIZE 1024

int main(int argc, char** argv)
{
	int client_socket;
	struct sockaddr_in server_addr;
	char buff[BUF_SIZE];

	int menuNum = -1;
	int fd;

	/* Initailize socket network and connect socket */
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

	/* Print menu */
	menu();

	while(menuNum != 13)
	{
		printf("select menu : ");
		scanf("%d", &menuNum);

		switch(menuNum)
		{
		case 0:
			menu();
			break;
		case 1:
			ggambback(fd);
			break;
		case 2:
			light(fd);
			break;
		case 3:
			PositionControlOnOff(fd);
			break;
		case 4:
			SpeedControlOnOff(fd);
			break;
		case 5:
			Desire_speed(fd);
			break;
		case 6:
			buzzer(fd);
			break;
		case 7:
			steering(fd);
			break;
		case 8:
			Speed_proportional(fd);
			break;
		case 9:
			Speed_integral(fd);
			break;
		case 10:
			Speed_differental(fd);
			break;
		case 11:
			ReadSpeed(fd);
			break;
		case 12:
			do
			{
				/* Read socket */
				read(client_socket, buff, BUF_SIZE);
				printf("%s\n", buff);
				joystick(fd, buff[0]);
			}while(buff[0] != 'i');
			break;
		}
	}

	close(client_socket);
	close(fd);
	return 0;
}
