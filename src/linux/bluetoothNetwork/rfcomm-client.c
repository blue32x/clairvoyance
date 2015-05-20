#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

#include <wiringPi.h>
#include <wiringSerial.h>

#include "util.h"
#include "carControl.h"

#define SERVER_DEVICE "00:1A:7D:DA:71:11"

int main(int argc, char **argv)
{
	struct sockaddr_rc addr = { 0 };
	int s, status, bytes_read;
	char buf[1024];

	int fd;
	char **joystickData;
	size_t numData;

	long Data2, Data4, Data6, Data7, Data13;
	long rf_count, lf_count, fl_count, bl_count, back_gear;
	rf_count=lf_count=fl_count=bl_count=back_gear=0;

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


	/* Initailize socket network and connect socket on bluetooth */
	// allocate a socket
	s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

	// set the connection parameters (who to connect to)
	addr.rc_family = AF_BLUETOOTH;
	addr.rc_channel = (uint8_t) 1;
	str2ba( SERVER_DEVICE, &addr.rc_bdaddr );

	// connect to server
	status = connect(s, (struct sockaddr *)&addr, sizeof(addr));

	if(status == 0)
	{
		printf("connect success\n");
	}
	else if(status < 0)
	{
		printf("connect failed\n");
		exit(1);
	}

	// read data from the server
	while(1)
	{
		bytes_read = read(s, buf, sizeof(buf));
		if( bytes_read > 0 )
		{
			size_t i;

			/* check received string data */
			printf("received [%s]\n", buf);

			/* split received string to string array and check */
			joystickData = strsplit(buf, "/", &numData);
			for (i = 0; i < numData; i++) {
				printf("%s-", joystickData[i]);
			}
			printf("\n");

			/* control model car by raw data */
			///// Control Speed
			if((atol(joystickData[13]) - Data13) == 1)			//back gear
			{
				back_gear++;
			}
			if(back_gear%2 == 1)						//back gear
			{
				back_speedControl(fd, atol(joystickData[0]));
			}
			else 
			{
				speedControl(fd, atol(joystickData[0]));
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
			Data13 = atol(joystickData[13]); 

			/* clean up heap allocation for after strplit() */
			for (i = 0; i < numData; i++) {
				free(joystickData[i]);
			}
			if (joystickData != NULL) {
				free(joystickData);
			}
		}
	}

	close(s);
	close(fd);
	return 0;
}
