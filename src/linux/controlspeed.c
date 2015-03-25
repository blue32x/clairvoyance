#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <wiringPi.h>
#include <wiringSerial.h>

int fd;

void speed_control()
{
  unsigned char op = 0x91;
  unsigned char len = 0x04;
  unsigned char RW = 0x01;
  unsigned char param = 0x64;
  unsigned char param1 = 0x00;
  unsigned char checkSum = 0xFA;

  serialPutchar (fd, op) ;
  serialPutchar (fd, len) ;
  serialPutchar (fd, RW) ;
  serialPutchar (fd, param) ;
  serialPutchar (fd, param1) ;
  serialPutchar (fd, checkSum) ;
 
}
//////////////////////////////////////////////////////////////////////////
void ggambback(int sel_ggambback)
{
  unsigned char op = 0xA1;
  unsigned char len = 0x03;
  unsigned char RW = 0x01;
  unsigned char param ;
  unsigned char checkSum ;

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



  serialPutchar (fd, op) ;
  serialPutchar (fd, len) ;
  serialPutchar (fd, RW) ;
  serialPutchar (fd, param) ;
  serialPutchar (fd, checkSum) ;

}
//////////////////////////////////////////////////////////////////////////////
void light(int sel_light)
{
  unsigned char op = 0xA0;
  unsigned char len = 0x03;
  unsigned char RW = 0x01;
  unsigned char param ;
  unsigned char checkSum ;

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

}
//////////////////////////////////////////////////
void boozer()
{
  unsigned char op = 0xA2;
  unsigned char len = 0x03;
  unsigned char RW = 0x01;
  unsigned char param = 0x64;
  unsigned char checkSum =0x0A;
  serialPutchar (fd, op) ;
  serialPutchar (fd, len) ;
  serialPutchar (fd, RW) ;
  serialPutchar (fd, param) ;
  serialPutchar (fd, checkSum) ;
}
////////////////////////////////////////////////////////
int main ()
{
   int sel_ggambback=0;
   int sel_light=0;
  if ((fd = serialOpen ("/dev/ttyAMA0", 19200)) < 0)
  {
    fprintf (stderr, "Unable to open serial device: %s\n", strerror (errno)) ;
    return 1 ;
  }

  if (wiringPiSetup () == -1)
  {
    fprintf (stdout, "Unable to start wiringPi: %s\n", strerror (errno)) ;
    return 1 ;
  }
 

 
  printf("  GGAM BBACK( off=0, right=1 ,left =2 ,all on = 3 ) : ");
  scanf("%d",&sel_ggambback);
 
  boozer();
  delay(2000);
  ggambback(sel_ggambback);

  printf("  light( off=0, forward=1 , back =2 ,all on = 3 ) : ");
  scanf("%d",&sel_light);
 
   boozer();
  delay(2000);
  light(sel_light);
  

  return 0 ;
}
