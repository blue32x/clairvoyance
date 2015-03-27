//Park minwook nim jak up joong
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <wiringPi.h>
#include <wiringSerial.h>

int fd;
/////////////////////////////////////////////////////////////////////////

void SpeedControlOnOff()
{

  unsigned char op = 0x90;
  unsigned char len = 0x03;
  unsigned char RW = 0x01;
  unsigned char param ;
  unsigned char checkSum ;
  int sel_SpeedControlOnOff=0;
 
  printf(" SpeedControlOnOff( off=0, on =1 ) : ");
  scanf("%d",&sel_SpeedControlOnOff);
 
 switch(sel_SpeedControlOnOff)
{
    case 0: 
            param = 0x00 ; 
            checkSum = 0x94 ;
            break;
    case 1: 
            param = 0x01 ;
            checkSum = 0x95 ;
            break;
}
   
  serialPutchar (fd, op) ;
  serialPutchar (fd, len) ;
  serialPutchar (fd, RW) ;
  serialPutchar (fd, param) ;
  serialPutchar (fd, checkSum) ;
}

////////////////////////////////////////////////////////////////////////////
void PositionControlOnOff()
{
  unsigned char op = 0x96;
  unsigned char len = 0x03;
  unsigned char RW = 0x01;
  unsigned char param ;
  unsigned char checkSum;
  int sel_PositionControlOnOff=0; 
  printf(" PositionControlOnOff( off=0, on =1 ) : ");
  scanf("%d",&sel_PositionControlOnOff);

 switch(sel_PositionControlOnOff)
{
    case 0: 
            param = 0x00 ; 
            checkSum = 0x9A ;
            break;
    case 1: 
            param = 0x01 ;
            checkSum = 0x9B ;
            break;
}
   
  serialPutchar (fd, op) ;
  serialPutchar (fd, len) ;
  serialPutchar (fd, RW) ;
  serialPutchar (fd, param) ;
  serialPutchar (fd, checkSum) ;
}
////////////////////////////////////////////////////////
void Desire_speed()  ///speed =encoder tic * 10/s   ex)1000=0x0064 * 10 
{
  unsigned char op = 0x91;
  unsigned char len = 0x04;
  unsigned char RW = 0x01;
  int speed1=0;
  int speed2=0;
  char param ;
  char param1 ;
  char checkSum;
  
  printf(" max speed is 01F4 (500) \n");
  printf(" input Speed value twice(ex:0x00 and 0x64 means speed 0x0064 : ");
  scanf("%x %x",&speed1,&speed2);
  param=speed2;
  param1=speed1;
  
  checkSum =((op+len+RW+param+param1)&0x00ff);


  serialPutchar (fd, op) ;
  serialPutchar (fd, len) ;
  serialPutchar (fd, RW) ;
  serialPutchar (fd, param) ;
  serialPutchar (fd, param1) ;
  serialPutchar (fd, checkSum) ;
 
}

//////////////////////////////////////////////////////////////////////////
void ggambback()
{
  unsigned char op = 0xA1;
  unsigned char len = 0x03;
  unsigned char RW = 0x01;
  unsigned char param ;
  unsigned char checkSum ;
  int sel_ggambback=0;
  
  printf("  GGAM BBACK( off=0, right=1 ,left =2 ,all on = 3 ) : ");
  scanf("%d",&sel_ggambback);
 
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
void light()
{
  unsigned char op = 0xA0;
  unsigned char len = 0x03;
  unsigned char RW = 0x01;
  unsigned char param ;
  unsigned char checkSum ;
  int sel_light=0;
  printf("  light( off=0, forward=1 , back =2 ,all on = 3 ) : ");
  scanf("%d",&sel_light);
  
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
void buzzer() //value*0.01seceond
{
  unsigned char op = 0xA2;
  unsigned char len = 0x03;
  unsigned char RW = 0x01;
  int buzzer_time=0;
  unsigned char param ;
  unsigned char checkSum ;
  
  
  printf(" buzzer time = value *0.01 s \n");
  printf(" Max value is 255(0xFF)\n");
  printf(" input buzzer time value : ");
  scanf("%x",&buzzer_time);
  param = buzzer_time;
  checkSum =((op+len+RW+param)&0x00ff);
  
  serialPutchar (fd, op) ;
  serialPutchar (fd, len) ;
  serialPutchar (fd, RW) ;
  serialPutchar (fd, param) ;
  serialPutchar (fd, checkSum) ;
}
//////////////////////////////////////////////////

void steering() // 1000 ~/ ~ 1500 ~ / 2000 (+1 =0.1 degree)
{
  unsigned char op = 0xA3;
  unsigned char len = 0x04;
  unsigned char RW = 0x01;
  unsigned char param1 ;
  unsigned char param2 ;
  unsigned char checkSum;
  int steering_value1 = 0;
  int steering_value2 = 0;
   
  printf(" steering= left(1000) < middle(1500) < right(2000) \n");
  printf(" +1 = 0.1 degree ,MaxLeft=03e8 , middle=05dc , MaxRight = 07d0 \n");
  printf(" input steering value twice(0x00 & 0x64 means 0x0064 : ");
  scanf("%x %x",&steering_value1,&steering_value2);
  param1 = steering_value2;
  param2 = steering_value1;
  checkSum =((op+len+RW+param1+param2) & 0x00ff);
  
  serialPutchar (fd, op) ;
  serialPutchar (fd, len) ;
  serialPutchar (fd, RW) ;
  serialPutchar (fd, param1) ;
  serialPutchar (fd, param2) ;
  serialPutchar (fd, checkSum) ;
}
//////////////////////////////////////////////////

void menu() 
{
  printf("_____________________________________________\n");
  printf("|               control board               |\n");
  printf("|      0.   menu()                          |\n");
  printf("|      1.   ggambback()                     |\n");
  printf("|      2.   lgiht()                         |\n");
  printf("|      3.   PositionControlOnOff()          |\n");
  printf("|      4.   SpeedControlOnOff()             |\n");
  printf("|      5.   Desire_speed()                  |\n");
  printf("|      6.   buzzer()                        |\n");
  printf("|      7.   steering()                      |\n");
  printf("|      8.   Exit                            |\n");
  printf("|___________________________________________|\n");
}
////////////////////////////////////////////////////////
int main ()
{
  int numb=0;

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
 
  menu();
  

  while(numb!=8)
  {
   printf("select number : ");
   scanf("%d",&numb);

 switch(numb)
{ 
  case 0:
         menu();
         break;
  case 1:
 	 ggambback();
 	 break;
  case 2:
 	 light();
  	 break;
  case 3:
  	PositionControlOnOff();
	break;
  case 4:
        SpeedControlOnOff(); 
        break;
  case 5: 
  	Desire_speed(); 
  	break;
  case 6:
  	buzzer(); 
        break;
  case 7:
	steering();
	break;
}
}



  return 0 ;
}


  
