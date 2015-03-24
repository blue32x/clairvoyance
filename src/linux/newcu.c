#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <wiringPi.h>
#include <wiringSerial.h>

int main ()
{
  int fd ;


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

/*
      printf ("\nOut: 0x80: ") ;
      fflush (stdout) ;
      write (fd,'A20301640A' ,10) ;
   
    printf ("\nOut: 0x80: ") ;
      fflush (stdout) ;
      serialPuts (fd, "A1030103A7") ;
    

     printf ("\nOut: : 0x82") ;
      fflush (stdout) ;
      serialPutchar (fd, 110) ;
*/
       printf ("\nOut: : 0x86") ;
      fflush (stdout) ;
serialPutchar (fd, 'A') ;
serialPutchar (fd, '0') ;
serialPutchar (fd, '0') ;
serialPutchar (fd, '3') ;
serialPutchar (fd, '0') ;
serialPutchar (fd, '1') ;
serialPutchar (fd, '0') ;
serialPutchar (fd, '3') ;
serialPutchar (fd, 'A') ;
serialPutchar (fd, '7') ;
  		






  printf ("\n") ;
  return 0 ;
}