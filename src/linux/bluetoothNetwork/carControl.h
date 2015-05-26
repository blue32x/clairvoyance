#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <wiringSerial.h>

#define BUFFER_SIZE 1024

// global variables
unsigned char speedParam = 0x00;
unsigned char speedParam2 = 0x00;

void speedControl(int fd, int value)
{
	// max speed is 01F4 (500)
	unsigned char op = 0x91;
	unsigned char len = 0x04;
	unsigned char RW = 0x01;
	unsigned char checkSum;

	char strParam[] = "ff";
	char strHex[BUFFER_SIZE];
	char *ptr;
	float ratio = 500/127;

    value *= -1;
	value = (long)((float)value*ratio);
	// above 2 digits of hexdecimal
	if( value < 256 )
	{       
		sprintf(strHex, "%x", value);
		speedParam = 0;

		strParam[0] = strHex[0];
		strParam[1] = strHex[1];
		speedParam2 = strtol(strParam, &ptr, 16);
		//printf("%s, %x\n", strParam, speedParam2);	// For Debug
	}
	else 
	{
		if( value > 500 )
		{
			value=500;
		}
		sprintf(strHex, "%x", value);
		strParam[0] = '0';
		strParam[1] = strHex[0];
		speedParam = strtol(strParam, &ptr, 16);
		//printf("%s, %x\n", strParam, speedParam);	// For Debug

		strParam[0] = strHex[1];
		strParam[1] = strHex[2];
		speedParam2 = strtol(strParam, &ptr, 16);
		//printf("%s, %x\n", strParam, speedParam2);	// For Debug
	}

	//printf("%x,  %x\n", speedParam, speedParam2);	// For debug

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
	float ratio = 500/127;

    value *= -1;
	value = (long)((float)value*ratio);

	if(value > 255)
	{
		value = 255;
	}
	value=255-value;
	sprintf(strHex, "%x", value); 
	speedParam=0xff;

	strParam[0] = strHex[0];
	strParam[1] = strHex[1];
	speedParam2 = strtol(strParam, &ptr, 16);
	//printf("%s, %x\n", strParam, speedParam2);	// For Debug

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
		value = 1500 - (float)value * ratio;
	}

	sprintf(strHex, "%x", value);
	//printf("%s\n", strHex);					// For Debug

	strParam[0] = '0';
	strParam[1] = strHex[0];
	param = strtol(strParam, &ptr, 16);
	//printf("%s, %x\n", strParam, param);	// For Debug

	strParam[0] = strHex[1];
	strParam[1] = strHex[2];
	param2 = strtol(strParam, &ptr, 16);
	//printf("%s, %x\n", strParam, param2);	// For Debug

	checkSum = ((op + len + RW + param2 + param) & 0x00ff);

	serialPutchar(fd, op);
	serialPutchar(fd, len);
	serialPutchar(fd, RW);
	serialPutchar(fd, param2);
	serialPutchar(fd, param);
	serialPutchar(fd, checkSum);
}

void right_flicker(int fd, long f_count)
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
void left_flicker(int fd, long f_count)
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

void forward_light(int fd, long l_count)
{

	unsigned char op = 0xA0;
	unsigned char len = 0x03;
	unsigned char RW = 0x01;
	unsigned char param ;
	unsigned char checkSum ;
	l_count = l_count % 2 ;

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
void back_light(int fd, long l_count)
{

	unsigned char op = 0xA0;
	unsigned char len = 0x03;
	unsigned char RW = 0x01;
	unsigned char param ;
	unsigned char checkSum ;
	l_count = l_count % 2 ;

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
