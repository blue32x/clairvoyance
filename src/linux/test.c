#include <stdio.h>
#include <stdlib.h>
#include "kbhit.h"
#include "getch.h"

int main()
{

char sz=0;
while(1)
{
  if( kbhit() )
 {
    sz=getch();
switch(sz)
{
   case 'a':
   printf("a \n");
   break;
 default:
   printf("dont know \n");
   break;
   }
}
}
 
}
 


 
