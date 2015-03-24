#include <termios.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>            // write(), read()를 정의하는 헤더파일  
#include <sys/signal.h>
#include <linux/ioctl.h>

#define BAUDRATE_UART1 B4800 // 2014 년 이후
//#define BAUDRATE_UART1 B19200 // 2014 년 이전 		
#define DEVICE_UART1 "/dev/ttyAMA0"
#define BUFF_SIZE 1024
int fd_UART1;
struct termios tio_UART1;

void UART1_Open(void)
{
    /*Non-blocking 모드로 시리얼 장치를 연다.(read함수 호출 후 즉각 리턴)*/
    fd_UART1 = open(DEVICE_UART1, O_RDWR | O_NOCTTY | O_NONBLOCK );
    if(fd_UART1 < 0){perror(DEVICE_UART1); exit(-1);}
    printf("success UART1 Open\n");
    /*BAUDRATE_UART1 : 전송 속도. CS8 : N81*/
    /*CLOCAL : Local connection. 모뎀 제어를 하지 않는다. */
    tio_UART1.c_cflag = BAUDRATE_UART1 | CS8 | CLOCAL | CREAD;
    tio_UART1.c_iflag = IGNPAR ; /*Parity에러가 있는 문자바이트를 무시한다.*/
    tio_UART1.c_oflag = 0;
    tio_UART1.c_lflag = 0;
    tio_UART1.c_cc[VMIN] = 1;
    tio_UART1.c_cc[VTIME] = 0;
    tcflush(fd_UART1, TCIFLUSH);
    tcsetattr(fd_UART1, TCSANOW, &tio_UART1);
}

// 차량 전진(step) 
void Go_Step_Forward(void)
{
    unsigned char op = 0xC0;
    write(fd_UART1, &op, 1) ;
    usleep(100000);
}

// 차량 후진(step) 
void Go_Step_Back(void)
{
    unsigned char op = 0xC1;
    write(fd_UART1, &op, 1) ;
    usleep(100000);
}

// 차량 전진(distance) 
void Go_Distance_Forward(void)
{
    unsigned char op = 0xC8;
    write(fd_UART1, &op, 1) ;
    usleep(100000);
}

// 차량 후진(distance) 
void Go_Distance_Back(void)
{
    unsigned char op = 0xC9;
    write(fd_UART1, &op, 1) ;
    usleep(100000);
}



//주행속도설정
void Set_Speed(unsigned short int data1, unsigned short int data2)
{
    unsigned char op = 0xA0;
    unsigned char param1 = (data1 & data1);
    unsigned char param2 = (data2 & data2);
    write(fd_UART1,&op, 1) ;
    write(fd_UART1,&param1, 1) ;
    write(fd_UART1,&param2, 1) ;
    usleep(100000);
}
// 스텝수를 설정 
void Set_Step(unsigned short int data1, unsigned short int data2)
{
    unsigned char op = 0xA3;
    unsigned char param1 = (data1 & data1);
    unsigned char param2 = (data2 & data2);
    write(fd_UART1,&op, 1) ;
    write(fd_UART1,&param1, 1) ;
    write(fd_UART1,&param2, 1) ;
    usleep(100000);
}
//가속도 설정
void Set_Accel(unsigned char data) 
{ 	
    unsigned char op = 0xA8 ;
    unsigned char param1 = (data & data);
    write(fd_UART1,&op, 1) ;
    write(fd_UART1,&data, 1) ;
    usleep(100000);
}



//주행 거리 설정
void Set_Distance(unsigned char data1, unsigned char data2)
{
    unsigned char op = 0xa7;
    unsigned char param1 = (data1 & data1);
    unsigned char param2 = (data2 & data2);
    write(fd_UART1, &op, 1);
    write(fd_UART1, &param1, 1);
    write(fd_UART1, &param2, 1);
}
//조향 장치 제어//800~ 2200 
void Set_Steering(unsigned char data1,unsigned char data2)
{
    unsigned char op = 0xf4;
    unsigned char param1 = (data1 & data1);
    unsigned char param2 = (data2 & data2);
    write(fd_UART1, &op, 1);
    write(fd_UART1, &param1, 1);
    write(fd_UART1, &param2, 1);
    usleep(100000);
}
// 조향 장치 속도 제어 // 0~ 100; 

void Set_SteeringSpeed(unsigned char data)
{
    unsigned char op = 0xf5;
    unsigned char param =(data & data);
    write(fd_UART1, &op, 1);
    write(fd_UART1, &param, 1);
    usleep(100000);
}


int Get_wheel_value(void)
{
    unsigned int wheel=0;
    int i=0;
    FILE *f;
    char Line[256];
    unsigned char value[3];
    f=fopen("save.txt","r");
    //fgets(Line,256,f);
    unsigned char test[100] ;
  
    fgets(test,100,f);
    //printf("%s\n",test);
   
    if(test[0]=='a')
    {   
            if(test[6]=='.'){wheel=(test[5]-48);}
            else if(test[7]=='.'){wheel=(test[5]-48)*10+(test[6]-48);}
            else if(test[8]=='.'){wheel=(test[5]-48)*100+(test[6]-48)*10+(test[7]-48);}
            else ;
        
        if(test[4]=='1')
        {
            wheel =wheel+1000;
        }
    }

    else if(test[0]=='b')
    {}
    else {;}


    //printf("%d\n",wheel);
    fclose(f);    
    return wheel;
}

/**********2014 년 이후 코드 (새로온 자동차에 적용)****************/

void Newset_steering(unsigned char data1, unsigned char data2)
{
    unsigned char op = 0xA3;
    unsigned char length = 0x04;
    unsigned char RW = 0x01;
    unsigned char param1 = (data2 & data2);
    unsigned char param2 = (data1 & data1);
    unsigned int checkSum =((op+length+RW+param1+param2)&0x00ff);
    checkSum = (unsigned char)checkSum; 
    write(fd_UART1, &op, 1);
    write(fd_UART1, &length, 1);
    write(fd_UART1, &RW, 1);
    write(fd_UART1, &param1, 1);
    write(fd_UART1, &param2, 1);
    write(fd_UART1, &checkSum, 1);
    usleep(100000);
}


void Newset_speed(unsigned char data1, unsigned char data2)
{
    unsigned char op = 0x91;
    unsigned char length = 0x04;
    unsigned char RW = 0x01;
    unsigned char param1 = (data2 & data2);
    unsigned char param2 = (data1 & data1);
    unsigned int checkSum =((op+length+RW+param1+param2)&0x00ff);
    checkSum = (unsigned char)checkSum; 
    write(fd_UART1, &op, 1);
    write(fd_UART1, &length, 1);
    write(fd_UART1, &RW, 1);
    write(fd_UART1, &param1, 1);
    write(fd_UART1, &param2, 1);
    write(fd_UART1, &checkSum, 1);
    usleep(100000);
}

void Newset_distance(unsigned char data1, unsigned char data2, unsigned char data3, unsigned char data4)
{
    unsigned char op = 0x97;
    unsigned char length = 0x06;
    unsigned char RW = 0x01;
    unsigned char param1 = (data4 & data4);
    unsigned char param2 = (data3 & data3);
    unsigned char param3 = (data2 & data2);
    unsigned char param4 = (data1 & data1);
    unsigned int checkSum =((op+length+RW+param1+param2+param3+param4)&0x00ff);
    checkSum = (unsigned char)checkSum; 
    write(fd_UART1, &op, 1);
    write(fd_UART1, &length, 1);
    write(fd_UART1, &RW, 1);
    write(fd_UART1, &param1, 1);
    write(fd_UART1, &param2, 1);
    write(fd_UART1, &param3, 1);
    write(fd_UART1, &param4, 1);
    write(fd_UART1, &checkSum, 1);
    usleep(100000);
}


void main()
{
	/*
    unsigned int v;
    unsigned int st1,st2;
    UART1_Open();
    //Set_SteeringSpeed(100);
    while(1){
    v=Get_wheel_value();
    //printf("%d\n",v);
    if(v>=0&& v<=128)
    {   
        v= (v*5)+1500;
        st1=((v & 0xff00)>>8);
        st2=(v & 0x00ff);
        Newset_steering(st1,st2);
        //Set_Steering(st1,st2);
        //printf("%d,%d\n",st1,st2);
    }
    else if(v>128&&v<=255)
    {
        v=1500-(255-v)*5;
        st1=((v & 0xff00)>>8);
        st2=(v & 0x00ff);
        Newset_steering(st1,st2);
        //Set_Steering(st1,st2);
        //printf("%d\n",v);
    }
    else if(v>=1000)
    {   v=v-1000;
        printf("%d\n",v);
        Set_Accel(v);
        //v=v*31;
        v=v*4;
        Newset_speed(((0xff00&v)>>8),(0x00ff&v));
        Newset_distance(0x00,0x00,0xff,0xff);
        //Set_Speed(((0xff00&v)>>8),(0x00ff&v));
        //Set_Distance(0xFF, 0xFF); 
        //Go_Distance_Forward(); 
    }


    }
    //Get_wheel_value();

	*/
    UART1_Open();
    Set_Accel(0x19);
    Set_Speed(0x05, 0xDC);
    Set_Step(0x00,0xFF);
    Set_Distance(0x00, 0xFF); 
    Set_Steering(0x05, 0xDC);
    Go_Step_Forward(); //설정된 Step만큼 전진한다.
    Go_Distance_Forward(); //설정된 거리만큼 전진한다.
}
