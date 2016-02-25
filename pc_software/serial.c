
#include <stdlib.h>
#include <stdio.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include "rs232.h"
#include "serial.h"

void bloed(void){
	fprintf(stdout,"hello");
	fflush(stdout);
}

/*
int serial_test_port(int nr,int bdrate)
{
  int i, n,
      cport_nr=nr;        
      

  unsigned char buf[4096];

  char mode[]={'8','N','1',0};


  if(RS232_OpenComport(cport_nr, bdrate, mode))
  {
    printf("Can not open comport\n");

    return(0);
  }

    n = RS232_PollComport(cport_nr, buf, 4095);

    if(n > 0)
    {
      buf[n] = 0;   // always put a "null" at the end of a string! 

      for(i=0; i < n; i++)
      {
        if(buf[i] < 32)  // replace unreadable control-codes by dots 
        {
          buf[i] = '.';
        }
      }

      printf("received %i bytes: %s\n", n, (char *)buf);
    

#ifdef _WIN32
    Sleep(100);
#else
    usleep(100000);  // sleep for 100 milliSeconds
#endif
  }

  return(0);
}
*/
