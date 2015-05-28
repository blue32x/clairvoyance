//pmw playing
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <wiringSerial.h>

#define BUFFER_SIZE 1024

// symbols for gear values
#define REV_G 0
#define GEAR1 1
#define GEAR2 1.2
#define GEAR3 1.3
#define GEAR4 1.4
#define GEAR_UP -1
#define GEAR_DW -2
#define GEAR_RE -3

// global variables
int gear = GEAR1;
int speedParam = 0;
int speedParam2 = 0;
long speed=0;


void stopCar(int fd)
{
	unsigned char op = 0x91;
	unsigned char len = 0x04;
	unsigned char RW = 0x01;
	unsigned char speedParam = 0x00;
	unsigned char speedParam2 = 0x00;   
	unsigned char checkSum;
	checkSum = ((op + len + RW + speedParam2 + speedParam) & 0x00ff);

	serialPutchar(fd, op);
	serialPutchar(fd, len);
	serialPutchar(fd, RW);
	serialPutchar(fd, speedParam2);
	serialPutchar(fd, speedParam);
	serialPutchar(fd, checkSum);
}
void speedControl(int fd, long value) 
{
	// max speed is 01F4 (500) 256 =0100
	unsigned char op = 0x91;
	unsigned char len = 0x04;
	unsigned char RW = 0x01;   
	unsigned char checkSum;

	char strParam[] = "ff";
	char strHex[BUFFER_SIZE];
	char *ptr;

	if(value == 0)
	{
		speed -= 10;
	}
	else
	{        
		speed += (-1*value)/20;
	}


	if(speed < 0)
	{
		speed= 0;
	}
	else if(speed > 500)
	{
		speed = 500;
	}



        if(speed < 256 ) 
     {   
        if(speed < 10)
       {   speedParam2 = speed;
       } 
         else 
        {     
		sprintf(strHex, "%x", speed);
		strParam[0] = strHex[0];
		strParam[1] = strHex[1];
        speedParam = 0x00;
		speedParam2 = strtol(strParam, &ptr, 16);
		printf("%s, %x\n", strParam, speedParam2);
         } 
     }
        else
        {
		sprintf(strHex, "%x", speedParam2);
		strParam[0] = strHex[1];
		strParam[1] = strHex[2];
		speedParam= 0x01;
        speedParam2 = strtol(strParam, &ptr, 16);
		printf("%s, %x\n", strParam, speedParam2);
        }	
	checkSum = ((op + len + RW + speedParam2 + speedParam) & 0x00ff);

	serialPutchar(fd, op);
	serialPutchar(fd, len);
	serialPutchar(fd, RW);
	serialPutchar(fd, speedParam2);
	serialPutchar(fd, speedParam);
	serialPutchar(fd, checkSum);
}

void back_speedControl(int fd, long value) //ff ff < ff 00
{
	unsigned char op = 0x91;
	unsigned char len = 0x04;
	unsigned char RW = 0x01;
	unsigned char checkSum;
	char strParam[] = "ff";
	char strHex[BUFFER_SIZE];
	char *ptr;


	if( value == 0)
	{
		speed-=10;
	}
	else
	{  
		speed+=(-value)/20;
	}

    
      speedParam =0xff;
    if (speed >256)
    { speed = 256; 
    }
    else if(speed <= 0)
    {speed = 0;
     speedParam =0x00;
    }    
   
    speed = 256 - speed;

    /*
    sprintf(strHex, "%x", speed); 
	strParam[0] = strHex[0];
	strParam[1] = strHex[1];
	speedParam2 = strtol(strParam, &ptr, 16);
    */

    speedParam2=speed;

	checkSum = ((op + len + RW + speedParam2 + speedParam) & 0x00ff);

	serialPutchar(fd, op);
	serialPutchar(fd, len);
	serialPutchar(fd, RW);
	serialPutchar(fd, speedParam2);
	serialPutchar(fd, speedParam);
	serialPutchar(fd, checkSum);

}




void steeringControl(int fd, long value) // 1000 ~/ ~ 1500 ~ / 2000 (+1 =0.1 degree)
{
	// steer value's range is -128 ~ 0 ~ 127
	// but steer param can have 1000(0x03e8, most left) ~ 1500(0x05dc, middle) ~ 2000(0x07d0, most right)
	// so, we need to that value convert to params

	unsigned char op = 0xA3;
	unsigned char len = 0x04;
	unsigned char RW = 0x01;
	unsigned char param;
	unsigned char param2;
	unsigned char checkSum;

	char strParam[] = "ff";
	char strHex[BUFFER_SIZE];
	char *ptr;
	float ratio = 500/127;

	// We consider to equal -128 to -127 for same range.
	if(value == -128)
		value = -127;

	if(value == 0)
	{
		value = 1500;
	}
	else
	{
		value = 1500 + (float)value * ratio;
	}

	sprintf(strHex, "%x", value);
	printf("%s\n", strHex);					// For Debug

	strParam[0] = '0';
	strParam[1] = strHex[0];
	param = strtol(strParam, &ptr, 16);
	printf("%s, %x\n", strParam, param);	// For Debug

	strParam[0] = strHex[1];
	strParam[1] = strHex[2];
	param2 = strtol(strParam, &ptr, 16);
	printf("%s, %x\n", strParam, param2);	// For Debug

	checkSum = ((op + len + RW + param2 + param) & 0x00ff);

	serialPutchar(fd, op);
	serialPutchar(fd, len);
	serialPutchar(fd, RW);
	serialPutchar(fd, param2);
	serialPutchar(fd, param);
	serialPutchar(fd, checkSum);
}

void right_flicker(int fd,int f_count)
{

	unsigned char op = 0xA1;
	unsigned char len = 0x03;
	unsigned char RW = 0x01;
	unsigned char param;
	unsigned char checkSum;
	f_count = f_count % 2;
	switch(f_count)
	{
	case 0: 
		param = 0x00 ; 
		checkSum = 0xA5 ;
		break;
	case 1: 
		param = 0x01 ;
		checkSum = 0xA6 ;
		break;
	case 2: 
		param = 0x02  ;
		checkSum = 0xA7;
		break;
	case 3: 
		param = 0x03;
		checkSum = 0xA8;
		break;
	}
	serialPutchar(fd, op);
	serialPutchar(fd, len);
	serialPutchar(fd, RW);
	serialPutchar(fd, param);
	serialPutchar(fd, checkSum);

}
void left_flicker(int fd,int f_count)
{

	unsigned char op = 0xA1;
	unsigned char len = 0x03;
	unsigned char RW = 0x01;
	unsigned char param;
	unsigned char checkSum;
	f_count = f_count % 2;
	switch(f_count)
	{
	case 0: 
		param = 0x00 ; 
		checkSum = 0xA5 ;
		break;
	case 1: 
		param = 0x02  ;
		checkSum = 0xA7;
		break;	
	}
	serialPutchar(fd, op);
	serialPutchar(fd, len);
	serialPutchar(fd, RW);
	serialPutchar(fd, param);
	serialPutchar(fd, checkSum);	
}

void forward_light(int fd,int l_count)
{

	unsigned char op = 0xA0;
	unsigned char len = 0x03;
	unsigned char RW = 0x01;
	unsigned char param ;
	unsigned char checkSum ;
	l_count=l_count % 2 ;

	switch(l_count)
	{
	case 0: 
		param = 0x00 ; 
		checkSum = 0xA4 ;
		break;
	case 1: 
		param = 0x01 ;
		checkSum = 0xA5 ;
		break;

	}

	serialPutchar (fd, op) ;
	serialPutchar (fd, len) ;
	serialPutchar (fd, RW) ;
	serialPutchar (fd, param) ;
	serialPutchar (fd, checkSum) ;

}
void back_light(int fd,int l_count)
{

	unsigned char op = 0xA0;
	unsigned char len = 0x03;
	unsigned char RW = 0x01;
	unsigned char param ;
	unsigned char checkSum ;
	l_count=l_count % 2 ;

	switch(l_count)
	{
	case 0: 
		param = 0x00 ; 
		checkSum = 0xA4 ;
		break;

	case 1: 
		param = 0x02  ;
		checkSum = 0xA6 ;
		break;	
	}

	serialPutchar (fd, op) ;
	serialPutchar (fd, len) ;
	serialPutchar (fd, RW) ;
	serialPutchar (fd, param) ;
	serialPutchar (fd, checkSum) ;

}

void soundControl(int fd) 
{
	unsigned char op = 0xA2;
	unsigned char len = 0x03;
	unsigned char RW = 0x01;
	unsigned char param = 0x01;
	unsigned char checkSum;

	checkSum = ((op + len + RW + param) & 0x00ff);

	serialPutchar(fd, op);
	serialPutchar(fd, len);
	serialPutchar(fd, RW);
	serialPutchar(fd, param);
	serialPutchar(fd, checkSum);
}
