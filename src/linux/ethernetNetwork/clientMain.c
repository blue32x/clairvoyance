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
	int bytes_read;
	struct sockaddr_in server_addr;
	char buf[BUF_SIZE];

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

	// read data from the server
	while(1)
	{
		bytes_read = read(client_socket, buf, sizeof(buf));
		if( bytes_read > 0 )
		{
			size_t i;
			long joystickData13 = 0;

			/* check received string data */
			printf("received [%s]\n", buf);

			/* split received string to string array and check */
			joystickData = strsplit(buf, "/", &numData);
			for (i = 0; i < numData; i++) {
				printf("%s-", joystickData[i]);
			}
			printf("\n");

			joystickData13 = atol(joystickData[13]);
			if(joystickData13 != 0)
			{
				joystickData13 = 1;
			}

			/* control model car by raw data */
			///// Control Speed
			if((joystickData13 - Data13) == 1)					//back gear
			{
				back_gear++;
			}
			if(back_gear%2 == 1)								//back gear
			{
				back_speedControl(fd, atol(joystickData[1]));
			}
			else 
			{
				speedControl(fd, atol(joystickData[1]));
			}

			///// Control Steer
			steeringControl(fd, atol(joystickData[0]));

			///// Control Flicker
			if((atol(joystickData[7]) - Data7) == 1)			//Right flicker
			{ 
				rf_count++; 
				right_flicker(fd, rf_count);
			}
			if((atol(joystickData[6]) - Data6) == 1)			//Left flicker
			{	
				lf_count++;
				left_flicker(fd, lf_count);
			}

			///// Control Light
			if((atol(joystickData[2]) - Data2) == 1)			//forward_light
			{	
				fl_count++;
				forward_light(fd, fl_count);
			}
			if((atol(joystickData[4]) - Data4) == 1)			//backward_light
			{	
				bl_count++;
				back_light(fd, bl_count);  
			}

			///// Control Buzzer
			if(atol(joystickData[3]) == 1)
			{
				soundControl(fd);
			}

			Data2 = atol(joystickData[2]);
			Data4 = atol(joystickData[4]); 	
			Data6 = atol(joystickData[6]); 
			Data7 = atol(joystickData[7]);
			Data13 = joystickData13; 

			/* clean up heap allocation for after strplit() */
			for (i = 0; i < numData; i++) {
				free(joystickData[i]);
			}
			if (joystickData != NULL) {
				free(joystickData);
			}
		}
	}

	close(client_socket);
	close(fd);
	return 0;
}
