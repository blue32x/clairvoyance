#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <termios.h>
#include <fcntl.h>
#include <linux/ioctl.h>
#include <sys/types.h>
#include <sys/signal.h>

#include <wiringPi.h>
#include <wiringSerial.h>

#include "util.h"
#include "carControl.h"

#define TARGET_DEVICE "00:1A:7D:DA:71:11"
#define BUF_SIZE 64

int main(int argc, char **argv)
{
	int fd_UART, fd_BLUE;
	struct termios tio_blue;
	int bytes_read;
	char buf[BUF_SIZE];

	char **joystickData;
	size_t numData;
	long Data2, Data4, Data6, Data7, Data13;
	long rf_count, lf_count, fl_count, bl_count, back_gear;

	Data2=Data4=Data6=Data7=Data13=0;
	rf_count=lf_count=fl_count=bl_count=back_gear=0;

	/* Initailize serial network */
	if((fd_UART = serialOpen ("/dev/ttyAMA0", 19200)) < 0)
	{
		fprintf (stderr, "Unable to open serial device: %s\n", strerror (errno)) ;
		return 1 ;
	}
	if(wiringPiSetup () == -1)
	{
		fprintf (stdout, "Unable to start wiringPi: %s\n", strerror (errno)) ;
		return 1 ;
	}


	/* Initailize usb device open */
	if((fd_BLUE = open ("/dev/bus/usb/001/004", O_RDWR | O_NOCTTY)) < 0)
	{
		fprintf (stderr, "Unable to bt device\n") ;
		return 1 ;
	}
	tio_blue.c_cflag = B230400 | CS8 | CLOCAL | CREAD;
	tio_blue.c_iflag = IGNPAR | ICRNL;
	tio_blue.c_oflag = 0;
	tio_blue.c_lflag = 0;
	tio_blue.c_cc[VMIN] = 1;
	tio_blue.c_cc[VTIME] = 0;

	tcflush(fd_BLUE, TCIFLUSH);
	tcsetattr(fd_BLUE, TCSANOW, &tio_blue);

	// read data from the server
	while(1)
	{
		bytes_read = read(fd_BLUE, buf, sizeof(buf));
		if( bytes_read > 0 )
		{
		printf("received [%s]\n", buf);
		continue;
			size_t i;
			long joystickData13 = 0;

			/* check received string data */
			buf[bytes_read] = '\0';
			printf("received [%s]\n", buf);

			/* split received string to string array and check */
			joystickData = strsplit(buf, "/", &numData);

			/*
			// debug for split
			for (i = 0; i < numData; i++) {
				printf("%s-", joystickData[i]);
			}
			printf("\n");
			*/

			joystickData13 = atol(joystickData[13]);
			if(joystickData13 != 0)
			{
				joystickData13 = 1;
			}

			/* control model car by raw data */
			///// Control Speed
			if((joystickData13 - Data13) == 1)			//back gear
			{
				back_gear++;
			}
			if(back_gear%2 == 1)								//back gear
			{
				back_speedControl(fd_UART, atol(joystickData[1]));
			}
			else 
			{
				speedControl(fd_UART, atol(joystickData[1]));
			}

			///// Control Steer
			steeringControl(fd_UART, atol(joystickData[0]));

			///// Control Flicker
			if((atol(joystickData[7]) - Data7) == 1)			//Right flicker
			{ 
				rf_count++; 
				right_flicker(fd_UART, rf_count);
			}
			if((atol(joystickData[6]) - Data6) == 1)			//Left flicker
			{	
				lf_count++;
				left_flicker(fd_UART, lf_count);
			}

			///// Control Light
			if((atol(joystickData[2]) - Data2) == 1)			//forward_light
			{	
				fl_count++;
				forward_light(fd_UART, fl_count);
			}
			if((atol(joystickData[4]) - Data4) == 1)			//backward_light
			{	
				bl_count++;
				back_light(fd_UART, bl_count);  
			}

			///// Control Buzzer
			if(atol(joystickData[3]) == 1)
			{
				soundControl(fd_UART);
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
		else
		{
			break;
		}
	}

	close(fd_BLUE);
	close(fd_UART);
	return 0;
}
