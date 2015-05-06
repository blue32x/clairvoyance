/*
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include </usr/include/linux/joystick.h>

#include<sys/types.h>

#include<sys/stat.h>
#include<fcntl.h>


 int joy_fd, *axis=NULL, num_of_axis=0, num_of_buttons=0, x;
char *button=NULL, name_of_joystick[80];
struct js_event js;
int Left_Forward;
int Left_Reverse;
int Right_Forward;
int Right_Reverse;

 main()
{    
 printf("1 \n");
 int fd = open("dev/input/js0",O_RDONLY);
     read(joy_fd, &js, sizeof(struct js_event));       // 조이스틱 상태를 읽어오는 부분
    printf("2\n"); 
 switch (js.type & ~JS_EVENT_INIT)
     {
             case JS_EVENT_AXIS:
                     axis   [ js.number ] = js.value;
                     break;
             case JS_EVENT_BUTTON:
                     button [ js.number ] = js.value;
                     break;
             case JS_EVENT_INIT:
                     button [ js.number ] = js.value;
                     break;            
     }
     Left_Forward = axis[0];
     Left_Reverse = axis[1];
     
     if( num_of_axis > 2 )    
     Right_Forward = axis[2];
     if( num_of_axis > 3 )
     Right_Reverse = axis[3];
     
    printf("X: %6d  ", axis[0] );
    printf("Y: %6d  ", axis[1] );
 
    for( x=0 ; x<num_of_buttons ; ++x )
    for( x=0 ; x<8 ; ++x )
    printf("B%d: %d  ", x, button[x] );
     printf("\r");
     printf("\n");
     fflush(stdout);
}

main()
{

int fd = open("dev/input/js0",O_RDONLY);


struct js_event{
	__u32 rime;
    	__s16 value;
	__u8 type;
	__u8 number;
};
struct js_event e;
read(fd,&e,sizeof(js_event));

while(1)
{
read(fd,&e,sizeof(js_event));
printf("%6d  ", e.value );
printf("%6d  ", e.type );
printf("%6d  ", e.number );
printf("%6d  ", e.time );
}
}




#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include </usr/include/linux/joystick.h>
#define JOY_DEV "/dev/input/js0"

int main()
{
	int joy_fd, *axis=NULL, num_of_axis=0, num_of_buttons=0, x;
	char *button=NULL, name_of_joystick[80];
	struct js_event js;

	if( ( joy_fd = open( JOY_DEV , O_RDONLY)) == -1 )
	{
		printf( "Couldn't open joystick\n" );
		return -1;
	}

	ioctl( joy_fd, JSIOCGAXES, &num_of_axis );
	ioctl( joy_fd, JSIOCGBUTTONS, &num_of_buttons );
	ioctl( joy_fd, JSIOCGNAME(80), &name_of_joystick );

	axis = (int *) calloc( num_of_axis, sizeof( int ) );
	button = (char *) calloc( num_of_buttons, sizeof( char ) );

	printf("Joystick detected: %s\n\t%d axis\n\t%d buttons\n\n"
		, name_of_joystick
		, num_of_axis
		, num_of_buttons );

	fcntl( joy_fd, F_SETFL, O_NONBLOCK );	/* use non-blocking mode */

	while( 1 ) 	/* infinite loop */
	{

			/* read the joystick state */
		read(joy_fd, &js, sizeof(struct js_event));
		
			/* see what to do with the event */
		switch (js.type & ~JS_EVENT_INIT)
		{
			case JS_EVENT_AXIS:
				axis   [ js.number ] = js.value;
				break;
			case JS_EVENT_BUTTON:
				button [ js.number ] = js.value;
				break;
		}

			/* print the results */
		printf( "X: %6d  Y: %6d  ", axis[0], axis[1] );
		
		if( num_of_axis > 2 )
			printf("Z: %6d  ", axis[2] );
			
		if( num_of_axis > 3 )
			printf("R: %6d  ", axis[3] );
			
		for( x=0 ; x<num_of_buttons ; ++x )
			printf("B%d: %d  ", x, button[x] );

		printf("  \r");
		fflush(stdout);
	}

	close( joy_fd );	/* too bad we never get here */
	return 0;
}
*/
/*
#include "pthread.h"
#include "sys/timeb.h"
#include "curses.h"
#include "fcntl.h"
#include "/usr/include/linux/joystick.h"

#define NUM_THREADS	3
#define JOY_DEV		"/dev/input/js0"
#define HZ				12

const char FILENAME[] = "./joystickLog.out";
char axisDesc[15][5] = {"X", "Y", "Z", "R", "A", "B", "C", "D", "E", "F", "G", "H", "I" };
char numAxis[20], numButtons[20], value[20][20], timeSec[20], timeMilli[20];
int num_of_axis;

void write_to_log(){
	sleep(1);
	int cnt;
	FILE * daFile;
	daFile = fopen(FILENAME, "w+");

//	WRITE A HEADER ON THE OUPUT FILE
	fprintf(daFile, "#,");
	for(cnt = 0; cnt < num_of_axis; cnt++)
		if(cnt < num_of_axis -1)	fprintf(daFile, "%s,", axisDesc[cnt]);
		else								fprintf(daFile, "%s", axisDesc[cnt]);
	fprintf(daFile, "\r\n");

	while(1){
		fprintf(daFile, "%s.%s, ", timeSec, timeMilli);
		for(cnt = 0; cnt < num_of_axis; cnt++){
			if(cnt < num_of_axis -1)	fprintf(daFile, "%s, ", value[cnt]);
			else								fprintf(daFile, "%s ", value[cnt]);
		}
		fprintf(daFile, "\r\n");
		usleep(83333);
	}
	
	fprintf(daFile, "%s.%s Len = %d ", timeSec, timeMilli,strlen(timeMilli));

	fprintf(daFile, "\r\n");
	fflush(daFile);
}

void write_to_screen(){
	struct timeb seconds;
	int cnt, joy_fd, *axis=NULL, num_of_buttons=0, x;
	char *button=NULL, name_of_joystick[80];
	struct js_event js;
	time_t theTime;

	wmove(stdscr, 8, 0);
	addstr("Type \"q\" to quit.\n");

	if( ( joy_fd = open( JOY_DEV , O_RDONLY)) == -1 ){
		wmove(stdscr, 3,0);
		addstr( "Couldn't open joystick " );
		addstr( JOY_DEV );
		wmove(stdscr, 10,0);
		while(1)usleep(16666);
	}

	ioctl( joy_fd, JSIOCGAXES, &num_of_axis );				//	GET THE NUMBER OF AXIS ON JS
	ioctl( joy_fd, JSIOCGBUTTONS, &num_of_buttons );		//	GET THE NUMBER OF BUTTONS ON THE JS
	ioctl( joy_fd, JSIOCGNAME(80), &name_of_joystick );	//	GET THE NAME OF THE JS

	axis = (int *) calloc( num_of_axis, sizeof( int ) );
	button = (char *) calloc( num_of_buttons, sizeof( char ) );

	sprintf(numAxis, "%d", num_of_axis);
	sprintf(numButtons, "%d", num_of_buttons);

	wmove(stdscr, 0,0);
	addstr("Joystick detected: ");

	wmove(stdscr, 0, 19);
	addstr(name_of_joystick);

	wmove(stdscr, 1, 0);
	addstr("Number of axis   :");

	wmove(stdscr, 1, 19);
	addstr(numAxis);
	
	wmove(stdscr, 2, 0);
	addstr("Number of buttons:");

	wmove (stdscr, 2, 19);
	addstr(numButtons);

//	CHANGE THE STATUS FLAG OF THE FILE DESCRIPTOR TO NON-BLOCKING MODE
	fcntl( joy_fd, F_SETFL, O_NONBLOCK );   

	while( 1 ){
		usleep(16666);
	
	//	READ THE JOYSTICK STATE, IT WILL BE RETURNED IN THE JS_EVENT STRUCT
		read(joy_fd, &js, sizeof(struct js_event));

	//	GET THE NUMBER OF SECONDS SINCE EPOCH
		ftime(&seconds);
		theTime = time(NULL);
		sprintf(timeSec, "%d", seconds.time);						
		sprintf(timeMilli, "%d", seconds.millitm);
               
	//	CHECK THE EVENT
		switch (js.type & ~JS_EVENT_INIT){
			case JS_EVENT_AXIS:
				axis   [ js.number ] = js.value;
				break;
			case JS_EVENT_BUTTON:
				button [ js.number ] = js.value;
				break;
		}
		//	ADD LEADING 0'S TO THE MILLISECOND STRING (IF NECESSARY)
			char temp[10] = "";
			while(strlen(temp) < (3 - strlen(timeMilli)))
				strcat(temp, "0");
			strcat(temp,timeMilli);
			strcpy(timeMilli, temp);

		//	PRINT THE RESULTS
			wmove(stdscr, 4, 0);
			for(cnt = 0; cnt < num_of_axis; cnt++)
				addstr("            ");
			for(cnt = 0; cnt < num_of_axis && cnt < 13; cnt++){
				sprintf(value[cnt], "%d", axis[cnt]);
				wmove(stdscr, 4, cnt * 10);
				addstr(axisDesc[cnt]);
				addstr(": ");
				wmove(stdscr, 4, (cnt * 10) + 2);
				addstr(value[cnt]);
			}
			
			wmove(stdscr, 6, 0);
			addstr(timeSec);
			wmove(stdscr, 6,10);
			addstr(".");
			wmove(stdscr, 6, 11);
			addstr(timeMilli);
			wmove(stdscr, 6, 15);
			addstr("=");
			wmove(stdscr, 6, 17);
			addstr(ctime(&theTime));
                       
			wmove(stdscr, 10, 0);
			refresh();
	}
}

void *ThreadProcs(void *threadid){
	int thread_id = (int)threadid;
	
	if(thread_id == 0){
		write_to_screen();
	}
	
	if(thread_id == 1){
	//	THIS THRE/usr/include/linux/joystick.hAD WILL MAKE THE PROGRAM EXIT
		int ch;
		nodelay(stdscr, TRUE);				//	SETUP NON BLOCKING INPUT
		while(1) {
			if ((ch = getch()) == ERR) usleep(16666);		//	USER HASN'T RESPONDED
			else if( ch == 'q'){			
				endwin();
				exit(0);			//	QUIT ALL THREADS
			}
		}
	}
	if(thread_id == 2){
		write_to_log();
	}
}

int main(int argc, char *argv[]){
	initscr(); 									//	INIT THE SCREEN FOR CURSES
	pthread_t threads[NUM_THREADS];
	int rc, t;
	for(t = 0; t < NUM_THREADS; t++){	//	MAKE 2 NEW THREADS
		rc = pthread_create(&threads[t], NULL, ThreadProcs, (void *)t);
		if (rc){
			printf("ERROR; return code from pthread_create() is %d\n", rc);
			exit(-1);pthread_exit(NULL);
		}
	}
	
	for(t = 0; t < NUM_THREADS; t++)
	pthread_join(threads[t], NULL);		//	WAIT FOR THREADS TO EXIT OR IT WILL RACE TO HERE.

	endwin();
	return 0;
}
*/


#include "pthread.h"

#include "sys/timeb.h"

#include "curses.h"

#include "fcntl.h"

#include "/usr/include/linux/joystick.h"

 

#define NUM_THREADS 3

#define JOY_DEV     "/dev/input/js0"

#define HZ              12

 

const char FILENAME[] = "./joystickLog.out";

char axisDesc[15][5] = {"X", "Y", "Z", "R", "A", "B", "C", "D", "E", "F", "G", "H", "I" };

char numAxis[20], numButtons[20], value[20][20], timeSec[20], timeMilli[20];

int num_of_axis;

 

void write_to_log(){

    sleep(1);

    int cnt;

    FILE * daFile;

    daFile = fopen(FILENAME, "w+");

 

//  WRITE A HEADER ON THE OUPUT FILE

    fprintf(daFile, "#,");

    for(cnt = 0; cnt < num_of_axis; cnt++)

        if(cnt < num_of_axis -1) fprintf(daFile, "%s,", axisDesc[cnt]);

        else                                fprintf(daFile, "%s", axisDesc[cnt]);

    fprintf(daFile, "\r\n");

 

    while(1){

        fprintf(daFile, "%s.%s, ", timeSec, timeMilli);

        for(cnt = 0; cnt < num_of_axis; cnt++){

            if(cnt < num_of_axis -1) fprintf(daFile, "%s, ", value[cnt]);

            else                                fprintf(daFile, "%s ", value[cnt]);

        }

        fprintf(daFile, "\r\n");

        usleep(83333);

    }

     

    fprintf(daFile, "%s.%s Len = %d ", timeSec, timeMilli,strlen(timeMilli));

 

    fprintf(daFile, "\r\n");

    fflush(daFile);

}

 

void write_to_screen(){

    struct timeb seconds;

    int cnt, joy_fd, *axis=NULL, num_of_buttons=0, x;

    char *button=NULL, name_of_joystick[80];

    struct js_event js;

    time_t theTime;

 

    wmove(stdscr, 8, 0);

    addstr("Type \"q\" to quit.\n");

 

    if( ( joy_fd = open( JOY_DEV , O_RDONLY)) == -1 ){

        wmove(stdscr, 3,0);

        addstr( "Couldn't open joystick " );

        addstr( JOY_DEV );

        wmove(stdscr, 10,0);

        while(1)usleep(16666);

    }

 

    ioctl( joy_fd, JSIOCGAXES, &num_of_axis );              //  GET THE NUMBER OF AXIS ON JS

    ioctl( joy_fd, JSIOCGBUTTONS, &num_of_buttons );        //  GET THE NUMBER OF BUTTONS ON THE JS

    ioctl( joy_fd, JSIOCGNAME(80), &name_of_joystick ); //  GET THE NAME OF THE JS

 

    axis = (int *) calloc( num_of_axis, sizeof( int ) );

    button = (char *) calloc( num_of_buttons, sizeof( char ) );

 

    sprintf(numAxis, "%d", num_of_axis);

    sprintf(numButtons, "%d", num_of_buttons);

 

    wmove(stdscr, 0,0);

    addstr("Joystick detected: ");

 

    wmove(stdscr, 0, 19);

    addstr(name_of_joystick);

 

    wmove(stdscr, 1, 0);

    addstr("Number of axis   :");

 

    wmove(stdscr, 1, 19);

    addstr(numAxis);

     

    wmove(stdscr, 2, 0);

    addstr("Number of buttons:");

 

    wmove (stdscr, 2, 19);

    addstr(numButtons);

 

//  CHANGE THE STATUS FLAG OF THE FILE DESCRIPTOR TO NON-BLOCKING MODE

    fcntl( joy_fd, F_SETFL, O_NONBLOCK );  

 

    while( 1 ){

        usleep(16666);

     

    //  READ THE JOYSTICK STATE, IT WILL BE RETURNED IN THE JS_EVENT STRUCT

        read(joy_fd, &js, sizeof(struct js_event));

 

    //  GET THE NUMBER OF SECONDS SINCE EPOCH

        ftime(&seconds);

        theTime = time(NULL);

        sprintf(timeSec, "%d", seconds.time);                      

        sprintf(timeMilli, "%d", seconds.millitm);

                

    //  CHECK THE EVENT

        switch (js.type & ~JS_EVENT_INIT){

            case JS_EVENT_AXIS:

                axis   [ js.number ] = js.value;

                break;

            case JS_EVENT_BUTTON:

                button [ js.number ] = js.value;

                break;

        }

        //  ADD LEADING 0'S TO THE MILLISECOND STRING (IF NECESSARY)

            char temp[10] = "";

            while(strlen(temp) < (3 - strlen(timeMilli)))

                strcat(temp, "0");

            strcat(temp,timeMilli);

            strcpy(timeMilli, temp);

 

        //  PRINT THE RESULTS

            wmove(stdscr, 4, 0);

            for(cnt = 0; cnt < num_of_axis; cnt++)

                addstr("            ");

            for(cnt = 0; cnt < num_of_axis && cnt < 13; cnt++){

                sprintf(value[cnt], "%d", axis[cnt]);

                wmove(stdscr, 4, cnt * 10);

                addstr(axisDesc[cnt]);

                addstr(": ");

                wmove(stdscr, 4, (cnt * 10) + 2);

                addstr(value[cnt]);

            }

             

            wmove(stdscr, 6, 0);

            addstr(timeSec);

            wmove(stdscr, 6,10);

            addstr(".");

            wmove(stdscr, 6, 11);

            addstr(timeMilli);

            wmove(stdscr, 6, 15);

            addstr("=");

            wmove(stdscr, 6, 17);

            addstr(ctime(&theTime));

                        

            wmove(stdscr, 10, 0);

            refresh();

    }

}

void *ThreadProcs(void *threadid){

    int thread_id = (int)threadid;


    if(thread_id == 0){

        write_to_screen();

    }

     

    if(thread_id == 1){

    //  THIS THREAD WILL MAKE THE PROGRAM EXIT

        int ch;

        nodelay(stdscr, TRUE);              //  SETUP NON BLOCKING INPUT

        while(1) {

            if ((ch = getch()) == ERR) usleep(16666);       //  USER HASN'T RESPONDED

            else if( ch == 'q'){           

                endwin();

                exit(0);            //  QUIT ALL THREADS

            }

        }

    }

    if(thread_id == 2){

        write_to_log();

    }

}

 

int main(int argc, char *argv[]){

    initscr();                                  //  INIT THE SCREEN FOR CURSES

    pthread_t threads[NUM_THREADS];

    int rc, t;

    for(t = 0; t < NUM_THREADS; t++){    //  MAKE 2 NEW THREADS

        rc = pthread_create(&threads[t], NULL, ThreadProcs, (void *)t);

        if (rc){

            printf("ERROR; return code from pthread_create() is %d\n", rc);

            exit(-1);pthread_exit(NULL);

        }

    }

     

    for(t = 0; t < NUM_THREADS; t++)

        pthread_join(threads[t], NULL);     //  WAIT FOR THREADS TO EXIT OR IT WILL RACE TO HERE.

 

    endwin();

    return 0;

}
