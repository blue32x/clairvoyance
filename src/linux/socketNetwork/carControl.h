#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <wiringSerial.h>
#include "getch.h"

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
unsigned char speedParam = 0x00;
unsigned char speedParam2 = 0x00;

void speedControl(int fd, int delta)
{
    unsigned char op = 0x91;
	unsigned char len = 0x04;
	unsigned char RW = 0x01;
	unsigned char checkSum;
	
	// above 2 digits of hexdecimal
	if(speedParam2 == 0xff && delta > 0)
	{
		speedParam += 0x01;
		speedParam2 = delta;
	}
	else if(speedParam2 == 0x00 && delta < 0)
	{
		speedParam -= 0x01;
		speedParam2 = 0xff + delta;
	}
	else
	{
		// below 2 digits of hexdecimal
		speedParam2 += delta;
	}

	printf("%x,  %x\n", speedParam, speedParam2);		// For debug

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

void accelControl(int fd, long value)
{
	//if accel value(=brake value, Y-axis value) is less than 0, increase current speed based on gear value.
	if(value < 0)
		speedControl(fd, -value * gear /* x Constant */);
}

void brakeControl(int fd, long value)
{
	//if brake value is more than 0, decrease current speed, sharply.
	if(value > 0)
		speedControl(fd, -value /* x Constant */);
	//else if brake value(=accel value, Y-axis value) is 0, decrease current speed, naturally.
	else if(value == 0)
		speedControl(fd, -10);
}

void flickerControl(int fd, unsigned char param)
{
	/*
	unsigned char op = 0xA1;
	unsigned char len = 0x03;
	unsigned char RW = 0x01;
	unsigned char param;
	unsigned char checkSum;

	switch(sel_ggambback)
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
	*/
}

void lightControl(int fd, unsigned char param)
{
	/*
	unsigned char op = 0xA0;
	unsigned char len = 0x03;
	unsigned char RW = 0x01;
	unsigned char param ;
	unsigned char checkSum ;
	int sel_light=0;

	switch(sel_light)
	{
	case 0: 
		param = 0x00 ; 
		checkSum = 0xA4 ;
		break;
	case 1: 
		param = 0x01 ;
		checkSum = 0xA5 ;
		break;
	case 2: 
		param = 0x02  ;
		checkSum = 0xA6 ;
		break;
	case 3: 
		param = 0x03;
		checkSum = 0xA7 ;
		break;

	}

	serialPutchar (fd, op) ;
	serialPutchar (fd, len) ;
	serialPutchar (fd, RW) ;
	serialPutchar (fd, param) ;
	serialPutchar (fd, checkSum) ;
	*/
}

void gearControl(int fd, int gearCommand)
{
	if(gearCommand == GEAR_RE)
		gear = REV_G;

	if(gearCommand == GEAR_UP)
	{
		if(gear == GEAR1)
			gear = GEAR2;
		if(gear == GEAR2)
			gear = GEAR3;
		if(gear == GEAR3)
			gear = GEAR4;
	}
	if(gearCommand == GEAR_DW)
	{
		if(gear == GEAR4)
			gear = GEAR3;
		if(gear == GEAR3)
			gear = GEAR2;
		if(gear == GEAR2)
			gear = GEAR1;
	}
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
