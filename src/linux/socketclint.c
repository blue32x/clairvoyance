#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include "util.h"

#define SERVER_DEVICE "00:1A:7D:DA:71:11"

int main(int argc, char **argv)
{
	struct sockaddr_rc addr = { 0 };
	int s, status, bytes_read;
	char buf[1024];
	char **joystickData;
	size_t numData;
	long  rf_count, lf_count, rl_count, ll_count,back_gear =0;
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
		if( bytes_read > 0 ) {
			size_t i;
			printf("received [%s]\n", buf);

			joystickData = strsplit(buf, "/", &numData);
			for (i = 0; i < numData; i++) {
				printf("%s-", joystickData[i]);
				free(joystickData[i]);
			}
			printf("\n");

			if (joystickData != NULL) {
				free(joystickData);
			}

			///    0 = steer /// 1= speed // 6 == leftFlick//7=right flick
			///    3 = buzzer // 2=forward light //4= backward light //13 == back speed  

			if( (joystick[13] - Data13 = 1) )//back gear
			{
				back_gear++;
			}
			if(  back_gear%2 = 1) )//back gear
			{

				back_speedControl(fd, joystickData[0]);
			}
			else 
			{
				speedControl(fd, joystickData[0]);
			}

			steeringControl(fd, joystickData[0]);

			if((joystick[7]-Data7)=1)//Right flicker
			{ 
				rf_count++; 
				right_flicker(int fd,int rf_count) ;
			}

			if((joystick[6]-Data6)=1)//Right flicker
			{	
				lf_count++;
				left_flicker(int fd,int lf_count);
			}

			if((joystick[2]-Data2)=1) //forward_light
			{	
				rl_count++;	
				forward_light(int fd,int rl_count);
			}
			if((joystick[4]-Data4)=1)
			{	
				ll_count++;
				back_light(int fd,int ll_count);  
			}


			if( joystickData[3] == 1 )
			{
				soundControl(int fd);
			}

			Data2=joystick[2]; //compare
			Data4=joystick[4]; 	
			Data6=joystick[6]; 
			Data7=joystick[7];
			Data13=joystick[13]; 

		}
	}
	close(s);
	return 0;
}
