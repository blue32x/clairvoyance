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

#define BUF_SIZE 64
#define NUM_DATA 14

#define PORT 6061
#define IP "192.168.0.82"

int main(int argc, char** argv)
{
	int client_socket;
	int bytes_read;
	struct sockaddr_in server_addr;
	char buf[BUF_SIZE];
	char *ptr;

	int fd;
	long data[NUM_DATA];

	long preData2, preData4, preData6, preData7, preData13;
	long rf_count, lf_count, fl_count, bl_count, back_gear;
	preData2 = preData4 = preData6 = preData7 = preData13 = 0;
	rf_count = lf_count = fl_count = bl_count = back_gear = 0;

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

	/* read data from the server */
	while(1)
	{
		bytes_read = read(client_socket, buf, sizeof(buf));
		if( bytes_read > 0 )
		{
			int i = 0;

			///// check received string data
			//printf("received [%s]\n", buf);

			///// split received string to string array and check
			ptr = strtok( buf, "/");
			data[i] = atol(ptr);
			//printf( "print string : %s ", ptr);
			printf( "print long type : %ld ", data[i++]);

			while( ptr = strtok( NULL, "/"))
			{
				data[i] = atol(ptr);
			//	printf( "%s ", ptr);
				printf( "%ld ", data[i++]);
			}
			printf( "\n");

			/*
			if(data[13] != 0)
			{
				data[13] = 1;
			}

			///// control model car by raw data
			// Control Speed
			if((data[13] - preData13) == 1)				
			{
				back_gear++;
			}
			if(back_gear%2 == 1)						
			{
				back_speedControl(fd, data[1]);
			}
			else 
			{
				speedControl(fd, data[1]);
			}

			// Control Steer
			steeringControl(fd, data[0]);

			// Control Flicker
			if((data[7] - preData7) == 1)		//Right flicker
			{ 
				rf_count++; 
				right_flicker(fd, rf_count);
			}
			if((data[6] - preData6) == 1)		//Left flicker
			{	
				lf_count++;
				left_flicker(fd, lf_count);
			}

			// Control Light
			if((data[2] - preData2) == 1)		//forward_light
			{	
				fl_count++;
				forward_light(fd, fl_count);
			}
			if((data[4] - preData4) == 1)		//backward_light
			{	
				bl_count++;
				back_light(fd, bl_count);  
			}

			// Control Buzzer
			if(data[3] == 1)
			{
				soundControl(fd);
			}

			preData2 = data[2];
			preData4 = data[4]; 	
			preData6 = data[6]; 
			preData7 = data[7];
			preData13 = data[13]; 
			*/
		}
	}
	close(client_socket);
	close(fd);
	return 0;
}
